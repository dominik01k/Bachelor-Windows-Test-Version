#pragma once

#include <memory>
#include <string>

#include "strategy/playerStrategy/PlayerStrategy.h"
#include "Python.h"
#include <unordered_map>
#include <pybind11/pybind11.h>
namespace py = pybind11;

class PlayerStrategyFactory {
public:
    static std::shared_ptr<PlayerStrategy> loadCppStrategy(std::string name);
    static py::object loadPythonStrategy(const std::string& requestedName);

private:
    static std::shared_ptr<PlayerStrategy> createCppStrategy(std::string name);
    static py::object createPythonStrategy(const std::string& requestedName);

    static std::unordered_map<std::string, std::shared_ptr<PlayerStrategy>> cppStrategies;
    static std::unordered_map<std::string, py::object> pythonStrategies;

    struct PythonStrategySearchInfo {
        std::string subPath;
        std::string fileName;
        std::string baseClassName;
    };

    static const std::vector<PythonStrategySearchInfo> pythonStrategyTypes;
};