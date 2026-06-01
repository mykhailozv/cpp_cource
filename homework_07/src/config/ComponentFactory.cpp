#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"

IBallisticSolver* createSolver(SolverType type)
{
    switch (type) {
    case SolverType::ANALYTICAL:
        return new AnalyticalSolver();
    }
    return nullptr;
}

ITargetProvider* createProvider(ProviderType type, const char* param)
{
    switch (type) {
    case ProviderType::JSON:
        return new JsonTargetProvider();
    }
    (void)param;
    return nullptr;
}

IConfigLoader* createLoader(LoaderType type)
{
    switch (type) {
    case LoaderType::FILE:
        return new FileConfigLoader("");
    }
    return nullptr;
}