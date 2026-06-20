#include <iostream>

#include "geometry/Coord.h"
#include "solvers/TableSolver.h"
#include "utils/Logging.h"

Coord TableSolver::solve(
    const Coord& dronePos,
    const Coord& targetPos,
    double altitude,
    double attackSpeed,
    double drag,
    double lift,
    double mass
) {
    ERROR("TableSolver::solve not implemented");
    return {0.0,0.0};
}

double TableSolver::calculateAmmoFlightTime(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    return 0.0;
}

double TableSolver::calculateHorizontalAmmoRange(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    return 0.0;
}