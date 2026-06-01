#pragma once

#include "interfaces/IBallisticSolver.h"

class AnalyticalSolver : public IBallisticSolver {
public:
    Coord solve(
        const Coord& dronePos,
        const Coord& targetPos,
        double altitude,
        double ammoSpeed,
        double drag,
        double lift
    ) override;
};