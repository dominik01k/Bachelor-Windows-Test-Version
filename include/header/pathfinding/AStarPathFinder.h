#pragma once

#include <SFML/System.hpp>
#include <vector>
#include <queue>
#include <unordered_set>
#include <cmath>
#include <set>
#include <unordered_map>
#include "pathfinding/MoveCommand.h"
#include <iostream>
#include "GridFieldType.h"

class AStarPathfinderContinuous {
public:
    using Grid = std::vector<std::vector<EGridFieldType>>;



    AStarPathfinderContinuous();

    std::vector<MoveCommand> findPath(sf::Vector2f startPos, sf::Angle startAngle, sf::Vector2f targetPos);
    std::vector<MoveCommand> findShootingPath(sf::Vector2f startPos, sf::Angle startAngle, sf::Vector2f targetPos);
    void setNewGrid(Grid newGrid);
    bool isPathClear(sf::Vector2f from, sf::Vector2f to) const;
    bool isThreadAngle(sf::Vector2f from, sf::Vector2f to) const;

private:
    struct Node {
        sf::Vector2f position;
        float angle;
        float gCost;
        float hCost;
        Node* parent;
        MoveCommand moveFromParent;

        Node(sf::Vector2f pos, float ang, float g, float h, Node* p, MoveCommand move)
            : position(pos), angle(ang), gCost(g), hCost(h), parent(p), moveFromParent(move) {}

        float fCost() const { return gCost + hCost; }
    };

    struct Compare {
        bool operator()(const Node* a, const Node* b) const {
            return a->fCost() > b->fCost();
        }
    };
    Grid grid;
    int tileSize = 1.f;
    float stepSize = 50.f;
    float maxTurnAngleDeg = 45.f;
    float angleStepDeg = 15.f;

    int inflationCells = 20;

    bool isWalkable(sf::Vector2f pos) const;
    float heuristic(sf::Vector2f a, sf::Vector2f b) const;
    std::vector<sf::Vector2f> generateDirectionalOffsets(float currentAngle, float stepDistance, int angleStepDegrees);
    void inflateObstacles(float radius);
    bool isFreeGridcell(int x, int y) const;
};
