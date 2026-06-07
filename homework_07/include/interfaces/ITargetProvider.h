#pragma once

struct Target;

class ITargetProvider{

public:
    virtual ~ITargetProvider() = default;
    
    virtual int getTargetCount() const = 0;
    virtual Target* getTarget(int idx) = 0;
    virtual bool init() = 0;

    // Returns INT_MAX if the simulation is not cyclic
    virtual int getSimCycleStep() = 0;
};