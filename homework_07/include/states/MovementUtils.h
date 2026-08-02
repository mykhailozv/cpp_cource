#pragma once

#include "states/DroneContext.h"
#include "states/StateStopped.h"
#include "states/StateMoving.h"
#include "states/StateAccelerating.h"
#include "states/StateDecelerating.h"
#include "states/StateTurning.h"
#include "utils/MathUtils.h"
#include <cmath>

/// Спільні допоміжні функції для розрахунку руху, які використовуються станами.
namespace MovementUtils {

inline void updateMoving(DroneContext& ctx)
{
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateMoving>();
    double distance = ctx.current->velocity * ctx.simTimeStep();
    ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;
}

inline void updateAccelerating(DroneContext& ctx)
{
    ctx.current->stateObj = std::make_unique<StateAccelerating>();
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateAccelerating>();
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
        ctx.next->stateObj = std::make_unique<StateMoving>();
    }
}

inline void stepWithAccelerating(DroneContext& ctx)
{
    ctx.current->stateObj = std::make_unique<StateAccelerating>();
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateAccelerating>();
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
        ctx.next->stateObj = std::make_unique<StateMoving>();
    }
}

inline void stepWithDecelerating(DroneContext& ctx)
{
    ctx.current->stateObj = std::make_unique<StateDecelerating>();
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateDecelerating>();
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
        ctx.next->stateObj = std::make_unique<StateStopped>();
    }
}

inline void stepRotation(double neededDir, DroneContext& ctx)
{
    ctx.current->stateObj = std::make_unique<StateTurning>();
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateTurning>();

    double delta = neededDir - ctx.current->direction;
    while (delta > M_PI)  delta -= 2.0 * M_PI;
    while (delta < -M_PI) delta += 2.0 * M_PI;

    double absDelta = std::fabs(delta);
    double maxRotation = ctx.angularSpeed() * ctx.simTimeStep();

    if (absDelta <= maxRotation + MathUtils::EPS) {
        ctx.next->upDirection(neededDir);
        ctx.next->stateObj = std::make_unique<StateStopped>();
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

    double timeToStop = ctx.current->velocity / dronA;
    double distance;

    ctx.current->stateObj = std::make_unique<StateDecelerating>();
    ctx.next->copyFrom(*ctx.current);
    ctx.next->stateObj = std::make_unique<StateDecelerating>();

    if (simTimeStep <= timeToStop) {
        distance = ctx.current->velocity * simTimeStep - (dronA * simTimeStep * simTimeStep) / 2.0;
        ctx.next->velocity = ctx.current->velocity - dronA * simTimeStep;
    } else {
        distance = MathUtils::calculateBrakingDistance(ctx.current->velocity, dronA);
        ctx.next->velocity = 0.0;
    }

    ctx.next->pos = ctx.current->pos + ctx.current->directionVector * distance;

    if (ctx.next->velocity < MathUtils::EPS) {
        ctx.next->stateObj = std::make_unique<StateStopped>();
    }
}

} // namespace MovementUtils