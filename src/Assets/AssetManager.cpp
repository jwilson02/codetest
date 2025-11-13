#include "AssetManager.h"
#include "AssetLoader.h"
#include "ResourcePool.h"
#include <iostream>
#include <algorithm>
#include <sys/stat.h>

namespace Assets {

AssetManager& AssetManager::GetInstance() {
    static AssetManager instance;
    return instance;
}

void AssetManager::Initialize(const MemoryBudget& budget) {
    if (m_Initialized) {
        std::cerr << "AssetManager already initialized" << std::endl;
        return;
    }

    m_MemoryBudget = budget;
    m_HotReloadEnabled = false;
    m_TimeSinceLastGC = 0.0f;
    m_BasePath = "./assets/";

    m_AssetLoader = std::make_unique<AssetLoader>();
    m_ResourcePool = std::make_unique<ResourcePool>();

    m_Initialized = true;

    std::cout << "AssetManager initialized" << std::endl;
    std::cout << "  Max Total Memory: " << (m_MemoryBudget.maxTotalMemory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Max Texture Memory: " << (m_MemoryBudget.maxTextureMemory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Max Audio Memory: " << (m_MemoryBudget.maxAudioMemory / (1024 * 1024)) << " MB" << std::endl;
}

void AssetManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_AssetMutex);

    // Unload all assets
    for (auto& pair : m_Assets) {
        if (pair.second) {
            pair.second->Unload();
        }
    }

    m_Assets.clear();
    m_Metadata.clear();
    m_FileModTimes.clear();

    m_AssetLoader.reset();
    m_ResourcePool.reset();

    m_Initialized = false;

    std::cout << "AssetManager shutdown" << std::endl;
}

void AssetManager::Unload(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_AssetMutex);

    auto it = m_Assets.find(filepath);
    if (it == m_Assets.end()) {
        return;
    }

    // Decrease reference count
    auto& metadata = m_Metadata[filepath];
    if (metadata.refCount > 0) {
        metadata.refCount--;
    }

    // Only unload if no more references
    if (metadata.refCount == 0) {
        size_t freedMemory = metadata.memorySize;

        it->second->Unload();
        m_Assets.erase(it);
        m_Metadata.erase(filepath);
        m_FileModTimes.erase(filepath);

        std::cout << "Unloaded asset: " << filepath
                  << " (freed " << (freedMemory / 1024) << " KB)" << std::endl;

        // Fire event
        Engine::EventSystem::GetInstance().Publish(
            AssetUnloadedEvent(filepath, freedMemory));
    }
}

void AssetManager::PreloadAssets(const std::vector<std::string>& filepaths,
                                  std::function<void()> callback) {
    std::cout << "Preloading " << filepaths.size() << " assets..." << std::endl;

    // TODO: Use AssetLoader for proper async preloading
    // For now, load synchronously
    for (const auto& filepath : filepaths) {
        AssetType type = DetermineAssetType(filepath);
        // Load based on type - this is simplified
        // In real implementation, we'd have type-specific loaders
    }

    if (callback) {
        callback();
    }
}

void AssetManager::AddDependency(const std::string& asset, const std::string& dependency) {
    auto& metadata = m_Metadata[asset];
    auto it = std::find(metadata.dependencies.begin(), metadata.dependencies.end(), dependency);
    if (it == metadata.dependencies.end()) {
        metadata.dependencies.push_back(dependency);
    }
}

bool AssetManager::IsLoaded(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(m_AssetMutex);
    return m_Assets.find(filepath) != m_Assets.end();
}

void AssetManager::Reload(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_AssetMutex);

    auto it = m_Assets.find(filepath);
    if (it == m_Assets.end()) {
        std::cerr << "Cannot reload asset that is not loaded: " << filepath << std::endl;
        return;
    }

    std::cout << "Hot-reloading asset: " << filepath << std::endl;

    // Unload and reload
    auto& asset = it->second;
    asset->Unload();

    std::string fullPath = GetFullPath(filepath);
    if (asset->Load(fullPath)) {
        m_Metadata[filepath].memorySize = asset->GetMemorySize();
        UpdateAccessTime(filepath);

        // Update file modification time
        m_FileModTimes[filepath] = GetFileModTime(fullPath);

        // Fire hot-reload event
        Engine::EventSystem::GetInstance().Publish(AssetHotReloadEvent(filepath));

        std::cout << "Successfully reloaded: " << filepath << std::endl;
    } else {
        std::cerr << "Failed to reload asset: " << filepath << std::endl;
    }
}

