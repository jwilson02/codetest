#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <typeindex>

namespace Assets {

/**
 * @brief Base interface for poolable resources
 */
class IPoolable {
public:
    virtual ~IPoolable() = default;

    /**
     * @brief Reset resource to initial state before reuse
     */
    virtual void Reset() = 0;

    /**
     * @brief Check if resource is currently in use
     */
    virtual bool IsInUse() const = 0;

    /**
     * @brief Mark resource as in use
     */
    virtual void SetInUse(bool inUse) = 0;
};

/**
 * @brief Smart pointer wrapper for pooled resources
 *
 * Automatically returns resource to pool when destroyed
 */
template<typename T>
class PooledResource {
public:
    PooledResource() : m_Resource(nullptr), m_Pool(nullptr) {}

    PooledResource(T* resource, std::function<void(T*)> returnFunc)
        : m_Resource(resource)
        , m_ReturnFunc(returnFunc) {}

    ~PooledResource() {
        if (m_Resource && m_ReturnFunc) {
            m_ReturnFunc(m_Resource);
        }
    }

    // Prevent copying
    PooledResource(const PooledResource&) = delete;
    PooledResource& operator=(const PooledResource&) = delete;

    // Allow moving
    PooledResource(PooledResource&& other) noexcept
        : m_Resource(other.m_Resource)
        , m_ReturnFunc(std::move(other.m_ReturnFunc)) {
        other.m_Resource = nullptr;
        other.m_ReturnFunc = nullptr;
    }

    PooledResource& operator=(PooledResource&& other) noexcept {
        if (this != &other) {
            if (m_Resource && m_ReturnFunc) {
                m_ReturnFunc(m_Resource);
            }
            m_Resource = other.m_Resource;
            m_ReturnFunc = std::move(other.m_ReturnFunc);
            other.m_Resource = nullptr;
            other.m_ReturnFunc = nullptr;
        }
        return *this;
    }

    T* Get() { return m_Resource; }
    const T* Get() const { return m_Resource; }

    T* operator->() { return m_Resource; }
    const T* operator->() const { return m_Resource; }

    T& operator*() { return *m_Resource; }
    const T& operator*() const { return *m_Resource; }

    bool IsValid() const { return m_Resource != nullptr; }

private:
    T* m_Resource;
    std::function<void(T*)> m_ReturnFunc;
};

/**
 * @brief Generic object pool for resource reuse
 *
 * Reduces allocation overhead by reusing objects
 */
template<typename T>
class ObjectPool {
public:
    using Factory = std::function<T*()>;
    using Deleter = std::function<void(T*)>;

    /**
     * @brief Create object pool
     * @param factory Function to create new objects
     * @param initialSize Initial pool size
     * @param maxSize Maximum pool size (0 = unlimited)
     */
    ObjectPool(Factory factory, size_t initialSize = 10, size_t maxSize = 0)
        : m_Factory(factory)
        , m_MaxSize(maxSize)
        , m_TotalCreated(0)
        , m_TotalAcquired(0)
        , m_TotalReused(0) {

        // Pre-allocate initial objects
        for (size_t i = 0; i < initialSize; ++i) {
            T* obj = m_Factory();
            if (obj) {
                m_Available.push(obj);
                m_TotalCreated++;
            }
        }
    }

    ~ObjectPool() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // Delete all available objects
        while (!m_Available.empty()) {
            delete m_Available.front();
            m_Available.pop();
        }

        // Delete all objects in use (if any)
        for (auto* obj : m_InUse) {
            delete obj;
        }
    }

    /**
     * @brief Acquire object from pool
     * @return Pooled resource wrapper
     */
    PooledResource<T> Acquire() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        T* obj = nullptr;

        if (!m_Available.empty()) {
            // Reuse existing object
            obj = m_Available.front();
            m_Available.pop();
            m_TotalReused++;
        } else if (m_MaxSize == 0 || m_TotalCreated < m_MaxSize) {
            // Create new object
            obj = m_Factory();
            if (obj) {
                m_TotalCreated++;
            }
        } else {
            // Pool is at max capacity and no objects available
            return PooledResource<T>();
        }

        if (obj) {
            m_InUse.push_back(obj);
            m_TotalAcquired++;

            // Reset object before use
            if constexpr (std::is_base_of<IPoolable, T>::value) {
                obj->Reset();
                obj->SetInUse(true);
            }

            // Create wrapper with return function
            auto returnFunc = [this](T* resource) {
                this->Return(resource);
            };

            return PooledResource<T>(obj, returnFunc);
        }

