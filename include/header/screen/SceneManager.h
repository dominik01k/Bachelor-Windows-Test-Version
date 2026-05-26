#pragma once

#include <memory>
#include "Scene.h"

class SceneManager {
public:
    void setScene(std::unique_ptr<Scene> newScene);
    void handleEvent(const sf::Event& event);
    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);
    void cleanup();

private:
    std::unique_ptr<Scene> currentScene;
};