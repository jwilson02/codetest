#pragma once

#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <memory>
#include <future>

namespace Assets {

/**
 * @brief Asset loading priority
 */
enum class LoadPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/**
 * @brief Asset loading task
 */
struct LoadTask {
    std::string filepath;
    LoadPriority priority;
    std::function<void()> loadFunc;
    std::function<void()> callback;
    std::function<void(const std::string&)> errorCallback;
    size_t estimatedSize; // For streaming decisions
    bool streaming;       // Is this a streaming load?

    LoadTask()
        : priority(LoadPriority::Normal)
        , estimatedSize(0)
        , streaming(false) {}

    // For priority queue ordering
    bool operator<(const LoadTask& other) const {
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};

/**
 * @brief Streaming chunk for large assets
 */
struct StreamChunk {
    unsigned char* data;
    size_t size;
    size_t offset;
    bool isLast;

    StreamChunk()
        : data(nullptr), size(0), offset(0), isLast(false) {}

    ~StreamChunk() {
        if (data) {
            delete[] data;
        }
    }
};

/**
 * @brief Streaming asset handle
 */
class StreamHandle {
public:
    StreamHandle(const std::string& filepath, size_t totalSize)
        : m_Filepath(filepath)
        , m_TotalSize(totalSize)
        , m_LoadedSize(0)
        , m_IsComplete(false) {}

    const std::string& GetFilepath() const { return m_Filepath; }
    size_t GetTotalSize() const { return m_TotalSize; }
    size_t GetLoadedSize() const { return m_LoadedSize; }
    float GetProgress() const {
        return m_TotalSize > 0 ? static_cast<float>(m_LoadedSize) / m_TotalSize : 0.0f;
    }
    bool IsComplete() const { return m_IsComplete; }

    void UpdateProgress(size_t loaded) { m_LoadedSize = loaded; }
    void SetComplete(bool complete) { m_IsComplete = complete; }

private:
    std::string m_Filepath;
    size_t m_TotalSize;
    size_t m_LoadedSize;
    bool m_IsComplete;
};

/**
 * @brief Async asset loader with threading
 *
 * Features:
 * - Multi-threaded loading
 * - Priority queue
 * - Streaming for large assets
 * - Progress tracking
 * - Cancellation support
 */
class AssetLoader {
public:
    /**
     * @brief Create asset loader
     * @param numThreads Number of worker threads (0 = auto-detect)
     */
    AssetLoader(size_t numThreads = 0);
    ~AssetLoader();

    /**
     * @brief Queue an asset for loading
     * @param task Load task
     */
    void QueueLoad(LoadTask&& task);

    /**
     * @brief Queue multiple assets for loading
     * @param tasks List of load tasks
     */
    void QueueLoadBatch(std::vector<LoadTask>&& tasks);

    /**
     * @brief Start streaming load for large asset
     * @param filepath Path to asset
     * @param chunkSize Size of each chunk
     * @param chunkCallback Callback for each chunk
     * @param completeCallback Callback when complete
     * @return Stream handle for tracking progress
     */
    std::shared_ptr<StreamHandle> StartStreamingLoad(
        const std::string& filepath,
        size_t chunkSize,
        std::function<void(StreamChunk*)> chunkCallback,
        std::function<void()> completeCallback = nullptr);

    /**
     * @brief Cancel a loading task
     * @param filepath Path to asset
     * @return True if task was cancelled
     */
    bool CancelLoad(const std::string& filepath);

    /**
     * @brief Cancel all pending loads
     */
    void CancelAll();

    /**
     * @brief Wait for all pending loads to complete
     * @param timeoutMs Timeout in milliseconds (0 = wait forever)
     * @return True if all loads completed
     */
    bool WaitForAll(uint32_t timeoutMs = 0);

    /**
     * @brief Check if loader is idle (no pending tasks)
     * @return True if idle
     */
    bool IsIdle() const;

    /**
     * @brief Get number of pending tasks
     * @return Task count
     */
    size_t GetPendingCount() const;

    /**
     * @brief Get number of active loads
     * @return Active count
     */
    size_t GetActiveCount() const;

