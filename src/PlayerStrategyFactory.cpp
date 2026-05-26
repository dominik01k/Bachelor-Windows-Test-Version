#include "PlayerStrategyFactory.h"
#include <iostream>
#include <filesystem>
#include "Logger.h"

namespace fs = std::filesystem;

std::unordered_map<std::string, std::shared_ptr<PlayerStrategy>> PlayerStrategyFactory::cppStrategies;
std::unordered_map<std::string, py::object> PlayerStrategyFactory::pythonStrategies;

const std::vector<PlayerStrategyFactory::PythonStrategySearchInfo> PlayerStrategyFactory::pythonStrategyTypes = {
    { "player/with_team_context", "base_strategy_with_team_context", "BaseStrategyPlayerWithTeamContext" },
    { "player/autonomous", "base_strategy_autonomous", "BaseStrategyPlayerAutonomous" }
};

std::shared_ptr<PlayerStrategy> PlayerStrategyFactory::loadCppStrategy(std::string name){
    LOG_DEBUG("Factory", "Loading C++ Strategy: " + name);
    return createCppStrategy(name);
}

py::object PlayerStrategyFactory::loadPythonStrategy(const std::string& requestedName){
    auto it = pythonStrategies.find(requestedName);
    if (it != pythonStrategies.end()) {
        LOG_TRACE("Factory", "Returning cached Python strategy: " + requestedName);
        return it->second;
    }

    auto created = createPythonStrategy(requestedName);
    if (created && !created.is_none()) {
        LOG_INFO("Factory", "Successfully loaded and cached Python strategy: " + requestedName);
        pythonStrategies[requestedName] = created;
    }
    return created;
}

std::shared_ptr<PlayerStrategy> PlayerStrategyFactory::createCppStrategy(std::string name){
    return PlayerStrategy::create(name);
}

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

py::object PlayerStrategyFactory::createPythonStrategy(const std::string& requestedName) {
    LOG_DEBUG("Factory", "Starting discovery for Python strategy: " + requestedName);
    PyGILState_STATE gstate = PyGILState_Ensure();

    try {
        fs::path exePath = fs::current_path();
        fs::path rootPath = fs::canonical(exePath / ".." / "src_python/python_strategies").parent_path();

        std::string safePath = rootPath.generic_string();

        LOG_TRACE("Factory", "Python root path set to: " + safePath);

        std::string cmd = "import sys\nsys.path.append('" + safePath + "')";
        PyRun_SimpleString(cmd.c_str());

        for (const auto& strategyType : pythonStrategyTypes) {
            fs::path strategyPath = fs::canonical(exePath / ".." / "src_python/python_strategies" / strategyType.subPath);
            LOG_TRACE("Factory", "Scanning sub-path: " + strategyType.subPath);

            std::string baseModule = "python_strategies." + strategyType.subPath + "." + strategyType.fileName;
            std::replace(baseModule.begin(), baseModule.end(), '/', '.');

            py::object baseModuleObj;
            try {
                baseModuleObj = py::module_::import(baseModule.c_str());
            } catch (const py::error_already_set& e) {
                LOG_ERROR("Factory", "Failed to import base module: " + baseModule);
                PyErr_Print();
                continue;
            }

            py::object baseClass;
            try {
                baseClass = baseModuleObj.attr(strategyType.baseClassName.c_str());
            } catch (const py::error_already_set& e) {
                LOG_ERROR("Factory", "Failed to access base class: " + strategyType.baseClassName);
                PyErr_Print();
                continue;
            }

            for (const auto& entry : fs::directory_iterator(strategyPath)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".py") continue;

                std::string moduleName = entry.path().stem().string();
                if (moduleName == strategyType.fileName) continue;

                std::string fullModule = "python_strategies." + strategyType.subPath + "." + moduleName;
                std::replace(fullModule.begin(), fullModule.end(), '/', '.');

                LOG_TRACE("Factory", "Attempting to import module: " + fullModule);

                py::object moduleObj;
                try {
                    moduleObj = py::module_::import(fullModule.c_str());
                } catch (const py::error_already_set& e) {
                    LOG_WARN("Factory", "Module couldn't be imported: " + fullModule);
                    PyErr_Print();
                    continue;
                }

                py::module_ builtins = py::module_::import("builtins");
                py::object dirFunc = builtins.attr("dir");
                py::list attributes = dirFunc(moduleObj);

                for (auto attrName : attributes) {
                    py::object attr = moduleObj.attr(attrName.cast<std::string>().c_str());
                    if (py::isinstance<py::type>(attr)) {
                        if (PyObject_IsSubclass(attr.ptr(), baseClass.ptr()) == 1 &&
                            attr.ptr() != baseClass.ptr()) {

                            py::object instance;
                            try {
                                instance = attr();
                            } catch (const py::error_already_set& e) {
                                LOG_ERROR("Factory", "Instantiation failed for class in module: " + fullModule);
                                PyErr_Print();
                                continue;
                            }

                            try {
                                py::object result = instance.attr("get_name")();
                                std::string strategyName = result.cast<std::string>();
                                std::string formattedName = "[Py] " + strategyName;

                                if (formattedName == requestedName) {
                                    LOG_INFO("Factory", "Match found! Strategy: " + strategyName + " from " + fullModule);
                                    PyGILState_Release(gstate);
                                    return instance;
                                }
                            } catch (const py::error_already_set& e) {
                                LOG_TRACE("Factory", "Attribute 'get_name' not found or failed in " + fullModule);
                                PyErr_Print();
                            }
                        }
                    }
                }
            }
        }

    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Factory", "Filesystem error: " + std::string(e.what()));
    } catch (const py::error_already_set& e) {
        LOG_ERROR("Factory", "Python error: " + std::string(e.what()));
        PyErr_Print();
    } catch (...) {
        LOG_ERROR("Factory", "Unknown error while loading Python strategies");
    }

    LOG_WARN("Factory", "No matching Python strategy found for: " + requestedName);
    PyGILState_Release(gstate);
    return py::none();
}