#pragma once

#include "screen/Scene.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <vector>
#include <string>
#include "screen/SceneManager.h"
#include "screen/TeamSelectionViewModel.h"
#include "GameMode.h"

class TeamSelectionScene : public Scene {
public:
    TeamSelectionScene(SceneManager& manager, sf::RenderWindow& win, int teamCount, GameMode gameMode);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
   TeamSelectionViewModel viewModel;
    bool startTournament;
    GameMode gameMode;

    void renderTeamElement(int index);
    void endTeamSelectionWindow();
    void renderActionButtons();
    void drawBackground(sf::RenderWindow& window);
    void beginTeamSelectionWindow(sf::RenderWindow& window);
    void renderTeamInputs();
};
