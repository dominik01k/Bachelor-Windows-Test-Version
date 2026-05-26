#include "collisionDetection/CollisionManager.h"
#include "Bullet.h"
#include <cmath>
#include <algorithm>
#include <mutex>
#include "Player.h"
#include <unordered_set>
#include <thread>
#include <future>
#include "Config.h"
#include "Logger.h"

static int GAME_WIDTH = g_config.gameWidth;
static int GAME_HEIGHT = g_config.gameHeight;


CollisionManager& CollisionManager::getInstance() {
    static CollisionManager instance;
    return instance;
}

void CollisionManager::registerObject(ICollidable* obj) {
    objects.push_back(obj);

    insertIntoGrid(obj);
    LOG_DEBUG("Collision", "Registered object type=" + obj->getTag());
}

void CollisionManager::unregisterObject(ICollidable* obj) {
    objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
    removeFromGrid(obj);
    LOG_DEBUG("Collision", "Unregistered object type=" + obj->getTag());
}

void CollisionManager::clear() {
    LOG_INFO("Collision", "Clearing CollisionManager");

    objects.clear();
    spatialGrid.clear();
    objectToCells.clear();
}

std::vector<CollisionManager::Cell> CollisionManager::getCellsForBounds(const sf::FloatRect& bounds) const {
    std::vector<Cell> cells;

    int xStart = static_cast<int>(std::floor(bounds.position.x / cellSize));
    int yStart = static_cast<int>(std::floor(bounds.position.y / cellSize));
    int xEnd   = static_cast<int>(std::floor((bounds.position.x + bounds.size.x) / cellSize));
    int yEnd   = static_cast<int>(std::floor((bounds.position.y + bounds.size.y) / cellSize));

    for (int x = xStart; x <= xEnd; ++x) {
        for (int y = yStart; y <= yEnd; ++y) {
            cells.emplace_back(x, y);
        }
    }

    return cells;
}


void CollisionManager::insertIntoGrid(ICollidable* obj) {
    const auto bounds = obj->getSprite()->getGlobalBounds();
    auto cells = getCellsForBounds(bounds);
    objectToCells[obj] = cells;
    for (const auto& cell : cells) {
        spatialGrid[cell].push_back(obj);
    }
}

void CollisionManager::removeFromGrid(ICollidable* obj) {
    auto it = objectToCells.find(obj);
    if (it != objectToCells.end()) {
        for (const auto& cell : it->second) {
            auto& vec = spatialGrid[cell];
            vec.erase(std::remove(vec.begin(), vec.end(), obj), vec.end());
        }
        objectToCells.erase(it);
    }
}

void CollisionManager::updateObjectCell(ICollidable* obj) {
    removeFromGrid(obj);
    insertIntoGrid(obj);
}

