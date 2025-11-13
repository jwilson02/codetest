#include "Collision.h"
#include "TileMap.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace World {

// ==================== Polygon Implementation ====================

AABB Polygon::getAABB() const {
    if (vertices.empty()) {
        return AABB();
    }

    float minX = vertices[0] + x;
    float minY = vertices[1] + y;
    float maxX = minX;
    float maxY = minY;

    for (size_t i = 0; i < vertices.size(); i += 2) {
        float vx = vertices[i] + x;
        float vy = vertices[i + 1] + y;
        minX = std::min(minX, vx);
        minY = std::min(minY, vy);
        maxX = std::max(maxX, vx);
        maxY = std::max(maxY, vy);
    }

    return AABB(minX, minY, maxX - minX, maxY - minY);
}

void Polygon::transform(float offsetX, float offsetY) {
    x += offsetX;
    y += offsetY;
}

// ==================== CollisionObject Implementation ====================

CollisionObject::CollisionObject(int id, CollisionShape shape)
    : m_id(id)
    , m_shape(shape)
    , m_layer(0)
    , m_collisionMask(0xFFFFFFFF)
    , m_static(false)
    , m_trigger(false)
    , m_enabled(true)
    , m_userData(nullptr)
{
}

CollisionObject::~CollisionObject() {
}

void CollisionObject::setRectangle(float x, float y, float width, float height) {
    m_shape = CollisionShape::Rectangle;
    m_rectangle = AABB(x, y, width, height);
    updateAABB();
}

void CollisionObject::setCircle(float x, float y, float radius) {
    m_shape = CollisionShape::Circle;
    m_circle = Circle(x, y, radius);
    updateAABB();
}

void CollisionObject::setPolygon(const std::vector<float>& vertices, float x, float y) {
    m_shape = CollisionShape::Polygon;
    m_polygon.vertices = vertices;
    m_polygon.x = x;
    m_polygon.y = y;
    updateAABB();
}

AABB CollisionObject::getAABB() const {
    return m_cachedAABB;
}

void CollisionObject::updateAABB() {
    switch (m_shape) {
        case CollisionShape::Rectangle:
            m_cachedAABB = m_rectangle;
            break;
        case CollisionShape::Circle:
            m_cachedAABB = m_circle.getAABB();
            break;
        case CollisionShape::Polygon:
            m_cachedAABB = m_polygon.getAABB();
            break;
        default:
            m_cachedAABB = AABB();
            break;
    }
}

// ==================== SpatialCell Implementation ====================

void SpatialCell::addObject(CollisionObject* obj) {
    m_objects.insert(obj);
}

void SpatialCell::removeObject(CollisionObject* obj) {
    m_objects.erase(obj);
}

// ==================== SpatialGrid Implementation ====================

SpatialGrid::SpatialGrid(float cellSize)
    : m_cellSize(cellSize)
{
}

SpatialGrid::~SpatialGrid() {
    clear();
}

void SpatialGrid::insert(CollisionObject* obj) {
    if (!obj || !obj->isEnabled()) return;

    AABB bounds = obj->getAABB();
    std::vector<std::string> cells;
    getCellsForAABB(bounds, cells);

    for (const auto& cellKey : cells) {
        auto it = m_cells.find(cellKey);
        if (it == m_cells.end()) {
            m_cells[cellKey] = std::make_unique<SpatialCell>();
        }
        m_cells[cellKey]->addObject(obj);
    }

    m_objectCells[obj] = cells;
}

void SpatialGrid::remove(CollisionObject* obj) {
    auto it = m_objectCells.find(obj);
    if (it == m_objectCells.end()) return;

    for (const auto& cellKey : it->second) {
        auto cellIt = m_cells.find(cellKey);
        if (cellIt != m_cells.end()) {
            cellIt->second->removeObject(obj);
        }
    }

    m_objectCells.erase(it);
}

void SpatialGrid::update(CollisionObject* obj) {
    remove(obj);
    insert(obj);
}

void SpatialGrid::clear() {
    m_cells.clear();
    m_objectCells.clear();
}