void AssetManager::CheckForHotReload() {
    if (!m_HotReloadEnabled) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_AssetMutex);

    // Check each loaded asset for changes
    for (const auto& pair : m_Assets) {
        const std::string& filepath = pair.first;
        std::string fullPath = GetFullPath(filepath);

        long long currentModTime = GetFileModTime(fullPath);
        auto it = m_FileModTimes.find(filepath);

        if (it != m_FileModTimes.end() && it->second != currentModTime) {
            std::cout << "Detected file change: " << filepath << std::endl;

            // Unlock mutex before calling Reload (which locks it again)
            m_AssetMutex.unlock();
            Reload(filepath);
            m_AssetMutex.lock();
        }
    }
}

MemoryStats AssetManager::GetMemoryStats() const {
    std::lock_guard<std::mutex> lock(m_AssetMutex);

    MemoryStats stats;

    for (const auto& pair : m_Metadata) {
        const auto& metadata = pair.second;
        if (metadata.isLoaded) {
            stats.totalUsed += metadata.memorySize;
            stats.assetCount++;

            switch (metadata.type) {
                case AssetType::Texture:
                    stats.textureUsed += metadata.memorySize;
                    break;
                case AssetType::Audio:
                    stats.audioUsed += metadata.memorySize;
                    break;
                case AssetType::Model:
                    stats.modelUsed += metadata.memorySize;
                    break;
                default:
                    stats.otherUsed += metadata.memorySize;
                    break;
            }
        }
    }

    return stats;
}

void AssetManager::GarbageCollect(bool aggressive) {
    std::lock_guard<std::mutex> lock(m_AssetMutex);

    std::vector<std::string> toUnload;

    for (auto& pair : m_Metadata) {
        const std::string& filepath = pair.first;
        auto& metadata = pair.second;

        // Only unload assets with zero references
        if (metadata.refCount == 0 && metadata.isLoaded) {
            if (aggressive) {
                toUnload.push_back(filepath);
            }
        }
    }

    // Unlock before unloading
    m_AssetMutex.unlock();

    for (const auto& filepath : toUnload) {
        Unload(filepath);
    }

    m_AssetMutex.lock();

    if (!toUnload.empty()) {
        std::cout << "Garbage collection: unloaded " << toUnload.size() << " assets" << std::endl;
    }
}

bool AssetManager::ValidateAsset(const std::string& filepath) {
    std::string fullPath = GetFullPath(filepath);

    // Check if file exists
    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) != 0) {
        std::cerr << "Asset validation failed: file not found: " << fullPath << std::endl;
        return false;
    }

    // Check file size
    if (buffer.st_size == 0) {
        std::cerr << "Asset validation failed: file is empty: " << fullPath << std::endl;
        return false;
    }

    // TODO: Add more validation (magic numbers, format checks, etc.)

    return true;
}

const AssetMetadata* AssetManager::GetMetadata(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(m_AssetMutex);

    auto it = m_Metadata.find(filepath);
    if (it != m_Metadata.end()) {
        return &it->second;
    }

    return nullptr;
}

void AssetManager::Update(float deltaTime) {
    if (!m_Initialized) {
        return;
    }

    m_TimeSinceLastGC += deltaTime;

    // Periodic garbage collection
    if (m_TimeSinceLastGC >= GC_INTERVAL) {
        GarbageCollect(false);
        m_TimeSinceLastGC = 0.0f;
    }

    // Check for hot-reload in development mode
    if (m_HotReloadEnabled) {
        CheckForHotReload();
    }

    // Check memory budget and warn if exceeded
    MemoryStats stats = GetMemoryStats();
    size_t threshold = (m_MemoryBudget.maxTotalMemory * m_MemoryBudget.warningThreshold) / 100;

    if (stats.totalUsed > threshold) {
        std::cout << "Warning: Memory usage at "
                  << ((stats.totalUsed * 100) / m_MemoryBudget.maxTotalMemory)
                  << "% of budget" << std::endl;
    }
}

// Private methods

