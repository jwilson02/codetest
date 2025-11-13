# Asset Pipeline and Resource Management System

A comprehensive asset management system for the SCUMM-style ARPG engine, featuring async loading, texture atlasing, resource pooling, hot-reloading, and memory budgeting.

## Overview

The asset pipeline provides:

- **Async Asset Loading**: Multi-threaded loading with priority queues
- **Texture Atlas Generation**: Pack sprites into optimized atlases
- **Resource Pooling**: Reuse objects to reduce allocation overhead
- **Hot-Reloading**: See asset changes without restarting (development mode)
- **Memory Budgeting**: Automatic memory management with LRU eviction
- **Asset Compression**: Pack and compress assets for distribution
- **Dependency Management**: Automatically load asset dependencies
- **Streaming**: Load large assets in chunks

## Architecture

### Core Components

```
Assets/
├── AssetManager      - Central asset management and caching
├── AssetLoader       - Async loading with worker threads
├── TextureAtlas      - Sprite sheet generation and packing
└── ResourcePool      - Object pooling for performance
```

### Directory Structure

```
project/
├── src/
│   └── Assets/
│       ├── AssetManager.h/cpp      - Main asset manager
│       ├── AssetLoader.h/cpp       - Async loading system
│       ├── TextureAtlas.h/cpp      - Atlas generation
│       └── ResourcePool.h/cpp      - Resource pooling
├── tools/
│   ├── pack_assets.py              - Asset packing script
│   └── generate_atlas.py           - Texture atlas generator
└── assets/
    ├── textures/                   - Texture images
    ├── audio/                      - Audio files
    ├── shaders/                    - Shader programs
    └── README.md                   - Asset guidelines
```

## Quick Start

### 1. Initialize the Asset System

```cpp
#include "Assets/AssetManager.h"

// Configure memory budget
Assets::MemoryBudget budget;
budget.maxTotalMemory = 512 * 1024 * 1024;     // 512 MB
budget.maxTextureMemory = 256 * 1024 * 1024;   // 256 MB
budget.maxAudioMemory = 128 * 1024 * 1024;     // 128 MB

// Initialize
auto& assetMgr = Assets::AssetManager::GetInstance();
assetMgr.Initialize(budget);
assetMgr.SetBasePath("./assets/");

// Enable hot-reloading (development only)
assetMgr.SetHotReloadEnabled(true);
```

### 2. Load Assets

```cpp
// Synchronous loading
auto texture = assetMgr.Load<Rendering::Texture>("textures/player.png");
if (texture && texture->IsValid()) {
    // Use texture
}

// Async loading with callback
assetMgr.LoadAsync<Rendering::Texture>(
    "textures/enemy.png",
    [](std::shared_ptr<Asset> asset) {
        // Asset loaded successfully
        std::cout << "Asset loaded: " << asset->GetPath() << std::endl;
    },
    [](const std::string& error) {
        // Error loading asset
        std::cerr << "Error: " << error << std::endl;
    }
);
```

### 3. Update Each Frame

```cpp
// In your main loop
void GameLoop() {
    float deltaTime = CalculateDeltaTime();

    // Update asset system (handles hot-reload and garbage collection)
    Assets::AssetManager::GetInstance().Update(deltaTime);

    // ... rest of game loop
}
```

## Asset Manager Features

### Memory Management

The asset manager automatically manages memory:

```cpp
// Get current memory usage
auto stats = assetMgr.GetMemoryStats();
std::cout << "Memory used: " << stats.totalUsed / (1024*1024) << " MB" << std::endl;
std::cout << "Texture memory: " << stats.textureUsed / (1024*1024) << " MB" << std::endl;
std::cout << "Asset count: " << stats.assetCount << std::endl;

// Manual garbage collection
assetMgr.GarbageCollect(true);  // true = aggressive mode
```

### LRU Eviction

When memory budget is exceeded, least-recently-used assets are automatically unloaded:

- Assets with `refCount > 0` are never evicted
- Assets are sorted by last access time
- Oldest unused assets are evicted first
- Eviction continues until enough memory is freed

### Asset Dependencies

