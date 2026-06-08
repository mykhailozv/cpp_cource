#pragma once

struct Coord;

class IBallisticSolver {
public:
    virtual ~IBallisticSolver() = default;

    virtual Coord solve(
        const Coord& dronePos,
        double altitude,
        double attackSpeed,
        double drag,
        double lift,
        double mass,
        double direction
    ) = 0;

    virtual double calculateAmmoFlightTime(
        double attackSpeed,
        double altitude,
        double drag,
        double lift,
        double mass
    ) = 0;

    virtual double calculateHorizontalAmmoRange(
        double attackSpeed,
        double altitude,
        double drag,
        double lift,
        double mass
    ) = 0;
};