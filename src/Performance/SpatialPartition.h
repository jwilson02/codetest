#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <array>

namespace Performance {

/**
 * @brief Axis-aligned bounding box
 */
struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

    /**
     * @brief Check if point is inside AABB
     */
    bool Contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    /**
     * @brief Check if this AABB intersects another
     */
    bool Intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    /**
     * @brief Get center point
     */
    glm::vec3 GetCenter() const {
        return (min + max) * 0.5f;
    }

    /**
     * @brief Get size
     */
    glm::vec3 GetSize() const {
        return max - min;
    }

    /**
     * @brief Expand to include point
     */
    void Expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    /**
     * @brief Expand to include another AABB
     */
    void Expand(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
};

/**
 * @brief Frustum for frustum culling
 */
struct Frustum {
    enum Plane {
        NEAR = 0,
        FAR,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        COUNT
    };

    std::array<glm::vec4, COUNT> planes;

    /**
     * @brief Extract frustum from view-projection matrix
     */
    void ExtractFromMatrix(const glm::mat4& viewProj);

    /**
     * @brief Check if AABB is inside frustum
     */
    bool Contains(const AABB& aabb) const;

    /**
     * @brief Check if sphere is inside frustum
     */
    bool Contains(const glm::vec3& center, float radius) const;
};

/**
 * @brief Spatial object interface
 */
struct ISpatialObject {
    virtual ~ISpatialObject() = default;
    virtual AABB GetBounds() const = 0;
    virtual uint32_t GetID() const = 0;
};

/**
 * @brief Quadtree for 2D spatial partitioning
 */
class Quadtree {
public:
    /**
     * @brief Construct quadtree
     * @param bounds Boundary of this quadtree
     * @param capacity Maximum objects per node before subdividing
     * @param maxDepth Maximum tree depth
     */
    Quadtree(const AABB& bounds, size_t capacity = 8, size_t maxDepth = 8);

    /**
     * @brief Destructor
     */
    ~Quadtree();

    /**
     * @brief Insert object into quadtree
     * @param object Object to insert
     * @return True if inserted successfully
     */
    bool Insert(ISpatialObject* object);

    /**
     * @brief Remove object from quadtree
     * @param object Object to remove
     * @return True if removed successfully
     */
    bool Remove(ISpatialObject* object);