```cpp
// Add dependency (dependent will load dependency automatically)
assetMgr.AddDependency("textures/character.png", "textures/character_normal.png");

// When character.png loads, character_normal.png loads too
auto texture = assetMgr.Load<Texture>("textures/character.png");
```

### Hot-Reloading

```cpp
// Enable hot-reloading
assetMgr.SetHotReloadEnabled(true);

// Subscribe to reload events
Engine::EventSystem::GetInstance().Subscribe<Assets::AssetHotReloadEvent>(
    [](const Assets::AssetHotReloadEvent& event) {
        std::cout << "Asset reloaded: " << event.GetPath() << std::endl;
        // Update references if needed
    }
);

// Asset changes are detected automatically
// No need to manually reload
```

## Texture Atlas System

### Generate Atlas from Code

```cpp
#include "Assets/TextureAtlas.h"

// Create atlas
auto atlas = std::make_shared<Assets::TextureAtlas>();

// Load sprites into atlas
std::vector<Assets::AtlasInput> inputs;
// ... populate inputs with sprite data

// Generate atlas
Assets::AtlasSettings settings;
settings.maxWidth = 2048;
settings.maxHeight = 2048;
settings.padding = 2;
settings.algorithm = Assets::PackingAlgorithm::MaxRects;

if (atlas->Generate(inputs, settings)) {
    // Save atlas
    atlas->Save("output/sprites_atlas.png", "output/sprites_atlas.json");

    // Use atlas
    atlas->Bind(0);
    auto* sprite = atlas->GetSprite("player_idle");
    // ... render sprite using UV coordinates
}
```

### Generate Atlas from Command Line

```bash
# Generate texture atlas from directory
./tools/generate_atlas.py assets/textures/characters/ output/characters_atlas.png

# Advanced options
./tools/generate_atlas.py assets/textures/ui/ output/ui_atlas.png \
  --width 2048 \
  --height 2048 \
  --padding 4 \
  --algorithm maxrects \
  --no-trim
```

### Use Generated Atlas

```cpp
// Load atlas
auto atlas = std::make_shared<Assets::TextureAtlas>();
atlas->Load("output/characters_atlas.png", "output/characters_atlas.json");

// Add to manager
Assets::AtlasManager::GetInstance().AddAtlas("characters", atlas);

// Get sprite info
auto* sprite = atlas->GetSprite("player_walk_01");
if (sprite) {
    std::cout << "Sprite position: " << sprite->rect.x << ", " << sprite->rect.y << std::endl;
    std::cout << "UV coords: " << sprite->uv.u0 << ", " << sprite->uv.v0 << std::endl;
}
```

## Async Loading System

### Worker Threads

```cpp
// AssetLoader is created automatically by AssetManager
// Configure number of threads
auto loader = std::make_unique<Assets::AssetLoader>(4);  // 4 worker threads

// Queue high-priority load
Assets::LoadTask task;
task.filepath = "textures/boss.png";
task.priority = Assets::LoadPriority::Critical;
task.loadFunc = []() { /* loading code */ };
task.callback = []() { /* completion callback */ };

loader->QueueLoad(std::move(task));
```

### Progressive Loading

```cpp
#include "Assets/AssetLoader.h"

auto loader = std::make_unique<Assets::AssetLoader>();
Assets::ProgressiveLoader progressLoader(*loader);

std::vector<std::string> assets = {
    "textures/player.png",
    "textures/enemy.png",
    "audio/music.ogg"
};

progressLoader.LoadWithProgress(
    assets,
    [](float progress, const std::string& current) {
        std::cout << "Loading " << current << " ("
                  << (int)(progress * 100) << "%)" << std::endl;
    },
    []() {
        std::cout << "All assets loaded!" << std::endl;
    }
);
```

### Level Preloading

```cpp
Assets::AssetPreloader preloader(*loader);

// Start preloading level assets in background
std::vector<std::string> levelAssets = {
    "textures/level2_tileset.png",
    "audio/level2_music.ogg",
    "data/level2.json"
};

preloader.PreloadLevel("level_2", levelAssets);

// Check if ready
if (preloader.IsLevelReady("level_2")) {
    TransitionToLevel("level_2");
}

// Monitor progress
float progress = preloader.GetLevelProgress("level_2");
DrawLoadingBar(progress);
```

