#pragma once

#include <string>

#include "interfaces/IBallisticSolver.h"
#include "interfaces/IResultExporter.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"

enum class SolverType { ANALYTICAL };
enum class ProviderType { JSON };
enum class LoaderType { FILE };
enum class ExporterType { JSON };

IBallisticSolver* createSolver(SolverType type);
ITargetProvider* createProvider(ProviderType type, const std::string& path);
IConfigLoader* createLoader(LoaderType type, const std::string& path);
IResultExporter* createExporter(ExporterType type, const std::string& path);