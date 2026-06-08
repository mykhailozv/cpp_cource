#pragma once

#include "config/DroneConfig.h"

struct StepTimer{
    int stepIndex{0}; // steps - загальний крок симуляції
    int timeIndex{0}; // timeStep - індекс у масиві переміщення target
    int nexTimeIndex{1};
    int timeCount{1}; // STEPS - кількість time steps у масиві

    int tickStep{0};     // крок всередині одного timeIndex
    int stepsPerTime{0}; // arrayUnitSteps скільки sim-кроків у одному array кроці

    double simTimeStep{0.0};
    double tickTime{0.0};
	bool isStepReady{false};
	int stepSize{0};

    void init(const DroneConfig& droneConfig, int timeSteps);

    void update(int newStep);
    void step();
	bool reset();

    bool operator<(int value) const;

    StepTimer& operator+=(int delta);
};

