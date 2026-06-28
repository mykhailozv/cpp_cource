#include <cmath>

#include "config/DroneConfig.h"
#include "simulation/StepTimer.h"
#include "utils/Logging.h"

void StepTimer::init(const DroneConfig& droneConfig, int timeSteps){
    simTimeStep = droneConfig.simTimeStep;
    timeCount = timeSteps;
    stepsPerTime = static_cast<int>(std::round(droneConfig.arrayTimeStep / droneConfig.simTimeStep));
    update(0);
}

void StepTimer::update(int newStep){
    stepIndex = newStep;
    timeIndex = (newStep / stepsPerTime) % timeCount;
    nexTimeIndex = (timeIndex + 1) % timeCount;
    tickStep = newStep % stepsPerTime;
    tickTime = tickStep * simTimeStep;
}

void StepTimer::step(){
    update(stepIndex + 1);
    isStepReady = true;
    stepSize = 1;
}

bool StepTimer::operator<(int value) const {
    return stepIndex < value;
}

StepTimer& StepTimer::operator+=(int delta) {
    update(stepIndex + delta);
    isStepReady = true;
    stepSize = delta;
    return *this;
}

bool StepTimer::reset() {
    if (!isStepReady) {
        ERROR("Reset is not ready. Call step() before reset");
        return false;
    }

    update(stepIndex - stepSize);
    isStepReady = false;
    stepSize = 0;
    return true;
}