void CollisionManager::notifyMoved(ICollidable* movedObj) {
    updateObjectCell(movedObj);
    const sf::FloatRect& boundsA = movedObj->getBounds();
    auto cells = getCellsForBounds(boundsA);

    std::unordered_set<ICollidable*> alreadyChecked;
    std::mutex mutex;

    std::vector<std::future<void>> futures;

    for (const auto& cell : cells) {

        auto it = spatialGrid.find(cell);
        if (it != spatialGrid.end()) {
            for (ICollidable* other : it->second) {
                if (other == movedObj) continue;

                if (alreadyChecked.count(other)) continue;
                alreadyChecked.insert(other);

                const sf::FloatRect& boundsB = other->getBounds();
                if (boundsA.findIntersection(boundsB)) {
                    if (pixelPerfectCollision(movedObj, other)) {
                        if (!checkOwnBullet(movedObj, other)) {
                            if(isObjectValid(movedObj) && isObjectValid(other)){
                                LOG_TRACE("Collision",
                                    "Collision detected between " +
                                    movedObj->getTag() + " and " +
                                    other->getTag()
                                );
                                movedObj->onCollision(other);
                                other->onCollision(movedObj);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool CollisionManager::isObjectValid(ICollidable* obj) {
    if(obj->getTag() == "Player") {
        Player* player = dynamic_cast<Player*>(obj);
        return !player->iscurrDead();
    }
    if(obj->getTag() == "Bullet") {
        Bullet* bullet = dynamic_cast<Bullet*>(obj);
        return !bullet->shouldBeDestroyed();
    }
    return true;
}

bool CollisionManager::checkOwnBullet(ICollidable* obj1, ICollidable* obj2) {
    return obj1->getUniqueTeamID() == obj2->getUniqueTeamID();
}

bool CollisionManager::pixelPerfectCollision(const ICollidable* a, const ICollidable* b, const sf::Vector2f& offsetA) {
    const sf::Sprite* spriteA = a->getSprite();
    const sf::Sprite* spriteB = b->getSprite();
    const auto& maskA = a->getCollisionMask();
    const auto& maskB = b->getCollisionMask();

    sf::Sprite spriteACopy = *spriteA;
    spriteACopy.move(offsetA);

    sf::FloatRect rectA = spriteACopy.getGlobalBounds();
    sf::FloatRect rectB = spriteB->getGlobalBounds();

    auto intersectionOpt = rectA.findIntersection(rectB);
    if (!intersectionOpt.has_value()) return false;

    const sf::FloatRect& intersection = intersectionOpt.value();

    sf::Transform invA = spriteACopy.getInverseTransform();
    sf::Transform invB = spriteB->getInverseTransform();

    for (int i = static_cast<int>(intersection.position.x); i < static_cast<int>(intersection.position.x + intersection.size.x); ++i) {
        for (int j = static_cast<int>(intersection.position.y); j < static_cast<int>(intersection.position.y + intersection.size.y); ++j) {
            sf::Vector2f point((float)i, (float)j);

            sf::Vector2f localA = invA.transformPoint(point);
            sf::Vector2f localB = invB.transformPoint(point);

            int xA = static_cast<int>(localA.x);
            int yA = static_cast<int>(localA.y);
            int xB = static_cast<int>(localB.x);
            int yB = static_cast<int>(localB.y);

            if (xA >= 0 && yA >= 0 && xA < (int)maskA.size() && yA < (int)maskA[0].size() &&
                xB >= 0 && yB >= 0 && xB < (int)maskB.size() && yB < (int)maskB[0].size()) {
                
                if (maskA[xA][yA] && maskB[xB][yB]) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool CollisionManager::canMoveTo(ICollidable* obj, const sf::Vector2f& offset) {
    sf::FloatRect currentBounds = obj->getBounds();
    sf::FloatRect futureBounds = currentBounds;
    futureBounds.position += offset;

    if (futureBounds.position.x < 0 || futureBounds.position.y < 0 ||
        futureBounds.position.x + futureBounds.size.x > GAME_WIDTH ||
        futureBounds.position.y + futureBounds.size.y > GAME_HEIGHT) {

        return false;
    }

    auto cells = getCellsForBounds(futureBounds);

    std::unordered_set<ICollidable*> alreadyChecked;

    for (const auto& cell : cells) {
        auto it = spatialGrid.find(cell);
        if (it != spatialGrid.end()) {
            for (ICollidable* other : it->second) {
                if (other == obj) continue;
                if (alreadyChecked.count(other)) continue;
                alreadyChecked.insert(other);

                if (other->getTag() == "Wall") {
                    sf::FloatRect otherBounds = other->getBounds();

                    auto intersection = futureBounds.findIntersection(otherBounds);
                    if (intersection.has_value()) {
                        auto rect = intersection.value();

                        if (pixelPerfectCollision(obj, other, offset)) {
                            LOG_TRACE("Collision",
                                "Object: " + obj->getTag() + 
                                " can't move in direction of (" + std::to_string(offset.x) + ", " + std::to_string(offset.y) + ")"
                            );
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}
 