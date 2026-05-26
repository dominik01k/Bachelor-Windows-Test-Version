#include "pathfinding/AStarPathFinder.h"

AStarPathfinderContinuous::AStarPathfinderContinuous() {}

bool AStarPathfinderContinuous::isWalkable(sf::Vector2f pos) const {
    int inflationCells = 10;
    if (pos.x < inflationCells || pos.y < inflationCells || 
        pos.x >= grid[0].size() * tileSize - inflationCells || 
        pos.y >= grid.size() * tileSize - inflationCells) {
        return false;
    }

    int x = static_cast<int>(pos.x / tileSize);
    int y = static_cast<int>(pos.y / tileSize);
    return isFreeGridcell(x,y);
}

bool AStarPathfinderContinuous::isFreeGridcell(int x, int y) const{
    if(grid[y][x] == EGridFieldType::FREE || grid[y][x] == EGridFieldType::BUSH){
        return true;
    }
    return false;
}

bool AStarPathfinderContinuous::isPathClear(sf::Vector2f from, sf::Vector2f to) const {
    const int steps = 10;
    for (int i = 1; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        sf::Vector2f point = from + t * (to - from);
        if (!isWalkable(point)) {
            return false;
        }
    }
    return true;
}

bool AStarPathfinderContinuous::isThreadAngle(sf::Vector2f from, sf::Vector2f to) const {
    const int steps = 10;
    for (int i = 1; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        sf::Vector2f point = from + t * (to - from);

        int x = static_cast<int>(point.x);
        int y = static_cast<int>(point.y);

        if (x < 0 || y < 0 || x >= grid[0].size()  || y >= grid.size() ) {
            return false;
        }   
        if (!isFreeGridcell(x, y)) {
            return false;
        }
    }
    return true;
}

float AStarPathfinderContinuous::heuristic(sf::Vector2f a, sf::Vector2f b) const {
    return std::hypot(b.x - a.x, b.y - a.y);
}

std::vector<MoveCommand> AStarPathfinderContinuous::findPath(sf::Vector2f startPos, sf::Angle startAngle, sf::Vector2f targetPos) {

    auto cmp = Compare{};
    std::priority_queue<Node*, std::vector<Node*>, Compare> openSet(cmp);
    std::vector<Node*> allNodes;
    std::unordered_map<std::string, Node*> visited;

    Node* startNode = new Node(startPos, startAngle.asDegrees(), 0.f, heuristic(startPos, targetPos), nullptr, {MoveType::MoveForward, 0.f});
    openSet.push(startNode);
    allNodes.push_back(startNode);

    auto getKey = [](const sf::Vector2f& pos) {
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        return std::to_string(x) + "_" + std::to_string(y);
    };

    visited[getKey(startNode->position)] = startNode;

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        float distanceToTarget = heuristic(current->position, targetPos);

        if (distanceToTarget <= 50.f && isPathClear(current->position, targetPos)) {
            std::vector<sf::Vector2f> pathPositions;
            Node* pathNode = current;
            while (pathNode != nullptr) {
                pathPositions.push_back(pathNode->position);
                pathNode = pathNode->parent;
            }
            std::reverse(pathPositions.begin(), pathPositions.end());

            pathPositions.push_back(targetPos);

            std::vector<MoveCommand> commands;
            sf::Vector2f currentPos = pathPositions[0];
            float currentAngle = startAngle.asDegrees();

            for (size_t i = 1; i < pathPositions.size(); ++i) {
                sf::Vector2f nextPos = pathPositions[i];
                sf::Vector2f delta = nextPos - currentPos;

                float targetAngle = std::atan2(delta.y, delta.x) * 180.f / 3.14159265f + 180.f;
                float angleDiff = targetAngle - currentAngle;

                while (angleDiff < -180.f) angleDiff += 360.f;
                while (angleDiff > 180.f) angleDiff -= 360.f;

                if (std::abs(angleDiff) > 1e-2f) {
                    MoveType rotateType = angleDiff > 0 ? MoveType::RotateRight : MoveType::RotateLeft;
                    commands.push_back({rotateType, std::abs(angleDiff)});
                    currentAngle = targetAngle;
                }

                float distance = std::hypot(delta.x, delta.y);
                commands.push_back({MoveType::MoveForward, distance});
                currentPos = nextPos;
            }

            for (Node* n : allNodes) delete n;
            return commands;
        }

        std::vector<sf::Vector2f> directions = generateDirectionalOffsets(current->angle, 50.f, 10);

        for (const sf::Vector2f& offset : directions) {
            sf::Vector2f newPos = current->position + offset;
            if (!isWalkable(newPos) || !isPathClear(current->position, newPos)) continue;

            float g = current->gCost + std::hypot(offset.x, offset.y);
            float h = heuristic(newPos, targetPos);

            Node* neighbor = new Node(newPos, 0.f, g, h, current, {MoveType::MoveForward, stepSize});
            std::string key = getKey(neighbor->position);

            auto it = visited.find(key);
            if (it != visited.end() && it->second->gCost <= g) {
                delete neighbor;
                continue;
            }

            visited[key] = neighbor;
            openSet.push(neighbor);
            allNodes.push_back(neighbor);
        }
    }

    for (Node* n : allNodes) delete n;
    return {};
}

