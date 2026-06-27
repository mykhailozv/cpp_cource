#include <cmath>
#include <iostream>

#include "geometry/Coord.h"
#include "solvers/TableSolver.h"
#include "utils/Logging.h"

TableSolver::TableSolver()
    : loaded(false)
{
}

TableSolver::TableSolver(const std::string& tablePath)
    : loaded(false)
{
    loaded = table.load((tablePath).c_str());
    if (!loaded) {
        ERROR("TableSolver: failed to load ballistic table from " << tablePath);
    }
}

bool TableSolver::loadTable(const std::string& tablePath)
{
    loaded = table.load(tablePath.c_str());
    if (!loaded) {
        ERROR("TableSolver: failed to load ballistic table from " << tablePath);
    }
    return loaded;
}

Coord TableSolver::solve(
    const Coord& dronePos,
    const Coord& targetPos,
    double altitude,
    double attackSpeed,
    double drag,
    double lift,
    double mass
) {
    (void)dronePos;
    (void)targetPos;

    double hDist = calculateHorizontalAmmoRange(attackSpeed, altitude, drag, lift, mass);
    double dir = (targetPos - dronePos).direction();
    return dronePos + Coord{std::cos(dir), std::sin(dir)} * hDist;
}

double TableSolver::calculateAmmoFlightTime(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    if (!loaded) {
        ERROR("TableSolver: table not loaded");
        return 0.0;
    }
    auto result = table.lookup(
        static_cast<float>(altitude),
        static_cast<float>(attackSpeed),
        static_cast<float>(mass),
        static_cast<float>(drag),
        static_cast<float>(lift)
    );
    return static_cast<double>(result.t);
}

double TableSolver::calculateHorizontalAmmoRange(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    if (!loaded) {
        ERROR("TableSolver: table not loaded");
        return 0.0;
    }
    auto result = table.lookup(
        static_cast<float>(altitude),
        static_cast<float>(attackSpeed),
        static_cast<float>(mass),
        static_cast<float>(drag),
        static_cast<float>(lift)
    );
    return static_cast<double>(result.hDist);
}