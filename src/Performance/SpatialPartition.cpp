#include "SpatialPartition.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Performance {

// ========== Frustum Implementation ==========

void Frustum::ExtractFromMatrix(const glm::mat4& viewProj) {
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[LEFT].x = viewProj[0][3] + viewProj[0][0];
    planes[LEFT].y = viewProj[1][3] + viewProj[1][0];
    planes[LEFT].z = viewProj[2][3] + viewProj[2][0];
    planes[LEFT].w = viewProj[3][3] + viewProj[3][0];

    // Right plane
    planes[RIGHT].x = viewProj[0][3] - viewProj[0][0];
    planes[RIGHT].y = viewProj[1][3] - viewProj[1][0];
    planes[RIGHT].z = viewProj[2][3] - viewProj[2][0];
    planes[RIGHT].w = viewProj[3][3] - viewProj[3][0];

    // Bottom plane
    planes[BOTTOM].x = viewProj[0][3] + viewProj[0][1];
    planes[BOTTOM].y = viewProj[1][3] + viewProj[1][1];
    planes[BOTTOM].z = viewProj[2][3] + viewProj[2][1];
    planes[BOTTOM].w = viewProj[3][3] + viewProj[3][1];

    // Top plane
    planes[TOP].x = viewProj[0][3] - viewProj[0][1];
    planes[TOP].y = viewProj[1][3] - viewProj[1][1];
    planes[TOP].z = viewProj[2][3] - viewProj[2][1];
    planes[TOP].w = viewProj[3][3] - viewProj[3][1];

    // Near plane
    planes[NEAR].x = viewProj[0][3] + viewProj[0][2];
    planes[NEAR].y = viewProj[1][3] + viewProj[1][2];
    planes[NEAR].z = viewProj[2][3] + viewProj[2][2];
    planes[NEAR].w = viewProj[3][3] + viewProj[3][2];

    // Far plane
    planes[FAR].x = viewProj[0][3] - viewProj[0][2];
    planes[FAR].y = viewProj[1][3] - viewProj[1][2];
    planes[FAR].z = viewProj[2][3] - viewProj[2][2];
    planes[FAR].w = viewProj[3][3] - viewProj[3][2];

    // Normalize planes
    for (int i = 0; i < COUNT; ++i) {
        float length = glm::length(glm::vec3(planes[i]));
        planes[i] /= length;
    }
}

