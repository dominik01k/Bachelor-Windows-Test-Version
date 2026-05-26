#pragma once

#include <unordered_map>
#include <vector>
#include <utility>
#include <cmath>
#include <set>
#include <SFML/Graphics.hpp>

class ICollidable;

namespace std {
    template <>
    struct hash<std::pair<int, int>> {
        size_t operator()(const std::pair<int, int>& p) const noexcept {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };
}

class CollisionManager {
public:
    static CollisionManager& getInstance();

    void registerObject(ICollidable* obj);
    void unregisterObject(ICollidable* obj);
    void updateObjectCell(ICollidable* obj);
    void clear();

    void notifyMoved(ICollidable* movedObj);
    bool canMoveTo(ICollidable* obj, const sf::Vector2f& offset);

private:
    CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;

    using Cell = std::pair<int, int>;
    std::unordered_map<Cell, std::vector<ICollidable*>> spatialGrid;
    std::unordered_map<ICollidable*, std::vector<Cell>> objectToCells;
    std::vector<ICollidable*> objects;

    float cellSize = 64.0f; 

    std::vector<Cell> getCellsForBounds(const sf::FloatRect& bounds) const;
    void insertIntoGrid(ICollidable* obj);
    void removeFromGrid(ICollidable* obj);

    bool pixelPerfectCollision(const ICollidable* a, const ICollidable* b, const sf::Vector2f& offsetA = {0.f, 0.f});
    bool checkOwnBullet(ICollidable* a, ICollidable* b);
    bool isObjectValid(ICollidable* obj);
};
