#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"
#include "exporters/JsonSimulationExporter.h"
#include "solvers/TableSolver.h"
#include "utils/Logging.h"

std::unique_ptr<IBallisticSolver> createSolver(SolverType type)
{
    switch (type) {
        case SolverType::ANALYTICAL:
            return std::make_unique<AnalyticalSolver>();
        case SolverType::TABLE:
            return std::make_unique<TableSolver>();
    }

    ERROR("createSolver: unsupported SolverType = " << static_cast<int>(type));

    return nullptr;
}

std::unique_ptr<ITargetProvider> createProvider(ProviderType type, const std::string& path)
{
    switch (type) {
        case ProviderType::JSON:
            return std::make_unique<JsonTargetProvider>(path + "targets.json");
    }

    ERROR("createProvider: unsupported ProviderType = " << static_cast<int>(type));

    return nullptr;
}

std::unique_ptr<IConfigLoader> createLoader(LoaderType type, const std::string& path)
{
    switch (type) {
        case LoaderType::FILE:
            return std::make_unique<FileConfigLoader>(path + "ammo.json", path + "config.json");
        case LoaderType::FILE_09:
            return std::make_unique<FileConfigLoader>(path + "ammo_09.json", path + "config.json");
    }

    ERROR("createLoader: unsupported LoaderType = " << static_cast<int>(type));

    return nullptr;
}

std::unique_ptr<IResultExporter> createExporter(ExporterType type, const std::string& path)
{
    switch (type) {
        case ExporterType::JSON:
            return std::make_unique<JsonSimulationExporter>(path + "simulation.json");
    }

    ERROR("createExporter: unsupported ExporterType = " << static_cast<int>(type));

    return nullptr;
}