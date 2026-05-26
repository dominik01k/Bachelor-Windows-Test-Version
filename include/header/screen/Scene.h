#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

class SceneManager;

class Scene {
    protected:
        sf::Texture backgroundTexture;
        sf::Sprite backgroundSprite;

        SceneManager& sceneManager;
        sf::RenderWindow& window;

        Scene(SceneManager& manager, sf::RenderWindow& win, std::string filename) : window(win), sceneManager(manager), backgroundSprite(loadBackground(filename)){
            scaleBackgroundToWindow();
        }

        const sf::Texture& loadBackground(std::string filename) {
            if (!backgroundTexture.loadFromFile("../assets/images/" + filename)) {
                std::cerr <<"Coulnd't load background texture" << std::endl;
            }
            return backgroundTexture;
        }

        void scaleBackgroundToWindow() {
            sf::Vector2u windowSize = window.getSize();
            sf::Vector2u textureSize = backgroundTexture.getSize();
            backgroundSprite.setScale(sf::Vector2f(
                float(windowSize.x) / textureSize.x,
                float(windowSize.y) / textureSize.y));
        }
    public:
        virtual ~Scene() = default;

        virtual void handleEvent(const sf::Event& event) = 0;
        virtual void update(sf::Time deltaTime) = 0;
        virtual void render(sf::RenderWindow& window) = 0;

        virtual void onExit() {

        }
};

    