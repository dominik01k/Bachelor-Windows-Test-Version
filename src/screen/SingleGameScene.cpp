#include "screen/SingleGameScene.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "screen/MainMenuScene.h"
#include "GameHandler.h"
#include "WindowUtils.h"
#include "Logger.h"

#ifdef ERROR
#undef ERROR
#endif

#ifdef WARNING
#undef WARNING
#endif

SingleGameScene::SingleGameScene(SceneManager& manager, sf::RenderWindow& win, std::vector<TeamConfig> teams)
    : Scene(manager, win, "back.png"), teams(teams) {
        loadTrophyTexture("trophy.png");
        LOG_INFO("UI", "SingleGameScene created with " + std::to_string(teams.size()) + " teams");
    }


void SingleGameScene::handleEvent(const sf::Event& event) {

}

void SingleGameScene::update(sf::Time deltaTime) {
    if(gameHandler){
        gameHandler->runGameLoop(deltaTime.asSeconds());
        GameResult result = gameHandler->getGameResult();
        winningTeamIdx = result.winningTeamIdx;
        
        if(winningTeamIdx || result.status == GameResultStatus::ABORTED){
            LOG_INFO("Game", "Game session finished");
            currGameState = SingleGameState::Finished;
            gameHandler.reset();
            WindowUtils::restore(window);

            if (winningTeamIdx) {
                LOG_INFO("Game", "Winner determined: Team Index " + std::to_string(*winningTeamIdx));
            } else {
                LOG_WARN("Game", "Game loop stopped: Status ABORTED");
            }
        }
    }
    ImGui::SFML::Update(window, deltaTime);
}

const sf::Texture& SingleGameScene::loadTrophyTexture(std::string filename) {
    if (!trophyTexture.loadFromFile("../assets/images/" + filename)) {
        LOG_ERROR("UI", "Failed to load trophy texture from path: ../assets/images/" + filename);
    } else {
        LOG_DEBUG("UI", "Trophy texture successfully loaded: " + filename);
    }
    return trophyTexture;
}

void SingleGameScene::drawBackground(sf::RenderWindow& window) {
    window.draw(backgroundSprite);
}

void SingleGameScene::render(sf::RenderWindow& window) {
    window.clear();
    drawBackground(window);

    if (currGameState == SingleGameState::WaitingToStart || currGameState == SingleGameState::Finished) {
        if (drawSingleGameWindow(window)) {
            ImGui::End();
            ImGui::PopStyleColor(7);
        }
    } else if (currGameState == SingleGameState::Running) {
        drawSingleGameWindowOverlay(window);
    }

    ImGui::SFML::Render(window);
}


void SingleGameScene::drawSingleGameWindowOverlay(sf::RenderWindow& window) {
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.3f));

    if (ImGui::Begin("OverlayQuitWindow", nullptr, flags)) {

        float contentWidth = ImGui::GetContentRegionAvail().x;

        const char* text = "Game running...";
        float textWidth = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX((contentWidth - textWidth) * 0.5f);
        ImGui::Text("%s", text);

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.17f, 0.39f, 0.17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.49f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.13f, 0.31f, 0.13f, 1.0f));

        float buttonWidth = 200.f;
        float buttonX = (contentWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosX(buttonX);

        if (ImGui::Button("Quit Game", ImVec2(buttonWidth, 50))) {
            LOG_WARN("Input", "User clicked 'Quit Game' - Aborting manually");

            currGameState = SingleGameState::Finished;
            gameHandler.reset();
            WindowUtils::restore(window);
        }

        ImGui::PopStyleColor(3); 
    }

    ImGui::End();
    ImGui::PopStyleColor(); 
}



bool SingleGameScene::drawSingleGameWindow(sf::RenderWindow& window) {
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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.17f, 0.39f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.49f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.13f, 0.31f, 0.13f, 1.0f));
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       ImVec4(0.14f, 0.28f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,ImVec4(0.19f, 0.36f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.24f, 0.45f, 0.24f, 1.0f));

    if (!ImGui::Begin("Single Game", nullptr, flags)) {
        return false;
    }

    ImGui::Text("Single Game");
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);   

    for (int i = 0; i < teams.size(); ++i) {
        const auto& team = teams[i];
        ImGui::Text("Team: %s", team.name.c_str());
        ImGui::Text("Teamstrategie: %s", team.teamStrategy.c_str());
        ImGui::Text("Spieler:");
        for (const auto& player : team.players) {
            ImGui::BulletText("%s", player.strategyName.c_str());
        }
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (currGameState == SingleGameState::WaitingToStart) {
        float buttonWidth = 200.f;
        ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
        if (ImGui::Button("Start Game", ImVec2(200, 50))) {
            LOG_INFO("Input", "Start Game button clicked");

            currGameState = SingleGameState::Running;
            gameHandler = std::make_unique<GameHandler>();
            gameHandler->initiateGame(teams);

            LOG_DEBUG("Game", "GameHandler initialized and game initiated");

            WindowUtils::minimize(window);
        }
    } else if (currGameState == SingleGameState::Finished) {
        ImGui::Spacing();
        float contentWidth = ImGui::GetContentRegionAvail().x;
        if(winningTeamIdx){

            float totalBlockWidth = 200.f;
        
            ImGui::SetCursorPosX((contentWidth - totalBlockWidth) * 0.5f);

            ImGui::BeginChild("WinnerFrame", ImVec2(200, 200), true, ImGuiWindowFlags_None);
        
            {
                float frameWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Spacing();

                float imageWidth = 128.f;
                float imageX = (frameWidth - imageWidth) * 0.5f;
                ImGui::SetCursorPosX(imageX);
                ImGui::Image(trophyTexture, sf::Vector2f(imageWidth, imageWidth));
        
                ImGui::Spacing();

                std::string winnerText = "Winner: " + teams[*winningTeamIdx].name;
                float textWidth = ImGui::CalcTextSize(winnerText.c_str()).x;
                float textX = (frameWidth - textWidth) * 0.5f;
                ImGui::SetCursorPosX(textX);
                ImGui::Text("%s", winnerText.c_str());
            }
        
            ImGui::EndChild();

        }
        else{
            ImGui::Text("The Game got aborted");
        }
    
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 200.f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalButtonWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((contentWidth - totalButtonWidth) * 0.5f);
    
        if (ImGui::Button("Back to Menu", ImVec2(buttonWidth, 50))) {
            LOG_INFO("Input", "Returning to Main Menu from Finished State");

            sceneManager.setScene(
                std::make_unique<MainMenuScene>(sceneManager, window)
            );
        }
    
        ImGui::SameLine();
    
        if (ImGui::Button("Play Again", ImVec2(buttonWidth, 50))) {
            LOG_INFO("Input", "Restarting Single Game session");
            
            sceneManager.setScene(
                std::make_unique<SingleGameScene>(sceneManager, window, teams)
            );
        }

        ImGui::Spacing();
    }  
    
    return true;
}