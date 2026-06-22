#include "states/StateStopped.h"
#include "states/MakeState.h"
#include "states/MovementUtils.h"
#include "utils/MathUtils.h"
#include <cmath>

std::unique_ptr<IDroneState> StateStopped::execute(DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::STOPPED);

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

    return makeStateIfChanged(ctx.current->state, ctx.next->state);
}