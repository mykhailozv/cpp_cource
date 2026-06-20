#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"
#include "exporters/JsonSimulationExporter.h"
#include "solvers/TableSolver.h"
#include "utils/Logging.h"

IBallisticSolver* createSolver(SolverType type)
{
    switch (type) {
        case SolverType::ANALYTICAL:
            return new AnalyticalSolver();
        case SolverType::TABLE:
            return new TableSolver();
    }

    ERROR("createSolver: unsupported SolverType = " << static_cast<int>(type));

    return nullptr;
}

ITargetProvider* createProvider(ProviderType type, const std::string& path)
{
    switch (type) {
        case ProviderType::JSON:
            return new JsonTargetProvider(path + "targets.json");
    }

    ERROR("createProvider: unsupported ProviderType = " << static_cast<int>(type));

    return nullptr;
}

IConfigLoader* createLoader(LoaderType type, const std::string& path)
{
    switch (type) {
        case LoaderType::FILE:
            return new FileConfigLoader(path + "ammo.json", path + "config.json");
    }

    ERROR("createLoader: unsupported LoaderType = " << static_cast<int>(type));

    return nullptr;
}

IResultExporter* createExporter(ExporterType type, const std::string& path)
{
    switch (type) {
        case ExporterType::JSON:
            return new JsonSimulationExporter(path + "simulation.json");
    }

    ERROR("createExporter: unsupported ExporterType = " << static_cast<int>(type));

    return nullptr;
}