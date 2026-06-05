#include <cmath>
#include <iostream>

#include "Types.h"
#include "utils/Logging.h"
#include "utils/MathUtils.h"

// Додавання координат
Coord Coord::operator+(const Coord& other) const {
    Coord result;
    result.x = x + other.x;
    result.y = y + other.y;
    return result;
}

// Віднімання координат
Coord Coord::operator-(const Coord& other) const {
    Coord result;
    result.x = x - other.x;
    result.y = y - other.y;
    return result;
}

// Множення на скаляр
Coord Coord::operator*(double s) const {
    Coord result;
    result.x = x * s;
    result.y = y * s;
    return result;
}

// Ділення на скаляр
Coord Coord::operator/(double s) const {
    Coord result;
    result.x = x / s;
    result.y = y / s;
    return result;
}

// Порівняння
bool Coord::operator==(const Coord& other) const {
    return (std::fabs(x - other.x) < MathUtils::EPS) && (std::fabs(y - other.y) < MathUtils::EPS);
}

double Coord::length() const {
    return std::sqrt(x*x + y*y);
}

Coord Coord::getNorm() const {
    double len = length();

    if (len < MathUtils::EPS)
    {
        return {0,0};
    }
    
    return (*this) / len;        
}

double Coord::distanceTo(const Coord& other) const{
    return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
}

double Coord::direction() const{
    return std::atan2(y, x);
}

void Target::removeData(){
    delete[] targets;
    targets = nullptr;
}

void Target::init(int targetId, int count){
    this->targetId = targetId;
    timeSteps = count;

    delete[] targets;
    targets = new Coord[count];
}

Target::~Target(){removeData();}


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

void SimStep::upDirection(double newDirection){
    if (std::fabs(direction - newDirection) < MathUtils::EPS) return;
    
    direction = newDirection;
    directionVector = {std::cos(direction), std::sin(direction)};
}