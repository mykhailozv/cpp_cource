#include <cmath>

#include "simulation/SimStep.h"
#include "utils/MathUtils.h"

void SimStep::upDirection(double newDirection){
    if (std::fabs(direction - newDirection) < MathUtils::EPS) return;
    
    direction = newDirection;
    directionVector = {std::cos(direction), std::sin(direction)};
}

void SimStep::copyFrom(const SimStep& other)
{
    pos = other.pos;
    direction = other.direction;
    directionVector = other.directionVector;
    // stateObj не копіюємо — буде встановлено окремо
    targetIdx = other.targetIdx;
    velocity = other.velocity;
    dropPoint = other.dropPoint;
    aimPoint = other.aimPoint;
    predictedTarget = other.predictedTarget;
}