void AStarPathfinderContinuous::inflateObstacles(float radius) {
    Grid inflatedGrid = grid;

    for (size_t y = 0; y < grid.size(); ++y) {
        for (size_t x = 0; x < grid[0].size(); ++x) {
            if (grid[y][x] == EGridFieldType::WALL) {
                for (int dy = -inflationCells; dy <= inflationCells; ++dy) {
                    for (int dx = -inflationCells; dx <= inflationCells; ++dx) {
                        int nx = static_cast<int>(x) + dx;
                        int ny = static_cast<int>(y) + dy;

                        inflatedGrid[ny][nx] = EGridFieldType::WALL;

                    }
                }
            }
        }
    }

    grid = inflatedGrid;
}


void AStarPathfinderContinuous::setNewGrid(Grid newGrid){
    this->grid = newGrid;
    inflateObstacles(5.f);
}

std::vector<MoveCommand> AStarPathfinderContinuous::findShootingPath(sf::Vector2f startPos, sf::Angle startAngle, sf::Vector2f targetPos) {
    std::vector<MoveCommand> commands;

    sf::Vector2f delta = targetPos - startPos;
    float targetAngle = std::atan2(delta.y, delta.x) * 180.f / 3.14159265f + 180.f;

    float currentAngle = startAngle.asDegrees();
    float angleDiff = targetAngle - currentAngle;

    while (angleDiff < -180.f) angleDiff += 360.f;
    while (angleDiff > 180.f) angleDiff -= 360.f;

    if (std::abs(angleDiff) > 1e-2f) {
        MoveType rotateType = angleDiff > 0 ? MoveType::RotateRight : MoveType::RotateLeft;
        commands.push_back({rotateType, std::abs(angleDiff)});
    }

    commands.push_back({MoveType::Shoot, 0.f});

    return commands;
}

std::vector<sf::Vector2f> AStarPathfinderContinuous::generateDirectionalOffsets(float currentAngle, float stepDistance, int angleStepDegrees) {
    std::vector<sf::Vector2f> offsets;

    for (int angleOffset = -180; angleOffset <= 180; angleOffset += angleStepDegrees) {
        float angleDeg = currentAngle + angleOffset;
        float angleRad = angleDeg * 3.14159265f / 180.f;

        float dx = std::cos(angleRad) * stepDistance;
        float dy = std::sin(angleRad) * stepDistance;

        sf::Vector2f offset(dx, dy);
        offsets.emplace_back(offset);
    }

    return offsets;
}