    /**
     * @brief Query objects in a region
     * @param range Query region
     * @param results Output vector for results
     */
    void Query(const AABB& range, std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Query nearest objects to a point
     * @param point Query point
     * @param maxResults Maximum number of results
     * @param results Output vector for results
     */
    void QueryNearest(const glm::vec3& point, size_t maxResults,
                     std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Clear all objects
     */
    void Clear();

    /**
     * @brief Get total number of objects
     */
    size_t GetObjectCount() const { return m_TotalObjects; }

    /**
     * @brief Get node count
     */
    size_t GetNodeCount() const;

private:
    struct Node {
        AABB bounds;
        std::vector<ISpatialObject*> objects;
        std::array<std::unique_ptr<Node>, 4> children;
        bool subdivided;

        Node(const AABB& bounds) : bounds(bounds), subdivided(false) {}
    };

    void Subdivide(Node* node);
    bool InsertIntoNode(Node* node, ISpatialObject* object, size_t depth);
    bool RemoveFromNode(Node* node, ISpatialObject* object);
    void QueryNode(const Node* node, const AABB& range,
                  std::vector<ISpatialObject*>& results) const;
    size_t CountNodes(const Node* node) const;

    std::unique_ptr<Node> m_Root;
    size_t m_Capacity;
    size_t m_MaxDepth;
    size_t m_TotalObjects;
};

/**
 * @brief Octree for 3D spatial partitioning
 */
class Octree {
public:
    /**
     * @brief Construct octree
     * @param bounds Boundary of this octree
     * @param capacity Maximum objects per node before subdividing
     * @param maxDepth Maximum tree depth
     */
    Octree(const AABB& bounds, size_t capacity = 8, size_t maxDepth = 8);

    /**
     * @brief Destructor
     */
    ~Octree();

    /**
     * @brief Insert object into octree
     * @param object Object to insert
     * @return True if inserted successfully
     */
    bool Insert(ISpatialObject* object);

    /**
     * @brief Remove object from octree
     * @param object Object to remove
     * @return True if removed successfully
     */
    bool Remove(ISpatialObject* object);

    /**
     * @brief Query objects in a region
     * @param range Query region
     * @param results Output vector for results
     */
    void Query(const AABB& range, std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Query objects visible in frustum
     * @param frustum View frustum
     * @param results Output vector for results
     */
    void QueryFrustum(const Frustum& frustum, std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Clear all objects
     */
    void Clear();

    /**
     * @brief Get total number of objects
     */
    size_t GetObjectCount() const { return m_TotalObjects; }

    /**
     * @brief Get node count
     */
    size_t GetNodeCount() const;

private:
    struct Node {
        AABB bounds;
        std::vector<ISpatialObject*> objects;
        std::array<std::unique_ptr<Node>, 8> children;
        bool subdivided;

        Node(const AABB& bounds) : bounds(bounds), subdivided(false) {}
    };

    void Subdivide(Node* node);
    bool InsertIntoNode(Node* node, ISpatialObject* object, size_t depth);
    bool RemoveFromNode(Node* node, ISpatialObject* object);
    void QueryNode(const Node* node, const AABB& range,
                  std::vector<ISpatialObject*>& results) const;
    void QueryFrustumNode(const Node* node, const Frustum& frustum,
                         std::vector<ISpatialObject*>& results) const;
    size_t CountNodes(const Node* node) const;

    std::unique_ptr<Node> m_Root;
    size_t m_Capacity;
    size_t m_MaxDepth;
    size_t m_TotalObjects;
};

/**
 * @brief Spatial hash grid for fast broad-phase collision detection
 */
class SpatialHashGrid {
public:
    /**
     * @brief Construct spatial hash grid
     * @param cellSize Size of each grid cell
     */
    explicit SpatialHashGrid(float cellSize = 10.0f);

    /**
     * @brief Insert object into grid
     * @param object Object to insert
     */
    void Insert(ISpatialObject* object);

    /**
     * @brief Remove object from grid
     * @param object Object to remove
     */
    void Remove(ISpatialObject* object);

    /**
     * @brief Update object position (remove and reinsert)
     * @param object Object to update
     */
    void Update(ISpatialObject* object);

    /**
     * @brief Query objects near a point
     * @param point Query point
     * @param radius Search radius
     * @param results Output vector for results
     */
    void Query(const glm::vec3& point, float radius,
              std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Query objects in a region
     * @param bounds Query region
     * @param results Output vector for results
     */
    void Query(const AABB& bounds, std::vector<ISpatialObject*>& results) const;

    /**
     * @brief Clear all objects
     */
    void Clear();

    /**
     * @brief Get total number of objects
     */
    size_t GetObjectCount() const { return m_ObjectCells.size(); }

    /**
     * @brief Get cell size
     */
    float GetCellSize() const { return m_CellSize; }

private:
    struct Cell {
        std::vector<ISpatialObject*> objects;
    };

    struct CellCoord {
        int x, y, z;

        bool operator==(const CellCoord& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct CellCoordHash {
        size_t operator()(const CellCoord& coord) const {
            // Hash combine
            size_t h1 = std::hash<int>()(coord.x);
            size_t h2 = std::hash<int>()(coord.y);
            size_t h3 = std::hash<int>()(coord.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    CellCoord GetCellCoord(const glm::vec3& position) const;
    std::vector<CellCoord> GetCellsForAABB(const AABB& bounds) const;

    float m_CellSize;
    std::unordered_map<CellCoord, Cell, CellCoordHash> m_Grid;
    std::unordered_map<ISpatialObject*, std::vector<CellCoord>> m_ObjectCells;
};

/**
 * @brief LOD (Level of Detail) manager
 */
class LODManager {
public:
    /**
     * @brief LOD level information
     */
    struct LODLevel {
        float distance;      // Distance threshold for this LOD
        uint32_t meshIndex;  // Mesh index for this LOD
        float screenSize;    // Screen size threshold (0-1)

        LODLevel() : distance(0.0f), meshIndex(0), screenSize(1.0f) {}
        LODLevel(float dist, uint32_t mesh, float screen)
            : distance(dist), meshIndex(mesh), screenSize(screen) {}
    };

    /**
     * @brief LOD object interface
     */
    struct ILODObject {
        virtual ~ILODObject() = default;
        virtual glm::vec3 GetPosition() const = 0;
        virtual float GetBoundingRadius() const = 0;
        virtual void SetLODLevel(uint32_t level) = 0;
        virtual const std::vector<LODLevel>& GetLODLevels() const = 0;
    };

    /**
     * @brief Update LOD for all objects based on camera position
     * @param objects List of LOD objects
     * @param cameraPos Camera position
     * @param viewportHeight Viewport height in pixels
     * @param fov Field of view in radians
     */
    static void UpdateLODs(std::vector<ILODObject*>& objects,
                          const glm::vec3& cameraPos,
                          float viewportHeight,
                          float fov);

    /**
     * @brief Calculate LOD level for an object
     * @param object LOD object
     * @param cameraPos Camera position
     * @param viewportHeight Viewport height in pixels
     * @param fov Field of view in radians
     * @return LOD level index
     */
    static uint32_t CalculateLODLevel(ILODObject* object,
                                     const glm::vec3& cameraPos,
                                     float viewportHeight,
                                     float fov);

    /**
     * @brief Calculate screen size for an object
     * @param position Object position
     * @param radius Object bounding radius
     * @param cameraPos Camera position
     * @param viewportHeight Viewport height in pixels
     * @param fov Field of view in radians
     * @return Screen size (0-1)
     */
    static float CalculateScreenSize(const glm::vec3& position,
                                    float radius,
                                    const glm::vec3& cameraPos,
                                    float viewportHeight,
                                    float fov);
};

} // namespace Performance