std::vector<CollisionObject*> SpatialGrid::query(const AABB& bounds) const {
    std::unordered_set<CollisionObject*> resultSet;
    std::vector<std::string> cells;
    getCellsForAABB(bounds, cells);

    for (const auto& cellKey : cells) {
        auto it = m_cells.find(cellKey);
        if (it != m_cells.end()) {
            const auto& objects = it->second->getObjects();
            resultSet.insert(objects.begin(), objects.end());
        }
    }

    return std::vector<CollisionObject*>(resultSet.begin(), resultSet.end());
}

std::vector<CollisionObject*> SpatialGrid::queryRadius(float x, float y, float radius) const {
    AABB bounds(x - radius, y - radius, radius * 2, radius * 2);
    return query(bounds);
}

std::string SpatialGrid::getCellKey(int cellX, int cellY) const {
    return std::to_string(cellX) + ":" + std::to_string(cellY);
}

void SpatialGrid::getCellsForAABB(const AABB& aabb, std::vector<std::string>& outCells) const {
    int minCellX = static_cast<int>(std::floor(aabb.left() / m_cellSize));
    int minCellY = static_cast<int>(std::floor(aabb.top() / m_cellSize));
    int maxCellX = static_cast<int>(std::floor(aabb.right() / m_cellSize));
    int maxCellY = static_cast<int>(std::floor(aabb.bottom() / m_cellSize));

    for (int cy = minCellY; cy <= maxCellY; ++cy) {
        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            outCells.push_back(getCellKey(cx, cy));
        }
    }
}

void SpatialGrid::rebuildGrid() {
    std::vector<CollisionObject*> allObjects;
    for (auto& pair : m_objectCells) {
        allObjects.push_back(pair.first);
    }

    clear();

    for (auto obj : allObjects) {
        insert(obj);
    }
}

// ==================== QuadTreeNode Implementation ====================

QuadTreeNode::QuadTreeNode(const AABB& bounds, int depth, int maxDepth, int maxObjects)
    : m_bounds(bounds)
    , m_depth(depth)
    , m_maxDepth(maxDepth)
    , m_maxObjects(maxObjects)
{
    m_children[0] = nullptr;
    m_children[1] = nullptr;
    m_children[2] = nullptr;
    m_children[3] = nullptr;
}

QuadTreeNode::~QuadTreeNode() {
    clear();
}

void QuadTreeNode::insert(CollisionObject* obj) {
    if (!isLeaf()) {
        int quadrant = getQuadrant(obj->getAABB());
        if (quadrant != -1) {
            m_children[quadrant]->insert(obj);
            return;
        }
    }

    m_objects.push_back(obj);

    if (isLeaf() && m_depth < m_maxDepth &&
        static_cast<int>(m_objects.size()) > m_maxObjects) {
        subdivide();
    }
}

void QuadTreeNode::remove(CollisionObject* obj) {
    auto it = std::find(m_objects.begin(), m_objects.end(), obj);
    if (it != m_objects.end()) {
        m_objects.erase(it);
        return;
    }

    if (!isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (m_children[i]) {
                m_children[i]->remove(obj);
            }
        }
    }
}

void QuadTreeNode::query(const AABB& bounds, std::vector<CollisionObject*>& results) const {
    if (!m_bounds.intersects(bounds)) {
        return;
    }

    for (auto obj : m_objects) {
        if (obj->getAABB().intersects(bounds)) {
            results.push_back(obj);
        }
    }

    if (!isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (m_children[i]) {
                m_children[i]->query(bounds, results);
            }
        }
    }
}

void QuadTreeNode::clear() {
    m_objects.clear();
    for (int i = 0; i < 4; ++i) {
        m_children[i].reset();
    }
}

