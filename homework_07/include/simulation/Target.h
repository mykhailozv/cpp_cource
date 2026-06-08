#pragma once

#include <vector>

#include "geometry/Coord.h"

struct Target{
	int targetId;
	int timeSteps;
	std::vector<Coord> targets;

	//Target(const Target&) = delete;
    Target& operator=(const Target&) = delete;

    void init(int targetId, int count);
	~Target();
};