## Resource Pooling

### Object Pool

```cpp
#include "Assets/ResourcePool.h"

// Create object pool
auto bufferFactory = []() { return new Assets::PoolableBuffer(4096); };
auto bufferPool = std::make_unique<Assets::ObjectPool<Assets::PoolableBuffer>>(
    bufferFactory,
    10,    // Initial size
    100    // Max size
);

// Acquire buffer from pool
auto buffer = bufferPool->Acquire();
if (buffer.IsValid()) {
    // Use buffer
    unsigned char* data = buffer->GetData();
    // ... work with data

    // Buffer automatically returns to pool when 'buffer' goes out of scope
}

// Check pool statistics
auto stats = bufferPool->GetStats();
std::cout << "Pool efficiency: " << (stats.reuseRatio * 100) << "%" << std::endl;
```

### Resource Pool Manager

```cpp
// Register pools for different types
Assets::ResourcePool poolMgr;

poolMgr.RegisterPool<MyCustomAsset>(
    []() { return new MyCustomAsset(); },
    20,   // Initial size
    200   // Max size
);

// Acquire from pool
auto asset = poolMgr.Acquire<MyCustomAsset>();
if (asset.IsValid()) {
    // Use asset
}

// Get statistics
auto stats = poolMgr.GetStats<MyCustomAsset>();
```

## Asset Packing Tools

### Pack Assets

```bash
# Pack all assets with compression
./tools/pack_assets.py assets/ packed/ --compression 6 --bundle

# Pack specific types
./tools/pack_assets.py assets/ packed/ \
  --types textures audio \
  --bundle

# Validate packed assets
./tools/pack_assets.py assets/ packed/ --validate
```

### Asset Bundle Format

The packer creates:

1. **Compressed Assets**: Individual `.zpk` files with compression
2. **Manifest**: `asset_manifest.json` with metadata
3. **Bundle** (optional): Single `assets.bundle` file containing all assets

```json
{
  "version": "1.0",
  "assets": [
    {
      "path": "textures/player.png.zpk",
      "original_path": "textures/player.png",
      "type": "textures",
      "size": 45632,
      "original_size": 102400,
      "compressed": true,
      "hash": "a1b2c3..."
    }
  ],
  "total_size": 1024000,
  "compressed_size": 512000
}
```

## Best Practices

### Memory Optimization

1. **Use Texture Atlases**: Combine sprites to reduce memory and draw calls
2. **Set Appropriate Budgets**: Configure memory budgets based on target hardware
3. **Enable Garbage Collection**: Let the system automatically unload unused assets
4. **Streaming for Large Assets**: Use streaming for assets > 10 MB

### Loading Performance

1. **Async Loading**: Use async loading for all non-critical assets
2. **Priority Queues**: Set appropriate priorities (Critical > High > Normal > Low)
3. **Preload Levels**: Load next level assets while current level is active
4. **Batch Loading**: Load related assets together

### Development Workflow

1. **Hot-Reloading**: Enable in development builds only
2. **Asset Validation**: Validate assets before adding to project
3. **Naming Conventions**: Follow consistent naming (see assets/README.md)
4. **Version Control**: Commit only source assets, build atlases/bundles separately

### Production Build

```cpp
// Disable hot-reloading
#ifndef NDEBUG
    assetMgr.SetHotReloadEnabled(true);   // Development
#else
    assetMgr.SetHotReloadEnabled(false);  // Production
#endif

// Increase memory budget for production
budget.maxTotalMemory = 1024 * 1024 * 1024;  // 1 GB

// Preload critical assets
assetMgr.PreloadAssets({
    "textures/ui_atlas.png",
    "audio/menu_music.ogg",
    "shaders/sprite.vert",
    "shaders/sprite.frag"
});
```

## Performance Metrics

### Expected Performance

