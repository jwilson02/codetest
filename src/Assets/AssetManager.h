#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <typeindex>
#include <future>
#include "../Engine/EventSystem.h"

namespace Assets {

// Forward declarations
class Asset;
class AssetLoader;
class ResourcePool;

/**
 * @brief Asset types supported by the engine
 */
enum class AssetType {
    Texture,
    Audio,
    Shader,
    Model,
    Animation,
    Font,
    Tilemap,
    Data,
    Unknown
};

/**
 * @brief Asset metadata and configuration
 */
struct AssetMetadata {
    std::string filepath;
    AssetType type;
    size_t memorySize;          // Size in bytes
    uint32_t refCount;          // Reference count
    bool isLoaded;              // Is currently loaded
    bool isStreaming;           // Should be streamed
    float lastAccessTime;       // For LRU eviction
    std::vector<std::string> dependencies; // Asset dependencies

    AssetMetadata()
        : type(AssetType::Unknown)
        , memorySize(0)
        , refCount(0)
        , isLoaded(false)
        , isStreaming(false)
        , lastAccessTime(0.0f) {}
};

/**
 * @brief Base class for all assets
 */
class Asset {
public:
    virtual ~Asset() = default;

    virtual bool Load(const std::string& filepath) = 0;
    virtual void Unload() = 0;
    virtual size_t GetMemorySize() const = 0;
    virtual AssetType GetType() const = 0;

    const std::string& GetPath() const { return m_Path; }
    bool IsLoaded() const { return m_IsLoaded; }

protected:
    std::string m_Path;
    bool m_IsLoaded = false;
};

/**
 * @brief Asset loading callback types
 */
using AssetLoadCallback = std::function<void(std::shared_ptr<Asset>)>;
using AssetErrorCallback = std::function<void(const std::string& error)>;

/**
 * @brief Asset loaded event
 */
class AssetLoadedEvent : public Engine::Event {
public:
    AssetLoadedEvent(const std::string& path, AssetType type)
        : m_Path(path), m_Type(type) {}

    const std::string& GetPath() const { return m_Path; }
    AssetType GetType() const { return m_Type; }

    std::string GetTypeName() const override { return "AssetLoadedEvent"; }

private:
    std::string m_Path;
    AssetType m_Type;
};

/**
 * @brief Asset unloaded event
 */
class AssetUnloadedEvent : public Engine::Event {
public:
    AssetUnloadedEvent(const std::string& path, size_t freedMemory)
        : m_Path(path), m_FreedMemory(freedMemory) {}

    const std::string& GetPath() const { return m_Path; }
    size_t GetFreedMemory() const { return m_FreedMemory; }

    std::string GetTypeName() const override { return "AssetUnloadedEvent"; }

private:
    std::string m_Path;
    size_t m_FreedMemory;
};

/**
 * @brief Asset hot-reload event
 */
class AssetHotReloadEvent : public Engine::Event {
public:
    AssetHotReloadEvent(const std::string& path)
        : m_Path(path) {}

    const std::string& GetPath() const { return m_Path; }

    std::string GetTypeName() const override { return "AssetHotReloadEvent"; }

private:
    std::string m_Path;
};

/**
 * @brief Memory budget configuration
 */
struct MemoryBudget {
    size_t maxTotalMemory;      // Maximum total memory for all assets (bytes)
    size_t maxTextureMemory;    // Maximum memory for textures
    size_t maxAudioMemory;      // Maximum memory for audio
    size_t maxModelMemory;      // Maximum memory for models
    size_t warningThreshold;    // Warning threshold (percentage 0-100)

    MemoryBudget()
        : maxTotalMemory(512 * 1024 * 1024)   // 512 MB default
        , maxTextureMemory(256 * 1024 * 1024) // 256 MB
        , maxAudioMemory(128 * 1024 * 1024)   // 128 MB
        , maxModelMemory(128 * 1024 * 1024)   // 128 MB
        , warningThreshold(80) {}
};

/**
 * @brief Memory usage statistics
 */
struct MemoryStats {
    size_t totalUsed;
    size_t textureUsed;
    size_t audioUsed;
    size_t modelUsed;
    size_t otherUsed;
    uint32_t assetCount;

