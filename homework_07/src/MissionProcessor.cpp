#include "MissionProcessor.h"
#include "simulation/Target.h"
#include "utils/Logging.h"
#include "utils/MathUtils.h"
#include <cmath>
#include <cstdio>
#include "third_party/nlohmann/json.hpp"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"
#include "interfaces/IResultExporter.h"
#include "config/DroneConfig.h"
#include "config/AmmoParams.h"


using json = nlohmann::json;

constexpr int MAX_STEPS = 10000;

MissionProcessor::MissionProcessor(
    IBallisticSolver* solver,
    ITargetProvider* targets,
    IResultExporter* exporter
)
    : solver(solver)
    , targets(targets)
    , exporter(exporter)
    , currentIndex(0)
{
}

bool MissionProcessor::init(const IConfigLoader* config)
{
    configLoader = config;
    droneConfig = config->getConfig();
    ammoParams = config->getAmmoParams();
    
    stepTimer.init(*droneConfig, targets->getSimCycleStep());
    steps.resize(MAX_STEPS);
    simStep = steps.data();
    initSimStep();
    inProgress = true;
    
    return calculateHorizontalAmmoRange() > MathUtils::EPS;
}

bool MissionProcessor::hasNext() const
{
    return (stepTimer < MAX_STEPS - 1) && inProgress;
}

Coord MissionProcessor::step()
{
    simStep = &steps[stepTimer.stepIndex];

    double bestTime = 1e9;
    int targetCount = targets->getTargetCount();
    Coord* targetLhs;
    Coord targetVelocity;
    Target* target;

    int bestTarget = -1;
    double rawTime;
    Coord targetPos;
    Coord nextTarget;
    double distance;

    for (int i = 0; i < targetCount; i++){
        target = targets->getTarget(i);
        targetLhs = &target->targets[stepTimer.timeIndex];
        targetVelocity = (target->targets[stepTimer.nexTimeIndex] - *targetLhs) / droneConfig->arrayTimeStep;
        rawTime = 0;
        targetPos = *targetLhs + targetVelocity * stepTimer.tickTime;

        rawTime += calculateInitVersionTimeToTarget(simStep, &targetPos, &targetVelocity);
        nextTarget = *targetLhs + targetVelocity * (rawTime + stepTimer.tickTime);
        rawTime = calculateInitVersionTimeToTarget(simStep, &nextTarget, &targetVelocity);

        //add ammo flight time
        rawTime += calculateAmmoFlightTime();

        if (rawTime < bestTime)
        {
            bestTarget = i;
            bestTime = rawTime;
        }
    }

    target = targets->getTarget(bestTarget);
    targetLhs = &target->targets[stepTimer.timeIndex];
    targetVelocity = (target->targets[stepTimer.nexTimeIndex] - *targetLhs)/ droneConfig->arrayTimeStep;
    simStep->predictedTarget = *targetLhs + targetVelocity * (bestTime + stepTimer.tickTime - droneConfig->simTimeStep); // sync??? +- droneConfig->simTimeStep
    distance = simStep->pos.distanceTo(simStep->predictedTarget);
    double neededDir = (simStep->predictedTarget - simStep->pos).direction();
    
    simStep->targetIdx = bestTarget;
    simStep->dropPoint = simStep->predictedTarget - Coord{std::cos(neededDir), std::sin(neededDir)} * calculateHorizontalAmmoRange();
    simStep->aimPoint = simStep->pos + simStep->directionVector * calculateHorizontalAmmoRange();

    DEBUG("Target : " << simStep->targetIdx);
    
    switch (simStep->state)
    {
        case MOVING:
            // You could take the hitRadius into account, but based on testing, the following implementation proved to be better
            if (distance >= calculateHorizontalAmmoRange() - MathUtils::EPS)
            {
                if(MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold)){
                    updateStop();                       
                } else {
                    // You could take the hitRadius into account, but based on testing, the following implementation proved to be better
                    if (distance > calculateHorizontalAmmoRange() + simStep->velocity * droneConfig->simTimeStep)
                    {
                        updateMoving();
                        (simStep + 1)->upDirection(neededDir);
                    } else {
                        //dronDir = neededDir;
                        if (std::fabs(distance - calculateHorizontalAmmoRange()) <= droneConfig->hitRadius - MathUtils::EPS)
                        {
                            inProgress = false;
                        } else {
                            //inProgress = false;
                            updateMoving();
                            (simStep + 1)->upDirection(neededDir);
                            //ERROR("Check stepToUpdateAmmoDown: ");
                        }
                    }
                }
            } else {
                neededDir += M_PI;

                if(MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold)){                        
                    updateStop();                        
                } else {
                    double s_brake = MathUtils::calculateBrakingDistance(simStep->velocity, droneConfig->acceleration);
                    if (distance + s_brake < calculateHorizontalAmmoRange() - MathUtils::EPS)
                    {
                        updateMoving();
                        (simStep + 1)->upDirection(neededDir);
                    } else {
                        updateStop();
                        (simStep + 1)->upDirection(neededDir);
                    }                            
                }
            }
            break;

        case ACCELERATING:
        case DECELERATING:

            if (distance >= calculateHorizontalAmmoRange() + MathUtils::calculateAccelerationPath(simStep->velocity, droneConfig->attackSpeed, droneConfig->acceleration) - MathUtils::EPS){
                if(MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold)){
                    updateStop();                        
                } else {
                    updateAccelerating();
                    (simStep + 1)->upDirection(neededDir);
                }
            } else {
                neededDir += M_PI;
                if(MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold)){
                    updateStop();
                } else {
                    //dronDir = neededDir;
                    
                    DronePhase dronePhase = determineMotionPhase(distance);
                    
                    if (dronePhase == ACCELERATING)
                    {
                        stepWithAccelerating();
                        (simStep + 1)->upDirection(neededDir);
                    } else {
                        stepWithDecelerating();
                        (simStep + 1)->upDirection(neededDir);
                    }
                }
            }
            break;
    
        case STOPPED:
        case TURNING:
            if (distance >= calculateHorizontalAmmoRange() + droneConfig->accelPath - MathUtils::EPS){
                
                if (MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold))
                {
                    stepRotation(neededDir);
                } else {
                    stepWithAccelerating();
                    (simStep + 1)->upDirection(neededDir);
                }
            } else {
                neededDir += M_PI;

                if (MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold))
                {
                    stepRotation(neededDir);
                } else {
                    stepWithAccelerating();
                    (simStep + 1)->upDirection(neededDir);
                }
            }
            break;

        default:
            break;
    }

    stepTimer += 1;
    return simStep->dropPoint;
}

