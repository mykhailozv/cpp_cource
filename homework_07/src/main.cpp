#include <memory>

#include "geometry/Coord.h"
#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include "utils/Logging.h"

#include "interfaces/IBallisticSolver.h"
#include "interfaces/IResultExporter.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"

//#include "SimpleSimulator.h"

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    std::string path;

    if (argc > 1) {
        path = argv[1];
        LOG("data folder: " << path);
    }

    auto solver = createSolver(SolverType::ANALYTICAL);

    if (!solver) {
        return 1;    
    }

    auto targetProvider = createProvider(ProviderType::JSON, path);

    if (!targetProvider) {
        return 1;    
    }

    auto config = createLoader(LoaderType::FILE, path);

    if (!config) {
        return 1;    
    }

    auto exporter = createExporter(ExporterType::JSON, path);

    if (!exporter) {
        return 1;    
    }
    
    if (!targetProvider->init()) {
        return 1;
    }

    if(!config->load()){
        return 1;
    }

    auto processor = std::make_unique<MissionProcessor>(
        std::move(solver),
        std::move(targetProvider),
        std::move(exporter)
    );
    
    if (!processor->init(config.get())) {
        return 1;
    }

    while (processor->hasNext()) {
        Coord drop = processor->step();
        (void)drop;
    }

    if(!processor->saveData()){
        return 1;
    }

    LOG("Complete Simulation");

    return 0;
}