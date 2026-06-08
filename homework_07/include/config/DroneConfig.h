#pragma once

#include <string>

#include "geometry/Coord.h"

struct DroneConfig {
	Coord startPos;     	// початкова позиція (x, y)
	double altitude;     	// висота
	double initialDir;   	// початковий напрямок (рад)
	double attackSpeed;  	// швидкість атаки (м/с)
	double accelPath;    	// шлях розгону (м)
    std::string ammoName;
	double arrayTimeStep;	// крок часу масиву цілей
	double simTimeStep;  	// крок симуляції
	double hitRadius;    	// радіус влучення
	double angularSpeed; 	// кутова швидкість (рад/с)
	double turnThreshold;	// поріг повороту (рад)
    double acceleration;    // прискорення дрона за модулем
};
