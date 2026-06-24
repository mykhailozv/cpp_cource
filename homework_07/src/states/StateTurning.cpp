#include "states/StateTurning.h"
#include "states/MovementUtils.h"
#include "utils/MathUtils.h"
#include <cmath>

std::unique_ptr<IDroneState> StateTurning::execute(DroneContext& ctx)
{
    if (ctx.distance >= ctx.ammoRange + ctx.accelPath() - MathUtils::EPS) {
        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::stepRotation(ctx.neededDir, ctx);
        } else {
            MovementUtils::stepWithAccelerating(ctx);
            ctx.next->upDirection(ctx.neededDir);
        }
    } else {
        ctx.neededDir += M_PI;

        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::stepRotation(ctx.neededDir, ctx);
        } else {
            MovementUtils::stepWithAccelerating(ctx);
            ctx.next->upDirection(ctx.neededDir);
        }
    }

    return nullptr;
}