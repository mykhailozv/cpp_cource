#pragma once

struct Coord;

class IBallisticSolver {
public:
    virtual ~IBallisticSolver() = default;

    virtual Coord solve(
        const Coord& dronePos,
        const Coord& targetPos,
        double altitude,
        double ammoSpeed,
        double drag,
        double lift
    ) = 0;
};