#include "TeamStrategyFactory.h"
#include <filesystem>
#include "Logger.h"

namespace fs = std::filesystem;

std::unordered_map<std::string, std::shared_ptr<TeamStrategy>> TeamStrategyFactory::cppStrategies;
std::unordered_map<std::string, py::object> TeamStrategyFactory::pythonStrategies;

std::shared_ptr<TeamStrategy> TeamStrategyFactory::loadCppStrategy(std::string name){
    LOG_DEBUG("Factory", "Loading C++ Team Strategy: " + name);

    auto it = cppStrategies.find(name);
    if (it != cppStrategies.end()) {
        LOG_TRACE("Factory", "C++ Team Strategy cache hit: " + name);
        return it->second;
    }

    auto created = createCppStrategy(name);
    if (created) {
        cppStrategies[name] = created;
        LOG_INFO("Factory", "C++ Team Strategy successfully created and cached: " + name);
    } else {
        LOG_WARN("Factory", "Failed to create C++ Team Strategy: " + name);
    }

    return created;
}

py::object TeamStrategyFactory::loadPythonStrategy(const std::string& name) {
    LOG_DEBUG("Factory", "Loading Python Team Strategy: " + name);

    auto it = pythonStrategies.find(name);
    if (it != pythonStrategies.end()) {
        LOG_TRACE("Factory", "Python Team Strategy cache hit: " + name);
        return it->second;
    }

    py::object created = createPythonStrategy(name);
    if (!created.is_none()) {
        pythonStrategies[name] = created;
        LOG_INFO("Factory", "Python Team Strategy successfully created and cached: " + name);
    } else {
        LOG_WARN("Factory", "Failed to create Python Team Strategy: " + name);
    }

    return created;
}

std::shared_ptr<TeamStrategy> TeamStrategyFactory::createCppStrategy(std::string name){
    return TeamStrategy::create(name);
}

py::object TeamStrategyFactory::createPythonStrategy(const std::string& requestedName) {
    std::string subPath = "team";
    std::string fileName = "base_strategy_team";
    std::string baseClassName = "BaseStrategyTeam";

    py::object instanceToReturn = py::none();

    {
        py::gil_scoped_acquire acquire;

        try {
            fs::path exePath = fs::current_path();
            fs::path strategyPath = fs::canonical(exePath / ".." / "src_python/python_strategies" / subPath);
            fs::path rootPath = strategyPath.parent_path().parent_path();

            LOG_TRACE("Factory", "Scanning Python Team Strategies in: " + strategyPath.generic_string());

            std::string safeRootPath = rootPath.generic_string();

            std::string cmd = "import sys\nsys.path.append('" + safeRootPath + "')";
            PyRun_SimpleString(cmd.c_str());

            std::string baseModule = "python_strategies." + subPath + "." + fileName;
            std::replace(baseModule.begin(), baseModule.end(), '/', '.');

            py::module_ baseMod = py::module_::import(baseModule.c_str());
            py::object baseClass = baseMod.attr(baseClassName.c_str());

            for (const auto& entry : fs::directory_iterator(strategyPath)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".py") continue;

                std::string moduleName = entry.path().stem().string();
                if (moduleName == fileName) continue;

                std::string fullModule = "python_strategies." + subPath + "." + moduleName;
                std::replace(fullModule.begin(), fullModule.end(), '/', '.');

                LOG_TRACE("Factory", "Trying to import module: " + fullModule);

                try {
                    py::module_ mod = py::module_::import(fullModule.c_str());
                    py::dict modDict = mod.attr("__dict__");

                    for (auto& item : modDict) {
                        if (py::isinstance<py::type>(item.second)) {
                            py::object cls = py::reinterpret_borrow<py::object>(item.second);

                            if (PyObject_IsSubclass(cls.ptr(), baseClass.ptr()) == 1 && !cls.is(baseClass)) {
                                
                                py::object instance;
                                try {
                                    instance = cls();
                                } catch (const py::error_already_set&) {
                                    LOG_ERROR("Factory", "Failed to instantiate class in: " + fullModule);
                                    continue;
                                }

                                std::string strategyName;
                                try {
                                    strategyName = instance.attr("get_name")().cast<std::string>();
                                } catch (const py::error_already_set&) {
                                    LOG_WARN("Factory", "Class in " + fullModule + " has no get_name() method.");
                                    continue;
                                }

                                LOG_TRACE("Factory", "Checking strategy: " + strategyName);

                                if ("[Py] " + strategyName == requestedName) {
                                    LOG_INFO("Factory", "Match found for Team Strategy: " + strategyName);
                                    instanceToReturn = instance;
                                    break;
                                }
                            }
                        }
                    }
                } catch (const py::error_already_set& e) {
                    LOG_ERROR("Factory", "Import of " + moduleName + " failed: " + std::string(e.what()));
                    continue;
                }

                if (!instanceToReturn.is_none()) break;
            }
        } catch (const fs::filesystem_error& e) {
            LOG_ERROR("Factory", "Filesystem error: " + std::string(e.what()));
        } catch (const py::error_already_set& e) {
            LOG_ERROR("Factory", "Python error: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("Factory", "Unknown error while loading Python team strategies");
        }
    }

    if (instanceToReturn.is_none()) {
        LOG_WARN("Factory", "No matching Python team strategy found for: " + requestedName);
    }

    return instanceToReturn;
}