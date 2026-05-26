#include "screen/TeamSelectionScene.h"
#include <imgui-SFML.h>
#include <iostream>
#include "screen/MainMenuScene.h"
#include "strategy/teamStrategy/TeamStrategy.h"
#include "strategy/playerStrategy/PlayerStrategy.h"
#include "PythonStrategyLoader.h"
#include "screen/TournamentScene.h"
#include "screen/SingleGameScene.h"
#include "Logger.h"

TeamSelectionScene::TeamSelectionScene(SceneManager& manager, sf::RenderWindow& win, int teamCount, GameMode gameMode)
    : Scene(manager, win, "back.png"), viewModel(teamCount), gameMode(gameMode) {
    
    LOG_INFO("UI", "TeamSelectionScene created for " + std::to_string(teamCount) + " teams");
    
    viewModel.loadStrategies();
    LOG_DEBUG("Config", "Strategies loaded into ViewModel");
}

void TeamSelectionScene::handleEvent(const sf::Event& event) {
}

void TeamSelectionScene::update(sf::Time deltaTime) {
    ImGui::SFML::Update(window, deltaTime);
}

void TeamSelectionScene::render(sf::RenderWindow& window) {
    drawBackground(window);
    beginTeamSelectionWindow(window);

    renderTeamInputs();
    renderActionButtons();

    endTeamSelectionWindow();
    ImGui::SFML::Render(window);
}

void TeamSelectionScene::drawBackground(sf::RenderWindow& window) {
    window.draw(backgroundSprite);
}

void TeamSelectionScene::renderActionButtons() {
    const float buttonWidth = 200.f;
    const float buttonHeight = 50.f;
    const float buttonsTotalWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonsTotalWidth) * 0.5f);

    if (ImGui::Button("Start", ImVec2(buttonWidth, buttonHeight))) {
        startTournament = true;

        LOG_INFO("Input", "Start clicked in TeamSelection. Mode: " + 
                 std::string(gameMode == GameMode::Tournament ? "Tournament" : "SingleGame"));
        
        auto teamConfigs = viewModel.getTeamConfigurations();
        LOG_DEBUG("Game", "Assembled " + std::to_string(teamConfigs.size()) + " team configurations");
        
        if (gameMode == GameMode::Tournament) {
            sceneManager.setScene(
                std::make_unique<TournamentScene>(sceneManager, window, teamConfigs)
            );
        } else {
            sceneManager.setScene(
                std::make_unique<SingleGameScene>(sceneManager, window, teamConfigs)
            );
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Back to Menu", ImVec2(buttonWidth, buttonHeight))) {
        LOG_INFO("Input", "Returning to Main Menu from TeamSelection");
        sceneManager.setScene(std::make_unique<MainMenuScene>(sceneManager, window));
    }

    ImGui::Spacing();
}

void TeamSelectionScene::endTeamSelectionWindow() {
    ImGui::End();
    ImGui::PopStyleColor(7);
}

void TeamSelectionScene::renderTeamInputs() {
    for (int i = 0; i < viewModel.getTeamCount(); ++i) {
        renderTeamElement(i);
        ImGui::Separator();
    }
    ImGui::Spacing();
}

void TeamSelectionScene::beginTeamSelectionWindow(sf::RenderWindow& window) {
    const sf::Vector2u winSize = window.getSize();
    const ImVec2 windowSize(650.f, 300.f);
    const ImVec2 centerPos(
        (winSize.x - windowSize.x) * 0.5f,
        (winSize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg,        ImVec4(0.f, 0.f, 0.f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Button,          ImVec4(0.17f, 0.39f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   ImVec4(0.22f, 0.49f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,    ImVec4(0.13f, 0.31f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,         ImVec4(0.14f, 0.28f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,  ImVec4(0.19f, 0.36f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,   ImVec4(0.24f, 0.45f, 0.24f, 1.0f));

    ImGui::Begin("Team Selection", nullptr, flags);
    ImGui::Text("Please choose the name and strategies for the %d teams:", viewModel.getTeamCount());
}

void TeamSelectionScene::renderTeamElement(int index) {
    auto& teamNames = viewModel.getTeamNames();
    const auto& teamStrategyOptions = viewModel.getTeamStrategyOptions();
    auto& selectedStrategies = viewModel.getSelectedTeamStrategies();
    auto& playerStrategyIndices = viewModel.getPlayerStrategyIndices();

    bool noTeamStrategy = (teamStrategyOptions[selectedStrategies[index]] == "No specific Team Strategy");
    const auto& playerStrats = viewModel.getPlayerStrategies(noTeamStrategy);

    std::vector<const char*> playerStratsCStr;
    for (const auto& s : playerStrats) playerStratsCStr.push_back(s.c_str());
    playerStratsCStr.push_back("Play yourself");
    int playYourselfIndex = static_cast<int>(playerStratsCStr.size()) - 1;

    char label[64];
    snprintf(label, sizeof(label), "Team %d Name", index + 1);
    char nameBuffer[64];
    strncpy(nameBuffer, teamNames[index].c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText(label, nameBuffer, sizeof(nameBuffer))) {
        teamNames[index] = nameBuffer;
    }

    snprintf(label, sizeof(label), "Team %d Strategy", index + 1);
    std::vector<const char*> teamStratsCStr;
    for (const auto& s : teamStrategyOptions) teamStratsCStr.push_back(s.c_str());

    if (ImGui::Combo(label, &selectedStrategies[index], teamStratsCStr.data(), (int)teamStratsCStr.size())) {
        LOG_TRACE("Input", "Team " + std::to_string(index) + " strategy changed to: " + teamStrategyOptions[selectedStrategies[index]]);
        
        bool switchedToAutonomous = (teamStrategyOptions[selectedStrategies[index]] == "No specific Team Strategy");
        if (!switchedToAutonomous) {
            for (int i = 0; i < 3; ++i) {
                int idx = playerStrategyIndices[index][i];
                if (idx >= (int)viewModel.getPlayerStrategies(true).size()) {
                    playerStrategyIndices[index][i] = 0;
                    LOG_TRACE("Config", "Reset player " + std::to_string(i) + " strategy for team " + std::to_string(index));
                }
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        snprintf(label, sizeof(label), "Player %d-%d Strategy", index + 1, i + 1);

        int& sel = playerStrategyIndices[index][i];
        int previous = sel;
        if (ImGui::Combo(label, &sel, playerStratsCStr.data(), (int)playerStratsCStr.size())) {
            auto controlled = viewModel.getControlledPlayer();
            if (sel == playYourselfIndex) {
                if (controlled.first != -1) {
                    sel = previous;
                    LOG_WARN("UI", "User tried to select second manual player (already assigned to team " + std::to_string(controlled.first) + ")");
                } else {
                    viewModel.setControlledPlayer(index, i);
                    LOG_INFO("Input", "Player " + std::to_string(index) + "-" + std::to_string(i) + " set to manual control");
                }
            } else {
                if (controlled == std::make_pair(index, i)) {
                    viewModel.clearControlledPlayer();
                    LOG_INFO("Input", "Manual control removed from player " + std::to_string(index) + "-" + std::to_string(i));
                }
                LOG_TRACE("Input", "Player " + std::to_string(index) + "-" + std::to_string(i) + " set to strategy: " + playerStratsCStr[sel]);
            }
        }
    }
}