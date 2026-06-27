#pragma once

#include <string>

#include "interfaces/IBallisticSolver.h"
#include "ballistic/BallisticTable.h"

class TableSolver : public IBallisticSolver{
public:
    TableSolver();
    TableSolver(const std::string& tablePath);

    bool loadTable(const std::string& tablePath);
    bool isLoaded() const { return loaded; }

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

private:
    BallisticTable table;
    bool loaded = false;
};