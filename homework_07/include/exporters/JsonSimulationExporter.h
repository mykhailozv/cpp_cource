#pragma once

#include <string>
#include "interfaces/IResultExporter.h"

class JsonSimulationExporter: public IResultExporter{
public:
    JsonSimulationExporter(const std::string& path);
    bool save(SimStep* simStep, int resultsCount) const override;

private:
    const std::string path;
};