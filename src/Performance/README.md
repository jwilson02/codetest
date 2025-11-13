# Performance Optimization and Profiling System

A comprehensive performance optimization and profiling system for the SCUMM-style ARPG game engine. This system provides tools for profiling, memory tracking, object pooling, spatial partitioning, and multithreading.

## Features

### 1. Performance Profiler
- **Frame time tracking** with microsecond precision
- **Hierarchical profiling** for nested function calls
- **Performance budgets** for each system (Rendering, Physics, AI, etc.)
- **Bottleneck detection** with automatic reporting
- **Export to JSON/CSV** for external analysis
- **60 FPS target** (16.67ms per frame)

### 2. Memory Tracker
- **Allocation tracking** with file/line information
- **Memory leak detection** at shutdown
- **Category-based tracking** (Rendering, Audio, Physics, etc.)
- **Peak memory monitoring**
- **Snapshot and comparison** for memory profiling
- **Detailed reports** with leak information

### 3. Object Pooling
- **Generic object pool** template for any type
- **Thread-safe** operations
- **Automatic growth** with configurable limits
- **Block allocator** for fixed-size objects
- **STL-compatible allocator** for containers

### 4. Spatial Partitioning
- **Quadtree** for 2D spatial queries
- **Octree** for 3D spatial queries
- **Spatial hash grid** for broad-phase collision detection
- **Frustum culling** for rendering optimization
- **LOD (Level of Detail)** system with distance-based switching

### 5. Thread Pool
- **Worker thread management** with automatic hardware detection
- **Task priorities** (Low, Normal, High, Critical)
- **Future-based results** for async operations
- **Parallel for/foreach** helpers
- **Job system** for game-specific tasks

## Performance Budgets

Target: **60 FPS (16.67ms per frame)**

| System     | Budget (ms) | Percentage |
|-----------|-------------|------------|
| Rendering | 8.00        | 48%        |
| Physics   | 2.00        | 12%        |
| AI        | 2.00        | 12%        |
| Gameplay  | 2.00        | 12%        |
| UI        | 1.00        | 6%         |
| Audio     | 0.50        | 3%         |
| Overhead  | 1.17        | 7%         |

## Usage Examples

### Profiling

```cpp
#include <Performance/Profiler.h>

// Initialize profiler
Performance::Profiler::GetInstance().Initialize(120); // 120 frames history

// In your game loop
void GameLoop() {
    // Begin frame
    Performance::Profiler::GetInstance().BeginFrame();

    // Profile a scope
    {
        PROFILE_SCOPE("Update");
        Update();
    }

    {
        PROFILE_SCOPE("Render");
        Render();
    }

    // End frame
    Performance::Profiler::GetInstance().EndFrame();
}

// Profile a function
void MyExpensiveFunction() {
    PROFILE_FUNCTION();
    // ... function code ...
}

// Print results
Performance::Profiler::GetInstance().PrintResults(true);

// Export to JSON
Performance::Profiler::GetInstance().ExportToJSON("profile.json");

// Detect bottlenecks
auto bottlenecks = Performance::Profiler::GetInstance().DetectBottlenecks(10.0);
for (const auto& bn : bottlenecks) {
    std::cout << "Bottleneck: " << bn.systemName << std::endl;
}
```

### Memory Tracking

```cpp
#include <Performance/MemoryTracker.h>

// Initialize memory tracker
Performance::MemoryTracker::GetInstance().Initialize();

// Track allocations
void* ptr = malloc(1024);
TRACK_ALLOC(ptr, 1024, Performance::MemoryCategory::General);

// Track deallocations
TRACK_FREE(ptr);
free(ptr);

// Or use tracked new/delete
auto* obj = NEW_TRACKED(MyObject, Performance::MemoryCategory::Gameplay);
DELETE_TRACKED(obj);

// Print stats
Performance::MemoryTracker::GetInstance().PrintStats();

// Take snapshots
Performance::MemoryTracker::GetInstance().TakeSnapshot("before_load");
// ... load assets ...
Performance::MemoryTracker::GetInstance().TakeSnapshot("after_load");
Performance::MemoryTracker::GetInstance().CompareWithSnapshot("before_load");

// Detect leaks
auto leaks = Performance::MemoryTracker::GetInstance().DetectLeaks();
if (!leaks.empty()) {
    Performance::MemoryTracker::GetInstance().PrintLeaks();
}
```

### Object Pooling

```cpp
#include <Performance/ObjectPool.h>

// Create a pool for particles
Performance::ObjectPool<Particle> particlePool(100, 1000);

// Acquire object from pool
auto particle = particlePool.Acquire();
particle->SetPosition(glm::vec3(0, 0, 0));
particle->SetVelocity(glm::vec3(1, 0, 0));

// Object automatically returns to pool when shared_ptr goes out of scope

// Check pool stats
std::cout << "Active: " << particlePool.GetActiveCount() << std::endl;
std::cout << "Free: " << particlePool.GetFreeCount() << std::endl;
```

### Spatial Partitioning

```cpp
#include <Performance/SpatialPartition.h>

// Create octree for 3D world
AABB worldBounds(glm::vec3(-1000, -1000, -1000), glm::vec3(1000, 1000, 1000));
Performance::Octree octree(worldBounds, 8, 8);

// Insert objects
for (auto* entity : entities) {
    octree.Insert(entity);
}

// Query visible objects
Performance::Frustum frustum;
frustum.ExtractFromMatrix(viewProjMatrix);

std::vector<Performance::ISpatialObject*> visibleObjects;
octree.QueryFrustum(frustum, visibleObjects);

// Render only visible objects
for (auto* obj : visibleObjects) {
    Render(obj);
}

// Update LOD based on camera distance
Performance::LODManager::UpdateLODs(lodObjects, cameraPos, viewportHeight, fov);
```

