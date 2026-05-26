#include "screen/MainMenuScene.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <screen/TeamSelectionScene.h>
#include "GameMode.h"
#include "Logger.h"

MainMenuScene::MainMenuScene(SceneManager& manager, sf::RenderWindow& win)
    : Scene(manager, win, "back.png") {
    
    LOG_INFO("UI", "MainMenuScene created and initialized");
}

void MainMenuScene::handleEvent(const sf::Event& event) {

}

void MainMenuScene::update(sf::Time deltaTime) {
    ImGui::SFML::Update(window, deltaTime);
}

void MainMenuScene::render(sf::RenderWindow& window) {
    window.draw(backgroundSprite);
    renderUI();
}

void MainMenuScene::renderUI() {
    sf::Vector2u winSize = window.getSize();
    ImVec2 windowSize = ImVec2(650.f, 300.f);
    ImVec2 centerPos = ImVec2(
        (winSize.x / 2.f) - (windowSize.x / 2.f),
        (winSize.y / 2.f) - (windowSize.y / 2.f)
    );

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.17f, 0.39f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.49f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.13f, 0.31f, 0.13f, 1.0f));
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       ImVec4(0.14f, 0.28f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,ImVec4(0.19f, 0.36f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.24f, 0.45f, 0.24f, 1.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Modus Auswahl", nullptr, flags);

    ImGui::Text("Choose a mode:");

    ImGui::Spacing();
    float buttonWidth = 200.f;
    float buttonHeight = 50.f;

    float buttonsTotalWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX((windowSize.x - buttonsTotalWidth) * 0.5f);

    bool tournamentSelected = (activeMode == 1);
    if (tournamentSelected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

    if (ImGui::Button("Tournament", ImVec2(buttonWidth, buttonHeight))) {
        activeMode = tournamentSelected ? 0 : 1;
        LOG_INFO("Input", "Mode toggled: Tournament is now " + std::string(activeMode == 1 ? "Active" : "Inactive"));
    }

    if (tournamentSelected)
        ImGui::PopStyleColor();

    ImGui::SameLine();

    bool singleGameSelected = (activeMode == 2);
    if (singleGameSelected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

    if (ImGui::Button("Single Game", ImVec2(buttonWidth, buttonHeight))) {
        activeMode = singleGameSelected ? 0 : 2;
        LOG_INFO("Input", "Mode toggled: Single Game is now " + std::string(activeMode == 2 ? "Active" : "Inactive"));
    }

    if (singleGameSelected)
        ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (activeMode == 1) {
        ImGui::Text("Amount of Teams in Tournament:");
        ImGui::Spacing();

        const char* teamOptions[] = { "4", "8", "16" };

        if (ImGui::BeginCombo("##team_count", teamOptions[teamIndex])) {
            for (int n = 0; n < 3; ++n) {
                bool isSelected = (teamIndex == n);

                if (ImGui::Selectable(teamOptions[n], isSelected)) {
                    teamIndex = n;
                    selectedTeamCount = std::stoi(teamOptions[n]);

                    LOG_INFO("Input", "Tournament team count set to: " + std::to_string(selectedTeamCount));
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

    } else if (activeMode == 2) {
        ImGui::Text("Amount of Teams in Single Game:");
        ImGui::Spacing();

        int oldValue = singleGameTeams;
        ImGui::InputInt("##single_game_teams", &singleGameTeams);

        if (singleGameTeams < 2) singleGameTeams = 2;
        if (singleGameTeams > 4) singleGameTeams = 4;

        if (oldValue != singleGameTeams) {
            LOG_INFO("Input", "Single Game team count changed: " + std::to_string(oldValue) + " -> " + std::to_string(singleGameTeams));
        }
    }

    if (activeMode != 0) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);

        if (ImGui::Button("Start", ImVec2(buttonWidth, buttonHeight))) {
            int count = (activeMode == 1) ? selectedTeamCount : singleGameTeams;
            GameMode gameMode = (activeMode == 1) ? GameMode::Tournament : GameMode::SingleGame;

            LOG_INFO("UI", "Transitioning to TeamSelectionScene | Mode: " +
                         std::string(activeMode == 1 ? "Tournament" : "SingleGame") +
                         " | Teams: " + std::to_string(count));

            sceneManager.setScene(
                std::make_unique<TeamSelectionScene>(sceneManager, window, count, gameMode)
            );
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(7);
}