    MemoryStats()
        : totalUsed(0), textureUsed(0), audioUsed(0)
        , modelUsed(0), otherUsed(0), assetCount(0) {}
};

/**
 * @brief Central asset management system
 *
 * Features:
 * - Async asset loading
 * - Reference counting and automatic unloading
 * - Memory budgeting with LRU eviction
 * - Hot-reloading support
 * - Dependency management
 * - Streaming asset loading
 * - Resource pooling integration
 */
class AssetManager {
public:
    static AssetManager& GetInstance();

    /**
     * @brief Initialize the asset manager
     * @param budget Memory budget configuration
     */
    void Initialize(const MemoryBudget& budget = MemoryBudget());

    /**
     * @brief Shutdown and cleanup all assets
     */
    void Shutdown();

    /**
     * @brief Load an asset synchronously
     * @tparam T Asset type
     * @param filepath Path to the asset
     * @return Shared pointer to loaded asset
     */
    template<typename T>
    std::shared_ptr<T> Load(const std::string& filepath);

    /**
     * @brief Load an asset asynchronously
     * @tparam T Asset type
     * @param filepath Path to the asset
     * @param callback Callback when asset is loaded
     * @param errorCallback Callback on error
     */
    template<typename T>
    void LoadAsync(const std::string& filepath,
                   AssetLoadCallback callback = nullptr,
                   AssetErrorCallback errorCallback = nullptr);

    /**
     * @brief Get a loaded asset
     * @tparam T Asset type
     * @param filepath Path to the asset
     * @return Shared pointer to asset or nullptr if not loaded
     */
    template<typename T>
    std::shared_ptr<T> Get(const std::string& filepath);

    /**
     * @brief Unload an asset
     * @param filepath Path to the asset
     */
    void Unload(const std::string& filepath);

    /**
     * @brief Preload multiple assets
     * @param filepaths List of asset paths
     * @param callback Callback when all assets are loaded
     */
    void PreloadAssets(const std::vector<std::string>& filepaths,
                       std::function<void()> callback = nullptr);

    /**
     * @brief Add asset dependency
     * @param asset Path to the asset
     * @param dependency Path to the dependency
     */
    void AddDependency(const std::string& asset, const std::string& dependency);

    /**
     * @brief Check if an asset is loaded
     * @param filepath Path to the asset
     * @return True if loaded
     */
    bool IsLoaded(const std::string& filepath) const;

    /**
     * @brief Reload an asset (hot-reload)
     * @param filepath Path to the asset
     */
    void Reload(const std::string& filepath);

    /**
     * @brief Enable/disable hot-reloading
     * @param enabled True to enable
     */
    void SetHotReloadEnabled(bool enabled) { m_HotReloadEnabled = enabled; }

    /**
     * @brief Check for file changes and hot-reload if needed
     * Should be called every frame in development mode
     */
    void CheckForHotReload();

    /**
     * @brief Get memory statistics
     * @return Current memory usage stats
     */
    MemoryStats GetMemoryStats() const;

    /**
     * @brief Get memory budget
     * @return Current memory budget
     */
    const MemoryBudget& GetMemoryBudget() const { return m_MemoryBudget; }

    /**
     * @brief Force garbage collection (unload unused assets)
     * @param aggressive If true, unload all unreferenced assets
     */
    void GarbageCollect(bool aggressive = false);

    /**
     * @brief Set asset base path
     * @param basePath Base directory for assets
     */
    void SetBasePath(const std::string& basePath) { m_BasePath = basePath; }

    /**
     * @brief Get asset base path
     * @return Base directory path
     */
    const std::string& GetBasePath() const { return m_BasePath; }

    /**
     * @brief Validate asset integrity
     * @param filepath Path to the asset
     * @return True if valid
     */
    bool ValidateAsset(const std::string& filepath);

    /**
     * @brief Get asset metadata
     * @param filepath Path to the asset
     * @return Asset metadata
     */
    const AssetMetadata* GetMetadata(const std::string& filepath) const;

