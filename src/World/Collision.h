#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>

namespace World {

// Forward declaration
class TileMap;

// Collision shape types
enum class CollisionShape {
    Rectangle,
    Circle,
    Polygon,
    Line
};

// Axis-Aligned Bounding Box
struct AABB {
    float x, y;
    float width, height;

    AABB() : x(0), y(0), width(0), height(0) {}
    AABB(float _x, float _y, float _w, float _h)
        : x(_x), y(_y), width(_w), height(_h) {}

    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
    float centerX() const { return x + width / 2.0f; }
    float centerY() const { return y + height / 2.0f; }

    bool intersects(const AABB& other) const {
        return !(right() <= other.left() ||
                 other.right() <= left() ||
                 bottom() <= other.top() ||
                 other.bottom() <= top());
    }

    bool contains(float px, float py) const {
        return px >= x && px <= right() && py >= y && py <= bottom();
    }

    AABB expand(float amount) const {
        return AABB(x - amount, y - amount,
                   width + amount * 2, height + amount * 2);
    }
};

// Circle collision shape
struct Circle {
    float x, y;
    float radius;

    Circle() : x(0), y(0), radius(0) {}
    Circle(float _x, float _y, float _r) : x(_x), y(_y), radius(_r) {}

    AABB getAABB() const {
        return AABB(x - radius, y - radius, radius * 2, radius * 2);
    }
};

// Polygon collision shape
struct Polygon {
    std::vector<float> vertices; // x1, y1, x2, y2, ...
    float x, y; // Position offset

    Polygon() : x(0), y(0) {}

    AABB getAABB() const;
    void transform(float offsetX, float offsetY);
};

// Collision object
class CollisionObject {
public:
    CollisionObject(int id, CollisionShape shape);
    ~CollisionObject();

    // ID and type
    int getID() const { return m_id; }
    void setLayer(int layer) { m_layer = layer; }
    int getLayer() const { return m_layer; }
    void setCollisionMask(unsigned int mask) { m_collisionMask = mask; }
    unsigned int getCollisionMask() const { return m_collisionMask; }

    // Shape
    CollisionShape getShape() const { return m_shape; }
    void setRectangle(float x, float y, float width, float height);
    void setCircle(float x, float y, float radius);
    void setPolygon(const std::vector<float>& vertices, float x, float y);

    // Properties
    void setStatic(bool isStatic) { m_static = isStatic; }
    bool isStatic() const { return m_static; }
    void setTrigger(bool isTrigger) { m_trigger = isTrigger; }
    bool isTrigger() const { return m_trigger; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Bounds
    AABB getAABB() const;
    void updateAABB();

    // Collision data
    const AABB& getRectangle() const { return m_rectangle; }
    const Circle& getCircle() const { return m_circle; }
    const Polygon& getPolygon() const { return m_polygon; }

    // User data
    void setUserData(void* data) { m_userData = data; }
    void* getUserData() const { return m_userData; }

private:
    int m_id;
    CollisionShape m_shape;
    int m_layer;
    unsigned int m_collisionMask;
    bool m_static;
    bool m_trigger;
    bool m_enabled;

    AABB m_rectangle;
    Circle m_circle;
    Polygon m_polygon;
    AABB m_cachedAABB;

    void* m_userData;
};

// Collision result
struct CollisionResult {
    CollisionObject* object;
    float penetrationX;
    float penetrationY;
    float normalX;
    float normalY;
    bool isTrigger;

    CollisionResult()
        : object(nullptr)
        , penetrationX(0)
        , penetrationY(0)
        , normalX(0)
        , normalY(0)
        , isTrigger(false)
    {}
};

// Spatial grid cell for partitioning
class SpatialCell {
public:
    void addObject(CollisionObject* obj);
    void removeObject(CollisionObject* obj);
    const std::unordered_set<CollisionObject*>& getObjects() const { return m_objects; }
    void clear() { m_objects.clear(); }

private:
    std::unordered_set<CollisionObject*> m_objects;
};

// Spatial hash grid for broad phase collision detection
class SpatialGrid {
public:
    SpatialGrid(float cellSize);
    ~SpatialGrid();

    void insert(CollisionObject* obj);
    void remove(CollisionObject* obj);
    void update(CollisionObject* obj);
    void clear();

    std::vector<CollisionObject*> query(const AABB& bounds) const;
    std::vector<CollisionObject*> queryRadius(float x, float y, float radius) const;

    void setCellSize(float size) { m_cellSize = size; rebuildGrid(); }
    float getCellSize() const { return m_cellSize; }

    // Statistics
    int getCellCount() const { return m_cells.size(); }
    int getObjectCount() const { return m_objectCells.size(); }

private:
    float m_cellSize;
    std::unordered_map<std::string, std::unique_ptr<SpatialCell>> m_cells;
    std::unordered_map<CollisionObject*, std::vector<std::string>> m_objectCells;

    std::string getCellKey(int cellX, int cellY) const;
    void getCellsForAABB(const AABB& aabb, std::vector<std::string>& outCells) const;
    void rebuildGrid();
};

// Quadtree node for hierarchical spatial partitioning
class QuadTreeNode {
public:
    QuadTreeNode(const AABB& bounds, int depth, int maxDepth, int maxObjects);
    ~QuadTreeNode();

    void insert(CollisionObject* obj);
    void remove(CollisionObject* obj);
    void query(const AABB& bounds, std::vector<CollisionObject*>& results) const;
    void clear();

