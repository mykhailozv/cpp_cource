#include "solvers/AnalyticalSolver.h"
#include "Types.h"

Coord AnalyticalSolver::solve(
    const Coord& dronePos,
    const Coord& targetPos,
    double altitude,
    double ammoSpeed,
    double drag,
    double lift
)
{
    // TODO: implement analytical solution
    (void)dronePos;
    (void)targetPos;
    (void)altitude;
    (void)ammoSpeed;
    (void)drag;
    (void)lift;
    return Coord{0.0, 0.0};
}