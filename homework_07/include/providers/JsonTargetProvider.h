#pragma once

#include <string>
#include "interfaces/ITargetProvider.h"

struct Target;

class JsonTargetProvider: public ITargetProvider {
public:
    JsonTargetProvider(const std::string& path);
    int getTargetCount() const override;
    Target* getTarget(int idx) override;
    int getSimCycleStep() override;

    ~JsonTargetProvider();

private:
    int targetCount;
    int timeSteps;
    Target* targetList;
    bool readTargetsCoord(const std::string& path);

};
