#pragma once

#include <string>
#include <vector>

class PythonStrategyLoader {
    public:
        enum class StrategyType{
            TEAM_STRATEGY,
            PLAYER_STRATEGY_TEAM_CONTEXT,
            PLAYER_STRATEGY_AUTONOMOUS
        };

        static std::vector<std::string> loadStrategies(StrategyType type);
    
    private:
        static std::vector<std::string> loadPythonStrategies(const std::string& subPath, const std::string& fileName, const std::string& baseClassName);
};