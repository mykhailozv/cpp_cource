#pragma once

#include "interfaces/ITargetProvider.h"

struct Target;

class JsonTargetProvider: public ITargetProvider {
public:
    int getTargetCount() const override;
    Target& getTarget(int idx) override;

};
