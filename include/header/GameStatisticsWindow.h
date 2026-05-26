#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include "Zone.h"

class GameStatisticsWindow {
public:
    GameStatisticsWindow();

    void updateStatistics(const std::vector<Zone*>& zones, float gameTimeLeft);

    bool isOpen() const;
    void closeWindow();

private:
    void drawStatistics(const std::vector<Zone*>& zones, float gameTimeLeft);
    void drawScrollableContent(const std::vector<Zone*>& zones, float gameTimeLeft);

private:
    sf::RenderWindow window;
    sf::Font font;

    float scrollOffset = 0.f;
    float maxScroll = 0.f;
    sf::FloatRect contentArea;

    sf::View contentView;
};