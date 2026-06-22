#pragma once

#include <memory>
#include "states/IDroneState.h"

class StateTurning : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "TURNING"; }
};