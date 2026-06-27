#pragma once

struct SimStep;

class IResultExporter{
public:
    virtual ~IResultExporter() = default;

    virtual bool save(SimStep* simStep, int resultsCount) const = 0;
};