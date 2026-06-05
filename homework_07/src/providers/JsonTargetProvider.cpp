#include <fstream>
#include <iostream>

#include "third_party/nlohmann/json.hpp"
#include "providers/JsonTargetProvider.h"
#include "Types.h"

using json = nlohmann::json;

JsonTargetProvider::JsonTargetProvider(const std::string& path){
    readTargetsCoord(path);
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, x, y)
bool JsonTargetProvider::readTargetsCoord(const std::string& path){
    std::ifstream readData(path);

    if (!readData.is_open())
    {
        std::cerr << "Failed to open targets.json file\n";
        return false;
    }

    json data = json::parse(readData);

    targetCount = data["targetCount"];
    timeSteps = data["timeSteps"];

    targetList = new Target[targetCount];

    for (int i = 0; i < targetCount; i++)
    {
        targetList[i].init(i, timeSteps);

        for (int j = 0; j < timeSteps; j++)
        {
            targetList[i].targets[j] = data["targets"][i]["positions"][j];
        }
        
    }
    return true;
}

JsonTargetProvider::~JsonTargetProvider(){
    delete [] targetList;
}

int JsonTargetProvider::getSimCycleStep(){
    return timeSteps;
}