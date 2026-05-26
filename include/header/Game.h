#pragma once

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "screen/SceneManager.h"

class Game {
public:
    Game();
    void run();
    void cleanup();

private:
    sf::RenderWindow window;
    SceneManager sceneManager;
    
    bool running = true;
};