### Thread Pool

```cpp
#include <Performance/ThreadPool.h>

// Create thread pool (auto-detect hardware threads)
Performance::ThreadPool threadPool;

// Enqueue tasks
auto future1 = threadPool.Enqueue(Performance::TaskPriority::High, []() {
    return ComputeExpensiveResult();
});

auto future2 = threadPool.Enqueue([]() {
    ProcessAILogic();
});

// Wait for results
int result = future1.get();
future2.wait();

// Parallel for loop
Performance::ParallelFor(threadPool, 0, 1000, [](size_t i) {
    ProcessEntity(i);
});

// Use job system
Performance::JobSystem::GetInstance().Initialize();

auto job = Performance::JobSystem::GetInstance().Schedule(
    Performance::TaskPriority::Normal,
    []() { return LoadAsset("texture.png"); }
);

auto asset = job.get();
```

## Optimization Guidelines

### CPU Optimization
1. **Profile first** - Always measure before optimizing
2. **Cache-friendly** - Keep data contiguous in memory
3. **Minimize allocations** - Use object pools for frequent allocations
4. **Batch operations** - Group similar operations together
5. **SIMD-friendly** - Use aligned data and simple operations

### Memory Optimization
1. **Pool frequent allocations** - Particles, bullets, effects
2. **Minimize fragmentation** - Use block allocators
3. **Track memory usage** - Monitor with MemoryTracker
4. **Fix leaks immediately** - Check reports regularly

### Rendering Optimization
1. **Frustum culling** - Only render visible objects
2. **Occlusion culling** - Don't render hidden objects
3. **LOD system** - Use lower detail for distant objects
4. **Batch draw calls** - Minimize state changes
5. **Spatial partitioning** - Use octree/quadtree for queries

### Multithreading
1. **Parallelize heavy systems** - AI, physics, procedural generation
2. **Avoid contention** - Minimize shared data access
3. **Use job system** - For game-specific tasks
4. **Profile threads** - Check for load imbalance

## SIMD Optimization (Optional)

For critical paths, consider SIMD optimizations:

```cpp
// Example: Vector operations with GLM (uses SIMD when available)
#define GLM_FORCE_SIMD_AVX2
#include <glm/glm.hpp>

// Batch process positions
void UpdatePositions(std::vector<glm::vec3>& positions,
                     const std::vector<glm::vec3>& velocities,
                     float deltaTime) {
    PROFILE_FUNCTION();

    // GLM will use SIMD instructions when available
    for (size_t i = 0; i < positions.size(); ++i) {
        positions[i] += velocities[i] * deltaTime;
    }
}
```

## Performance Metrics Overlay

Integrate with your UI system to show real-time metrics:

```cpp
void RenderPerformanceOverlay() {
    auto& profiler = Performance::Profiler::GetInstance();
    auto stats = profiler.GetAverageFrameStats(60);

    UI::Text("FPS: " + std::to_string(stats.fps), 10, 10);
    UI::Text("Frame: " + std::to_string(stats.frameTimeMS) + "ms", 10, 30);
    UI::Text("Draw Calls: " + std::to_string(stats.drawCalls), 10, 50);
    UI::Text("Objects: " + std::to_string(stats.objectsRendered), 10, 70);

    // Show bottlenecks
    auto bottlenecks = profiler.DetectBottlenecks(10.0);
    int y = 100;
    for (const auto& bn : bottlenecks) {
        UI::Text("[!] " + bn.systemName + ": " +
                std::to_string(bn.timeMS) + "ms", 10, y);
        y += 20;
    }
}
```

## Analyzing Profile Data

Use the included Python tool to analyze exported profile data:

```bash
# Analyze JSON export
python tools/profile_analyzer.py profile.json --output report.html

# Analyze CSV export
python tools/profile_analyzer.py frame_history.csv --threshold 2.0

# Generate report quietly
python tools/profile_analyzer.py profile.json --quiet
```

The analyzer will:
- Detect performance issues
- Highlight bottlenecks
- Provide optimization recommendations
- Generate detailed HTML reports

## Best Practices

1. **Always profile in Release builds** - Debug builds have overhead
2. **Profile on target hardware** - Performance varies by platform
3. **Check frame time variance** - Consistent performance is important
4. **Monitor memory throughout development** - Catch leaks early
5. **Use spatial partitioning** - Essential for large worlds
6. **Pool frequently allocated objects** - Especially for particles/bullets
7. **Keep systems under budget** - Stay within allocated time
8. **Parallelize independent systems** - Use thread pool effectively

## Building

The Performance module is automatically built with the main project:

```bash
mkdir build && cd build
cmake .. -DENABLE_PROFILING=ON -DENABLE_MEMORY_TRACKING=ON
cmake --build .
```

Options:
- `ENABLE_PROFILING` - Enable performance profiling (default: ON)
- `ENABLE_MEMORY_TRACKING` - Enable memory tracking (default: ON)

## Dependencies

- **C++17** or later
- **GLM** for math operations
- **pthreads** for threading (included in C++11)
- **Python 3.6+** for profile analyzer (optional)

## License

Same as the main project.
