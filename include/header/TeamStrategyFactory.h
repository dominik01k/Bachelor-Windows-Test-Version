#pragma once

#include <memory>
#include <string>
#include "strategy/teamStrategy/TeamStrategy.h"
#include "Python.h"
#include <unordered_map>
#include <pybind11/pybind11.h>
namespace py = pybind11;



class TeamStrategyFactory {
public:
    static std::shared_ptr<TeamStrategy> loadCppStrategy(std::string name);
    static py::object loadPythonStrategy(const std::string& name);


private:
    static std::shared_ptr<TeamStrategy> createCppStrategy(std::string name);
    static py::object createPythonStrategy(const std::string& requestedName);

    static std::unordered_map<std::string, std::shared_ptr<TeamStrategy>> cppStrategies;
    static std::unordered_map<std::string, py::object> pythonStrategies;

};