std::string AssetManager::GetFullPath(const std::string& filepath) const {
    // If already absolute path, return as-is
    if (filepath.empty() || filepath[0] == '/') {
        return filepath;
    }

    return m_BasePath + filepath;
}

AssetType AssetManager::DetermineAssetType(const std::string& filepath) const {
    // Get file extension
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return AssetType::Unknown;
    }

    std::string ext = filepath.substr(dotPos + 1);

    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Determine type based on extension
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga") {
        return AssetType::Texture;
    } else if (ext == "wav" || ext == "mp3" || ext == "ogg") {
        return AssetType::Audio;
    } else if (ext == "glsl" || ext == "vert" || ext == "frag" || ext == "shader") {
        return AssetType::Shader;
    } else if (ext == "obj" || ext == "fbx" || ext == "gltf" || ext == "glb") {
        return AssetType::Model;
    } else if (ext == "anim") {
        return AssetType::Animation;
    } else if (ext == "ttf" || ext == "otf") {
        return AssetType::Font;
    } else if (ext == "tmx" || ext == "json") {
        return AssetType::Tilemap;
    } else if (ext == "dat" || ext == "xml") {
        return AssetType::Data;
    }

    return AssetType::Unknown;
}

bool AssetManager::CanLoadAsset(size_t requiredMemory, AssetType type) {
    MemoryStats stats = GetMemoryStats();

    // Check total memory budget
    if (stats.totalUsed + requiredMemory > m_MemoryBudget.maxTotalMemory) {
        return false;
    }

    // Check type-specific budgets
    switch (type) {
        case AssetType::Texture:
            return stats.textureUsed + requiredMemory <= m_MemoryBudget.maxTextureMemory;
        case AssetType::Audio:
            return stats.audioUsed + requiredMemory <= m_MemoryBudget.maxAudioMemory;
        case AssetType::Model:
            return stats.modelUsed + requiredMemory <= m_MemoryBudget.maxModelMemory;
        default:
            return true;
    }
}

void AssetManager::EvictLRU(size_t requiredMemory) {
    // Build list of evictable assets (ref count == 0)
    struct EvictCandidate {
        std::string filepath;
        float lastAccessTime;
        size_t memorySize;
    };

    std::vector<EvictCandidate> candidates;

    for (auto& pair : m_Metadata) {
        if (pair.second.refCount == 0 && pair.second.isLoaded) {
            candidates.push_back({
                pair.first,
                pair.second.lastAccessTime,
                pair.second.memorySize
            });
        }
    }

    // Sort by access time (oldest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const EvictCandidate& a, const EvictCandidate& b) {
            return a.lastAccessTime < b.lastAccessTime;
        });

    // Evict until we have enough memory
    size_t freedMemory = 0;
    std::vector<std::string> toUnload;

    for (const auto& candidate : candidates) {
        if (freedMemory >= requiredMemory) {
            break;
        }

        toUnload.push_back(candidate.filepath);
        freedMemory += candidate.memorySize;
    }

    // Unlock and unload
    m_AssetMutex.unlock();

    for (const auto& filepath : toUnload) {
        std::cout << "Evicting asset (LRU): " << filepath << std::endl;
        Unload(filepath);
    }

    m_AssetMutex.lock();
}

void AssetManager::UpdateAccessTime(const std::string& filepath) {
    // Use a simple counter as time (could use real time)
    static float accessCounter = 0.0f;
    accessCounter += 1.0f;

    auto it = m_Metadata.find(filepath);
    if (it != m_Metadata.end()) {
        it->second.lastAccessTime = accessCounter;
    }
}

void AssetManager::LoadDependencies(const std::string& filepath) {
    auto it = m_Metadata.find(filepath);
    if (it == m_Metadata.end()) {
        return;
    }

    for (const auto& dependency : it->second.dependencies) {
        if (!IsLoaded(dependency)) {
            // Load dependency (this is simplified)
            std::cout << "Loading dependency: " << dependency << " for " << filepath << std::endl;
        }
    }
}

long long AssetManager::GetFileModTime(const std::string& filepath) const {
    struct stat buffer;
    if (stat(filepath.c_str(), &buffer) == 0) {
        return static_cast<long long>(buffer.st_mtime);
    }
    return 0;
}

} // namespace Assets
