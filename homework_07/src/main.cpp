#include "geometry/Coord.h"
#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include "utils/Logging.h"

#include "interfaces/ITargetProvider.h" // from targetProvider->init()
#include "interfaces/IConfigLoader.h" // from config->load()

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

    IBallisticSolver* solver = createSolver(SolverType::ANALYTICAL);

    if (!solver) {
        return 1;    
    }

    ITargetProvider* targetProvider = createProvider(ProviderType::JSON, path);

    if (!targetProvider) {
        return 1;    
    }

    IConfigLoader* config = createLoader(LoaderType::FILE_09, path);

    if (!config) {
        return 1;    
    }

    IResultExporter* exporter = createExporter(ExporterType::JSON, path);

    if (!exporter) {
        return 1;    
    }
    
    if (!targetProvider->init()) {
        return 1;
    }

    if(!config->load()){
        return 1;
    }

    MissionProcessor* processor;

    processor = new MissionProcessor(solver, targetProvider, exporter);
    if (!processor->init(config)) {
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
    
    delete processor;
    delete exporter;
    delete solver;
    delete targetProvider;
    delete config;

    return 0;
}
