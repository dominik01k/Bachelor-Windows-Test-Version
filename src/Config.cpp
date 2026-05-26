#include "Config.h"
#include <string>
#include "Logger.h"

Config g_config;

void Config::loadFromArgs(int argc, char* argv[]) {
    LOG_INFO("Setup", "Starting to parse command line arguments (Count: " + std::to_string(argc - 1) + ")");

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--ml") {
            mlMode = true;
            LOG_DEBUG("Setup", "Argument detected: ML Mode enabled");
        } else if (arg == "--debug") {
            debugMode = true;
            LOG_DEBUG("Setup", "Argument detected: Debug Mode enabled");
        } else if (arg == "--userMl"){
            userControlledMlMode = true;
            LOG_DEBUG("Setup", "Argument detected: User Controlled ML Mode enabled");
        } else if (arg.find("--width=") == 0) {
            try {
                userClientWidth = std::stoi(arg.substr(8));
                LOG_DEBUG("Setup", "Custom width set to: " + std::to_string(userClientWidth));
            } catch (const std::exception&) {
                LOG_ERROR("Setup", "Invalid width argument: " + arg);
            }
        } else if (arg.find("--height=") == 0) {
            try {
                userClientHeight = std::stoi(arg.substr(9));
                LOG_DEBUG("Setup", "Custom height set to: " + std::to_string(userClientHeight));
            } catch (const std::exception&) {
                LOG_ERROR("Setup", "Invalid height argument: " + arg);
            }
        } else {
            LOG_WARN("Setup", "Unknown command line argument ignored: " + arg);
        }
    }

    LOG_TRACE("Setup", "Final Configuration: Width=" + std::to_string(userClientWidth) + 
              ", Height=" + std::to_string(userClientHeight) + 
              ", ML=" + (mlMode ? "ON" : "OFF") + 
              ", Debug=" + (debugMode ? "ON" : "OFF"));
}