#pragma once

#include <vector>
#include <memory>

#include "simulation/SimStep.h"
#include "simulation/StepTimer.h"
#include "states/IDroneState.h"

struct AmmoParams;
struct Coord;
struct DroneConfig;
class IBallisticSolver;
class IConfigLoader;
class IResultExporter;
class ITargetProvider;

class MissionProcessor {
public:
    MissionProcessor(
        std::unique_ptr<IBallisticSolver> solver,
        std::unique_ptr<ITargetProvider> targets,
        std::unique_ptr<IResultExporter> exporter
    );

    bool init(const IConfigLoader* config);
    bool hasNext() const;
    Coord step();
    bool reset();
    void changeSolver(IBallisticSolver* newSolver);
    bool saveData();

    ~MissionProcessor();
    
private:
    std::unique_ptr<IBallisticSolver> solver;
    std::unique_ptr<ITargetProvider> targets;
    std::unique_ptr<IResultExporter> exporter;
    const IConfigLoader* configLoader;

    const DroneConfig* droneConfig;
    const AmmoParams* ammoParams;
    int currentIndex;

    StepTimer stepTimer;
    std::vector<SimStep> steps;
    SimStep* simStep;
    bool inProgress;

    double calculateInitVersionTimeToTarget(
        const SimStep* simStep,
        const Coord* targetPos,
        const Coord* targetVelocity
    );

    double calculateRotationTime(double dronDir, double neededDir, double dronVelocity);

    double calculateAmmoFlightTime();
    double calculateHorizontalAmmoRange();

    void initSimStep();
};