#include "Pathfinding.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace AI {

// ============================================================================
// PATHFINDING IMPLEMENTATION
// ============================================================================

Pathfinding::Pathfinding()
    : m_width(0)
    , m_height(0)
    , m_cellSize(1.0f)
    , m_debugDraw(false)
{
}

Pathfinding::~Pathfinding() {
    Shutdown();
}

void Pathfinding::Initialize(int width, int height, float cellSize) {
    m_width = width;
    m_height = height;
    m_cellSize = cellSize;

    // Initialize grid
    m_grid.resize(height);
    for (int y = 0; y < height; y++) {
        m_grid[y].resize(width);
        for (int x = 0; x < width; x++) {
            m_grid[y][x] = GridCell(CellType::WALKABLE, 1.0f);
        }
    }
}

void Pathfinding::Shutdown() {
    m_grid.clear();
    m_width = 0;
    m_height = 0;
}

void Pathfinding::SetCellType(int x, int y, CellType type) {
    if (IsInBounds(x, y)) {
        m_grid[y][x].type = type;

        // Update movement cost based on type
        switch (type) {
            case CellType::WALKABLE:
                m_grid[y][x].movementCost = 1.0f;
                break;
            case CellType::BLOCKED:
                m_grid[y][x].movementCost = std::numeric_limits<float>::max();
                break;
            case CellType::DIFFICULT_TERRAIN:
                m_grid[y][x].movementCost = 2.0f;
                break;
            case CellType::WATER:
                m_grid[y][x].movementCost = 3.0f;
                break;
            case CellType::HAZARD:
                m_grid[y][x].movementCost = 5.0f;
                break;
        }
    }
}

void Pathfinding::SetCellType(const GridPos& pos, CellType type) {
    SetCellType(pos.x, pos.y, type);
}

CellType Pathfinding::GetCellType(int x, int y) const {
    const GridCell* cell = GetCell(x, y);
    return cell ? cell->type : CellType::BLOCKED;
}

CellType Pathfinding::GetCellType(const GridPos& pos) const {
    return GetCellType(pos.x, pos.y);
}

void Pathfinding::SetCellMovementCost(int x, int y, float cost) {
    if (IsInBounds(x, y)) {
        m_grid[y][x].movementCost = cost;
    }
}

void Pathfinding::SetCellMovementCost(const GridPos& pos, float cost) {
    SetCellMovementCost(pos.x, pos.y, cost);
}

float Pathfinding::GetCellMovementCost(int x, int y) const {
    const GridCell* cell = GetCell(x, y);
    return cell ? cell->movementCost : std::numeric_limits<float>::max();
}

float Pathfinding::GetCellMovementCost(const GridPos& pos) const {
    return GetCellMovementCost(pos.x, pos.y);
}

void Pathfinding::SetArea(int x1, int y1, int x2, int y2, CellType type) {
    int minX = std::min(x1, x2);
    int maxX = std::max(x1, x2);
    int minY = std::min(y1, y2);
    int maxY = std::max(y1, y2);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            SetCellType(x, y, type);
        }
    }
}

void Pathfinding::SetCircle(int centerX, int centerY, int radius, CellType type) {
    int radiusSquared = radius * radius;

    for (int y = centerY - radius; y <= centerY + radius; y++) {
        for (int x = centerX - radius; x <= centerX + radius; x++) {
            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy <= radiusSquared) {
                SetCellType(x, y, type);
            }
        }
    }
}

PathResult Pathfinding::FindPath(const GridPos& start, const GridPos& goal,
                                 bool allowDiagonal, bool smoothPath) {
    PathRequest request;
    request.start = start;
    request.goal = goal;
    request.allowDiagonal = allowDiagonal;
    request.smoothPath = smoothPath;

    return FindPath(request);
}

PathResult Pathfinding::FindPath(const WorldPos& start, const WorldPos& goal,
                                 bool allowDiagonal, bool smoothPath) {
    GridPos gridStart = WorldToGrid(start);
    GridPos gridGoal = WorldToGrid(goal);

    return FindPath(gridStart, gridGoal, allowDiagonal, smoothPath);
}

PathResult Pathfinding::FindPath(const PathRequest& request) {
    return AStar(request);
}

GridPos Pathfinding::WorldToGrid(const WorldPos& worldPos) const {
    int x = static_cast<int>(worldPos.x / m_cellSize);
    int y = static_cast<int>(worldPos.y / m_cellSize);
    return GridPos(x, y);
}

WorldPos Pathfinding::GridToWorld(const GridPos& gridPos) const {
    float x = (gridPos.x + 0.5f) * m_cellSize; // Center of cell
    float y = (gridPos.y + 0.5f) * m_cellSize;
    return WorldPos(x, y);
}

