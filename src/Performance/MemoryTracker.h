#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstddef>
#include <cstdint>

namespace Performance {

/**
 * @brief Memory allocation information
 */
struct AllocationInfo {
    void* address;
    size_t size;
    const char* file;
    int line;
    const char* function;
    uint64_t timestamp;
    uint32_t allocationID;

    AllocationInfo()
        : address(nullptr), size(0), file(nullptr), line(0),
          function(nullptr), timestamp(0), allocationID(0) {}
};

/**
 * @brief Memory leak information
 */
struct MemoryLeak {
    size_t size;
    const char* file;
    int line;
    const char* function;
    uint32_t allocationID;

    MemoryLeak()
        : size(0), file(nullptr), line(0), function(nullptr), allocationID(0) {}
};

/**
 * @brief Memory statistics
 */
struct MemoryStats {
    size_t totalAllocated;      // Total bytes currently allocated
    size_t totalFreed;          // Total bytes freed
    size_t peakAllocated;       // Peak memory usage
    size_t allocationCount;     // Number of active allocations
    size_t freeCount;           // Total number of frees
    size_t totalAllocationCount; // Total allocations made

    MemoryStats()
        : totalAllocated(0), totalFreed(0), peakAllocated(0),
          allocationCount(0), freeCount(0), totalAllocationCount(0) {}
};

/**
 * @brief Memory category for tracking different allocation types
 */
enum class MemoryCategory {
    General,
    Rendering,
    Audio,
    Physics,
    AI,
    Gameplay,
    UI,
    Assets,
    COUNT
};

/**
 * @brief Memory tracker for profiling and leak detection
 *
 * Tracks all memory allocations and deallocations, detects leaks,
 * and provides detailed memory usage statistics. Thread-safe.
 */
class MemoryTracker {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the memory tracker instance
     */
    static MemoryTracker& GetInstance();

    /**
     * @brief Initialize the memory tracker
     */
    void Initialize();

    /**
     * @brief Shutdown the memory tracker and report leaks
     */
    void Shutdown();

    /**
     * @brief Enable or disable memory tracking
     * @param enabled True to enable, false to disable
     */
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

    /**
     * @brief Check if memory tracking is enabled
     * @return True if enabled, false otherwise
     */
    bool IsEnabled() const { return m_Enabled; }

    /**
     * @brief Track a memory allocation
     * @param ptr Pointer to allocated memory
     * @param size Size of allocation in bytes
     * @param file Source file name
     * @param line Source line number
     * @param function Function name
     * @param category Memory category
     */
    void TrackAllocation(void* ptr, size_t size, const char* file, int line,
                        const char* function, MemoryCategory category = MemoryCategory::General);

    /**
     * @brief Track a memory deallocation
     * @param ptr Pointer to freed memory
     */
    void TrackDeallocation(void* ptr);

    /**
     * @brief Get current memory statistics
     * @return Memory statistics
     */
    MemoryStats GetStats() const;

    /**
     * @brief Get memory statistics for a specific category
     * @param category Memory category
     * @return Memory statistics for that category
     */
    MemoryStats GetCategoryStats(MemoryCategory category) const;

    /**
     * @brief Detect memory leaks
     * @return List of detected memory leaks
     */
    std::vector<MemoryLeak> DetectLeaks() const;

    /**
     * @brief Get the number of active allocations
     * @return Number of allocations not yet freed
     */
    size_t GetAllocationCount() const;

    /**
     * @brief Get total allocated memory
     * @return Total bytes currently allocated
     */
    size_t GetTotalAllocated() const;

    /**
     * @brief Get peak memory usage
     * @return Peak bytes allocated at any point
     */
    size_t GetPeakAllocated() const;

    /**
     * @brief Print memory statistics to console
     */
    void PrintStats() const;

    /**
     * @brief Print detected leaks to console
     */
    void PrintLeaks() const;

    /**
     * @brief Export memory report to file
     * @param filepath Path to output file
     */
    void ExportReport(const std::string& filepath) const;

    /**
     * @brief Take a memory snapshot
     * @param name Name for this snapshot
     */
    void TakeSnapshot(const std::string& name);

    /**
     * @brief Compare current memory state with a snapshot
     * @param snapshotName Name of snapshot to compare against
     */
    void CompareWithSnapshot(const std::string& snapshotName) const;

    /**
     * @brief Reset all tracking data
     */
    void Reset();

    /**
     * @brief Get category name as string
     * @param category Memory category
     * @return Category name
     */
    static const char* GetCategoryName(MemoryCategory category);

private:
    MemoryTracker() = default;
    ~MemoryTracker() = default;
    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;

    /**
     * @brief Get current timestamp in microseconds
     * @return Current timestamp
     */
    uint64_t GetTimestamp() const;

    bool m_Enabled = false;
    uint32_t m_NextAllocationID = 1;

    // Allocation tracking
    std::unordered_map<void*, AllocationInfo> m_Allocations;
    std::unordered_map<MemoryCategory, MemoryStats> m_CategoryStats;

    // Global stats
    MemoryStats m_GlobalStats;

    // Snapshots
    struct MemorySnapshot {
        std::string name;
        MemoryStats stats;
        size_t allocationCount;
    };
    std::unordered_map<std::string, MemorySnapshot> m_Snapshots;

    // Thread safety
    mutable std::mutex m_Mutex;
};

/**
 * @brief Custom allocator that integrates with MemoryTracker
 */
template<typename T>
class TrackedAllocator {
public:
    using value_type = T;

    TrackedAllocator() noexcept = default;

    template<typename U>
    TrackedAllocator(const TrackedAllocator<U>&) noexcept {}

    T* allocate(size_t n) {
        T* ptr = static_cast<T*>(::operator new(n * sizeof(T)));
        MemoryTracker::GetInstance().TrackAllocation(
            ptr, n * sizeof(T), __FILE__, __LINE__, __FUNCTION__);
        return ptr;
    }

    void deallocate(T* ptr, size_t n) noexcept {
        MemoryTracker::GetInstance().TrackDeallocation(ptr);
        ::operator delete(ptr);
    }
};

template<typename T, typename U>
bool operator==(const TrackedAllocator<T>&, const TrackedAllocator<U>&) noexcept {
    return true;
}

template<typename T, typename U>
bool operator!=(const TrackedAllocator<T>&, const TrackedAllocator<U>&) noexcept {
    return false;
}

} // namespace Performance

// Memory tracking macros
#ifdef MEMORY_TRACKING_ENABLED
    #define TRACK_ALLOC(ptr, size, category) \
        Performance::MemoryTracker::GetInstance().TrackAllocation( \
            ptr, size, __FILE__, __LINE__, __FUNCTION__, category)

    #define TRACK_FREE(ptr) \
        Performance::MemoryTracker::GetInstance().TrackDeallocation(ptr)

    #define NEW_TRACKED(type, category) \
        [&]() { \
            auto ptr = new type; \
            TRACK_ALLOC(ptr, sizeof(type), category); \
            return ptr; \
        }()

    #define DELETE_TRACKED(ptr) \
        do { \
            TRACK_FREE(ptr); \
            delete ptr; \
        } while(0)
#else
    #define TRACK_ALLOC(ptr, size, category) ((void)0)
    #define TRACK_FREE(ptr) ((void)0)
    #define NEW_TRACKED(type, category) new type
    #define DELETE_TRACKED(ptr) delete ptr
#endif