void QuadTreeNode::subdivide() {
    float halfWidth = m_bounds.width / 2.0f;
    float halfHeight = m_bounds.height / 2.0f;
    float x = m_bounds.x;
    float y = m_bounds.y;

    // Top-left, top-right, bottom-left, bottom-right
    m_children[0] = std::make_unique<QuadTreeNode>(
        AABB(x, y, halfWidth, halfHeight),
        m_depth + 1, m_maxDepth, m_maxObjects);

    m_children[1] = std::make_unique<QuadTreeNode>(
        AABB(x + halfWidth, y, halfWidth, halfHeight),
        m_depth + 1, m_maxDepth, m_maxObjects);

    m_children[2] = std::make_unique<QuadTreeNode>(
        AABB(x, y + halfHeight, halfWidth, halfHeight),
        m_depth + 1, m_maxDepth, m_maxObjects);

    m_children[3] = std::make_unique<QuadTreeNode>(
        AABB(x + halfWidth, y + halfHeight, halfWidth, halfHeight),
        m_depth + 1, m_maxDepth, m_maxObjects);

    // Redistribute objects
    std::vector<CollisionObject*> objectsCopy = m_objects;
    m_objects.clear();

    for (auto obj : objectsCopy) {
        int quadrant = getQuadrant(obj->getAABB());
        if (quadrant != -1) {
            m_children[quadrant]->insert(obj);
        } else {
            m_objects.push_back(obj);
        }
    }
}

int QuadTreeNode::getQuadrant(const AABB& objBounds) const {
    float midX = m_bounds.centerX();
    float midY = m_bounds.centerY();

    bool topHalf = objBounds.bottom() < midY;
    bool bottomHalf = objBounds.top() > midY;
    bool leftHalf = objBounds.right() < midX;
    bool rightHalf = objBounds.left() > midX;

    if (topHalf && leftHalf) return 0;
    if (topHalf && rightHalf) return 1;
    if (bottomHalf && leftHalf) return 2;
    if (bottomHalf && rightHalf) return 3;

    return -1; // Spans multiple quadrants
}

// ==================== CollisionSystem Implementation ====================

CollisionSystem::CollisionSystem()
    : m_broadPhase(BroadPhase::SpatialGrid)
    , m_nextObjectID(1)
    , m_tilemap(nullptr)
    , m_worldWidth(10000.0f)
    , m_worldHeight(10000.0f)
    , m_debugDraw(false)
{
    m_spatialGrid = std::make_unique<SpatialGrid>(128.0f);
}

CollisionSystem::~CollisionSystem() {
    shutdown();
}

bool CollisionSystem::initialize(float worldWidth, float worldHeight) {
    m_worldWidth = worldWidth;
    m_worldHeight = worldHeight;

    rebuildBroadPhase();
    return true;
}

void CollisionSystem::shutdown() {
    m_objects.clear();
    if (m_spatialGrid) {
        m_spatialGrid->clear();
    }
    if (m_quadTree) {
        m_quadTree->clear();
    }
}

void CollisionSystem::setBroadPhaseAlgorithm(BroadPhase algorithm) {
    if (m_broadPhase != algorithm) {
        m_broadPhase = algorithm;
        rebuildBroadPhase();
    }
}

void CollisionSystem::setCellSize(float size) {
    if (m_spatialGrid) {
        m_spatialGrid->setCellSize(size);
    }
}

CollisionObject* CollisionSystem::createObject(CollisionShape shape) {
    int id = m_nextObjectID++;
    auto obj = std::make_unique<CollisionObject>(id, shape);
    CollisionObject* objPtr = obj.get();
    m_objects[id] = std::move(obj);

    return objPtr;
}

void CollisionSystem::destroyObject(int objectID) {
    auto it = m_objects.find(objectID);
    if (it != m_objects.end()) {
        CollisionObject* obj = it->second.get();

        if (m_spatialGrid) {
            m_spatialGrid->remove(obj);
        }
        if (m_quadTree) {
            m_quadTree->remove(obj);
        }

        m_objects.erase(it);
    }
}

void CollisionSystem::destroyObject(CollisionObject* obj) {
    if (obj) {
        destroyObject(obj->getID());
    }
}

CollisionObject* CollisionSystem::getObject(int objectID) {
    auto it = m_objects.find(objectID);
    if (it != m_objects.end()) {
        return it->second.get();
    }
    return nullptr;
}

void CollisionSystem::updateObject(CollisionObject* obj) {
    if (!obj) return;

    obj->updateAABB();

    if (m_spatialGrid) {
        m_spatialGrid->update(obj);
    }
    if (m_quadTree) {
        m_quadTree->remove(obj);
        m_quadTree->insert(obj);
    }
}

