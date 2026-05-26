#pragma once

#include "screen/Scene.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <vector>
#include <string>
#include "screen/SceneManager.h"
#include "StrategyConfigurations.h"
#include <tuple>
#include <functional>
#include <optional>
#include "GameHandler.h"
#include <thread>


class SingleGameScene : public Scene {
    public:
        SingleGameScene(SceneManager& manager, sf::RenderWindow& window, std::vector<TeamConfig> teams);
    
        void handleEvent(const sf::Event& event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::RenderWindow& window) override;
    
    private:
        enum class SingleGameState {
            WaitingToStart,
            Running,
            Finished
        };

        bool drawSingleGameWindow(sf::RenderWindow& window);
        void drawBackground(sf::RenderWindow& window);
        std::vector<TeamConfig> teams;
        const sf::Texture& loadTrophyTexture(std::string filename);
        void drawSingleGameWindowOverlay(sf::RenderWindow& window);

        bool gameStarted = false;
        bool gameFinished = false;
        std::optional<size_t> winningTeamIdx;
        sf::Texture trophyTexture;

        SingleGameState currGameState = SingleGameState::WaitingToStart;

        std::unique_ptr<GameHandler> gameHandler;
};