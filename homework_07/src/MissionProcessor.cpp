#include "MissionProcessor.h"
#include "Types.h"

MissionProcessor::MissionProcessor(
    IBallisticSolver* solver,
    ITargetProvider* targets,
    IConfigLoader* config
)
    : solver(solver)
    , targets(targets)
    , configLoader(config)
    , droneCfg(nullptr)
    , currentIndex(0)
{
}

void MissionProcessor::init(const char* configSource)
{
    // TODO: implement
    (void)configSource;
}

bool MissionProcessor::hasNext() const
{
    // TODO: implement
    return false;
}

Coord MissionProcessor::step()
{
    // TODO: implement
    return Coord{0.0, 0.0};
}

void MissionProcessor::reset()
{
    // TODO: implement
    currentIndex = 0;
}

void MissionProcessor::changeSolver(IBallisticSolver* newSolver)
{
    // TODO: implement
    solver = newSolver;
}