std::vector<CollisionResult> CollisionSystem::checkCollisions(CollisionObject* obj) {
    std::vector<CollisionResult> results;

    if (!obj || !obj->isEnabled()) {
        return results;
    }

    // Broad phase
    auto candidates = broadPhaseQuery(obj->getAABB().expand(10.0f));

    m_stats.collisionChecks = 0;

    // Narrow phase
    for (auto other : candidates) {
        if (other == obj || !other->isEnabled()) continue;
        if (!CollisionLayers::shouldCollide(obj->getCollisionMask(), other->getLayer())) {
            continue;
        }

        m_stats.collisionChecks++;

        CollisionResult result;
        result.object = other;
        result.isTrigger = other->isTrigger();

        bool collided = false;

        // Check based on shape types
        if (obj->getShape() == CollisionShape::Rectangle &&
            other->getShape() == CollisionShape::Rectangle) {
            collided = checkAABBvsAABB(obj->getRectangle(), other->getRectangle(), result);
        } else if (obj->getShape() == CollisionShape::Circle &&
                   other->getShape() == CollisionShape::Circle) {
            collided = checkCirclevsCircle(obj->getCircle(), other->getCircle(), result);
        } else if (obj->getShape() == CollisionShape::Rectangle &&
                   other->getShape() == CollisionShape::Circle) {
            collided = checkAABBvsCircle(obj->getRectangle(), other->getCircle(), result);
        } else if (obj->getShape() == CollisionShape::Circle &&
                   other->getShape() == CollisionShape::Rectangle) {
            collided = checkAABBvsCircle(other->getRectangle(), obj->getCircle(), result);
            // Flip normals
            result.normalX = -result.normalX;
            result.normalY = -result.normalY;
        }

        if (collided) {
            results.push_back(result);
            m_stats.collisionsDetected++;

            // Trigger callbacks
            if (result.isTrigger && m_triggerCallback) {
                m_triggerCallback(obj, other);
            } else if (!result.isTrigger && m_collisionCallback) {
                m_collisionCallback(obj, other);
            }
        }
    }

    return results;
}

std::vector<CollisionResult> CollisionSystem::checkCollisionsInArea(const AABB& area) {
    std::vector<CollisionResult> results;
    auto objects = broadPhaseQuery(area);

    for (auto obj : objects) {
        if (obj->getAABB().intersects(area)) {
            CollisionResult result;
            result.object = obj;
            results.push_back(result);
        }
    }

    return results;
}

bool CollisionSystem::checkLine(float x1, float y1, float x2, float y2,
                                std::vector<CollisionResult>& results) {
    // Simple line collision check
    float minX = std::min(x1, x2);
    float minY = std::min(y1, y2);
    float maxX = std::max(x1, x2);
    float maxY = std::max(y1, y2);

    AABB lineBounds(minX, minY, maxX - minX, maxY - minY);
    auto objects = broadPhaseQuery(lineBounds);

    for (auto obj : objects) {
        // Simplified line intersection test
        CollisionResult result;
        result.object = obj;
        results.push_back(result);
    }

    return !results.empty();
}

bool CollisionSystem::checkPoint(float x, float y, std::vector<CollisionResult>& results) {
    AABB pointBounds(x - 1, y - 1, 2, 2);
    auto objects = broadPhaseQuery(pointBounds);

    for (auto obj : objects) {
        if (obj->getAABB().contains(x, y)) {
            CollisionResult result;
            result.object = obj;
            results.push_back(result);
        }
    }

    return !results.empty();
}

bool CollisionSystem::checkCircle(float x, float y, float radius,
                                 std::vector<CollisionResult>& results) {
    AABB circleBounds(x - radius, y - radius, radius * 2, radius * 2);
    return checkCollisionsInArea(circleBounds).size() > 0;
}

bool CollisionSystem::raycast(float startX, float startY, float endX, float endY,
                              RaycastResult& result) {
    auto results = raycastAll(startX, startY, endX, endY);
    if (!results.empty()) {
        result = results[0];
        return true;
    }
    return false;
}