        return PooledResource<T>();
    }

    /**
     * @brief Get pool statistics
     */
    struct Stats {
        size_t available;
        size_t inUse;
        size_t totalCreated;
        size_t totalAcquired;
        size_t totalReused;
        float reuseRatio;
    };

    Stats GetStats() const {
        std::lock_guard<std::mutex> lock(m_Mutex);

        Stats stats;
        stats.available = m_Available.size();
        stats.inUse = m_InUse.size();
        stats.totalCreated = m_TotalCreated;
        stats.totalAcquired = m_TotalAcquired;
        stats.totalReused = m_TotalReused;
        stats.reuseRatio = m_TotalAcquired > 0 ?
            static_cast<float>(m_TotalReused) / m_TotalAcquired : 0.0f;

        return stats;
    }

    /**
     * @brief Shrink pool by removing unused objects
     * @param targetSize Target pool size
     */
    void Shrink(size_t targetSize = 0) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        while (m_Available.size() > targetSize) {
            delete m_Available.front();
            m_Available.pop();
            m_TotalCreated--;
        }
    }

    /**
     * @brief Clear all unused objects
     */
    void Clear() {
        Shrink(0);
    }

private:
    void Return(T* obj) {
        if (!obj) return;

        std::lock_guard<std::mutex> lock(m_Mutex);

        // Remove from in-use list
        auto it = std::find(m_InUse.begin(), m_InUse.end(), obj);
        if (it != m_InUse.end()) {
            m_InUse.erase(it);
        }

        // Mark as not in use
        if constexpr (std::is_base_of<IPoolable, T>::value) {
            obj->SetInUse(false);
        }

        // Return to available pool
        m_Available.push(obj);
    }

    Factory m_Factory;
    size_t m_MaxSize;
    size_t m_TotalCreated;
    size_t m_TotalAcquired;
    size_t m_TotalReused;

    std::queue<T*> m_Available;
    std::vector<T*> m_InUse;
    mutable std::mutex m_Mutex;
};

/**
 * @brief Resource pool manager
 *
 * Manages multiple object pools for different resource types
 */
class ResourcePool {
public:
    ResourcePool() = default;
    ~ResourcePool() = default;

    /**
     * @brief Register a new object pool
     * @tparam T Object type
     * @param factory Factory function to create objects
     * @param initialSize Initial pool size
     * @param maxSize Maximum pool size
     */
    template<typename T>
    void RegisterPool(typename ObjectPool<T>::Factory factory,
                     size_t initialSize = 10,
                     size_t maxSize = 0) {
        std::type_index typeIndex = std::type_index(typeid(T));

        auto pool = std::make_shared<ObjectPool<T>>(factory, initialSize, maxSize);
        m_Pools[typeIndex] = std::static_pointer_cast<void>(pool);
    }

    /**
     * @brief Acquire object from pool
     * @tparam T Object type
     * @return Pooled resource wrapper
     */
    template<typename T>
    PooledResource<T> Acquire() {
        std::type_index typeIndex = std::type_index(typeid(T));

        auto it = m_Pools.find(typeIndex);
        if (it != m_Pools.end()) {
            auto pool = std::static_pointer_cast<ObjectPool<T>>(it->second);
            return pool->Acquire();
        }

        return PooledResource<T>();
    }

    /**
     * @brief Get pool statistics
     * @tparam T Object type
     */
    template<typename T>
    typename ObjectPool<T>::Stats GetStats() const {
        std::type_index typeIndex = std::type_index(typeid(T));

        auto it = m_Pools.find(typeIndex);
        if (it != m_Pools.end()) {
            auto pool = std::static_pointer_cast<ObjectPool<T>>(it->second);
            return pool->GetStats();
        }

        return typename ObjectPool<T>::Stats();
    }

    /**
     * @brief Shrink a specific pool
     * @tparam T Object type
     * @param targetSize Target pool size
     */
    template<typename T>
    void ShrinkPool(size_t targetSize = 0) {
        std::type_index typeIndex = std::type_index(typeid(T));

        auto it = m_Pools.find(typeIndex);
        if (it != m_Pools.end()) {
            auto pool = std::static_pointer_cast<ObjectPool<T>>(it->second);
            pool->Shrink(targetSize);
        }
    }

    /**
     * @brief Clear all pools
     */
    void ClearAll() {
        m_Pools.clear();
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_Pools;
};

/**
 * @brief Example poolable buffer for common use cases
 */
class PoolableBuffer : public IPoolable {
public:
    PoolableBuffer(size_t size = 4096)
        : m_Size(size)
        , m_InUse(false) {
        m_Data = new unsigned char[size];
    }

    ~PoolableBuffer() {
        delete[] m_Data;
    }

    void Reset() override {
        // Clear buffer data if needed
    }

    bool IsInUse() const override { return m_InUse; }
    void SetInUse(bool inUse) override { m_InUse = inUse; }

    unsigned char* GetData() { return m_Data; }
    size_t GetSize() const { return m_Size; }

    void Resize(size_t newSize) {
        if (newSize > m_Size) {
            delete[] m_Data;
            m_Data = new unsigned char[newSize];
            m_Size = newSize;
        }
    }

private:
    unsigned char* m_Data;
    size_t m_Size;
    bool m_InUse;
};

} // namespace Assets
