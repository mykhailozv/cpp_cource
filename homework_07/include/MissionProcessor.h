#pragma once

#include "Types.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"
#include "interfaces/IResultExporter.h"

struct Coord;
struct DroneConfig;

class MissionProcessor {
public:
    MissionProcessor(
        IBallisticSolver* solver,
        ITargetProvider* targets,
        IResultExporter* exporter
    );

    void init(const IConfigLoader* config);
    bool hasNext() const;
    Coord step();
    bool reset();
    void changeSolver(IBallisticSolver* newSolver);
    bool saveData();

    ~MissionProcessor();
    
private:
    IBallisticSolver* solver;
    ITargetProvider* targets;
    IResultExporter* exporter;
    const IConfigLoader* configLoader;

    const DroneConfig* droneConfig;
    const AmmoParams* ammoParams;
    int currentIndex;

    StepTimer stepTimer;
    SimStep* steps;
    SimStep* simStep;
    bool inProgress;

    double calculateInitVersionTimeToTarget(
        const SimStep* simStep,
        const Coord* targetPos,
        const Coord* targetVelocity
    );

    double calculateRotationTime(double dronDir, double neededDir, double dronVelocity);
    void updateStop();
    void updateMoving();
    void updateAccelerating();
    DronePhase determineMotionPhase(double distance);
    void stepWithAccelerating();
    void stepWithDecelerating();
    void stepRotation(double neededDir);

    double calculateAmmoFlightTime();
    double calculateHorizontalAmmoRange();

    void initSimStep();
};