std::vector<CollisionSystem::RaycastResult> CollisionSystem::raycastAll(
    float startX, float startY, float endX, float endY) {

    std::vector<RaycastResult> results;

    // Get all objects along the ray
    float minX = std::min(startX, endX);
    float minY = std::min(startY, endY);
    float maxX = std::max(startX, endX);
    float maxY = std::max(startY, endY);

    AABB rayBounds(minX, minY, maxX - minX, maxY - minY);
    auto objects = broadPhaseQuery(rayBounds);

    // TODO: Implement proper ray-shape intersection
    // For now, just check AABB intersections

    return results;
}

bool CollisionSystem::checkTileCollision(float x, float y, float width, float height) {
    if (!m_tilemap) return false;

    AABB bounds(x, y, width, height);
    int tileWidth = m_tilemap->getTileWidth();
    int tileHeight = m_tilemap->getTileHeight();

    int startTileX = static_cast<int>(bounds.left() / tileWidth);
    int startTileY = static_cast<int>(bounds.top() / tileHeight);
    int endTileX = static_cast<int>(bounds.right() / tileWidth);
    int endTileY = static_cast<int>(bounds.bottom() / tileHeight);

    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            if (m_tilemap->isSolid(tx, ty)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::pair<int, int>> CollisionSystem::getTilesInArea(
    float x, float y, float width, float height) {

    std::vector<std::pair<int, int>> tiles;

    if (!m_tilemap) return tiles;

    int tileWidth = m_tilemap->getTileWidth();
    int tileHeight = m_tilemap->getTileHeight();

    int startTileX = static_cast<int>(x / tileWidth);
    int startTileY = static_cast<int>(y / tileHeight);
    int endTileX = static_cast<int>((x + width) / tileWidth);
    int endTileY = static_cast<int>((y + height) / tileHeight);

    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            tiles.push_back({tx, ty});
        }
    }

    return tiles;
}

void CollisionSystem::resolveCollision(CollisionObject* obj, const CollisionResult& result) {
    if (!obj || obj->isTrigger() || result.isTrigger) return;

    // Simple push-out resolution
    if (obj->getShape() == CollisionShape::Rectangle) {
        AABB rect = obj->getRectangle();
        rect.x += result.normalX * result.penetrationX;
        rect.y += result.normalY * result.penetrationY;
        obj->setRectangle(rect.x, rect.y, rect.width, rect.height);
    }
}

void CollisionSystem::resolveTileCollision(float& x, float& y, float width, float height,
                                          float velX, float velY) {
    if (!m_tilemap) return;

    AABB bounds(x, y, width, height);
    auto tiles = getTilesInArea(x, y, width, height);

    for (const auto& tile : tiles) {
        if (m_tilemap->isSolid(tile.first, tile.second)) {
            int tileWidth = m_tilemap->getTileWidth();
            int tileHeight = m_tilemap->getTileHeight();

            AABB tileAABB(tile.first * tileWidth, tile.second * tileHeight,
                         tileWidth, tileHeight);

            // Simple axis-aligned resolution
            if (velX > 0) {
                x = tileAABB.left() - width;
            } else if (velX < 0) {
                x = tileAABB.right();
            }

            if (velY > 0) {
                y = tileAABB.top() - height;
            } else if (velY < 0) {
                y = tileAABB.bottom();
            }
        }
    }
}

void CollisionSystem::update(float deltaTime) {
    m_stats = Stats();
    m_stats.totalObjects = m_objects.size();

    for (auto& pair : m_objects) {
        if (pair.second->isStatic()) {
            m_stats.staticObjects++;
        } else {
            m_stats.dynamicObjects++;
        }
    }
}

void CollisionSystem::renderDebug() {
    // Debug rendering would integrate with your rendering system
    // Draw collision shapes, spatial grid cells, etc.
}

