#include "MissionProcessor.h"
#include "states/StateStopped.h"
#include "states/StateMoving.h"
#include "states/StateAccelerating.h"
#include "states/StateDecelerating.h"
#include "states/StateTurning.h"
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

std::unique_ptr<IDroneState> MissionProcessor::makeStateFromPhase(int phase) const
{
    switch (static_cast<DronePhase>(phase)) {
        case DronePhase::STOPPED:       return std::make_unique<StateStopped>();
        case DronePhase::MOVING:        return std::make_unique<StateMoving>();
        case DronePhase::ACCELERATING:  return std::make_unique<StateAccelerating>();
        case DronePhase::DECELERATING:  return std::make_unique<StateDecelerating>();
        case DronePhase::TURNING:       return std::make_unique<StateTurning>();
    }
    return std::make_unique<StateStopped>();
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
    
    // Заповнюємо контекст для станів
    DroneContext ctx;
    ctx.cfg = droneConfig;
    ctx.current = simStep;
    ctx.next = simStep + 1;
    ctx.distance = distance;
    ctx.neededDir = neededDir;
    ctx.ammoRange = calculateHorizontalAmmoRange();
    ctx.inProgress = &inProgress;

    // Створюємо стан на основі simStep->state та виконуємо його
    // Стан не зберігається між кроками — наступний крок прочитає
    // свіжий simStep->state з даних (записаний попереднім викликом у simStep+1)
    auto stateObj = makeStateFromPhase(simStep->state);
    stateObj->execute(ctx);
    
    stepTimer += 1;
    return simStep->dropPoint;
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
    simStep->state = static_cast<int>(DronePhase::STOPPED);
    simStep->velocity = 0.0;
}

MissionProcessor::~MissionProcessor(){
}

bool MissionProcessor::saveData(){
    return exporter->save(steps.data(), stepTimer.stepIndex);
}