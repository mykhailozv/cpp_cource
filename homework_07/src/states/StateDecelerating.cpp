#include "states/StateDecelerating.h"
#include "states/MovementUtils.h"
#include "utils/MathUtils.h"
#include <cmath>

std::unique_ptr<IDroneState> StateDecelerating::execute(DroneContext& ctx)
{
    if (ctx.distance >= ctx.ammoRange + MathUtils::calculateAccelerationPath(ctx.current->velocity, ctx.attackSpeed(), ctx.acceleration()) - MathUtils::EPS) {
        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::updateStop(ctx);
        } else {
            MovementUtils::updateAccelerating(ctx);
            ctx.next->upDirection(ctx.neededDir);
        }
    } else {
        ctx.neededDir += M_PI;
        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::updateStop(ctx);
        } else {
            double remainingDistance = ctx.ammoRange + ctx.accelPath() - ctx.distance;
            double stoppingDistance = MathUtils::calculateBrakingDistance(ctx.current->velocity, ctx.acceleration());

            if (remainingDistance <= stoppingDistance + MathUtils::EPS) {
                MovementUtils::stepWithDecelerating(ctx);
                ctx.next->upDirection(ctx.neededDir);
            } else {
                MovementUtils::stepWithAccelerating(ctx);
                ctx.next->upDirection(ctx.neededDir);
            }
        }
    }

    return nullptr;
}