bool Pathfinding::IsWalkable(int x, int y) const {
    const GridCell* cell = GetCell(x, y);
    return cell && cell->IsWalkable();
}

bool Pathfinding::IsWalkable(const GridPos& pos) const {
    return IsWalkable(pos.x, pos.y);
}

bool Pathfinding::IsInBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool Pathfinding::IsInBounds(const GridPos& pos) const {
    return IsInBounds(pos.x, pos.y);
}

bool Pathfinding::HasLineOfSight(const GridPos& start, const GridPos& end) const {
    // Bresenham's line algorithm
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (!IsWalkable(x0, y0)) {
            return false;
        }

        if (x0 == x1 && y0 == y1) {
            return true;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

bool Pathfinding::HasLineOfSight(const WorldPos& start, const WorldPos& end) const {
    GridPos gridStart = WorldToGrid(start);
    GridPos gridEnd = WorldToGrid(end);
    return HasLineOfSight(gridStart, gridEnd);
}

GridPos Pathfinding::FindNearestWalkable(const GridPos& pos, int maxRadius) const {
    if (IsWalkable(pos)) {
        return pos;
    }

    // Search in expanding rings
    for (int radius = 1; radius <= maxRadius; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Only check cells on the edge of the current ring
                if (std::abs(dx) != radius && std::abs(dy) != radius) {
                    continue;
                }

                GridPos candidate(pos.x + dx, pos.y + dy);
                if (IsWalkable(candidate)) {
                    return candidate;
                }
            }
        }
    }

    return pos; // Return original if no walkable found
}

PathResult Pathfinding::AStar(const PathRequest& request) {
    PathResult result;

    // Validate start and goal
    if (!IsInBounds(request.start) || !IsInBounds(request.goal)) {
        return result;
    }

    if (!IsWalkable(request.start) || !IsWalkable(request.goal)) {
        return result;
    }

    // If start == goal
    if (request.start == request.goal) {
        result.gridPath.push_back(request.start);
        result.worldPath.push_back(GridToWorld(request.start));
        result.success = true;
        result.totalCost = 0.0f;
        return result;
    }

    // A* algorithm
    std::priority_queue<AStarNode, std::vector<AStarNode>, AStarNodeCompare> openSet;
    std::unordered_set<GridPos, GridPos::Hash> closedSet;
    std::unordered_map<GridPos, float, GridPos::Hash> gScore;
    std::unordered_map<GridPos, GridPos, GridPos::Hash> cameFrom;

    // Initialize start node
    AStarNode startNode(request.start);
    startNode.g = 0.0f;
    startNode.h = CalculateHeuristic(request.start, request.goal, request.allowDiagonal);
    startNode.f = startNode.g + startNode.h;

    openSet.push(startNode);
    gScore[request.start] = 0.0f;

    while (!openSet.empty()) {
        AStarNode current = openSet.top();
        openSet.pop();

        // Check if we've already processed this node
        if (closedSet.find(current.pos) != closedSet.end()) {
            continue;
        }

        // Found the goal
        if (current.pos == request.goal) {
            ReconstructPath(cameFrom, request.start, request.goal, result);
            result.success = true;
            result.totalCost = current.g;

            if (request.smoothPath) {
                SmoothPath(result);
            }

            return result;
        }

        closedSet.insert(current.pos);

        // Check neighbors
        std::vector<GridPos> neighbors = GetNeighbors(current.pos, request.allowDiagonal);

        for (const GridPos& neighbor : neighbors) {
            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            float movementCost = CalculateMovementCost(current.pos, neighbor);
            float tentativeG = current.g + movementCost;

            // Check if this path exceeds maximum cost
            if (tentativeG > request.maxCost) {
                continue;
            }

            // Check if this is a better path
            auto gIt = gScore.find(neighbor);
            if (gIt == gScore.end() || tentativeG < gIt->second) {
                cameFrom[neighbor] = current.pos;
                gScore[neighbor] = tentativeG;

                AStarNode neighborNode(neighbor);
                neighborNode.g = tentativeG;
                neighborNode.h = CalculateHeuristic(neighbor, request.goal, request.allowDiagonal);
                neighborNode.f = neighborNode.g + neighborNode.h;

                openSet.push(neighborNode);
            }
        }
    }

    // No path found
    return result;
}

float Pathfinding::CalculateHeuristic(const GridPos& a, const GridPos& b, bool allowDiagonal) const {
    int dx = std::abs(a.x - b.x);
    int dy = std::abs(a.y - b.y);

    if (allowDiagonal) {
        // Octile distance (allows diagonal movement)
        float D = 1.0f;
        float D2 = 1.414f; // sqrt(2)
        return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
    } else {
        // Manhattan distance
        return static_cast<float>(dx + dy);
    }
}

