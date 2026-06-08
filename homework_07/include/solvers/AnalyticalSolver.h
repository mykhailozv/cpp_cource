#pragma once

#include "interfaces/IBallisticSolver.h"

class AnalyticalSolver : public IBallisticSolver {
public:
    Coord solve(
        const Coord& dronePos,
        const Coord& targetPos,
        double altitude,
        double attackSpeed,
        double drag,
        double lift,
        double mass
    ) override;

    double calculateAmmoFlightTime(
        double attackSpeed,
        double altitude,
        double drag,
        double lift,
        double mass
    ) override;

    double calculateHorizontalAmmoRange(
        double attackSpeed,
        double altitude,
        double drag,
        double lift,
        double mass
    ) override;
};