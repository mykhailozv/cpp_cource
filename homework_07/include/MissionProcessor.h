#pragma once

#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"

struct Coord;
struct DroneConfig;

class MissionProcessor {
public:
    MissionProcessor(
        IBallisticSolver* solver,
        ITargetProvider* targets,
        IConfigLoader* config
    );

    void init(const char* configSource);
    bool hasNext() const;
    Coord step();
    void reset();
    void changeSolver(IBallisticSolver* newSolver);

private:
    IBallisticSolver* solver;
    ITargetProvider* targets;
    IConfigLoader* configLoader;

    DroneConfig* droneCfg;
    int currentIndex;
};