float Pathfinding::CalculateMovementCost(const GridPos& from, const GridPos& to) const {
    const GridCell* cell = GetCell(to);
    if (!cell) {
        return std::numeric_limits<float>::max();
    }

    float baseCost = cell->movementCost;

    // Diagonal movement costs more
    int dx = std::abs(to.x - from.x);
    int dy = std::abs(to.y - from.y);

    if (dx > 0 && dy > 0) {
        baseCost *= 1.414f; // sqrt(2)
    }

    return baseCost;
}

void Pathfinding::SmoothPath(PathResult& result) const {
    if (result.gridPath.size() <= 2) {
        return;
    }

    std::vector<GridPos> smoothedPath;
    smoothedPath.push_back(result.gridPath[0]);

    size_t currentIndex = 0;

    while (currentIndex < result.gridPath.size() - 1) {
        size_t furthestVisible = currentIndex + 1;

        // Find the furthest point we can see from current
        for (size_t i = currentIndex + 2; i < result.gridPath.size(); i++) {
            if (HasLineOfSight(result.gridPath[currentIndex], result.gridPath[i])) {
                furthestVisible = i;
            } else {
                break;
            }
        }

        smoothedPath.push_back(result.gridPath[furthestVisible]);
        currentIndex = furthestVisible;
    }

    result.gridPath = smoothedPath;

    // Rebuild world path
    result.worldPath.clear();
    for (const GridPos& gridPos : result.gridPath) {
        result.worldPath.push_back(GridToWorld(gridPos));
    }
}