void MissionProcessor::updateMoving(){
    *(simStep + 1) = *simStep;
    // Calculate distance traveled at constant velocity
    double distance = simStep->velocity * droneConfig->simTimeStep;
    (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;
}

void MissionProcessor::updateAccelerating(){
    simStep->state = ACCELERATING;
    *(simStep + 1) = *simStep;
    double timeToMax = (droneConfig->attackSpeed - simStep->velocity) / droneConfig->acceleration;

    if (droneConfig->simTimeStep <= timeToMax) {
        double distance = simStep->velocity * droneConfig->simTimeStep + (droneConfig->acceleration * droneConfig->simTimeStep * droneConfig->simTimeStep) / 2.0;
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;

        (simStep + 1)->velocity = simStep->velocity + droneConfig->acceleration * droneConfig->simTimeStep;
    } else {

        double distance = simStep->velocity * timeToMax + (droneConfig->acceleration * timeToMax * timeToMax) / 2.0 + droneConfig->attackSpeed * (droneConfig->simTimeStep - timeToMax);
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;

        (simStep + 1)->velocity = droneConfig->attackSpeed;
    }

    if ((simStep + 1)->velocity > droneConfig->attackSpeed - MathUtils::EPS)
    {
        (simStep + 1)->state = MOVING;
    }
    
}

void MissionProcessor::stepWithAccelerating()
{
    simStep->state = ACCELERATING;
    *(simStep + 1) = *simStep;
    double timeToMax = (droneConfig->attackSpeed - simStep->velocity) / droneConfig->acceleration;
    
    if (droneConfig->simTimeStep <= timeToMax + MathUtils::EPS) {
        double averageVelocity = simStep->velocity + droneConfig->acceleration * droneConfig->simTimeStep * 0.5;
        double distance = averageVelocity * droneConfig->simTimeStep;
        
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;
        (simStep + 1)->velocity = simStep->velocity + droneConfig->acceleration * droneConfig->simTimeStep;
    } else {
        double accDistance = (simStep->velocity + droneConfig->attackSpeed) * 0.5 * timeToMax;
        double cruiseDistance = droneConfig->attackSpeed * (droneConfig->simTimeStep - timeToMax);
        double totalDistance = accDistance + cruiseDistance;
        
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * totalDistance;
        (simStep + 1)->velocity = droneConfig->attackSpeed;
    }

    if ((simStep + 1)->velocity > droneConfig->attackSpeed - MathUtils::EPS)
    {
        (simStep + 1)->velocity = droneConfig->attackSpeed;
        (simStep + 1)->state = MOVING;
    }

}

void MissionProcessor::stepWithDecelerating()
{
    simStep->state = DECELERATING;
    *(simStep + 1) = *simStep;
    double timeToStop = simStep->velocity / droneConfig->acceleration;
    
    if (droneConfig->simTimeStep <= timeToStop + MathUtils::EPS) {
        double averageVelocity = simStep->velocity - droneConfig->acceleration * droneConfig->simTimeStep * 0.5;
        double distance = averageVelocity * droneConfig->simTimeStep;
        
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;
        (simStep + 1)->velocity = simStep->velocity - droneConfig->acceleration * droneConfig->simTimeStep;
    } else {
        double stopDistance = MathUtils::calculateBrakingDistance(simStep->velocity, droneConfig->acceleration);
        
        (simStep + 1)->pos = simStep->pos + simStep->directionVector * stopDistance;
        (simStep + 1)->velocity = 0.0;
    }

    if ((simStep + 1)->velocity < MathUtils::EPS)
    {
        (simStep + 1)->velocity = 0.0;
        (simStep + 1)->state = STOPPED;
    }
}

void MissionProcessor::stepRotation(double neededDir)
{
    simStep->state = TURNING;
    *(simStep + 1) = *simStep;

    double delta = neededDir - simStep->direction;
    
    while (delta > M_PI)  delta -= 2.0 * M_PI;
    while (delta < -M_PI) delta += 2.0 * M_PI;
    
    double absDelta = std::fabs(delta);
    double maxRotation = droneConfig->angularSpeed * droneConfig->simTimeStep;
    
    if (absDelta <= maxRotation + MathUtils::EPS) {
        (simStep + 1)->upDirection(neededDir);
        (simStep + 1)->state = STOPPED;
    } else {
        (simStep + 1)->upDirection(simStep->direction + (delta / absDelta) * maxRotation);
    }
    
    while ((simStep + 1)->direction > M_PI)  (simStep + 1)->direction -= 2.0 * M_PI;
    while ((simStep + 1)->direction < -M_PI) (simStep + 1)->direction += 2.0 * M_PI;
}

void MissionProcessor::updateStop(){
    double dronA = droneConfig->acceleration;
    double simTimeStep = droneConfig->simTimeStep;

    simStep->state = DECELERATING;
    double timeToStop = simStep->velocity / dronA;
    double distance;

    *(simStep + 1) = *simStep;

    if (simTimeStep <= timeToStop) {
        distance = simStep->velocity * simTimeStep - (dronA * simTimeStep * simTimeStep) / 2.0;
        (simStep + 1)->velocity = simStep->velocity - dronA * simTimeStep;
    } else {
        // Drone stops before simTimeStep ends
        // Calculate distance to stop: s = v0^2 / (2*a)
        distance = MathUtils::calculateBrakingDistance(simStep->velocity, dronA);
        // Velocity becomes 0
        (simStep + 1)->velocity = 0.0;
    }

    (simStep + 1)->pos = simStep->pos + simStep->directionVector * distance;
    
    if ((simStep + 1)->velocity < MathUtils::EPS)
    {
        (simStep + 1)->state = STOPPED;
    }
    
}

bool MissionProcessor::reset()
{
    return stepTimer.reset();
}

void MissionProcessor::changeSolver(IBallisticSolver* newSolver)
{
    solver = newSolver;
}

double MissionProcessor::calculateRotationTime(double dronDir, double neededDir, double dronVelocity){
    return MathUtils::calculateRotationTime(dronDir, neededDir, dronVelocity, droneConfig->turnThreshold, droneConfig->acceleration, droneConfig->angularSpeed);
}

double MissionProcessor::calculateInitVersionTimeToTarget(
    const SimStep* simStep,
    const Coord* targetPos,
    const Coord* targetVelocity
) {

    double neededDir = (*targetPos - simStep->pos).direction();
    double accelerationTime = (droneConfig->attackSpeed - simStep->velocity) / droneConfig->acceleration;
    double distance = simStep->pos.distanceTo(*targetPos);
    double accelerationPathFromV = MathUtils::calculateAccelerationPath(simStep->velocity, droneConfig->attackSpeed, droneConfig->acceleration);

    // чи можемо летіти до таргета одразу?
    if (calculateHorizontalAmmoRange() + accelerationPathFromV <= distance){
        if (!MathUtils::needStopForRotation(simStep->direction, neededDir, droneConfig->turnThreshold))
        {
            return accelerationTime + (distance - accelerationPathFromV - calculateHorizontalAmmoRange()) / droneConfig->attackSpeed;
        }
    } 

    double timeToStop{0};
    Coord cDronPos{simStep->pos};

    timeToStop = simStep->velocity / droneConfig->acceleration;
    double brakeDistance = MathUtils::calculateBrakingDistance(simStep->velocity, droneConfig->acceleration);
    cDronPos = cDronPos + simStep->directionVector * brakeDistance;
    double distanceAfterStop = cDronPos.distanceTo(*targetPos);

    double neededDirAfterStop = (*targetPos - cDronPos).direction();

    // чи достатньо буде відстані якщо зупинитися?
    if (simStep->state != STOPPED && simStep->state != TURNING)
    {
        if (calculateHorizontalAmmoRange() + droneConfig->accelPath <= distanceAfterStop){
            if (!MathUtils::needStopForRotation(simStep->direction, neededDirAfterStop, droneConfig->turnThreshold))
            {
                return timeToStop + droneConfig->accelPath + (distanceAfterStop - droneConfig->accelPath - calculateHorizontalAmmoRange()) / droneConfig->attackSpeed;
            } else {
                double rotationTime = calculateRotationTime(simStep->direction, neededDirAfterStop, 0);
                return rotationTime + timeToStop + droneConfig->accelPath + (distanceAfterStop - droneConfig->accelPath - calculateHorizontalAmmoRange()) / droneConfig->attackSpeed;
            }
        }
    }
    
    // ми не можемо летіти одразу, а якщо зупинимося не вистачить дистанції на прискорення

    double sumTime{0.0};
    double reverseDirection = neededDir + M_PI;
    double cVelocity = simStep->velocity;
    double cDistance = distance;

    // відповідно перевіряємо, чи вже летимо від об'єкту у правильному напрямку
    if (MathUtils::needStopForRotation(simStep->direction, reverseDirection, droneConfig->turnThreshold))
    {
        // ні треба зупинятися і повертати
        sumTime += timeToStop;
        cVelocity = 0.0;
        // поки гальмуватимемо reverseDir зміниться на
        reverseDirection = neededDirAfterStop + M_PI;
        double rotationTime = calculateRotationTime(simStep->direction, reverseDirection, cVelocity);
        sumTime += rotationTime;
        cDistance = distanceAfterStop;
    } else {
        // інакше поточна позиція є початковою
        cDronPos = simStep->pos;
    }
    
    // наразі носом від об'єкту, щоб віддалитися на потрібну відстань
    double neededAdditionalDistance = calculateHorizontalAmmoRange() + droneConfig->accelPath - cDistance;
    double maneuverTime, stopDronVelocity;
    MathUtils::solveStopAtEnd(cVelocity, droneConfig->attackSpeed, droneConfig->acceleration, neededAdditionalDistance, maneuverTime, stopDronVelocity);
    sumTime += maneuverTime;

    //віддалилися, але можливо не встигли зупинитися
    if (stopDronVelocity > MathUtils::EPS)
    {
        timeToStop = stopDronVelocity / droneConfig->acceleration;
        sumTime += timeToStop;
        
        // це додаткова відстань, вона буде проходити на максимальній швидкості
        double brakeDistance = MathUtils::calculateBrakingDistance(stopDronVelocity, droneConfig->acceleration);
        sumTime += brakeDistance / droneConfig->attackSpeed;
    }

    // розвертаємося до цілі - на 180, бо точно летіли від неї, крім того ми точно зупинилися
    double rotationTime = calculateRotationTime(0, M_PI, 0);
    sumTime += rotationTime;
    
    // залишилося лише прискоритися (бо зайву, якщо вона є ми вже додали)
    sumTime += accelerationTime;

    return sumTime;
}

DronePhase MissionProcessor::determineMotionPhase(double distance)
{
    double remainingDistance = calculateHorizontalAmmoRange() + droneConfig->accelPath - distance;
    double stoppingDistance = MathUtils::calculateBrakingDistance(simStep->velocity, droneConfig->acceleration);
    
    if (remainingDistance <= stoppingDistance + MathUtils::EPS) {
        return DECELERATING;
    }

    return ACCELERATING;

}

double MissionProcessor::calculateAmmoFlightTime(){
    return solver->calculateAmmoFlightTime(droneConfig->attackSpeed, droneConfig->altitude, ammoParams->drag, ammoParams->lift, ammoParams->mass);
}

double MissionProcessor::calculateHorizontalAmmoRange(){
    return solver->calculateHorizontalAmmoRange(droneConfig->attackSpeed, droneConfig->altitude, ammoParams->drag, ammoParams->lift, ammoParams->mass);
}

void MissionProcessor::initSimStep(){
    simStep->pos.x = droneConfig->startPos.x;
    simStep->pos.y = droneConfig->startPos.y;
    simStep->direction = droneConfig->initialDir;
    simStep->directionVector = {std::cos(droneConfig->initialDir), std::sin(droneConfig->initialDir)};
    simStep->targetIdx = -1;
    simStep->state = STOPPED;
    simStep->velocity = 0.0;
}

MissionProcessor::~MissionProcessor(){
}

bool MissionProcessor::saveData(){
    return exporter->save(steps.data(), stepTimer.stepIndex);
}
