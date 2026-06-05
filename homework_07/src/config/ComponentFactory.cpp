#include "config/ComponentFactory.h"
#include "interfaces/IResultExporter.h"
#include "solvers/AnalyticalSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"
#include "exporters/JsonSimulationExporter.h"

IBallisticSolver* createSolver(SolverType type)
{
    switch (type) {
    case SolverType::ANALYTICAL:
        return new AnalyticalSolver();
    }
    return nullptr;
}

ITargetProvider* createProvider(ProviderType type, const std::string& path)
{
    switch (type) {
        case ProviderType::JSON:
            return new JsonTargetProvider(path + "targets.json");
    }
    
    return nullptr;
}

IConfigLoader* createLoader(LoaderType type, const std::string& path)
{
    switch (type) {
        case LoaderType::FILE:
            return new FileConfigLoader(path + "ammo.json", path + "config.json");
    }

    return nullptr;
}

IResultExporter* createExporter(ExporterType type, const std::string& path)
{
    switch (type) {
        case ExporterType::JSON:
            return new JsonSimulationExporter(path + "simulation.json");
    }

    return nullptr;
}