bool Frustum::Contains(const AABB& aabb) const {
    // Check AABB against all frustum planes
    for (int i = 0; i < COUNT; ++i) {
        const glm::vec4& plane = planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);

        // Get positive vertex (furthest point in plane normal direction)
        glm::vec3 pVertex;
        pVertex.x = (normal.x >= 0.0f) ? aabb.max.x : aabb.min.x;
        pVertex.y = (normal.y >= 0.0f) ? aabb.max.y : aabb.min.y;
        pVertex.z = (normal.z >= 0.0f) ? aabb.max.z : aabb.min.z;

        // Check if positive vertex is outside
        if (glm::dot(normal, pVertex) + plane.w < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Frustum::Contains(const glm::vec3& center, float radius) const {
    // Check sphere against all frustum planes
    for (int i = 0; i < COUNT; ++i) {
        const glm::vec4& plane = planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);

        float distance = glm::dot(normal, center) + plane.w;
        if (distance < -radius) {
            return false;
        }
    }

    return true;
}

// ========== Quadtree Implementation ==========

Quadtree::Quadtree(const AABB& bounds, size_t capacity, size_t maxDepth)
    : m_Root(std::make_unique<Node>(bounds)), m_Capacity(capacity),
      m_MaxDepth(maxDepth), m_TotalObjects(0) {
}

Quadtree::~Quadtree() {
    Clear();
}

bool Quadtree::Insert(ISpatialObject* object) {
    if (!object) return false;

    bool result = InsertIntoNode(m_Root.get(), object, 0);
    if (result) {
        m_TotalObjects++;
    }
    return result;
}

bool Quadtree::Remove(ISpatialObject* object) {
    if (!object) return false;

    bool result = RemoveFromNode(m_Root.get(), object);
    if (result) {
        m_TotalObjects--;
    }
    return result;
}

void Quadtree::Query(const AABB& range, std::vector<ISpatialObject*>& results) const {
    QueryNode(m_Root.get(), range, results);
}

void Quadtree::QueryNearest(const glm::vec3& point, size_t maxResults,
                           std::vector<ISpatialObject*>& results) const {
    // Simple implementation: query entire tree and sort by distance
    std::vector<ISpatialObject*> allObjects;
    Query(m_Root->bounds, allObjects);

    // Sort by distance
    std::sort(allObjects.begin(), allObjects.end(),
        [&point](ISpatialObject* a, ISpatialObject* b) {
            glm::vec3 centerA = a->GetBounds().GetCenter();
            glm::vec3 centerB = b->GetBounds().GetCenter();
            float distA = glm::length(centerA - point);
            float distB = glm::length(centerB - point);
            return distA < distB;
        });

    // Take top N
    size_t count = std::min(maxResults, allObjects.size());
    results.insert(results.end(), allObjects.begin(), allObjects.begin() + count);
}

void Quadtree::Clear() {
    m_Root->objects.clear();
    for (auto& child : m_Root->children) {
        child.reset();
    }
    m_Root->subdivided = false;
    m_TotalObjects = 0;
}

size_t Quadtree::GetNodeCount() const {
    return CountNodes(m_Root.get());
}

void Quadtree::Subdivide(Node* node) {
    glm::vec3 center = node->bounds.GetCenter();
    glm::vec3 size = node->bounds.GetSize() * 0.5f;

    // Create 4 quadrants (2D)
    AABB nw(node->bounds.min, center);
    AABB ne(glm::vec3(center.x, node->bounds.min.y, node->bounds.min.z),
            glm::vec3(node->bounds.max.x, center.y, node->bounds.max.z));
    AABB sw(glm::vec3(node->bounds.min.x, center.y, node->bounds.min.z),
            glm::vec3(center.x, node->bounds.max.y, node->bounds.max.z));
    AABB se(center, node->bounds.max);

    node->children[0] = std::make_unique<Node>(nw);
    node->children[1] = std::make_unique<Node>(ne);
    node->children[2] = std::make_unique<Node>(sw);
    node->children[3] = std::make_unique<Node>(se);

    node->subdivided = true;
}

bool Quadtree::InsertIntoNode(Node* node, ISpatialObject* object, size_t depth) {
    if (!node->bounds.Intersects(object->GetBounds())) {
        return false;
    }

    // If we have capacity or reached max depth, add here
    if (node->objects.size() < m_Capacity || depth >= m_MaxDepth) {
        node->objects.push_back(object);
        return true;
    }

    // Subdivide if needed
    if (!node->subdivided) {
        Subdivide(node);

        // Redistribute existing objects
        std::vector<ISpatialObject*> oldObjects = node->objects;
        node->objects.clear();

        for (auto* obj : oldObjects) {
            bool inserted = false;
            for (auto& child : node->children) {
                if (child && InsertIntoNode(child.get(), obj, depth + 1)) {
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                node->objects.push_back(obj);
            }
        }
    }

    // Try to insert into children
    for (auto& child : node->children) {
        if (child && InsertIntoNode(child.get(), object, depth + 1)) {
            return true;
        }
    }

    // If it doesn't fit in any child, keep it here
    node->objects.push_back(object);
    return true;
}

bool Quadtree::RemoveFromNode(Node* node, ISpatialObject* object) {
    // Try to remove from this node
    auto it = std::find(node->objects.begin(), node->objects.end(), object);
    if (it != node->objects.end()) {
        node->objects.erase(it);
        return true;
    }

    // Try children
    if (node->subdivided) {
        for (auto& child : node->children) {
            if (child && RemoveFromNode(child.get(), object)) {
                return true;
            }
        }
    }

    return false;
}

void Quadtree::QueryNode(const Node* node, const AABB& range,
                        std::vector<ISpatialObject*>& results) const {
    if (!node->bounds.Intersects(range)) {
        return;
    }

    // Add objects from this node
    for (auto* object : node->objects) {
        if (object->GetBounds().Intersects(range)) {
            results.push_back(object);
        }
    }

    // Query children
    if (node->subdivided) {
        for (const auto& child : node->children) {
            if (child) {
                QueryNode(child.get(), range, results);
            }
        }
    }
}

size_t Quadtree::CountNodes(const Node* node) const {
    if (!node) return 0;

    size_t count = 1;
    if (node->subdivided) {
        for (const auto& child : node->children) {
            count += CountNodes(child.get());
        }
    }
    return count;
}

// ========== Octree Implementation ==========

Octree::Octree(const AABB& bounds, size_t capacity, size_t maxDepth)
    : m_Root(std::make_unique<Node>(bounds)), m_Capacity(capacity),
      m_MaxDepth(maxDepth), m_TotalObjects(0) {
}

Octree::~Octree() {
    Clear();
}

bool Octree::Insert(ISpatialObject* object) {
    if (!object) return false;

    bool result = InsertIntoNode(m_Root.get(), object, 0);
    if (result) {
        m_TotalObjects++;
    }
    return result;
}

bool Octree::Remove(ISpatialObject* object) {
    if (!object) return false;

    bool result = RemoveFromNode(m_Root.get(), object);
    if (result) {
        m_TotalObjects--;
    }
    return result;
}

void Octree::Query(const AABB& range, std::vector<ISpatialObject*>& results) const {
    QueryNode(m_Root.get(), range, results);
}

void Octree::QueryFrustum(const Frustum& frustum, std::vector<ISpatialObject*>& results) const {
    QueryFrustumNode(m_Root.get(), frustum, results);
}

void Octree::Clear() {
    m_Root->objects.clear();
    for (auto& child : m_Root->children) {
        child.reset();
    }
    m_Root->subdivided = false;
    m_TotalObjects = 0;
}

size_t Octree::GetNodeCount() const {
    return CountNodes(m_Root.get());
}

void Octree::Subdivide(Node* node) {
    glm::vec3 center = node->bounds.GetCenter();
    glm::vec3 size = node->bounds.GetSize() * 0.5f;

    // Create 8 octants
    for (int i = 0; i < 8; ++i) {
        glm::vec3 offset;
        offset.x = (i & 1) ? size.x * 0.5f : -size.x * 0.5f;
        offset.y = (i & 2) ? size.y * 0.5f : -size.y * 0.5f;
        offset.z = (i & 4) ? size.z * 0.5f : -size.z * 0.5f;

        glm::vec3 childCenter = center + offset;
        AABB childBounds(childCenter - size * 0.5f, childCenter + size * 0.5f);

        node->children[i] = std::make_unique<Node>(childBounds);
    }

    node->subdivided = true;
}

bool Octree::InsertIntoNode(Node* node, ISpatialObject* object, size_t depth) {
    if (!node->bounds.Intersects(object->GetBounds())) {
        return false;
    }

    // If we have capacity or reached max depth, add here
    if (node->objects.size() < m_Capacity || depth >= m_MaxDepth) {
        node->objects.push_back(object);
        return true;
    }

    // Subdivide if needed
    if (!node->subdivided) {
        Subdivide(node);

        // Redistribute existing objects
        std::vector<ISpatialObject*> oldObjects = node->objects;
        node->objects.clear();

        for (auto* obj : oldObjects) {
            bool inserted = false;
            for (auto& child : node->children) {
                if (child && InsertIntoNode(child.get(), obj, depth + 1)) {
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                node->objects.push_back(obj);
            }
        }
    }

    // Try to insert into children
    for (auto& child : node->children) {
        if (child && InsertIntoNode(child.get(), object, depth + 1)) {
            return true;
        }
    }

    // If it doesn't fit in any child, keep it here
    node->objects.push_back(object);
    return true;
}

bool Octree::RemoveFromNode(Node* node, ISpatialObject* object) {
    auto it = std::find(node->objects.begin(), node->objects.end(), object);
    if (it != node->objects.end()) {
        node->objects.erase(it);
        return true;
    }

    if (node->subdivided) {
        for (auto& child : node->children) {
            if (child && RemoveFromNode(child.get(), object)) {
                return true;
            }
        }
    }

    return false;
}

void Octree::QueryNode(const Node* node, const AABB& range,
                      std::vector<ISpatialObject*>& results) const {
    if (!node->bounds.Intersects(range)) {
        return;
    }

    for (auto* object : node->objects) {
        if (object->GetBounds().Intersects(range)) {
            results.push_back(object);
        }
    }

    if (node->subdivided) {
        for (const auto& child : node->children) {
            if (child) {
                QueryNode(child.get(), range, results);
            }
        }
    }
}

void Octree::QueryFrustumNode(const Node* node, const Frustum& frustum,
                             std::vector<ISpatialObject*>& results) const {
    if (!frustum.Contains(node->bounds)) {
        return;
    }

    for (auto* object : node->objects) {
        if (frustum.Contains(object->GetBounds())) {
            results.push_back(object);
        }
    }

    if (node->subdivided) {
        for (const auto& child : node->children) {
            if (child) {
                QueryFrustumNode(child.get(), frustum, results);
            }
        }
    }
}

size_t Octree::CountNodes(const Node* node) const {
    if (!node) return 0;

    size_t count = 1;
    if (node->subdivided) {
        for (const auto& child : node->children) {
            count += CountNodes(child.get());
        }
    }
    return count;
}

// ========== Spatial Hash Grid Implementation ==========

SpatialHashGrid::SpatialHashGrid(float cellSize)
    : m_CellSize(cellSize) {
}

void SpatialHashGrid::Insert(ISpatialObject* object) {
    if (!object) return;

    auto cells = GetCellsForAABB(object->GetBounds());
    m_ObjectCells[object] = cells;

    for (const auto& coord : cells) {
        m_Grid[coord].objects.push_back(object);
    }
}

void SpatialHashGrid::Remove(ISpatialObject* object) {
    if (!object) return;

    auto it = m_ObjectCells.find(object);
    if (it == m_ObjectCells.end()) return;

    for (const auto& coord : it->second) {
        auto gridIt = m_Grid.find(coord);
        if (gridIt != m_Grid.end()) {
            auto& objects = gridIt->second.objects;
            objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());

            if (objects.empty()) {
                m_Grid.erase(gridIt);
            }
        }
    }

    m_ObjectCells.erase(it);
}

void SpatialHashGrid::Update(ISpatialObject* object) {
    Remove(object);
    Insert(object);
}

void SpatialHashGrid::Query(const glm::vec3& point, float radius,
                           std::vector<ISpatialObject*>& results) const {
    AABB queryBounds(point - glm::vec3(radius), point + glm::vec3(radius));
    Query(queryBounds, results);
}

void SpatialHashGrid::Query(const AABB& bounds,
                           std::vector<ISpatialObject*>& results) const {
    auto cells = GetCellsForAABB(bounds);
    std::unordered_set<ISpatialObject*> uniqueObjects;

    for (const auto& coord : cells) {
        auto it = m_Grid.find(coord);
        if (it != m_Grid.end()) {
            for (auto* object : it->second.objects) {
                if (uniqueObjects.insert(object).second) {
                    if (object->GetBounds().Intersects(bounds)) {
                        results.push_back(object);
                    }
                }
            }
        }
    }
}

void SpatialHashGrid::Clear() {
    m_Grid.clear();
    m_ObjectCells.clear();
}

SpatialHashGrid::CellCoord SpatialHashGrid::GetCellCoord(const glm::vec3& position) const {
    CellCoord coord;
    coord.x = static_cast<int>(std::floor(position.x / m_CellSize));
    coord.y = static_cast<int>(std::floor(position.y / m_CellSize));
    coord.z = static_cast<int>(std::floor(position.z / m_CellSize));
    return coord;
}

std::vector<SpatialHashGrid::CellCoord>
SpatialHashGrid::GetCellsForAABB(const AABB& bounds) const {
    std::vector<CellCoord> cells;

    CellCoord minCoord = GetCellCoord(bounds.min);
    CellCoord maxCoord = GetCellCoord(bounds.max);

    for (int x = minCoord.x; x <= maxCoord.x; ++x) {
        for (int y = minCoord.y; y <= maxCoord.y; ++y) {
            for (int z = minCoord.z; z <= maxCoord.z; ++z) {
                cells.push_back({x, y, z});
            }
        }
    }

    return cells;
}

// ========== LOD Manager Implementation ==========

void LODManager::UpdateLODs(std::vector<ILODObject*>& objects,
                           const glm::vec3& cameraPos,
                           float viewportHeight,
                           float fov) {
    for (auto* object : objects) {
        if (!object) continue;

        uint32_t lodLevel = CalculateLODLevel(object, cameraPos, viewportHeight, fov);
        object->SetLODLevel(lodLevel);
    }
}

uint32_t LODManager::CalculateLODLevel(ILODObject* object,
                                      const glm::vec3& cameraPos,
                                      float viewportHeight,
                                      float fov) {
    if (!object) return 0;

    const auto& levels = object->GetLODLevels();
    if (levels.empty()) return 0;

    float distance = glm::length(object->GetPosition() - cameraPos);
    float screenSize = CalculateScreenSize(object->GetPosition(),
                                          object->GetBoundingRadius(),
                                          cameraPos, viewportHeight, fov);

    // Find appropriate LOD level based on distance and screen size
    for (size_t i = 0; i < levels.size(); ++i) {
        if (distance < levels[i].distance || screenSize > levels[i].screenSize) {
            return i;
        }
    }

    return static_cast<uint32_t>(levels.size() - 1);
}

float LODManager::CalculateScreenSize(const glm::vec3& position,
                                     float radius,
                                     const glm::vec3& cameraPos,
                                     float viewportHeight,
                                     float fov) {
    float distance = glm::length(position - cameraPos);
    if (distance < 0.001f) return 1.0f;

    // Calculate projected size on screen
    float fovRadians = glm::radians(fov);
    float angularSize = 2.0f * std::atan(radius / distance);
    float screenSize = (angularSize / fovRadians) * viewportHeight;

    // Normalize to 0-1 range
    return std::min(screenSize / viewportHeight, 1.0f);
}

} // namespace Performance
