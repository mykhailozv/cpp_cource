#pragma once

#include <memory>
#include "geometry/Coord.h"

class IDroneState;

struct SimStep {
	Coord pos;          	// позиція дрона
	double direction;    	// напрямок (рад)
    Coord directionVector;  // dx, dy
	std::unique_ptr<IDroneState> stateObj; // стан автомата
	int   targetIdx;    	// індекс поточної цілі
    double velocity;
	Coord dropPoint;    	// точка скиду (куди летить дрон)
	Coord aimPoint;     	// куди впаде бомба (якщо скинути зараз)
	Coord predictedTarget;  // прогнозована позиція цілі

    void upDirection(double newDirection);
    void copyFrom(const SimStep& other);
};