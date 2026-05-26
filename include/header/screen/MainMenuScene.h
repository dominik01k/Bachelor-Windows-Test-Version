#pragma once

#include <imgui.h>
#include<imgui-SFML.h>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Matchup.h"
#include "screen/Scene.h"
#include "screen/SceneManager.h"

class MainMenuScene : public Scene{
    public:
        MainMenuScene(SceneManager& manager, sf::RenderWindow& window);

        void handleEvent(const sf::Event& event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::RenderWindow& window) override;
    
    private:

        int activeMode = 0; 
        int teamIndex = 0;
        int selectedTeamCount = 4;
        int singleGameTeams = 2;
        float widthPercent = 0.25f;  
        float heightPercent = 0.3f;

        void renderUI();
};