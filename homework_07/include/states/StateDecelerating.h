#pragma once

#include <memory>
#include "states/IDroneState.h"

class StateDecelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "DECELERATING"; }
    const int getId() const override {return 2;}
};