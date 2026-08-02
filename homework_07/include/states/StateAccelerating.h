#pragma once

#include <memory>
#include "states/IDroneState.h"

class StateAccelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "ACCELERATING"; }
    const int getId() const override {return 1;}
};