#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <limits>

namespace AI {

// Grid cell types
enum class CellType {
    WALKABLE,
    BLOCKED,
    DIFFICULT_TERRAIN, // Slower movement
    WATER,
    HAZARD
};

// Position in the grid
struct GridPos {
    int x;
    int y;

    GridPos() : x(0), y(0) {}
    GridPos(int _x, int _y) : x(_x), y(_y) {}

    bool operator==(const GridPos& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const GridPos& other) const {
        return !(*this == other);
    }

    // For use in hash maps
    struct Hash {
        size_t operator()(const GridPos& pos) const {
            return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.y) << 1);
        }
    };
};

// World position (floating point)
struct WorldPos {
    float x;
    float y;

    WorldPos() : x(0.0f), y(0.0f) {}
    WorldPos(float _x, float _y) : x(_x), y(_y) {}

    float DistanceTo(const WorldPos& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// Pathfinding request
struct PathRequest {
    GridPos start;
    GridPos goal;
    bool allowDiagonal;
    bool smoothPath;
    float maxCost; // Maximum cost to allow (prevents pathfinding through too much difficult terrain)

    PathRequest()
        : allowDiagonal(true)
        , smoothPath(true)
        , maxCost(std::numeric_limits<float>::max())
    {}
};

// Path result
struct PathResult {
    std::vector<GridPos> gridPath;
    std::vector<WorldPos> worldPath;
    float totalCost;
    bool success;

    PathResult() : totalCost(0.0f), success(false) {}
};

// Pathfinding grid cell
struct GridCell {
    CellType type;
    float movementCost; // Cost multiplier for this cell

    GridCell()
        : type(CellType::WALKABLE)
        , movementCost(1.0f)
    {}

    GridCell(CellType t, float cost = 1.0f)
        : type(t)
        , movementCost(cost)
    {}

    bool IsWalkable() const {
        return type != CellType::BLOCKED;
    }
};

// A* pathfinding node
struct AStarNode {
    GridPos pos;
    float g; // Cost from start
    float h; // Heuristic cost to goal
    float f; // Total cost (g + h)
    GridPos parent;
    bool hasParent;

    AStarNode()
        : g(0.0f), h(0.0f), f(0.0f), hasParent(false)
    {}

    AStarNode(const GridPos& position)
        : pos(position), g(0.0f), h(0.0f), f(0.0f), hasParent(false)
    {}
};

// Comparison for priority queue (min-heap based on f value)
struct AStarNodeCompare {
    bool operator()(const AStarNode& a, const AStarNode& b) const {
        return a.f > b.f; // Greater than for min-heap
    }
};

/**
 * Pathfinding - A* pathfinding system with grid-based navigation
 */
class Pathfinding {
public:
    Pathfinding();
    ~Pathfinding();

    // Initialization
    void Initialize(int width, int height, float cellSize = 1.0f);
    void Shutdown();

    // Grid management
    void SetCellType(int x, int y, CellType type);
    void SetCellType(const GridPos& pos, CellType type);
    CellType GetCellType(int x, int y) const;
    CellType GetCellType(const GridPos& pos) const;

    void SetCellMovementCost(int x, int y, float cost);
    void SetCellMovementCost(const GridPos& pos, float cost);
    float GetCellMovementCost(int x, int y) const;
    float GetCellMovementCost(const GridPos& pos) const;

    // Bulk updates
    void SetArea(int x1, int y1, int x2, int y2, CellType type);
    void SetCircle(int centerX, int centerY, int radius, CellType type);

    // Pathfinding
    PathResult FindPath(const GridPos& start, const GridPos& goal,
                       bool allowDiagonal = true, bool smoothPath = true);
    PathResult FindPath(const WorldPos& start, const WorldPos& goal,
                       bool allowDiagonal = true, bool smoothPath = true);
    PathResult FindPath(const PathRequest& request);

    // Conversion between grid and world coordinates
    GridPos WorldToGrid(const WorldPos& worldPos) const;
    WorldPos GridToWorld(const GridPos& gridPos) const;

    // Queries
    bool IsWalkable(int x, int y) const;
    bool IsWalkable(const GridPos& pos) const;
    bool IsInBounds(int x, int y) const;
    bool IsInBounds(const GridPos& pos) const;

    // Line of sight check
    bool HasLineOfSight(const GridPos& start, const GridPos& end) const;
    bool HasLineOfSight(const WorldPos& start, const WorldPos& end) const;

    // Nearest walkable position
    GridPos FindNearestWalkable(const GridPos& pos, int maxRadius = 10) const;

    // Flocking and group movement helpers
    WorldPos GetFlockingAvoidance(const WorldPos& currentPos, float radius,
                                  const std::vector<WorldPos>& neighbors) const;
    WorldPos GetSeparationVector(const WorldPos& currentPos,
                                const std::vector<WorldPos>& neighbors,
                                float separationRadius) const;

    // Grid properties
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetCellSize() const { return m_cellSize; }

    // Debug
    void EnableDebugDrawing(bool enable) { m_debugDraw = enable; }
    bool IsDebugDrawingEnabled() const { return m_debugDraw; }

private:
    // Grid data
    std::vector<std::vector<GridCell>> m_grid;
    int m_width;
    int m_height;
    float m_cellSize;

    // Debug
    bool m_debugDraw;

    // A* algorithm implementation
    PathResult AStar(const PathRequest& request);

    // Heuristic functions
    float CalculateHeuristic(const GridPos& a, const GridPos& b, bool allowDiagonal) const;
    float CalculateMovementCost(const GridPos& from, const GridPos& to) const;

    // Path smoothing
    void SmoothPath(PathResult& result) const;

    // Get neighbors
    std::vector<GridPos> GetNeighbors(const GridPos& pos, bool allowDiagonal) const;

    // Helper to get grid cell
    GridCell* GetCell(int x, int y);
    const GridCell* GetCell(int x, int y) const;
    GridCell* GetCell(const GridPos& pos);
    const GridCell* GetCell(const GridPos& pos) const;

    // Path reconstruction
    void ReconstructPath(const std::unordered_map<GridPos, GridPos, GridPos::Hash>& cameFrom,
                        const GridPos& start, const GridPos& goal,
                        PathResult& result) const;
};

/**
 * FlockingBehavior - Implements flocking behaviors for group movement
 */
class FlockingBehavior {
public:
    struct FlockingParams {
        float separationRadius = 2.0f;
        float alignmentRadius = 5.0f;
        float cohesionRadius = 5.0f;
        float separationWeight = 1.5f;
        float alignmentWeight = 1.0f;
        float cohesionWeight = 1.0f;
        float maxSpeed = 5.0f;
        float maxForce = 0.5f;
    };

    static WorldPos CalculateSeparation(const WorldPos& position,
                                       const std::vector<WorldPos>& neighbors,
                                       const FlockingParams& params);

    static WorldPos CalculateAlignment(const WorldPos& velocity,
                                      const std::vector<WorldPos>& neighborVelocities,
                                      const FlockingParams& params);

    static WorldPos CalculateCohesion(const WorldPos& position,
                                     const std::vector<WorldPos>& neighbors,
                                     const FlockingParams& params);

    static WorldPos CalculateFlocking(const WorldPos& position,
                                     const WorldPos& velocity,
                                     const std::vector<WorldPos>& neighbors,
                                     const std::vector<WorldPos>& neighborVelocities,
                                     const FlockingParams& params);

    // Obstacle avoidance
    static WorldPos CalculateObstacleAvoidance(const WorldPos& position,
                                              const WorldPos& velocity,
                                              const std::vector<WorldPos>& obstacles,
                                              float avoidanceRadius);

private:
    static WorldPos Limit(const WorldPos& vector, float maxLength);
    static WorldPos Normalize(const WorldPos& vector);
    static float Length(const WorldPos& vector);
};

} // namespace AI
