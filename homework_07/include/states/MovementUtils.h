#pragma once

#include "states/DroneContext.h"
#include "simulation/DronePhase.h"
#include "utils/MathUtils.h"
#include <cmath>

/// Спільні допоміжні функції для розрахунку руху, які використовуються станами.
namespace MovementUtils {

inline void updateMoving(DroneContext& ctx)
{
    *ctx.next = *ctx.current;
    double distance = ctx.current->velocity * ctx.simTimeStep();
    ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
}

inline void updateAccelerating(DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::ACCELERATING);
    *ctx.next = *ctx.current;
    double timeToMax = (ctx.attackSpeed() - ctx.current->velocity) / ctx.acceleration();

    if (ctx.simTimeStep() <= timeToMax) {
        double distance = ctx.current->velocity * ctx.simTimeStep()
                        + (ctx.acceleration() * ctx.simTimeStep() * ctx.simTimeStep()) / 2.0;
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
        ctx.next->velocity = ctx.current->velocity + ctx.acceleration() * ctx.simTimeStep();
    } else {
        double distance = ctx.current->velocity * timeToMax
                        + (ctx.acceleration() * timeToMax * timeToMax) / 2.0
                        + ctx.attackSpeed() * (ctx.simTimeStep() - timeToMax);
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
        ctx.next->velocity = ctx.attackSpeed();
    }

    if (ctx.next->velocity > ctx.attackSpeed() - MathUtils::EPS) {
        ctx.next->state = static_cast<int>(DronePhase::MOVING);
    }
}

inline void stepWithAccelerating(DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::ACCELERATING);
    *ctx.next = *ctx.current;
    double timeToMax = (ctx.attackSpeed() - ctx.current->velocity) / ctx.acceleration();

    if (ctx.simTimeStep() <= timeToMax + MathUtils::EPS) {
        double averageVelocity = ctx.current->velocity + ctx.acceleration() * ctx.simTimeStep() * 0.5;
        double distance = averageVelocity * ctx.simTimeStep();
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
        ctx.next->velocity = ctx.current->velocity + ctx.acceleration() * ctx.simTimeStep();
    } else {
        double accDistance = (ctx.current->velocity + ctx.attackSpeed()) * 0.5 * timeToMax;
        double cruiseDistance = ctx.attackSpeed() * (ctx.simTimeStep() - timeToMax);
        double totalDistance = accDistance + cruiseDistance;
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * totalDistance;
        ctx.next->velocity = ctx.attackSpeed();
    }

    if (ctx.next->velocity > ctx.attackSpeed() - MathUtils::EPS) {
        ctx.next->velocity = ctx.attackSpeed();
        ctx.next->state = static_cast<int>(DronePhase::MOVING);
    }
}

inline void stepWithDecelerating(DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::DECELERATING);
    *ctx.next = *ctx.current;
    double timeToStop = ctx.current->velocity / ctx.acceleration();

    if (ctx.simTimeStep() <= timeToStop + MathUtils::EPS) {
        double averageVelocity = ctx.current->velocity - ctx.acceleration() * ctx.simTimeStep() * 0.5;
        double distance = averageVelocity * ctx.simTimeStep();
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
        ctx.next->velocity = ctx.current->velocity - ctx.acceleration() * ctx.simTimeStep();
    } else {
        double stopDistance = MathUtils::calculateBrakingDistance(ctx.current->velocity, ctx.acceleration());
        ctx.next->pos = ctx.current->pos + ctx.current->directionVector * stopDistance;
        ctx.next->velocity = 0.0;
    }

    if (ctx.next->velocity < MathUtils::EPS) {
        ctx.next->velocity = 0.0;
        ctx.next->state = static_cast<int>(DronePhase::STOPPED);
    }
}

inline void stepRotation(double neededDir, DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::TURNING);
    *ctx.next = *ctx.current;

    double delta = neededDir - ctx.current->direction;
    while (delta > M_PI)  delta -= 2.0 * M_PI;
    while (delta < -M_PI) delta += 2.0 * M_PI;

    double absDelta = std::fabs(delta);
    double maxRotation = ctx.angularSpeed() * ctx.simTimeStep();

    if (absDelta <= maxRotation + MathUtils::EPS) {
        ctx.next->upDirection(neededDir);
        ctx.next->state = static_cast<int>(DronePhase::STOPPED);
    } else {
        ctx.next->upDirection(ctx.current->direction + (delta / absDelta) * maxRotation);
    }

    while (ctx.next->direction > M_PI)  ctx.next->direction -= 2.0 * M_PI;
    while (ctx.next->direction < -M_PI) ctx.next->direction += 2.0 * M_PI;
}

inline void updateStop(DroneContext& ctx)
{
    double dronA = ctx.acceleration();
    double simTimeStep = ctx.simTimeStep();

    ctx.current->state = static_cast<int>(DronePhase::DECELERATING);
    double timeToStop = ctx.current->velocity / dronA;
    double distance;

    *ctx.next = *ctx.current;

    if (simTimeStep <= timeToStop) {
        distance = ctx.current->velocity * simTimeStep - (dronA * simTimeStep * simTimeStep) / 2.0;
        ctx.next->velocity = ctx.current->velocity - dronA * simTimeStep;
    } else {
        distance = MathUtils::calculateBrakingDistance(ctx.current->velocity, dronA);
        ctx.next->velocity = 0.0;
    }

    ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;

    if (ctx.next->velocity < MathUtils::EPS) {
        ctx.next->state = static_cast<int>(DronePhase::STOPPED);
    }
}

} // namespace MovementUtils