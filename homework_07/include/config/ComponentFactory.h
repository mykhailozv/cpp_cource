#pragma once

#include <string>

class IBallisticSolver;
class IResultExporter;
class ITargetProvider;
class IConfigLoader;

enum class SolverType { ANALYTICAL, TABLE };
enum class ProviderType { JSON };
enum class LoaderType { FILE, FILE_09 };
enum class ExporterType { JSON };

IBallisticSolver* createSolver(SolverType type);
ITargetProvider* createProvider(ProviderType type, const std::string& path);
IConfigLoader* createLoader(LoaderType type, const std::string& path);
IResultExporter* createExporter(ExporterType type, const std::string& path);