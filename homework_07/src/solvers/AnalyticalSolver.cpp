#include <cmath>
#include <iostream>

#include "geometry/Coord.h"
#include "solvers/AnalyticalSolver.h"
#include "utils/Logging.h"
#include "utils/MathUtils.h"

Coord AnalyticalSolver::solve(
    const Coord& dronePos,
    const Coord& targetPos,
    double altitude,
    double attackSpeed,
    double drag,
    double lift,
    double mass
) {
    double horizontalAmmoRange = calculateHorizontalAmmoRange(attackSpeed, altitude, drag, lift, mass);
    double direction = (dronePos - targetPos).direction();
    Coord directionVector = {std::cos(direction), std::sin(direction)};
    Coord dropPoint = targetPos + directionVector * horizontalAmmoRange;
    return dropPoint;
}

double AnalyticalSolver::calculateAmmoFlightTime(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    double V0 = attackSpeed;
    double Z0 = altitude;

    double flightTime = 0.0;

    double a = drag * MathUtils::GRAVITY * mass - 2 * drag * drag * lift * V0;

    if (std::fabs(a) < MathUtils::EPS) {
        ERROR("a is too close to zero");
        return flightTime;
    }

    double b = -3 * MathUtils::GRAVITY * mass * mass + 3 * drag * lift * mass * V0;
    double c = 6 * mass * mass * Z0;

    // Solve cubic equation using Cardano's method (trigonometric form, p < 0)
    double p = - b * b / (3 * a * a);

    if (p >= -MathUtils::EPS) {
        ERROR("p must be negative for Cardano trig solution");
        return flightTime;
    }

    double q = 2 * b * b * b / (27 * a * a * a) + c / a;
    
    double acCosArg = 1.5 * q / p * std::sqrt(-3/p);
    
    if (acCosArg > 1.0)
    {
        ERROR("Invalid acos argument > 1.0 (" << acCosArg << "). Task cannot be solved");
        return flightTime;
    } else if (acCosArg < -1.0)
    {
        ERROR("Invalid acos argument < -1.0 (" << acCosArg << "). Task cannot be solved");
        return flightTime;
    }

    double phi = std::acos(acCosArg);
    flightTime = 2 * std::sqrt(-p/3) * std::cos((phi + 4 * M_PI) / 3) - b / (3 * a);

    if (flightTime <= MathUtils::EPS) {
        ERROR("Invalid time t: " << flightTime);
        return 0;
    }
    return flightTime;
}

double AnalyticalSolver::calculateHorizontalAmmoRange(
    double attackSpeed,
    double altitude,
    double drag,
    double lift,
    double mass
){
    double ammoFlightTime = calculateAmmoFlightTime(attackSpeed, altitude, drag, lift, mass);

    double V0{attackSpeed}, Z0{altitude};

    double t_2 = ammoFlightTime * ammoFlightTime;
    double t_3 = t_2 * ammoFlightTime;
    double t_4 = t_3 * ammoFlightTime;
    double t_5 = t_4 * ammoFlightTime;
    double d_2 = drag * drag;
    double d_3 = d_2 * drag;
    double d_4 = d_3 * drag;
    double l_2 = lift * lift;
    double l_3 = l_2 * lift;
    double l_4 = l_3 * lift;
    double m_2 = mass * mass;
    double m_3 = m_2 * mass;
    double m_4 = m_3 * mass;
    
    double horizontalAmmoRange = V0 * ammoFlightTime - t_2 * drag * V0 / (2 * mass)
            + t_3 * (6 * drag * MathUtils::GRAVITY * lift * mass - 6 * d_2 * (l_2 - 1) * V0) / (36 * m_2)
            + t_4 * (-6 * d_2 * MathUtils::GRAVITY * lift * (1 + l_2 + l_4) * mass + 3 * d_3 * l_2 *(1 + l_2) * V0 + 6 * d_3 * l_4 * (1 + l_2) * V0) / (36 * (1 + l_2) * (1 + l_2) * m_3)
            + t_5 * (3 * d_3 * MathUtils::GRAVITY * l_3 * mass - 3 * d_4 * l_2 * (1 + l_2) * V0) / (36 * (1 + l_2) * m_4);
            
    return horizontalAmmoRange;
}
