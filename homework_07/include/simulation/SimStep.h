#pragma once

#include "geometry/Coord.h"

struct SimStep {
	Coord pos;          	// позиція дрона
	double direction;    	// напрямок (рад)
    Coord directionVector;  // dx, dy
	int   state;        	// стан автомата (0-4)
	int   targetIdx;    	// індекс поточної цілі
    double velocity;
	Coord dropPoint;    	// точка скиду (куди летить дрон)
	Coord aimPoint;     	// куди впаде бомба (якщо скинути зараз)
	Coord predictedTarget;  // прогнозована позиція цілі

    void upDirection(double newDirection);
};
