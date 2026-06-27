#include <iostream>

#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include "Types.h"
#include "interfaces/IResultExporter.h"
#include "utils/Logging.h"

//#include "SimpleSimulator.h"

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    std::string path;

    if (argc > 1) {
        path = argv[1];
        std::cout << "data folder: " << path << "\n\n";
    }

    std::cout << "[LOG] SimpleSimulator::run() - starting mission..." << std::endl;

    IBallisticSolver* solver = createSolver(SolverType::ANALYTICAL);
    ITargetProvider* targetProvider = createProvider(ProviderType::JSON, path);
    IConfigLoader* config = createLoader(LoaderType::FILE, path);
    IResultExporter* exporter = createExporter(ExporterType::JSON, path);
    
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
