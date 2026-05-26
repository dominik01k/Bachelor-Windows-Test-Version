#include "screen/TournamentScene.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "screen/MainMenuScene.h"
#include <tuple>
#include "Logger.h"

TournamentScene::TournamentScene(SceneManager& manager, sf::RenderWindow& win, std::vector<TeamConfig> teams)
    : Scene(manager, win, "back.png"), viewModel(std::move(teams)) {
        LOG_INFO("UI", "TournamentScene created with " + std::to_string(teams.size()) + " teams");
        loadTrophyTexture("trophy.png");
    }

const sf::Texture& TournamentScene::loadTrophyTexture(std::string filename) {
    if (!trophyTexture.loadFromFile("../assets/images/" + filename)) {
        LOG_ERROR("UI", "Failed to load trophy texture: " + filename);
    } else {
        LOG_DEBUG("UI", "Trophy texture loaded: " + filename);
    }
    return trophyTexture;
}

void TournamentScene::handleEvent(const sf::Event& event) {

}

void TournamentScene::update(sf::Time deltaTime) {
    if(gameState == SingleGameState::Running){
        gameState = viewModel.updateTournamentViewModel(deltaTime.asSeconds());
    }
    ImGui::SFML::Update(window, deltaTime);
}

void TournamentScene::render(sf::RenderWindow& window) {
    window.clear();
    drawBackground(window);
    setupWindowPositionAndStyle(window);
    if(gameState == SingleGameState::Running){
        drawTournamentWindowOverlay(window);
    }
    else{   
        drawTournamentWindow(window);
    }

    ImGui::End();
    ImGui::PopStyleColor(7);
    ImGui::SFML::Render(window);
}

void TournamentScene::drawBackground(sf::RenderWindow& window) {
    window.draw(backgroundSprite);
}

void TournamentScene::drawTournamentWindowOverlay(sf::RenderWindow& window) {

    if (ImGui::Begin("OverlayQuitWindow", nullptr, windowFlags)) {

        float contentWidth = ImGui::GetContentRegionAvail().x;

        const char* text = "Game running...";
        float textWidth = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX((contentWidth - textWidth) * 0.5f);
        ImGui::Text("%s", text);

        ImGui::Spacing();

        float buttonWidth = 200.f;
        float buttonX = (contentWidth - buttonWidth) * 0.5f;
        ImGui::SetCursorPosX(buttonX);

        if (ImGui::Button("Quit Game", ImVec2(buttonWidth, 50))) {
            LOG_WARN("Input", "Tournament manually aborted by user");

            viewModel.reset();
            gameState = SingleGameState::Aborted;
        }

    }
}