| Operation | Time (Debug) | Time (Release) |
|-----------|-------------|----------------|
| Load 256x256 PNG | ~2-5 ms | ~1-2 ms |
| Load 2048x2048 PNG | ~20-50 ms | ~10-20 ms |
| Generate 2048x2048 Atlas | ~100-200 ms | ~50-100 ms |
| Memory allocation (pooled) | ~0.001 ms | ~0.0005 ms |
| Hot-reload check (1000 assets) | ~5-10 ms | N/A |

### Memory Usage

| Asset Type | Typical Size |
|------------|-------------|
| 256x256 RGBA texture | 256 KB |
| 2048x2048 RGBA texture | 16 MB |
| 10s audio (OGG, stereo) | ~200 KB |
| Simple 3D model (1000 tris) | ~50 KB |

## Troubleshooting

### Asset Not Loading

```cpp
// Validate asset before loading
if (!assetMgr.ValidateAsset("textures/missing.png")) {
    std::cerr << "Asset validation failed" << std::endl;
}

// Check if asset exists
std::string fullPath = assetMgr.GetBasePath() + "textures/missing.png";
// Check file system...
```

### Memory Budget Exceeded

```cpp
// Get memory stats
auto stats = assetMgr.GetMemoryStats();

if (stats.totalUsed > budget.maxTotalMemory) {
    // Force garbage collection
    assetMgr.GarbageCollect(true);

    // Or increase budget
    budget.maxTotalMemory *= 2;
}
```

### Hot-Reload Not Working

1. Ensure hot-reload is enabled: `SetHotReloadEnabled(true)`
2. Check file system permissions
3. Verify `Update()` is called every frame
4. Asset must be loaded before changes are detected

## API Reference

### AssetManager

- `Initialize(budget)` - Initialize with memory budget
- `Load<T>(path)` - Load asset synchronously
- `LoadAsync<T>(path, callback, errorCallback)` - Load async
- `Get<T>(path)` - Get loaded asset
- `Unload(path)` - Unload asset
- `IsLoaded(path)` - Check if loaded
- `Reload(path)` - Hot-reload asset
- `Update(deltaTime)` - Update (call every frame)
- `GetMemoryStats()` - Get memory usage
- `GarbageCollect(aggressive)` - Force cleanup

### TextureAtlas

- `Generate(inputs, settings)` - Generate atlas
- `Load(imagePath, dataPath)` - Load atlas
- `Save(imagePath, dataPath)` - Save atlas
- `GetSprite(name)` - Get sprite info
- `Bind(unit)` - Bind texture
- `GetTextureID()` - Get OpenGL texture ID

### AssetLoader

- `QueueLoad(task)` - Queue single task
- `QueueLoadBatch(tasks)` - Queue multiple tasks
- `StartStreamingLoad(...)` - Start streaming
- `WaitForAll(timeout)` - Wait for completion
- `GetStats()` - Get loading statistics
- `Pause()` / `Resume()` - Control loading

### ResourcePool

- `RegisterPool<T>(factory, initial, max)` - Register pool
- `Acquire<T>()` - Acquire resource
- `GetStats<T>()` - Get pool statistics
- `ShrinkPool<T>(size)` - Shrink pool

## Future Enhancements

- [ ] GPU texture compression (DXT, ETC, ASTC)
- [ ] Multi-threaded texture atlas generation
- [ ] Asset encryption/obfuscation
- [ ] Incremental asset updates
- [ ] Cloud asset delivery
- [ ] Asset usage analytics
- [ ] Automatic LOD generation
- [ ] Procedural asset generation

## Examples

See `/examples/` directory for complete examples:

- `asset_loading_example.cpp` - Basic asset loading
- `texture_atlas_example.cpp` - Atlas generation
- `resource_pooling_example.cpp` - Object pooling
- `async_loading_example.cpp` - Async loading with progress

## Additional Documentation

- [Asset Creation Guidelines](assets/README.md)
- [AssetManager API](src/Assets/AssetManager.h)
- [TextureAtlas API](src/Assets/TextureAtlas.h)
- [ResourcePool API](src/Assets/ResourcePool.h)
- [AssetLoader API](src/Assets/AssetLoader.h)

## License

Part of the SCUMM-style ARPG Engine.
See LICENSE file for details.
