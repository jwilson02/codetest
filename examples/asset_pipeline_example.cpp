/**
 * Asset Pipeline Example
 *
 * Demonstrates usage of the asset management system including:
 * - Loading assets with AssetManager
 * - Generating texture atlases
 * - Using resource pooling
 * - Async loading with progress
 */

#include "../src/Assets/AssetManager.h"
#include "../src/Assets/AssetLoader.h"
#include "../src/Assets/TextureAtlas.h"
#include "../src/Assets/ResourcePool.h"
#include <iostream>
#include <thread>
#include <chrono>

// Example custom asset type
class ExampleAsset : public Assets::Asset {
public:
    bool Load(const std::string& filepath) override {
        m_Path = filepath;
        m_IsLoaded = true;
        m_Data = "Example asset data from: " + filepath;
        return true;
    }

    void Unload() override {
        m_IsLoaded = false;
        m_Data.clear();
    }

    size_t GetMemorySize() const override {
        return m_Data.size();
    }

    Assets::AssetType GetType() const override {
        return Assets::AssetType::Data;
    }

    const std::string& GetData() const { return m_Data; }

private:
    std::string m_Data;
};

// Example poolable resource
class PoolableResource : public Assets::IPoolable {
public:
    PoolableResource() : m_InUse(false), m_Value(0) {}

    void Reset() override {
        m_Value = 0;
    }

    bool IsInUse() const override { return m_InUse; }
    void SetInUse(bool inUse) override { m_InUse = inUse; }

    void SetValue(int value) { m_Value = value; }
    int GetValue() const { return m_Value; }

private:
    bool m_InUse;
    int m_Value;
};

void Example_BasicAssetLoading() {
    std::cout << "\n=== Basic Asset Loading ===" << std::endl;

    // Initialize asset manager
    Assets::MemoryBudget budget;
    budget.maxTotalMemory = 512 * 1024 * 1024;  // 512 MB

    auto& assetMgr = Assets::AssetManager::GetInstance();
    assetMgr.Initialize(budget);
    assetMgr.SetBasePath("./assets/");

    // Load an asset synchronously
    auto asset = assetMgr.Load<ExampleAsset>("data/example.txt");
    if (asset) {
        std::cout << "Loaded asset: " << asset->GetPath() << std::endl;
        std::cout << "Data: " << asset->GetData() << std::endl;
    }

    // Check memory usage
    auto stats = assetMgr.GetMemoryStats();
    std::cout << "Memory used: " << stats.totalUsed << " bytes" << std::endl;
    std::cout << "Asset count: " << stats.assetCount << std::endl;

    // Unload asset
    assetMgr.Unload("data/example.txt");
}

void Example_AsyncLoading() {
    std::cout << "\n=== Async Asset Loading ===" << std::endl;

    auto& assetMgr = Assets::AssetManager::GetInstance();

    // Load asset asynchronously
    std::cout << "Starting async load..." << std::endl;

    assetMgr.LoadAsync<ExampleAsset>(
        "data/async_example.txt",
        [](std::shared_ptr<Assets::Asset> asset) {
            std::cout << "✓ Async load complete: " << asset->GetPath() << std::endl;
        },
        [](const std::string& error) {
            std::cerr << "✗ Async load failed: " << error << std::endl;
        }
    );

    // Wait a bit for async load to complete
    std::cout << "Doing other work while loading..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Example_ResourcePooling() {
    std::cout << "\n=== Resource Pooling ===" << std::endl;

    // Create object pool
    auto resourceFactory = []() { return new PoolableResource(); };
    auto pool = std::make_unique<Assets::ObjectPool<PoolableResource>>(
        resourceFactory,
        5,    // Initial size
        20    // Max size
    );

    std::cout << "Initial pool stats:" << std::endl;
    auto stats = pool->GetStats();
    std::cout << "  Available: " << stats.available << std::endl;
    std::cout << "  In use: " << stats.inUse << std::endl;

    // Acquire resources
    {
        auto res1 = pool->Acquire();
        auto res2 = pool->Acquire();
        auto res3 = pool->Acquire();

        res1->SetValue(42);
        res2->SetValue(100);

        std::cout << "\nAfter acquiring 3 resources:" << std::endl;
        stats = pool->GetStats();
        std::cout << "  Available: " << stats.available << std::endl;
        std::cout << "  In use: " << stats.inUse << std::endl;

        // Resources automatically return to pool when they go out of scope
    }

    std::cout << "\nAfter resources released:" << std::endl;
    stats = pool->GetStats();
    std::cout << "  Available: " << stats.available << std::endl;
    std::cout << "  In use: " << stats.inUse << std::endl;
    std::cout << "  Reuse ratio: " << (stats.reuseRatio * 100) << "%" << std::endl;
}

