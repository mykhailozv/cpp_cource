#pragma once

struct Target;

class ITargetProvider{

public:
    virtual ~ITargetProvider() = default;
    
    virtual int getTargetCount() const = 0;
    virtual Target& getTarget(int idx) = 0;
};