    bool isLeaf() const { return m_children[0] == nullptr; }
    const AABB& getBounds() const { return m_bounds; }

private:
    AABB m_bounds;
    int m_depth;
    int m_maxDepth;
    int m_maxObjects;

    std::vector<CollisionObject*> m_objects;
    std::unique_ptr<QuadTreeNode> m_children[4];

    void subdivide();
    int getQuadrant(const AABB& objBounds) const;
};

// Main collision system
class CollisionSystem {
public:
    enum class BroadPhase {
        SpatialGrid,
        QuadTree,
        BruteForce
    };

    CollisionSystem();
    ~CollisionSystem();

    // Initialization
    bool initialize(float worldWidth, float worldHeight);
    void shutdown();

    // Configuration
    void setBroadPhaseAlgorithm(BroadPhase algorithm);
    BroadPhase getBroadPhaseAlgorithm() const { return m_broadPhase; }
    void setCellSize(float size);

    // Object management
    CollisionObject* createObject(CollisionShape shape);
    void destroyObject(int objectID);
    void destroyObject(CollisionObject* obj);
    CollisionObject* getObject(int objectID);
    void updateObject(CollisionObject* obj);

    // Collision queries
    std::vector<CollisionResult> checkCollisions(CollisionObject* obj);
    std::vector<CollisionResult> checkCollisionsInArea(const AABB& area);
    bool checkLine(float x1, float y1, float x2, float y2,
                  std::vector<CollisionResult>& results);
    bool checkPoint(float x, float y, std::vector<CollisionResult>& results);
    bool checkCircle(float x, float y, float radius,
                    std::vector<CollisionResult>& results);

    // Raycasting
    struct RaycastResult {
        CollisionObject* object;
        float hitX, hitY;
        float normalX, normalY;
        float distance;
        float fraction; // 0.0 to 1.0

        RaycastResult() : object(nullptr), hitX(0), hitY(0),
                         normalX(0), normalY(0), distance(0), fraction(0) {}
    };
    bool raycast(float startX, float startY, float endX, float endY,
                RaycastResult& result);
    std::vector<RaycastResult> raycastAll(float startX, float startY,
                                         float endX, float endY);

    // Tilemap collision
    void setTileMap(TileMap* tilemap) { m_tilemap = tilemap; }
    TileMap* getTileMap() const { return m_tilemap; }
    bool checkTileCollision(float x, float y, float width, float height);
    std::vector<std::pair<int, int>> getTilesInArea(float x, float y,
                                                    float width, float height);

    // Collision resolution
    void resolveCollision(CollisionObject* obj, const CollisionResult& result);
    void resolveTileCollision(float& x, float& y, float width, float height,
                             float velX, float velY);

    // Update
    void update(float deltaTime);

    // Callbacks
    using CollisionCallback = std::function<void(CollisionObject*, CollisionObject*)>;
    void setCollisionCallback(CollisionCallback callback) { m_collisionCallback = callback; }
    void setTriggerCallback(CollisionCallback callback) { m_triggerCallback = callback; }

    // Debug
    void setDebugDraw(bool enable) { m_debugDraw = enable; }
    bool isDebugDrawEnabled() const { return m_debugDraw; }
    void renderDebug();

    // Statistics
    struct Stats {
        int totalObjects;
        int staticObjects;
        int dynamicObjects;
        int collisionChecks;
        int collisionsDetected;
        float broadPhaseTime;
        float narrowPhaseTime;
    };
    const Stats& getStats() const { return m_stats; }

private:
    BroadPhase m_broadPhase;
    std::unique_ptr<SpatialGrid> m_spatialGrid;
    std::unique_ptr<QuadTreeNode> m_quadTree;

    std::unordered_map<int, std::unique_ptr<CollisionObject>> m_objects;
    int m_nextObjectID;

    TileMap* m_tilemap;
    float m_worldWidth;
    float m_worldHeight;

    CollisionCallback m_collisionCallback;
    CollisionCallback m_triggerCallback;

    bool m_debugDraw;
    Stats m_stats;

    // Narrow phase collision detection
    bool checkAABBvsAABB(const AABB& a, const AABB& b, CollisionResult& result);
    bool checkCirclevsCircle(const Circle& a, const Circle& b, CollisionResult& result);
    bool checkAABBvsCircle(const AABB& rect, const Circle& circle, CollisionResult& result);
    bool checkPolygonvsPolygon(const Polygon& a, const Polygon& b, CollisionResult& result);

    // SAT (Separating Axis Theorem) for polygon collision
    bool checkSAT(const Polygon& a, const Polygon& b, CollisionResult& result);

    // Broad phase queries
    std::vector<CollisionObject*> broadPhaseQuery(const AABB& bounds);

    // Helper methods
    void rebuildBroadPhase();
};

// Collision layers utility
class CollisionLayers {
public:
    static constexpr unsigned int LAYER_WORLD = 1 << 0;
    static constexpr unsigned int LAYER_PLAYER = 1 << 1;
    static constexpr unsigned int LAYER_ENEMY = 1 << 2;
    static constexpr unsigned int LAYER_PROJECTILE = 1 << 3;
    static constexpr unsigned int LAYER_TRIGGER = 1 << 4;
    static constexpr unsigned int LAYER_ITEM = 1 << 5;
    static constexpr unsigned int LAYER_NPC = 1 << 6;
    static constexpr unsigned int LAYER_DESTRUCTIBLE = 1 << 7;

    static bool shouldCollide(unsigned int maskA, unsigned int layerB) {
        return (maskA & layerB) != 0;
    }
};

} // namespace World