std::vector<GridPos> Pathfinding::GetNeighbors(const GridPos& pos, bool allowDiagonal) const {
    std::vector<GridPos> neighbors;

    // Cardinal directions
    const int dx4[] = {0, 1, 0, -1};
    const int dy4[] = {-1, 0, 1, 0};

    // Diagonal directions
    const int dx8[] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int dy8[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    const int* dx = allowDiagonal ? dx8 : dx4;
    const int* dy = allowDiagonal ? dy8 : dy4;
    int count = allowDiagonal ? 8 : 4;

    for (int i = 0; i < count; i++) {
        GridPos neighbor(pos.x + dx[i], pos.y + dy[i]);

        if (IsWalkable(neighbor)) {
            // For diagonal movement, check if cardinal neighbors are walkable
            if (allowDiagonal && dx[i] != 0 && dy[i] != 0) {
                if (!IsWalkable(pos.x + dx[i], pos.y) || !IsWalkable(pos.x, pos.y + dy[i])) {
                    continue; // Can't cut corners
                }
            }

            neighbors.push_back(neighbor);
        }
    }

    return neighbors;
}

GridCell* Pathfinding::GetCell(int x, int y) {
    if (!IsInBounds(x, y)) {
        return nullptr;
    }
    return &m_grid[y][x];
}

const GridCell* Pathfinding::GetCell(int x, int y) const {
    if (!IsInBounds(x, y)) {
        return nullptr;
    }
    return &m_grid[y][x];
}

GridCell* Pathfinding::GetCell(const GridPos& pos) {
    return GetCell(pos.x, pos.y);
}

const GridCell* Pathfinding::GetCell(const GridPos& pos) const {
    return GetCell(pos.x, pos.y);
}

void Pathfinding::ReconstructPath(const std::unordered_map<GridPos, GridPos, GridPos::Hash>& cameFrom,
                                 const GridPos& start, const GridPos& goal,
                                 PathResult& result) const {
    result.gridPath.clear();
    result.worldPath.clear();

    GridPos current = goal;
    result.gridPath.push_back(current);

    while (current != start) {
        auto it = cameFrom.find(current);
        if (it == cameFrom.end()) {
            break;
        }
        current = it->second;
        result.gridPath.push_back(current);
    }

    // Reverse to get path from start to goal
    std::reverse(result.gridPath.begin(), result.gridPath.end());

    // Convert to world coordinates
    for (const GridPos& gridPos : result.gridPath) {
        result.worldPath.push_back(GridToWorld(gridPos));
    }
}

WorldPos Pathfinding::GetFlockingAvoidance(const WorldPos& currentPos, float radius,
                                          const std::vector<WorldPos>& neighbors) const {
    return FlockingBehavior::CalculateSeparation(currentPos, neighbors,
                                                FlockingBehavior::FlockingParams());
}

WorldPos Pathfinding::GetSeparationVector(const WorldPos& currentPos,
                                         const std::vector<WorldPos>& neighbors,
                                         float separationRadius) const {
    FlockingBehavior::FlockingParams params;
    params.separationRadius = separationRadius;
    return FlockingBehavior::CalculateSeparation(currentPos, neighbors, params);
}

// ============================================================================
// FLOCKING BEHAVIOR IMPLEMENTATION
// ============================================================================

WorldPos FlockingBehavior::CalculateSeparation(const WorldPos& position,
                                              const std::vector<WorldPos>& neighbors,
                                              const FlockingParams& params) {
    WorldPos steer(0.0f, 0.0f);
    int count = 0;

    for (const WorldPos& neighbor : neighbors) {
        float distance = position.DistanceTo(neighbor);
        if (distance > 0.0f && distance < params.separationRadius) {
            WorldPos diff(position.x - neighbor.x, position.y - neighbor.y);
            diff.x /= distance; // Normalize and weight by distance
            diff.y /= distance;
            steer.x += diff.x;
            steer.y += diff.y;
            count++;
        }
    }

    if (count > 0) {
        steer.x /= count;
        steer.y /= count;
    }

    return steer;
}

WorldPos FlockingBehavior::CalculateAlignment(const WorldPos& velocity,
                                             const std::vector<WorldPos>& neighborVelocities,
                                             const FlockingParams& params) {
    WorldPos steer(0.0f, 0.0f);
    int count = 0;

    for (const WorldPos& neighborVel : neighborVelocities) {
        steer.x += neighborVel.x;
        steer.y += neighborVel.y;
        count++;
    }

    if (count > 0) {
        steer.x /= count;
        steer.y /= count;

        // Subtract current velocity to get steering force
        steer.x -= velocity.x;
        steer.y -= velocity.y;

        steer = Limit(steer, params.maxForce);
    }

    return steer;
}

WorldPos FlockingBehavior::CalculateCohesion(const WorldPos& position,
                                            const std::vector<WorldPos>& neighbors,
                                            const FlockingParams& params) {
    WorldPos center(0.0f, 0.0f);
    int count = 0;

    for (const WorldPos& neighbor : neighbors) {
        float distance = position.DistanceTo(neighbor);
        if (distance < params.cohesionRadius) {
            center.x += neighbor.x;
            center.y += neighbor.y;
            count++;
        }
    }

    if (count > 0) {
        center.x /= count;
        center.y /= count;

        // Steer towards center
        WorldPos steer(center.x - position.x, center.y - position.y);
        return Limit(steer, params.maxForce);
    }

    return WorldPos(0.0f, 0.0f);
}

WorldPos FlockingBehavior::CalculateFlocking(const WorldPos& position,
                                             const WorldPos& velocity,
                                             const std::vector<WorldPos>& neighbors,
                                             const std::vector<WorldPos>& neighborVelocities,
                                             const FlockingParams& params) {
    WorldPos separation = CalculateSeparation(position, neighbors, params);
    WorldPos alignment = CalculateAlignment(velocity, neighborVelocities, params);
    WorldPos cohesion = CalculateCohesion(position, neighbors, params);

    // Apply weights
    separation.x *= params.separationWeight;
    separation.y *= params.separationWeight;
    alignment.x *= params.alignmentWeight;
    alignment.y *= params.alignmentWeight;
    cohesion.x *= params.cohesionWeight;
    cohesion.y *= params.cohesionWeight;

    // Combine
    WorldPos result;
    result.x = separation.x + alignment.x + cohesion.x;
    result.y = separation.y + alignment.y + cohesion.y;

    return Limit(result, params.maxForce);
}

WorldPos FlockingBehavior::CalculateObstacleAvoidance(const WorldPos& position,
                                                      const WorldPos& velocity,
                                                      const std::vector<WorldPos>& obstacles,
                                                      float avoidanceRadius) {
    WorldPos steer(0.0f, 0.0f);

    for (const WorldPos& obstacle : obstacles) {
        float distance = position.DistanceTo(obstacle);
        if (distance < avoidanceRadius) {
            WorldPos diff(position.x - obstacle.x, position.y - obstacle.y);
            float length = Length(diff);
            if (length > 0.0f) {
                diff.x /= length;
                diff.y /= length;
                // Weight by distance
                float weight = (avoidanceRadius - distance) / avoidanceRadius;
                diff.x *= weight;
                diff.y *= weight;
                steer.x += diff.x;
                steer.y += diff.y;
            }
        }
    }

    return steer;
}

WorldPos FlockingBehavior::Limit(const WorldPos& vector, float maxLength) {
    float length = Length(vector);
    if (length > maxLength && length > 0.0f) {
        WorldPos result;
        result.x = (vector.x / length) * maxLength;
        result.y = (vector.y / length) * maxLength;
        return result;
    }
    return vector;
}

WorldPos FlockingBehavior::Normalize(const WorldPos& vector) {
    float length = Length(vector);
    if (length > 0.0f) {
        WorldPos result;
        result.x = vector.x / length;
        result.y = vector.y / length;
        return result;
    }
    return WorldPos(0.0f, 0.0f);
}

float FlockingBehavior::Length(const WorldPos& vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

} // namespace AI
