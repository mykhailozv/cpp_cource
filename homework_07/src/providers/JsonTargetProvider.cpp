#include <fstream>
#include <iostream>

#include "third_party/nlohmann/json.hpp"
#include "providers/JsonTargetProvider.h"
#include "Types.h"
#include "utils/Logging.h"

using json = nlohmann::json;

JsonTargetProvider::JsonTargetProvider(const std::string& path)
: path(path) {
}

int JsonTargetProvider::getTargetCount() const
{
    return targetCount;
}

Target* JsonTargetProvider::getTarget(int idx)
{
    if (idx < targetCount) {
        return &targetList[idx];
    }

    static Target dummy{};
    return &dummy;
}

bool JsonTargetProvider::init(){
    return readTargetsCoord(path);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, x, y)
bool JsonTargetProvider::readTargetsCoord(const std::string& path){
    std::ifstream readData(path);

    if (!readData.is_open())
    {
        ERROR("Failed to open targets.json file");
        return false;
    }

    json data;

    try {
        data = json::parse(readData);
    } catch (const json::parse_error& e) {
        ERROR("JSON parse error: " << e.what());
        return false;
    }

    if (!data.contains("targetCount")){
        ERROR("Missing key: targetCount");
        return false;
    }

    targetCount = data["targetCount"];

    if (!data.contains("timeSteps")){
        ERROR("Missing key: timeSteps");
        return false;
    }

    timeSteps = data["timeSteps"];

    targetList.resize(targetCount);

    try {
        for (int i = 0; i < targetCount; i++) {
            targetList[i].init(i, timeSteps);

            for (int j = 0; j < timeSteps; j++)
            {
                targetList[i].targets[j] = data["targets"][i]["positions"][j];
            }
            
        }
    } catch (const json::exception& e) {
        ERROR("Targets parsing error: " << e.what());
        return false;
    }

    return true;
}

JsonTargetProvider::~JsonTargetProvider(){
}

int JsonTargetProvider::getSimCycleStep(){
    return timeSteps;
}