#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <mutex>
#include <type_traits>

namespace Performance {

/**
 * @brief Generic object pool for efficient memory management
 *
 * Reduces allocation overhead by reusing objects instead of
 * continuously allocating and deallocating. Thread-safe.
 *
 * @tparam T Type of objects to pool
 */
template<typename T>
class ObjectPool {
public:
    /**
     * @brief Construct object pool
     * @param initialSize Number of objects to preallocate
     * @param maxSize Maximum pool size (0 = unlimited)
     */
    explicit ObjectPool(size_t initialSize = 32, size_t maxSize = 1024)
        : m_MaxSize(maxSize), m_TotalCreated(0), m_ActiveCount(0) {
        Reserve(initialSize);
    }

    /**
     * @brief Destructor
     */
    ~ObjectPool() {
        Clear();
    }

    /**
     * @brief Acquire an object from the pool
     * @param args Constructor arguments for new objects
     * @return Shared pointer to object
     */
    template<typename... Args>
    std::shared_ptr<T> Acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        T* obj = nullptr;

        // Try to get from free list
        if (!m_FreeList.empty()) {
            obj = m_FreeList.front();
            m_FreeList.pop();
        } else {
            // Create new object
            obj = new T(std::forward<Args>(args)...);
            m_TotalCreated++;
        }

        m_ActiveCount++;

        // Return with custom deleter that returns to pool
        return std::shared_ptr<T>(obj, [this](T* ptr) {
            this->Release(ptr);
        });
    }

    /**
     * @brief Reserve space for objects
     * @param count Number of objects to preallocate
     */
    void Reserve(size_t count) {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (size_t i = 0; i < count; ++i) {
            if (m_MaxSize > 0 && m_TotalCreated >= m_MaxSize) {
                break;
            }

            T* obj = new T();
            m_FreeList.push(obj);
            m_TotalCreated++;
        }
    }

    /**
     * @brief Clear all pooled objects
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        while (!m_FreeList.empty()) {
            T* obj = m_FreeList.front();
            m_FreeList.pop();
            delete obj;
        }

        m_TotalCreated = 0;
        m_ActiveCount = 0;
    }

    /**
     * @brief Get number of objects currently in use
     * @return Active object count
     */
    size_t GetActiveCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_ActiveCount;
    }

    /**
     * @brief Get number of available objects in pool
     * @return Free object count
     */
    size_t GetFreeCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_FreeList.size();
    }

    /**
     * @brief Get total number of objects created
     * @return Total created count
     */
    size_t GetTotalCreated() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_TotalCreated;
    }

    /**
     * @brief Set maximum pool size
     * @param maxSize Maximum size (0 = unlimited)
     */
    void SetMaxSize(size_t maxSize) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_MaxSize = maxSize;
    }

    /**
     * @brief Get maximum pool size
     * @return Maximum size
     */
    size_t GetMaxSize() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_MaxSize;
    }

private:
    /**
     * @brief Release object back to pool
     * @param obj Object to release
     */
    void Release(T* obj) {
        if (!obj) return;

        std::lock_guard<std::mutex> lock(m_Mutex);

        m_ActiveCount--;

        // Check if pool is at max size
        if (m_MaxSize > 0 && m_FreeList.size() >= m_MaxSize) {
            delete obj;
            m_TotalCreated--;
        } else {
            // Reset object if it has a reset method
            if constexpr (std::is_member_function_pointer_v<decltype(&T::Reset)>) {
                obj->Reset();
            }
            m_FreeList.push(obj);
        }
    }

    std::queue<T*> m_FreeList;
    size_t m_MaxSize;
    size_t m_TotalCreated;
    size_t m_ActiveCount;
    mutable std::mutex m_Mutex;
};

/**
 * @brief Fixed-size block allocator for small objects
 *
 * Efficiently allocates objects of a fixed size from preallocated blocks.
 * Ideal for particle systems, bullets, etc.
 *
 * @tparam T Type of objects to allocate
 * @tparam BlockSize Number of objects per block
 */
template<typename T, size_t BlockSize = 256>
class BlockAllocator {
public:
    BlockAllocator() : m_CurrentBlock(nullptr), m_CurrentIndex(0) {
        AllocateBlock();
    }

    ~BlockAllocator() {
        Clear();
    }

    /**
     * @brief Allocate an object
     * @return Pointer to allocated object
     */
    T* Allocate() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_CurrentIndex >= BlockSize) {
            AllocateBlock();
        }

        return &m_CurrentBlock->objects[m_CurrentIndex++];
    }

    /**
     * @brief Deallocate an object (no-op, memory is freed when block is freed)
     * @param obj Object to deallocate
     */
    void Deallocate(T* obj) {
        // Block allocator doesn't track individual deallocations
        // Objects are freed when blocks are freed
    }

    /**
     * @brief Clear all allocated blocks
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (auto block : m_Blocks) {
            delete block;
        }

        m_Blocks.clear();
        m_CurrentBlock = nullptr;
        m_CurrentIndex = 0;
    }

    /**
     * @brief Get total allocated memory
     * @return Total bytes allocated
     */
    size_t GetAllocatedMemory() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Blocks.size() * sizeof(Block);
    }

    /**
     * @brief Get number of allocated blocks
     * @return Block count
     */
    size_t GetBlockCount() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Blocks.size();
    }

private:
    struct Block {
        T objects[BlockSize];
    };

    void AllocateBlock() {
        auto block = new Block();
        m_Blocks.push_back(block);
        m_CurrentBlock = block;
        m_CurrentIndex = 0;
    }

    std::vector<Block*> m_Blocks;
    Block* m_CurrentBlock;
    size_t m_CurrentIndex;
    mutable std::mutex m_Mutex;
};

/**
 * @brief Pool allocator for STL containers
 *
 * @tparam T Value type
 * @tparam PoolSize Size of the pool
 */
template<typename T, size_t PoolSize = 1024>
class PoolAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = PoolAllocator<U, PoolSize>;
    };

    PoolAllocator() noexcept = default;

    template<typename U>
    PoolAllocator(const PoolAllocator<U, PoolSize>&) noexcept {}

    pointer allocate(size_type n) {
        if (n > PoolSize) {
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_FreeList.empty()) {
            return static_cast<pointer>(::operator new(n * sizeof(T)));
        }

        pointer ptr = m_FreeList.back();
        m_FreeList.pop_back();
        return ptr;
    }

    void deallocate(pointer ptr, size_type n) noexcept {
        if (n > PoolSize) {
            ::operator delete(ptr);
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_FreeList.size() < PoolSize) {
            m_FreeList.push_back(ptr);
        } else {
            ::operator delete(ptr);
        }
    }

private:
    static std::vector<pointer> m_FreeList;
    static std::mutex m_Mutex;
};

template<typename T, size_t PoolSize>
std::vector<typename PoolAllocator<T, PoolSize>::pointer>
    PoolAllocator<T, PoolSize>::m_FreeList;

template<typename T, size_t PoolSize>
std::mutex PoolAllocator<T, PoolSize>::m_Mutex;

} // namespace Performance
