#include "screen/SceneManager.h"

void SceneManager::setScene(std::unique_ptr<Scene> newScene) {
    if (currentScene) {
        currentScene->onExit();
        currentScene.reset();
    }

    currentScene = std::move(newScene);
}

void SceneManager::handleEvent(const sf::Event& event) {
    if (currentScene) currentScene->handleEvent(event);
}
void SceneManager::update(sf::Time deltaTime) {
    if (currentScene) currentScene->update(deltaTime);
}

void SceneManager::render(sf::RenderWindow& window) {
    if (currentScene) currentScene->render(window);
}

void SceneManager::cleanup() {
    if (currentScene) {
        currentScene->onExit();
        currentScene.reset();
    }
}