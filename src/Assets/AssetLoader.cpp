#include "AssetLoader.h"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace Assets {

AssetLoader::AssetLoader(size_t numThreads)
    : m_Running(true)
    , m_Paused(false)
    , m_ActiveTasks(0)
    , m_TotalLoaded(0)
    , m_TotalFailed(0)
    , m_BytesLoaded(0)
    , m_MaxConcurrentLoads(4) {

    // Auto-detect number of threads if not specified
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 2; // Fallback
        }
    }

    m_NumThreads = std::min(numThreads, size_t(8)); // Cap at 8 threads

    // Create worker threads
    for (size_t i = 0; i < m_NumThreads; ++i) {
        m_Workers.emplace_back(&AssetLoader::WorkerThread, this);
    }

    std::cout << "AssetLoader initialized with " << m_NumThreads << " worker threads" << std::endl;
}

AssetLoader::~AssetLoader() {
    // Signal threads to stop
    m_Running = false;
    m_QueueCondition.notify_all();

    // Wait for all threads to finish
    for (auto& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "AssetLoader shutdown" << std::endl;
}

void AssetLoader::QueueLoad(LoadTask&& task) {
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_TaskQueue.push(std::move(task));
    }
    m_QueueCondition.notify_one();
}

void AssetLoader::QueueLoadBatch(std::vector<LoadTask>&& tasks) {
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        for (auto& task : tasks) {
            m_TaskQueue.push(std::move(task));
        }
    }
    m_QueueCondition.notify_all();
}

std::shared_ptr<StreamHandle> AssetLoader::StartStreamingLoad(
    const std::string& filepath,
    size_t chunkSize,
    std::function<void(StreamChunk*)> chunkCallback,
    std::function<void()> completeCallback) {

    // Create stream handle
    // TODO: Get actual file size
    size_t totalSize = 0;
    auto handle = std::make_shared<StreamHandle>(filepath, totalSize);

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_StreamHandles.push_back(handle);
    }

    // Queue streaming load task
    LoadTask task;
    task.filepath = filepath;
    task.priority = LoadPriority::High;
    task.streaming = true;
    task.estimatedSize = totalSize;

    task.loadFunc = [this, filepath, chunkSize, chunkCallback, completeCallback, handle]() {
        // TODO: Implement actual streaming load
        // For now, just simulate it
        std::cout << "Streaming load: " << filepath << std::endl;

        // Simulate chunk loading
        size_t loaded = 0;
        const size_t totalChunks = 10; // Simulate 10 chunks

        for (size_t i = 0; i < totalChunks; ++i) {
            if (!m_Running) break;

            auto* chunk = new StreamChunk();
            chunk->size = chunkSize;
            chunk->offset = loaded;
            chunk->isLast = (i == totalChunks - 1);

            if (chunkCallback) {
                chunkCallback(chunk);
            }

            loaded += chunkSize;
            handle->UpdateProgress(loaded);

            delete chunk;
        }

        handle->SetComplete(true);

        if (completeCallback) {
            completeCallback();
        }
    };

    QueueLoad(std::move(task));

    return handle;
}

bool AssetLoader::CancelLoad(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_QueueMutex);

    // Remove from queue
    // Note: priority_queue doesn't support removal, so we'd need a different structure
    // For now, we'll mark it as cancelled in a separate set
    // This is a simplified implementation

    return false; // Not implemented in this basic version
}

void AssetLoader::CancelAll() {
    std::lock_guard<std::mutex> lock(m_QueueMutex);

    // Clear task queue
    std::priority_queue<LoadTask> empty;
    std::swap(m_TaskQueue, empty);

    std::cout << "Cancelled all pending loads" << std::endl;
}

