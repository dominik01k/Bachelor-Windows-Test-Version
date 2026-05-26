#include "Game.h"
#include "screen/MainMenuScene.h"
#include <iostream>
#include "Config.h"
#include "Logger.h"

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(g_config.userClientWidth, g_config.userClientHeight)),
             "Tournament Bracket",
             sf::Style::Titlebar | sf::Style::Close)
{
    LOG_INFO("Game", "Game constructor called");

    if (!ImGui::SFML::Init(window)) {
        LOG_ERROR("Setup", "Failed to initialize ImGui-SFML");
    } else {
        LOG_INFO("Setup", "ImGui-SFML initialized successfully");
    }

    LOG_INFO("UI",
        "Window created: " +
        std::to_string(g_config.userClientWidth) + "x" +
        std::to_string(g_config.userClientHeight)
    );

    ImGuiStyle& style = ImGui::GetStyle();
    style.ItemSpacing.y = 10.0f;

    LOG_DEBUG("UI", "ImGui style configured");

    sceneManager.setScene(std::make_unique<MainMenuScene>(sceneManager, window));
    LOG_INFO("UI", "MainMenuScene set as initial scene");
}

void Game::run() {
    LOG_INFO("Game", "Game loop started");

    sf::Clock clock;

    try {
        while (running && window.isOpen()) {

            while (auto eventOpt = window.pollEvent()) {
                sf::Event event = *eventOpt;

                if (event.is<sf::Event::Closed>()) {
                    LOG_INFO("Game", "Window close event received");
                    running = false;
                }

                ImGui::SFML::ProcessEvent(window, event);
                sceneManager.handleEvent(event);
            }

            if (!running) {
                LOG_INFO("Game", "Exiting game loop");
                break;
            }

            sf::Time deltaTime = clock.restart();

            LOG_TRACE("Loop", 
                "DeltaTime=" + std::to_string(deltaTime.asSeconds())
            );

            sceneManager.update(deltaTime);

            window.clear();
            sceneManager.render(window);

            ImGui::SFML::Render(window);
            window.display();
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Game", std::string("Exception in Game loop: ") + e.what());
    }
    catch (...) {
        LOG_ERROR("Game", "Unknown exception in Game loop");
    }

    cleanup();
}

void Game::cleanup() {
    LOG_INFO("Game", "Cleaning up game");

    sceneManager.cleanup();
    LOG_DEBUG("UI", "SceneManager cleaned up");

    ImGui::SFML::Shutdown();
    LOG_INFO("Setup", "ImGui-SFML shutdown");

    if (window.isOpen()) {
        window.close();
        LOG_INFO("UI", "Window closed");
    }

    LOG_INFO("Game", "Game cleanup finished");
}