void Example_TextureAtlas() {
    std::cout << "\n=== Texture Atlas ===" << std::endl;

    // Note: This example shows the API, but won't work without actual image files
    std::cout << "To generate a texture atlas:" << std::endl;
    std::cout << "  1. Use the command-line tool:" << std::endl;
    std::cout << "     ./tools/generate_atlas.py assets/textures/sprites/ output/atlas.png" << std::endl;
    std::cout << "\n  2. Or use the C++ API:" << std::endl;
    std::cout << "     auto atlas = std::make_shared<Assets::TextureAtlas>();" << std::endl;
    std::cout << "     atlas->Load(\"output/atlas.png\", \"output/atlas.json\");" << std::endl;
    std::cout << "     auto* sprite = atlas->GetSprite(\"player_walk_01\");" << std::endl;

    // Example atlas manager usage
    auto& atlasMgr = Assets::AtlasManager::GetInstance();
    std::cout << "\nAtlas manager ready for managing multiple atlases" << std::endl;
}

void Example_MemoryBudgeting() {
    std::cout << "\n=== Memory Budgeting ===" << std::endl;

    auto& assetMgr = Assets::AssetManager::GetInstance();

    // Get current budget
    const auto& budget = assetMgr.GetMemoryBudget();
    std::cout << "Memory Budget Configuration:" << std::endl;
    std::cout << "  Max Total: " << (budget.maxTotalMemory / (1024*1024)) << " MB" << std::endl;
    std::cout << "  Max Texture: " << (budget.maxTextureMemory / (1024*1024)) << " MB" << std::endl;
    std::cout << "  Max Audio: " << (budget.maxAudioMemory / (1024*1024)) << " MB" << std::endl;
    std::cout << "  Warning Threshold: " << budget.warningThreshold << "%" << std::endl;

    // Get current usage
    auto stats = assetMgr.GetMemoryStats();
    std::cout << "\nCurrent Memory Usage:" << std::endl;
    std::cout << "  Total: " << stats.totalUsed << " bytes" << std::endl;
    std::cout << "  Textures: " << stats.textureUsed << " bytes" << std::endl;
    std::cout << "  Audio: " << stats.audioUsed << " bytes" << std::endl;
    std::cout << "  Assets: " << stats.assetCount << std::endl;

    // Garbage collection
    std::cout << "\nRunning garbage collection..." << std::endl;
    assetMgr.GarbageCollect(false);
}

void Example_HotReloading() {
    std::cout << "\n=== Hot-Reloading ===" << std::endl;

    auto& assetMgr = Assets::AssetManager::GetInstance();

    std::cout << "Enabling hot-reload..." << std::endl;
    assetMgr.SetHotReloadEnabled(true);

    // Subscribe to hot-reload events
    auto& eventSys = Engine::EventSystem::GetInstance();
    eventSys.Subscribe<Assets::AssetHotReloadEvent>(
        [](const Assets::AssetHotReloadEvent& event) {
            std::cout << "🔥 Asset hot-reloaded: " << event.GetPath() << std::endl;
        }
    );

    std::cout << "Hot-reload is enabled!" << std::endl;
    std::cout << "The system will automatically detect and reload changed assets." << std::endl;
    std::cout << "Call assetMgr.Update(deltaTime) every frame to check for changes." << std::endl;
}

void Example_ProgressiveLoading() {
    std::cout << "\n=== Progressive Loading ===" << std::endl;

    auto loader = std::make_unique<Assets::AssetLoader>(2);
    Assets::ProgressiveLoader progressLoader(*loader);

    std::vector<std::string> assetsToLoad = {
        "data/item1.txt",
        "data/item2.txt",
        "data/item3.txt",
        "data/item4.txt",
        "data/item5.txt"
    };

    std::cout << "Loading " << assetsToLoad.size() << " assets with progress..." << std::endl;

    progressLoader.LoadWithProgress(
        assetsToLoad,
        [](float progress, const std::string& current) {
            int percent = static_cast<int>(progress * 100);
            std::cout << "  [" << percent << "%] Loading: " << current << std::endl;
        },
        []() {
            std::cout << "  ✓ All assets loaded!" << std::endl;
        }
    );

    // Wait for loading to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "  Asset Pipeline System Examples" << std::endl;
    std::cout << "======================================" << std::endl;

    try {
        // Run examples
        Example_BasicAssetLoading();
        Example_AsyncLoading();
        Example_ResourcePooling();
        Example_TextureAtlas();
        Example_MemoryBudgeting();
        Example_HotReloading();
        Example_ProgressiveLoading();

        std::cout << "\n======================================" << std::endl;
        std::cout << "  All examples completed!" << std::endl;
        std::cout << "======================================" << std::endl;

        // Cleanup
        Assets::AssetManager::GetInstance().Shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