    /**
     * @brief Update asset manager (call every frame)
     * @param deltaTime Time since last frame
     */
    void Update(float deltaTime);

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    /**
     * @brief Get full path to asset
     */
    std::string GetFullPath(const std::string& filepath) const;

    /**
     * @brief Determine asset type from filepath
     */
    AssetType DetermineAssetType(const std::string& filepath) const;

    /**
     * @brief Check if memory budget allows loading
     */
    bool CanLoadAsset(size_t requiredMemory, AssetType type);

    /**
     * @brief Evict least recently used assets to free memory
     */
    void EvictLRU(size_t requiredMemory);

    /**
     * @brief Update asset access time
     */
    void UpdateAccessTime(const std::string& filepath);

    /**
     * @brief Load asset dependencies recursively
     */
    void LoadDependencies(const std::string& filepath);

    /**
     * @brief Get file modification time
     */
    long long GetFileModTime(const std::string& filepath) const;

    std::unordered_map<std::string, std::shared_ptr<Asset>> m_Assets;
    std::unordered_map<std::string, AssetMetadata> m_Metadata;
    std::unordered_map<std::string, long long> m_FileModTimes;

    mutable std::mutex m_AssetMutex;
    std::unique_ptr<AssetLoader> m_AssetLoader;
    std::unique_ptr<ResourcePool> m_ResourcePool;

    MemoryBudget m_MemoryBudget;
    std::string m_BasePath;
    bool m_HotReloadEnabled;
    bool m_Initialized;
    float m_TimeSinceLastGC;

    // Constants
    static constexpr float GC_INTERVAL = 30.0f; // Garbage collect every 30 seconds
};

// Template implementations

template<typename T>
std::shared_ptr<T> AssetManager::Load(const std::string& filepath) {
    static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

    std::lock_guard<std::mutex> lock(m_AssetMutex);

    // Check if already loaded
    auto it = m_Assets.find(filepath);
    if (it != m_Assets.end()) {
        UpdateAccessTime(filepath);
        m_Metadata[filepath].refCount++;
        return std::static_pointer_cast<T>(it->second);
    }

    // Create new asset
    auto asset = std::make_shared<T>();
    std::string fullPath = GetFullPath(filepath);

    // Load the asset
    if (!asset->Load(fullPath)) {
        return nullptr;
    }

    // Check memory budget
    size_t memSize = asset->GetMemorySize();
    if (!CanLoadAsset(memSize, asset->GetType())) {
        EvictLRU(memSize);

        if (!CanLoadAsset(memSize, asset->GetType())) {
            asset->Unload();
            return nullptr;
        }
    }

    // Store asset and metadata
    m_Assets[filepath] = asset;

    AssetMetadata& metadata = m_Metadata[filepath];
    metadata.filepath = filepath;
    metadata.type = asset->GetType();
    metadata.memorySize = memSize;
    metadata.refCount = 1;
    metadata.isLoaded = true;

    UpdateAccessTime(filepath);

    // Load dependencies
    LoadDependencies(filepath);

    // Fire event
    Engine::EventSystem::GetInstance().Publish(
        AssetLoadedEvent(filepath, asset->GetType()));

    return asset;
}

template<typename T>
void AssetManager::LoadAsync(const std::string& filepath,
                              AssetLoadCallback callback,
                              AssetErrorCallback errorCallback) {
    // This would use the AssetLoader for async loading
    // For now, we'll do a simple async load
    std::async(std::launch::async, [this, filepath, callback, errorCallback]() {
        try {
            auto asset = Load<T>(filepath);
            if (asset && callback) {
                callback(asset);
            } else if (!asset && errorCallback) {
                errorCallback("Failed to load asset: " + filepath);
            }
        } catch (const std::exception& e) {
            if (errorCallback) {
                errorCallback(std::string("Exception loading asset: ") + e.what());
            }
        }
    });
}

template<typename T>
std::shared_ptr<T> AssetManager::Get(const std::string& filepath) {
    static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

    std::lock_guard<std::mutex> lock(m_AssetMutex);

    auto it = m_Assets.find(filepath);
    if (it != m_Assets.end()) {
        UpdateAccessTime(filepath);
        return std::static_pointer_cast<T>(it->second);
    }

    return nullptr;
}

} // namespace Assets
