#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameStatisticsWindow.h"
#include "TeamZoneColors.h"
#include <iostream>
#include <sstream>
#include "Config.h"
#include <iomanip>
#include <algorithm>
#include "Logger.h"

#if defined(_WIN32)
#include <Windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef ERROR
#undef ERROR
#endif
#ifdef WARNING
#undef WARNING
#endif

GameStatisticsWindow::GameStatisticsWindow()
    : window(sf::VideoMode(sf::Vector2u(
          g_config.statisticsWindowWidth,
          g_config.statisticsWindowHeight)),
          "Statistics Window")
{
    LOG_INFO("Map", "Statistics window created (" + std::to_string(g_config.statisticsWindowWidth) + "x" + std::to_string(g_config.statisticsWindowHeight) + ")");
    
    if (!font.openFromFile("../assets/fonts/Andale Mono.ttf")) {
        LOG_ERROR("Resources", "Failed to load font: ../assets/fonts/Andale Mono.ttf");
    } else {
        LOG_DEBUG("Resources", "Statistics font loaded successfully");
    }

    contentArea = sf::FloatRect(sf::Vector2f(75.f, 150.f), sf::Vector2f(200.f, 200.f));
    contentView.setSize(sf::Vector2f(contentArea.size.x, contentArea.size.y));
}


void GameStatisticsWindow::drawStatistics(const std::vector<Zone*>& zones, float gameTimeLeft) {
    window.clear();

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("../assets/images/game_score_back.png")) {
        LOG_ERROR("Resources", "Failed to load statistics background texture: ../assets/images/game_score_back.png");
    }

    sf::Vector2u windowSize = window.getSize();
    window.setView(window.getDefaultView());

    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(sf::Vector2f(
        static_cast<float>(windowSize.x) / backgroundTexture.getSize().x,
        static_cast<float>(windowSize.y) / backgroundTexture.getSize().y
    ));
    window.draw(backgroundSprite);

    sf::RectangleShape border;
    border.setPosition(contentArea.position);
    border.setSize(contentArea.size);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(1.f);
    window.draw(border);


    sf::FloatRect viewport(
        sf::Vector2f(
        contentArea.position.x / windowSize.x,
        contentArea.position.y / windowSize.y),
        sf::Vector2f(
        contentArea.size.x / windowSize.x,
        contentArea.size.y / windowSize.y)
    );

    contentView.setViewport(viewport);

    float contentHeight = 20.f;

    for (const Zone* zone : zones) {
        int teamCount = zone->getTeamProgressMap().size();
        float boxHeight = 14.f + teamCount * 11.f + 6.f;
        contentHeight += boxHeight + 8.f;
    }

    contentHeight += 120.f;

    float previousMaxScroll = maxScroll;
    maxScroll = std::max(0.f, contentHeight - contentArea.size.y);
    
    if(std::abs(previousMaxScroll - maxScroll) > 1.0f) {
        LOG_TRACE("Map", "Statistics content height updated to: " + std::to_string(contentHeight));
    }

    contentView.setCenter(
        sf::Vector2f(
        contentArea.size.x / 2.f,
        contentArea.size.y / 2.f + scrollOffset)
    );

    window.setView(contentView);

    glEnable(GL_SCISSOR_TEST);

    glScissor(
        static_cast<int>(contentArea.position.x),
        static_cast<int>(windowSize.y - (contentArea.position.y + contentArea.size.y)),
        static_cast<int>(contentArea.size.x),
        static_cast<int>(contentArea.size.y)
    );

    drawScrollableContent(zones, gameTimeLeft);

    glDisable(GL_SCISSOR_TEST);

    window.display();
}

