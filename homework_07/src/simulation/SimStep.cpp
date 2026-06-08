#include <cmath>

#include "simulation/SimStep.h"
#include "utils/MathUtils.h"

void SimStep::upDirection(double newDirection){
    if (std::fabs(direction - newDirection) < MathUtils::EPS) return;
    
    direction = newDirection;
    directionVector = {std::cos(direction), std::sin(direction)};
}