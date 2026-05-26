#pragma once

#include "screen/Scene.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <vector>
#include <string>
#include "screen/SceneManager.h"
#include "StrategyConfigurations.h"
#include "screen/TournamentViewModel.h"
#include <tuple>
#include <functional>

namespace std {
    template<>
    struct hash<std::tuple<int, int, int>> {
        std::size_t operator()(const std::tuple<int, int, int>& t) const noexcept {
            std::size_t h1 = std::hash<int>()(std::get<0>(t));
            std::size_t h2 = std::hash<int>()(std::get<1>(t));
            std::size_t h3 = std::hash<int>()(std::get<2>(t));
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};

class TournamentScene : public Scene {
    public:
        TournamentScene(SceneManager& manager, sf::RenderWindow& window, std::vector<TeamConfig> teams);
    
        void handleEvent(const sf::Event& event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::RenderWindow& window) override;
    
    private:
        TournamentViewModel viewModel;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
        
        float scale = 2.5f / std::sqrt(static_cast<float>(viewModel.getTeams().size()));

        const float boxWidth   = 140.f * scale;
        const float boxHeight  = 30.f  * scale;
        const float xSpacing   = 180.f * scale;
        const float ySpacing   = 20.f  * scale;

        const float ySpacingBase = 20.f;
        const float baseX      = 50.f  * scale;
        const float baseY      = 30.f  * scale;


        void drawTournamentWindow(sf::RenderWindow& window);
        void drawTournamentWindowOverlay(sf::RenderWindow& window);
        void drawBackground(sf::RenderWindow& window);


        void setupWindowPositionAndStyle(sf::RenderWindow& window);
        const sf::Texture& loadTrophyTexture(std::string filename);

        sf::Texture trophyTexture;

        SingleGameState gameState;

};
