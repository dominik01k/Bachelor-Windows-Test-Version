#include "Config.h"
#include <iostream>
#include <string>
#include "Game.h"
#include "PythonEnvironment.h"
#include "MLDataCollector.h"
#include "Logger.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "--- PROGRAMM STARTET JETZT ---" << std::endl;
    
    g_config.loadFromArgs(argc, argv);

    Logger::init();

    PythonEnvironment::initialize();
    
    {
        Game game;
        game.run();
    }

    PythonEnvironment::finalize();
    return 0;
}