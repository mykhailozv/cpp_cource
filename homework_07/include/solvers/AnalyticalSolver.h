#pragma once

#include "interfaces/IBallisticSolver.h"

class AnalyticalSolver : public IBallisticSolver {
public:
    Coord solve(
        const Coord& dronePos,
        double altitude,
        double attackSpeed,
        double drag,
        double lift,
        double mass,
        double direction
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