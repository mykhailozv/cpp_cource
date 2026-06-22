#pragma once

#include <memory>
#include "states/IDroneState.h"
#include "states/StateStopped.h"
#include "states/StateMoving.h"
#include "states/StateAccelerating.h"
#include "states/StateDecelerating.h"
#include "states/StateTurning.h"
#include "simulation/DronePhase.h"

// Повертає nullptr, якщо стан не змінився (nextState == currentState).
inline std::unique_ptr<IDroneState> makeStateIfChanged(int currentState, int nextState)
{
    if (currentState == nextState)
        return nullptr;

    switch (static_cast<DronePhase>(nextState)) {
        case DronePhase::STOPPED:       return std::make_unique<StateStopped>();
        case DronePhase::MOVING:        return std::make_unique<StateMoving>();
        case DronePhase::ACCELERATING:  return std::make_unique<StateAccelerating>();
        case DronePhase::DECELERATING:  return std::make_unique<StateDecelerating>();
        case DronePhase::TURNING:       return std::make_unique<StateTurning>();
    }
    return nullptr;
}