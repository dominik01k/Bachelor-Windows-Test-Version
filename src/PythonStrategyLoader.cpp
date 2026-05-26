#include "PythonStrategyLoader.h"
#include <Python.h>
#include <filesystem>
#include <pybind11/embed.h>
#include "Logger.h"

namespace fs = std::filesystem;
namespace py = pybind11;

std::vector<std::string> PythonStrategyLoader::loadStrategies(StrategyType type) {
    switch(type) {
        case StrategyType::TEAM_STRATEGY:
            LOG_DEBUG("Factory", "Initiating load for TEAM_STRATEGY");
            return loadPythonStrategies("team", "base_strategy_team", "BaseStrategyTeam");
        case StrategyType::PLAYER_STRATEGY_TEAM_CONTEXT:
            LOG_DEBUG("Factory", "Initiating load for PLAYER_STRATEGY_TEAM_CONTEXT");
            return loadPythonStrategies("player/with_team_context", "base_strategy_with_team_context", "BaseStrategyPlayerWithTeamContext");
        case StrategyType::PLAYER_STRATEGY_AUTONOMOUS:
            LOG_DEBUG("Factory", "Initiating load for PLAYER_STRATEGY_AUTONOMOUS");
            return loadPythonStrategies("player/autonomous", "base_strategy_autonomous", "BaseStrategyPlayerAutonomous");
        default:
            LOG_WARN("Factory", "Unknown StrategyType requested");
            return {};
    }
}

std::vector<std::string> PythonStrategyLoader::loadPythonStrategies(
    const std::string& subPath,
    const std::string& fileName,
    const std::string& baseClassName
) {
    std::vector<std::string> result;

    py::gil_scoped_acquire gil;

    try {
        fs::path exePath = fs::current_path();
        fs::path strategyPath = fs::canonical(exePath / ".." / "src_python/python_strategies" / subPath);
        fs::path rootPath = strategyPath.parent_path().parent_path();

        LOG_TRACE("Factory", "Resolved Python root path: " + rootPath.string());
        LOG_TRACE("Factory", "Scanning for strategies in: " + strategyPath.string());

        py::module sys = py::module::import("sys");
        sys.attr("path").attr("append")(rootPath.string());

        std::string fullModule = "python_strategies." + subPath + "." + fileName;
        std::replace(fullModule.begin(), fullModule.end(), '/', '.');

        LOG_DEBUG("Factory", "Attempting to import base module: " + fullModule);
        py::module baseMod = py::module::import(fullModule.c_str());
        py::object baseClass = baseMod.attr(baseClassName.c_str());

        for (const auto& entry : fs::directory_iterator(strategyPath)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".py") continue;

            std::string moduleName = entry.path().stem().string();

            if (moduleName == fileName) continue; 

            std::string fullModuleName = "python_strategies." + subPath + "." + moduleName;
            std::replace(fullModuleName.begin(), fullModuleName.end(), '/', '.');

            try {
                py::object module = py::module_::import(fullModuleName.c_str());
                py::dict module_dict = module.attr("__dict__");

                for (auto item : module_dict) {
                    py::handle name = item.first;
                    py::handle obj = item.second;

                    if (py::isinstance<py::type>(obj)) {
                        py::object is_subclass = py::eval("issubclass");
                        py::object is_sub = is_subclass(obj, baseClass);

                        if (is_sub.cast<bool>() && !obj.equal(baseClass)) {
                            std::string className = std::string(py::str(name));
                            LOG_TRACE("Factory", "Found potential Strategy Class: " + className + " in " + fullModuleName);

                            py::object instance = obj();
                            if (py::hasattr(instance, "get_name")) {
                                py::object name_result = instance.attr("get_name")();
                                std::string strategyName = name_result.cast<std::string>();
                                
                                result.push_back("[Py] " + strategyName);
                                LOG_INFO("Factory", "Successfully loaded Python strategy: '" + strategyName + "' (Class: " + className + ")");
                            } else {
                                LOG_WARN("Factory", "Method 'get_name()' is missing in strategy class: " + className);
                            }
                        }
                    }
                }

            } catch (const py::error_already_set& e) {
                LOG_ERROR("Factory", "Python error while loading module " + fullModuleName + ": " + std::string(e.what()));
            }
        }

    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Factory", "Filesystem error while resolving paths for " + subPath + ": " + std::string(e.what()));
    } catch (const py::error_already_set& e) {
        LOG_ERROR("Factory", "Python initialization/import error in " + subPath + ": " + std::string(e.what()));
    } catch (const std::exception& e) {
        LOG_ERROR("Factory", "Unknown C++ exception in loadPythonStrategies: " + std::string(e.what()));
    }

    LOG_DEBUG("Factory", "Finished loading strategies from " + subPath + ". Found " + std::to_string(result.size()) + " strategies.");
    return result;
}