bool AssetLoader::WaitForAll(uint32_t timeoutMs) {
    auto startTime = std::chrono::steady_clock::now();

    while (!IsIdle()) {
        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed >= timeoutMs) {
                return false; // Timeout
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return true;
}

bool AssetLoader::IsIdle() const {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    return m_TaskQueue.empty() && m_ActiveTasks == 0;
}

size_t AssetLoader::GetPendingCount() const {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    return m_TaskQueue.size();
}

size_t AssetLoader::GetActiveCount() const {
    return m_ActiveTasks;
}

void AssetLoader::SetMaxConcurrentLoads(size_t maxConcurrent) {
    m_MaxConcurrentLoads = std::max(size_t(1), maxConcurrent);
}

AssetLoader::Stats AssetLoader::GetStats() const {
    Stats stats;
    stats.totalLoaded = m_TotalLoaded;
    stats.totalFailed = m_TotalFailed;
    stats.pendingTasks = GetPendingCount();
    stats.activeTasks = m_ActiveTasks;
    stats.bytesLoaded = m_BytesLoaded;
    stats.averageLoadTime = 0.0f; // TODO: Track load times

    return stats;
}

void AssetLoader::Pause() {
    m_Paused = true;
    std::cout << "AssetLoader paused" << std::endl;
}

void AssetLoader::Resume() {
    m_Paused = false;
    m_QueueCondition.notify_all();
    std::cout << "AssetLoader resumed" << std::endl;
}

// Private methods

void AssetLoader::WorkerThread() {
    while (m_Running) {
        LoadTask task;

        if (GetNextTask(task)) {
            // Wait if we've hit max concurrent loads
            while (m_ActiveTasks >= m_MaxConcurrentLoads && m_Running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Wait if paused
            while (m_Paused && m_Running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            if (!m_Running) break;

            ProcessTask(task);
        } else {
            // No tasks available, wait for notification
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_QueueCondition.wait(lock, [this] {
                return !m_TaskQueue.empty() || !m_Running;
            });
        }
    }
}

void AssetLoader::ProcessTask(LoadTask& task) {
    m_ActiveTasks++;

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Execute load function
        if (task.loadFunc) {
            task.loadFunc();
        }

        // Execute callback on success
        if (task.callback) {
            task.callback();
        }

        m_TotalLoaded++;
        m_BytesLoaded += task.estimatedSize;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "Loaded asset: " << task.filepath
                  << " (" << duration.count() << "ms)"
                  << " [Priority: " << static_cast<int>(task.priority) << "]"
                  << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Failed to load asset: " << task.filepath
                  << " - " << e.what() << std::endl;

        if (task.errorCallback) {
            task.errorCallback(e.what());
        }

        m_TotalFailed++;
    }

    m_ActiveTasks--;
}

bool AssetLoader::GetNextTask(LoadTask& outTask) {
    std::lock_guard<std::mutex> lock(m_QueueMutex);

    if (m_TaskQueue.empty()) {
        return false;
    }

    outTask = std::move(const_cast<LoadTask&>(m_TaskQueue.top()));
    m_TaskQueue.pop();

    return true;
}

// ProgressiveLoader implementation

ProgressiveLoader::ProgressiveLoader(AssetLoader& loader)
    : m_Loader(loader)
    , m_LoadedCount(0)
    , m_TotalCount(0) {
}

void ProgressiveLoader::LoadWithProgress(
    const std::vector<std::string>& filepaths,
    ProgressCallback progressCallback,
    std::function<void()> completeCallback) {

    m_LoadedCount = 0;
    m_TotalCount = filepaths.size();

    std::cout << "Loading " << m_TotalCount << " assets with progress tracking..." << std::endl;

    for (const auto& filepath : filepaths) {
        LoadTask task;
        task.filepath = filepath;
        task.priority = LoadPriority::Normal;

        task.callback = [this, filepath, progressCallback, completeCallback]() {
            m_LoadedCount++;

            if (progressCallback) {
                float progress = GetProgress();
                progressCallback(progress, filepath);
            }

            if (m_LoadedCount >= m_TotalCount && completeCallback) {
                completeCallback();
            }
        };

        m_Loader.QueueLoad(std::move(task));
    }
}

float ProgressiveLoader::GetProgress() const {
    return m_TotalCount > 0 ? static_cast<float>(m_LoadedCount) / m_TotalCount : 0.0f;
}

void ProgressiveLoader::Cancel() {
    m_Loader.CancelAll();
}

// AssetPreloader implementation

AssetPreloader::AssetPreloader(AssetLoader& loader)
    : m_Loader(loader) {
}

void AssetPreloader::PreloadLevel(const std::string& levelName,
                                  const std::vector<std::string>& assetList) {
    std::lock_guard<std::mutex> lock(m_LevelMutex);

    LevelData& levelData = m_Levels[levelName];
    levelData.assets = assetList;
    levelData.loadedCount = 0;
    levelData.complete = false;

    std::cout << "Preloading level: " << levelName
              << " (" << assetList.size() << " assets)" << std::endl;

    for (const auto& asset : assetList) {
        LoadTask task;
        task.filepath = asset;
        task.priority = LoadPriority::Low; // Background loading

        task.callback = [this, levelName]() {
            std::lock_guard<std::mutex> lock(m_LevelMutex);
            auto it = m_Levels.find(levelName);
            if (it != m_Levels.end()) {
                it->second.loadedCount++;
                if (it->second.loadedCount >= it->second.assets.size()) {
                    it->second.complete = true;
                    std::cout << "Level preload complete: " << levelName << std::endl;
                }
            }
        };

        m_Loader.QueueLoad(std::move(task));
    }
}

bool AssetPreloader::IsLevelReady(const std::string& levelName) const {
    std::lock_guard<std::mutex> lock(m_LevelMutex);

    auto it = m_Levels.find(levelName);
    if (it != m_Levels.end()) {
        return it->second.complete;
    }

    return false;
}

float AssetPreloader::GetLevelProgress(const std::string& levelName) const {
    std::lock_guard<std::mutex> lock(m_LevelMutex);

    auto it = m_Levels.find(levelName);
    if (it != m_Levels.end()) {
        const auto& levelData = it->second;
        return levelData.assets.empty() ? 0.0f :
            static_cast<float>(levelData.loadedCount) / levelData.assets.size();
    }

    return 0.0f;
}

void AssetPreloader::CancelLevel(const std::string& levelName) {
    std::lock_guard<std::mutex> lock(m_LevelMutex);
    m_Levels.erase(levelName);
    // TODO: Actually cancel pending tasks for this level
}

void AssetPreloader::ClearLevel(const std::string& levelName) {
    std::lock_guard<std::mutex> lock(m_LevelMutex);
    m_Levels.erase(levelName);
}

} // namespace Assets