    /**
     * @brief Set maximum concurrent loads
     * @param maxConcurrent Maximum number of concurrent loads
     */
    void SetMaxConcurrentLoads(size_t maxConcurrent);

    /**
     * @brief Get loading statistics
     */
    struct Stats {
        size_t totalLoaded;
        size_t totalFailed;
        size_t pendingTasks;
        size_t activeTasks;
        size_t bytesLoaded;
        float averageLoadTime;
    };

    Stats GetStats() const;

    /**
     * @brief Pause loading
     */
    void Pause();

    /**
     * @brief Resume loading
     */
    void Resume();

    /**
     * @brief Check if paused
     * @return True if paused
     */
    bool IsPaused() const { return m_Paused; }

private:
    /**
     * @brief Worker thread function
     */
    void WorkerThread();

    /**
     * @brief Process a single load task
     */
    void ProcessTask(LoadTask& task);

    /**
     * @brief Get next task from queue
     */
    bool GetNextTask(LoadTask& outTask);

    std::vector<std::thread> m_Workers;
    std::priority_queue<LoadTask> m_TaskQueue;
    std::vector<std::shared_ptr<StreamHandle>> m_StreamHandles;

    mutable std::mutex m_QueueMutex;
    std::condition_variable m_QueueCondition;

    std::atomic<bool> m_Running;
    std::atomic<bool> m_Paused;
    std::atomic<size_t> m_ActiveTasks;
    std::atomic<size_t> m_TotalLoaded;
    std::atomic<size_t> m_TotalFailed;
    std::atomic<size_t> m_BytesLoaded;

    size_t m_MaxConcurrentLoads;
    size_t m_NumThreads;
};

/**
 * @brief Helper for loading assets with progress callback
 */
class ProgressiveLoader {
public:
    using ProgressCallback = std::function<void(float progress, const std::string& current)>;

    ProgressiveLoader(AssetLoader& loader);

    /**
     * @brief Load multiple assets with progress tracking
     * @param filepaths List of asset paths
     * @param progressCallback Progress update callback
     * @param completeCallback Completion callback
     */
    void LoadWithProgress(
        const std::vector<std::string>& filepaths,
        ProgressCallback progressCallback,
        std::function<void()> completeCallback = nullptr);

    /**
     * @brief Get current progress
     * @return Progress 0.0 to 1.0
     */
    float GetProgress() const;

    /**
     * @brief Cancel current batch
     */
    void Cancel();

private:
    AssetLoader& m_Loader;
    std::atomic<size_t> m_LoadedCount;
    std::atomic<size_t> m_TotalCount;
    std::mutex m_ProgressMutex;
};

/**
 * @brief Asset pre-loader for level transitions
 *
 * Loads assets for next level while current level is active
 */
class AssetPreloader {
public:
    AssetPreloader(AssetLoader& loader);

    /**
     * @brief Start preloading assets for a level
     * @param levelName Level identifier
     * @param assetList List of assets to preload
     */
    void PreloadLevel(const std::string& levelName,
                     const std::vector<std::string>& assetList);

    /**
     * @brief Check if level is ready
     * @param levelName Level identifier
     * @return True if all assets loaded
     */
    bool IsLevelReady(const std::string& levelName) const;

    /**
     * @brief Get level loading progress
     * @param levelName Level identifier
     * @return Progress 0.0 to 1.0
     */
    float GetLevelProgress(const std::string& levelName) const;

    /**
     * @brief Cancel level preloading
     * @param levelName Level identifier
     */
    void CancelLevel(const std::string& levelName);

    /**
     * @brief Clear preloaded level data
     * @param levelName Level identifier
     */
    void ClearLevel(const std::string& levelName);

private:
    struct LevelData {
        std::vector<std::string> assets;
        size_t loadedCount;
        bool complete;

        LevelData() : loadedCount(0), complete(false) {}
    };

    AssetLoader& m_Loader;
    std::unordered_map<std::string, LevelData> m_Levels;
    mutable std::mutex m_LevelMutex;
};

} // namespace Assets
