#pragma once

#include <string>

struct Config {
    bool mlMode = false;
    bool debugMode = false;
    bool userControlledMlMode = false;

    int fileLogLevel = 1;
    int terminalLogLevel = 2;

    int userClientWidth = 1200;
    int userClientHeight = 800;

    int gameWindowWidth = 800;
    int gameWindowHeight = 600;

    int statisticsWindowWidth = 350;
    int statisticsWindowHeight = 500;

    int gameWidth = 800;
    int gameHeight = 600;
    
    int frameLimit = 60;

    std::string logPath = "logs/";
    std::string mlDataPath = "ml_data/";

    void loadFromArgs(int argc, char* argv[]);
};

extern Config g_config;