void GameStatisticsWindow::drawScrollableContent(const std::vector<Zone*>& zones, float gameTimeLeft) {
    const float baseHeight = 14.f;
    const float perTeamHeight = 11.f;
    const float padding = 6.f;

    const float boxWidth = contentArea.size.x - 20.f;

    float xOffset = 10.f;
    float yOffset = 10.f;

    const unsigned int headerFontSize = 10;
    const unsigned int labelFontSize = 8;

    auto drawCenteredHeader = [&](const std::string& text, float yPos) {
        sf::Text header(font, text, headerFontSize);
        header.setFillColor(sf::Color::White);

        float width = header.getLocalBounds().size.x;
        header.setPosition(sf::Vector2f(xOffset + (boxWidth - width) / 2.f, yPos));

        window.draw(header);
    };

    drawCenteredHeader("Zone Statistics:", yOffset);
    yOffset += 16.f;

    std::unordered_map<int, int> teamTotalProgress;
    std::unordered_map<int, int> zoneCounts;

    for (size_t i = 0; i < zones.size(); ++i) {
        const Zone* zone = zones[i];

        int teamCount = zone->getTeamProgressMap().size();
        float boxHeight = baseHeight + teamCount * perTeamHeight + padding;

        sf::RectangleShape zoneRect({boxWidth, boxHeight});
        zoneRect.setPosition(sf::Vector2f(xOffset, yOffset));
        zoneRect.setFillColor(sf::Color(20, 20, 20, 200));
        window.draw(zoneRect);

        sf::Text zoneLabel(font, "Zone " + std::to_string(i), labelFontSize);
        zoneLabel.setFillColor(sf::Color::White);
        zoneLabel.setPosition(sf::Vector2f(xOffset + 5.f, yOffset + 1.f));
        window.draw(zoneLabel);

        float teamYOffset = yOffset + 14.f;

        for (const auto& [teamID, progress] : zone->getTeamProgressMap()) {
            sf::Text text(
                font,
                "Team " + std::to_string(teamID) + ": " + std::to_string((int)progress) + "%",
                labelFontSize
            );

            text.setFillColor(TeamZoneColors::getColorForTeam(teamID));
            text.setPosition(sf::Vector2f(xOffset + 8.f, teamYOffset));
            window.draw(text);

            teamYOffset += perTeamHeight;

            teamTotalProgress[teamID] += (int)progress;
            zoneCounts[teamID]++;
        }

        yOffset += boxHeight + 8.f;
    }

    yOffset += 8.f;
    drawCenteredHeader("Overall Team Progress:", yOffset);
    yOffset += 16.f;

    for (const auto& [teamID, total] : teamTotalProgress) {
        int count = zoneCounts[teamID];
        int avg = count > 0 ? total / count : 0;

        sf::Text text(
            font,
            "Team " + TeamZoneColors::getColorNameForTeam(teamID) + ": " + std::to_string(avg) + "%",
            labelFontSize
        );

        text.setFillColor(TeamZoneColors::getColorForTeam(teamID));
        text.setPosition(sf::Vector2f(xOffset + 8.f, yOffset));
        window.draw(text);

        yOffset += 13.f;
    }

    yOffset += 12.f;
    drawCenteredHeader("Game Time Left:", yOffset);
    yOffset += 15.f;

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << gameTimeLeft;

    sf::Text timeText(font, ss.str() + "s", labelFontSize);
    timeText.setFillColor(sf::Color::White);

    float width = timeText.getLocalBounds().size.x;
    timeText.setPosition(sf::Vector2f(xOffset + (boxWidth - width) / 2.f, yOffset));

    window.draw(timeText);
}

bool GameStatisticsWindow::isOpen() const {
    return window.isOpen();
}

void GameStatisticsWindow::closeWindow() {
    if (window.isOpen()) {
        LOG_INFO("Map", "Statistics window closed manually or via system");
        window.close();
    }
}

void GameStatisticsWindow::updateStatistics(const std::vector<Zone*>& zones, float gameTimeLeft) {
    while (auto event = window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            LOG_INFO("Input", "User requested statistics window close via OS event");
            closeWindow();
        }

        if (event->is<sf::Event::MouseWheelScrolled>()) {
            float delta = event->getIf<sf::Event::MouseWheelScrolled>()->delta;

            float oldOffset = scrollOffset;

            scrollOffset -= delta * 20.f;
            scrollOffset = std::clamp(scrollOffset, 0.f, maxScroll);

            if (oldOffset != scrollOffset) {
                LOG_TRACE("Input", "Statistics scroll adjusted to: " + std::to_string(scrollOffset));
            }
        }
    }

    if (!window.isOpen()) return;

    drawStatistics(zones, gameTimeLeft);
}