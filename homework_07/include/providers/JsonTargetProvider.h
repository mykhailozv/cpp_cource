#pragma once

#include <string>
#include <vector>

#include "interfaces/ITargetProvider.h"

struct Target;

class JsonTargetProvider: public ITargetProvider {
public:
    JsonTargetProvider(const std::string& path);
    int getTargetCount() const override;
    Target* getTarget(int idx) override;
    bool init() override;
    int getSimCycleStep() override;

    ~JsonTargetProvider();

private:
    int targetCount;
    int timeSteps;
    std::vector<Target> targetList;
    std::string path;
    bool readTargetsCoord(const std::string& path);

};
