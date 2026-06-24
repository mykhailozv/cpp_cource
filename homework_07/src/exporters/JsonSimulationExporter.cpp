#include <iostream>
#include <fstream>

#include "exporters/JsonSimulationExporter.h"
#include "utils/Logging.h"
#include "third_party/nlohmann/json.hpp"
#include "simulation/SimStep.h"
#include "states/IDroneState.h"

using json = nlohmann::json;

JsonSimulationExporter::JsonSimulationExporter(const std::string& path):path(path){
}


bool JsonSimulationExporter::save(SimStep* simStep, int resultsCount) const{
    json out;
    out["totalSteps"] = resultsCount;
    out["steps"] = json::array();

    for (int i = 0; i < resultsCount; i++) {
        json step;
        step["position"]        = {{"x", simStep[i].pos.x}, {"y", simStep[i].pos.y}};
        step["direction"]       = simStep[i].direction;
        step["state"]           = simStep[i].stateObj ? simStep[i].stateObj->getId() : 0;
        step["targetIndex"]     = simStep[i].targetIdx;
        step["dropPoint"]       = {{"x", simStep[i].dropPoint.x},
                                {"y", simStep[i].dropPoint.y}};
        step["aimPoint"]        = {{"x", simStep[i].aimPoint.x},
                                {"y", simStep[i].aimPoint.y}};
        step["predictedTarget"] = {{"x", simStep[i].predictedTarget.x},
                                {"y", simStep[i].predictedTarget.y}};
        out["steps"].push_back(step);
    }

    std::ofstream outputStream(path);
    
    if (!outputStream.is_open()) {
        ERROR("Failed to open simulation.json file");
        return false;
    }

    outputStream << out.dump(2);
    return true;
}