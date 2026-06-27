#pragma once

#include <memory>
#include <string>

class IBallisticSolver;
class IResultExporter;
class ITargetProvider;
class IConfigLoader;

enum class SolverType { ANALYTICAL, TABLE };
enum class ProviderType { JSON };
enum class LoaderType { FILE, FILE_09 };
enum class ExporterType { JSON };

std::unique_ptr<IBallisticSolver> createSolver(SolverType type, const std::string& path="");
std::unique_ptr<ITargetProvider> createProvider(ProviderType type, const std::string& path);
std::unique_ptr<IConfigLoader> createLoader(LoaderType type, const std::string& path);
std::unique_ptr<IResultExporter> createExporter(ExporterType type, const std::string& path);