#pragma once

#include <SFML/Graphics.hpp>
#include "PlayerEntry.h"
#include "Zone.h"
#include "GameState.h"
#include<iostream>
#include "TeamAI.h"

class GameGandler;

class Team
{
private:
    static int nextTeamID; 
    int teamID;
    std::vector<PlayerEntry> players;
    GameState currState;

    std::shared_ptr<TeamAI> teamAI;


    enum StrategyType { CAPTURE_UNCONTESTED , CONTEST_ENEMY};
    StrategyType currentStrategy = CAPTURE_UNCONTESTED;

    friend class GameHandler;
public:
    Team(std::shared_ptr<TeamAI> teamAI);
    Team(const Team&) = delete;
    Team& operator=(const Team&) = delete;
    void addPlayer(Player* newPlayer, std::shared_ptr<PlayerAI> newPlayerAI);
    std::vector<PlayerEntry> getPlayers();

    void updateGameState(GameState newCurrState);
    int getTeamID();

    void update(float deltaTime, const GameState gameState);


};