bool CollisionSystem::checkAABBvsAABB(const AABB& a, const AABB& b, CollisionResult& result) {
    if (!a.intersects(b)) {
        return false;
    }

    // Calculate penetration
    float leftPen = a.right() - b.left();
    float rightPen = b.right() - a.left();
    float topPen = a.bottom() - b.top();
    float bottomPen = b.bottom() - a.top();

    float minPen = std::min({leftPen, rightPen, topPen, bottomPen});

    if (minPen == leftPen) {
        result.normalX = -1.0f;
        result.normalY = 0.0f;
        result.penetrationX = leftPen;
    } else if (minPen == rightPen) {
        result.normalX = 1.0f;
        result.normalY = 0.0f;
        result.penetrationX = rightPen;
    } else if (minPen == topPen) {
        result.normalX = 0.0f;
        result.normalY = -1.0f;
        result.penetrationY = topPen;
    } else {
        result.normalX = 0.0f;
        result.normalY = 1.0f;
        result.penetrationY = bottomPen;
    }

    return true;
}

bool CollisionSystem::checkCirclevsCircle(const Circle& a, const Circle& b,
                                         CollisionResult& result) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = a.radius + b.radius;

    if (distSq >= radiusSum * radiusSum) {
        return false;
    }

    float dist = std::sqrt(distSq);
    if (dist > 0) {
        result.normalX = dx / dist;
        result.normalY = dy / dist;
        result.penetrationX = (radiusSum - dist) * result.normalX;
        result.penetrationY = (radiusSum - dist) * result.normalY;
    }

    return true;
}

bool CollisionSystem::checkAABBvsCircle(const AABB& rect, const Circle& circle,
                                       CollisionResult& result) {
    // Find closest point on rectangle to circle center
    float closestX = std::max(rect.left(), std::min(circle.x, rect.right()));
    float closestY = std::max(rect.top(), std::min(circle.y, rect.bottom()));

    float dx = circle.x - closestX;
    float dy = circle.y - closestY;
    float distSq = dx * dx + dy * dy;

    if (distSq >= circle.radius * circle.radius) {
        return false;
    }

    float dist = std::sqrt(distSq);
    if (dist > 0) {
        result.normalX = dx / dist;
        result.normalY = dy / dist;
        float penetration = circle.radius - dist;
        result.penetrationX = penetration * result.normalX;
        result.penetrationY = penetration * result.normalY;
    }

    return true;
}

bool CollisionSystem::checkPolygonvsPolygon(const Polygon& a, const Polygon& b,
                                           CollisionResult& result) {
    return checkSAT(a, b, result);
}

bool CollisionSystem::checkSAT(const Polygon& a, const Polygon& b, CollisionResult& result) {
    // Simplified SAT implementation
    // TODO: Implement full Separating Axis Theorem
    return false;
}

std::vector<CollisionObject*> CollisionSystem::broadPhaseQuery(const AABB& bounds) {
    switch (m_broadPhase) {
        case BroadPhase::SpatialGrid:
            if (m_spatialGrid) {
                return m_spatialGrid->query(bounds);
            }
            break;

        case BroadPhase::QuadTree:
            if (m_quadTree) {
                std::vector<CollisionObject*> results;
                m_quadTree->query(bounds, results);
                return results;
            }
            break;

        case BroadPhase::BruteForce:
        default:
            std::vector<CollisionObject*> results;
            for (auto& pair : m_objects) {
                if (pair.second->isEnabled() && pair.second->getAABB().intersects(bounds)) {
                    results.push_back(pair.second.get());
                }
            }
            return results;
    }

    return std::vector<CollisionObject*>();
}

void CollisionSystem::rebuildBroadPhase() {
    switch (m_broadPhase) {
        case BroadPhase::SpatialGrid:
            if (!m_spatialGrid) {
                m_spatialGrid = std::make_unique<SpatialGrid>(128.0f);
            }
            m_spatialGrid->clear();
            for (auto& pair : m_objects) {
                m_spatialGrid->insert(pair.second.get());
            }
            break;

        case BroadPhase::QuadTree:
            m_quadTree = std::make_unique<QuadTreeNode>(
                AABB(0, 0, m_worldWidth, m_worldHeight), 0, 6, 8);
            for (auto& pair : m_objects) {
                m_quadTree->insert(pair.second.get());
            }
            break;

        case BroadPhase::BruteForce:
        default:
            // No data structure needed
            break;
    }
}

} // namespace World
