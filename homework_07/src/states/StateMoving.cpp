#include "states/StateMoving.h"
#include "states/MakeState.h"
#include "states/MovementUtils.h"
#include "utils/MathUtils.h"
#include <cmath>

std::unique_ptr<IDroneState> StateMoving::execute(DroneContext& ctx)
{
    ctx.current->state = static_cast<int>(DronePhase::MOVING);

    if (ctx.distance >= ctx.ammoRange - MathUtils::EPS) {
        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::updateStop(ctx);
        } else {
            if (ctx.distance > ctx.ammoRange + ctx.current->velocity * ctx.simTimeStep()) {
                MovementUtils::updateMoving(ctx);
                ctx.next->upDirection(ctx.neededDir);
            } else {
                if (std::fabs(ctx.distance - ctx.ammoRange) <= ctx.hitRadius() - MathUtils::EPS) {
                    *ctx.inProgress = false;
                    MovementUtils::updateMoving(ctx);
                } else {
                    MovementUtils::updateMoving(ctx);
                    ctx.next->upDirection(ctx.neededDir);
                }
            }
        }
    } else {
        ctx.neededDir += M_PI;

        if (MathUtils::needStopForRotation(ctx.current->direction, ctx.neededDir, ctx.turnThreshold())) {
            MovementUtils::updateStop(ctx);
        } else {
            double s_brake = MathUtils::calculateBrakingDistance(ctx.current->velocity, ctx.acceleration());
            if (ctx.distance + s_brake < ctx.ammoRange - MathUtils::EPS) {
                MovementUtils::updateMoving(ctx);
                ctx.next->upDirection(ctx.neededDir);
            } else {
                MovementUtils::updateStop(ctx);
                ctx.next->upDirection(ctx.neededDir);
            }
        }
    }

    return makeStateIfChanged(ctx.current->state, ctx.next->state);
}