void TournamentScene::drawTournamentWindow(sf::RenderWindow& window) {

    ImGui::Begin("Tunierbaum", nullptr, windowFlags);
    ImGui::Text("Tournament");
    ImGui::Separator();


    const auto& rounds = viewModel.getRounds();
    const auto& teams  = viewModel.getTeams();


    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    int numMatches = static_cast<int>(teams.size()) / 2;
    float totalBracketHeight = baseY + numMatches * (2 * boxHeight + ySpacingBase) + 100.0f;

    float canvasHeight = std::max(totalBracketHeight, 400.0f);
    ImVec2 canvasSize{ ImGui::GetContentRegionAvail().x, canvasHeight };

    ImGui::BeginChild("BracketCanvas", canvasSize, false, ImGuiWindowFlags_HorizontalScrollbar);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();


    std::vector<std::vector<float>> matchTopYs(rounds.size());

    ImVec4 winnerColorVec = ImVec4(0.17f, 0.39f, 0.17f, 1.0f);
    ImU32  winnerBoxColor = ImGui::ColorConvertFloat4ToU32(winnerColorVec); 

    for (int roundIdx = 0; roundIdx < (int)rounds.size(); ++roundIdx) {
        const auto& matches = rounds[roundIdx];
        float x = baseX + roundIdx * xSpacing;

        std::vector<float>& thisRoundTops = matchTopYs[roundIdx];
        thisRoundTops.resize(matches.size());

        

        for (int matchIdx = 0; matchIdx < (int)matches.size(); ++matchIdx) {
            const auto& match = matches[matchIdx];

            float matchTopY;
            if (roundIdx == 0) {
                float ySpacing = (matchIdx == 0) ? ySpacingBase * 3.0f : ySpacingBase;
                matchTopY = baseY + matchIdx * (2 * boxHeight + ySpacing);
            } else {
                float prev0 = matchTopYs[roundIdx - 1][matchIdx * 2 + 0];
                float prev1 = matchTopYs[roundIdx - 1][matchIdx * 2 + 1];
                matchTopY = (prev0 + prev1) * 0.5f;
            }
            thisRoundTops[matchIdx] = matchTopY;

            for (int slot = 0; slot < 2; ++slot) {
                float y = matchTopY + slot * boxHeight;
                std::string label = "TBD";
                if (slot == 0 && match.firstTeamIndex) {
                    label = teams[*match.firstTeamIndex].name;
                } else if (slot == 1 && match.secondTeamIndex) {
                    label = teams[*match.secondTeamIndex].name;
                }

                ImVec2 boxMin{ canvasPos.x + x, canvasPos.y + y };
                ImVec2 boxMax{ boxMin.x + boxWidth, boxMin.y + boxHeight };

                bool isWinner = match.winnerIndex.has_value() &&
                    ((slot == 0 && match.firstTeamIndex && *match.winnerIndex == *match.firstTeamIndex) ||
                    (slot == 1 && match.secondTeamIndex && *match.winnerIndex == *match.secondTeamIndex));

                ImU32 boxColor    = isWinner ? winnerBoxColor : IM_COL32(255,255,255,200);
                ImU32 borderColor = isWinner ? IM_COL32(255,215, 0,255) : IM_COL32(200,200,200,255);

                drawList->AddRectFilled(boxMin, boxMax, boxColor, 6.0f);
                drawList->AddRect(boxMin, boxMax, borderColor, 6.0f, 0, 2.0f);

                ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
                ImVec2 textPos{
                    boxMin.x + (boxWidth  - textSize.x) * 0.5f,
                    boxMin.y + (boxHeight - textSize.y) * 0.5f
                };
                drawList->AddText(textPos, IM_COL32_BLACK, label.c_str());
            }
        }
    }

    for (int roundIdx = 1; roundIdx < (int)rounds.size(); ++roundIdx) {
        float parentX = baseX + (roundIdx - 1) * xSpacing;
        float childX  = baseX + roundIdx       * xSpacing;

        for (int matchIdx = 0; matchIdx < (int)rounds[roundIdx].size(); ++matchIdx) {
            float prevTop0 = matchTopYs[roundIdx - 1][matchIdx * 2 + 0];
            float prevTop1 = matchTopYs[roundIdx - 1][matchIdx * 2 + 1];
            float childTop = matchTopYs[roundIdx][matchIdx];

            ImVec2 start0 {
                canvasPos.x + parentX + boxWidth,
                canvasPos.y + prevTop0 + boxHeight
            };
            ImVec2 start1 {
                canvasPos.x + parentX + boxWidth,
                canvasPos.y + prevTop1 + boxHeight
            };
            ImVec2 end {
                canvasPos.x + childX,
                canvasPos.y + childTop + boxHeight
            };

            drawList->AddLine(start0, end, IM_COL32(200,200,200,255), 2.0f);
            drawList->AddLine(start1, end, IM_COL32(200,200,200,255), 2.0f);
        }
    }

    ImGui::EndChild();

    float contentWidth = ImGui::GetContentRegionAvail().x;

    if(gameState != SingleGameState::Aborted){
        if (!viewModel.isFinished()) {
            ImGui::Separator();
            ImGui::Spacing();
            float buttonWidth = 250.f;
            ImGui::SetCursorPosX((contentWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("Simulate next Game", ImVec2(buttonWidth, 40))) {
                LOG_INFO("Input", "Simulating next match");

                viewModel.progressNextRound();
                gameState = SingleGameState::Running;
            }
            ImGui::Spacing();
        } else {
            auto winnerIdx = viewModel.getWinningTeamIndex();
            if (winnerIdx) {
                ImGui::Separator();
                ImGui::Spacing();

                float totalBlockWidth = 200.f;
                ImGui::SetCursorPosX((contentWidth - totalBlockWidth) * 0.5f);

                ImGui::BeginChild("WinnerFrame", ImVec2(200, 200), true);

                float frameWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Spacing();

                float imageWidth = 128.f;
                float imageX = (frameWidth - imageWidth) * 0.5f;
                ImGui::SetCursorPosX(imageX);
                ImGui::Image(trophyTexture, sf::Vector2f(imageWidth, imageWidth));

                ImGui::Spacing();
                std::string winnerText = "Winner: " + teams[*winnerIdx].name;
                float textWidth = ImGui::CalcTextSize(winnerText.c_str()).x;
                float textX = (frameWidth - textWidth) * 0.5f;
                ImGui::SetCursorPosX(textX);
                ImGui::Text("%s", winnerText.c_str());

                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float buttonWidth = 200.f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                float totalButtonWidth = buttonWidth * 2 + spacing;
                ImGui::SetCursorPosX((contentWidth - totalButtonWidth) * 0.5f);

                if (ImGui::Button("Back to Menu", ImVec2(buttonWidth, 50))) {
                    sceneManager.setScene(std::make_unique<MainMenuScene>(sceneManager, window));
                }

                ImGui::SameLine();

                if (ImGui::Button("Play Again", ImVec2(buttonWidth, 50))) {
                    sceneManager.setScene(std::make_unique<TournamentScene>(sceneManager, window, teams));
                }

                ImGui::Spacing();
            }
        }
    }
    else{
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("The Game got aborted");
    
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 200.f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalButtonWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((contentWidth - totalButtonWidth) * 0.5f);

        if (ImGui::Button("Back to Menu", ImVec2(buttonWidth, 50))) {
            LOG_INFO("Input", "Tournament -> Returning to Main Menu");
            sceneManager.setScene(std::make_unique<MainMenuScene>(sceneManager, window));
        }

        ImGui::SameLine();

        if (ImGui::Button("Play Again", ImVec2(buttonWidth, 50))) {
            LOG_INFO("Input", "Restarting TournamentScene");
        
            sceneManager.setScene(std::make_unique<TournamentScene>(sceneManager, window, teams));
        }

        ImGui::Spacing();

    }
}

void TournamentScene::setupWindowPositionAndStyle(sf::RenderWindow& window) {
    const sf::Vector2u winSize = window.getSize();
    const ImVec2 windowSize(650.f, 300.f);
    const ImVec2 centerPos(
        (winSize.x - windowSize.x) * 0.5f,
        (winSize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,        ImVec4(0.f, 0.f, 0.f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Button,          ImVec4(0.17f, 0.39f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   ImVec4(0.22f, 0.49f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,    ImVec4(0.13f, 0.31f, 0.13f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,         ImVec4(0.14f, 0.28f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,  ImVec4(0.19f, 0.36f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,   ImVec4(0.24f, 0.45f, 0.24f, 1.0f));
}