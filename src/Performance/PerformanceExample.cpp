/**
 * @file PerformanceExample.cpp
 * @brief Example demonstrating the Performance Optimization System
 *
 * This file shows how to integrate and use the performance profiling,
 * memory tracking, object pooling, spatial partitioning, and multithreading
 * systems in your game.
 */

#include "Profiler.h"
#include "MemoryTracker.h"
#include "ObjectPool.h"
#include "SpatialPartition.h"
#include "ThreadPool.h"

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

// ========== Example Entity Class ==========

class Entity : public Performance::ISpatialObject {
public:
    Entity(uint32_t id, const glm::vec3& pos, float radius)
        : m_ID(id), m_Position(pos), m_Radius(radius) {}

    Performance::AABB GetBounds() const override {
        return Performance::AABB(
            m_Position - glm::vec3(m_Radius),
            m_Position + glm::vec3(m_Radius)
        );
    }

    uint32_t GetID() const override { return m_ID; }

    glm::vec3 GetPosition() const { return m_Position; }
    void SetPosition(const glm::vec3& pos) { m_Position = pos; }

private:
    uint32_t m_ID;
    glm::vec3 m_Position;
    float m_Radius;
};

// ========== Example Particle Class ==========

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;

    Particle() : position(0.0f), velocity(0.0f), lifetime(1.0f) {}

    void Reset() {
        position = glm::vec3(0.0f);
        velocity = glm::vec3(0.0f);
        lifetime = 1.0f;
    }
};

// ========== Example LOD Object ==========

class LODEntity : public Performance::LODManager::ILODObject {
public:
    LODEntity(const glm::vec3& pos, float radius)
        : m_Position(pos), m_Radius(radius), m_CurrentLOD(0) {
        // Define LOD levels
        m_LODLevels.push_back({100.0f, 0, 0.1f});  // High detail
        m_LODLevels.push_back({500.0f, 1, 0.05f}); // Medium detail
        m_LODLevels.push_back({1000.0f, 2, 0.01f}); // Low detail
    }

    glm::vec3 GetPosition() const override { return m_Position; }
    float GetBoundingRadius() const override { return m_Radius; }

    void SetLODLevel(uint32_t level) override { m_CurrentLOD = level; }

    const std::vector<Performance::LODManager::LODLevel>& GetLODLevels() const override {
        return m_LODLevels;
    }

    uint32_t GetCurrentLOD() const { return m_CurrentLOD; }

private:
    glm::vec3 m_Position;
    float m_Radius;
    uint32_t m_CurrentLOD;
    std::vector<Performance::LODManager::LODLevel> m_LODLevels;
};

// ========== Example Game System ==========

class GameSystem {
public:
    void Initialize() {
        PROFILE_FUNCTION();

        std::cout << "\n========== Initializing Performance System ==========\n" << std::endl;

        // Initialize profiler
        Performance::Profiler::GetInstance().Initialize(120);
        std::cout << "[Performance] Profiler initialized" << std::endl;

        // Initialize memory tracker
        Performance::MemoryTracker::GetInstance().Initialize();
        std::cout << "[Performance] Memory tracker initialized" << std::endl;

        // Initialize job system
        Performance::JobSystem::GetInstance().Initialize();
        std::cout << "[Performance] Job system initialized" << std::endl;

        // Create object pools
        m_ParticlePool = std::make_unique<Performance::ObjectPool<Particle>>(100, 1000);
        std::cout << "[Performance] Particle pool created (capacity: 100)" << std::endl;

        // Create spatial partitioning
        Performance::AABB worldBounds(
            glm::vec3(-1000, -1000, -1000),
            glm::vec3(1000, 1000, 1000)
        );
        m_Octree = std::make_unique<Performance::Octree>(worldBounds, 8, 8);
        std::cout << "[Performance] Octree created" << std::endl;

        std::cout << "\n====================================================\n" << std::endl;
    }

    void Shutdown() {
        PROFILE_FUNCTION();

        std::cout << "\n========== Shutting Down Performance System ==========\n" << std::endl;

        // Print final statistics
        Performance::Profiler::GetInstance().PrintResults(true);
        Performance::MemoryTracker::GetInstance().PrintStats();

        // Export profiling data
        Performance::Profiler::GetInstance().ExportToJSON("profile_final.json");
        Performance::Profiler::GetInstance().ExportToCSV("frame_history.csv");

        // Shutdown systems
        Performance::JobSystem::GetInstance().Shutdown();
        Performance::MemoryTracker::GetInstance().Shutdown();
        Performance::Profiler::GetInstance().Shutdown();

        std::cout << "\n====================================================\n" << std::endl;
    }

    void Update(float deltaTime) {
        PROFILE_FUNCTION();

        // Begin profiling frame
        Performance::Profiler::GetInstance().BeginFrame();

        // Update different systems
        UpdatePhysics(deltaTime);
        UpdateAI(deltaTime);
        UpdateGameplay(deltaTime);
        UpdateParticles(deltaTime);

        // End profiling frame
        Performance::Profiler::GetInstance().EndFrame();
    }

    void Render(const glm::mat4& viewProj, const glm::vec3& cameraPos) {
        PROFILE_FUNCTION();

        size_t objectsRendered = 0;
        size_t objectsCulled = 0;

        {
            PROFILE_SCOPE("Frustum Culling");

            // Extract frustum from view-projection matrix
            Performance::Frustum frustum;
            frustum.ExtractFromMatrix(viewProj);

            // Query visible objects
            std::vector<Performance::ISpatialObject*> visibleObjects;
            m_Octree->QueryFrustum(frustum, visibleObjects);

            objectsRendered = visibleObjects.size();
            objectsCulled = m_Entities.size() - objectsRendered;

            // Render visible objects
            {
                PROFILE_SCOPE("Draw Calls");
                for (auto* obj : visibleObjects) {
                    // Render object...
                }
            }
        }

        {
            PROFILE_SCOPE("LOD Update");
            // Update LOD for LOD-enabled objects
            std::vector<Performance::LODManager::ILODObject*> lodPtrs;
            for (auto& obj : m_LODEntities) {
                lodPtrs.push_back(&obj);
            }
            Performance::LODManager::UpdateLODs(lodPtrs, cameraPos, 720, 60.0f);
        }

        // Update render stats
        Performance::Profiler::GetInstance().UpdateRenderStats(
            objectsRendered, // draw calls (simplified)
            objectsRendered * 100, // triangles (estimated)
            objectsRendered,
            objectsCulled
        );
    }

    void SimulateFrame(float deltaTime, const glm::mat4& viewProj, const glm::vec3& cameraPos) {
        PROFILE_SCOPE("Frame");

        Update(deltaTime);
        Render(viewProj, cameraPos);

        // Check for bottlenecks every 60 frames
        static int frameCount = 0;
        if (++frameCount >= 60) {
            frameCount = 0;

            auto bottlenecks = Performance::Profiler::GetInstance().DetectBottlenecks(10.0);
            if (!bottlenecks.empty()) {
                std::cout << "\n[Performance] Bottlenecks detected:" << std::endl;
                for (const auto& bn : bottlenecks) {
                    std::cout << "  [" << (bn.isCritical ? "CRITICAL" : "WARNING") << "] "
                              << bn.systemName << ": " << bn.timeMS << "ms "
                              << "(over budget by " << bn.overBudgetPercent << "%)"
                              << std::endl;
                }
            }
        }
    }

    void CreateTestEntities(size_t count) {
        PROFILE_FUNCTION();

        for (size_t i = 0; i < count; ++i) {
            float x = ((rand() % 2000) - 1000) / 1.0f;
            float y = ((rand() % 2000) - 1000) / 1.0f;
            float z = ((rand() % 2000) - 1000) / 1.0f;

            auto entity = std::make_unique<Entity>(i, glm::vec3(x, y, z), 10.0f);
            m_Octree->Insert(entity.get());
            m_Entities.push_back(std::move(entity));
        }

        std::cout << "[Performance] Created " << count << " test entities" << std::endl;
    }

    void CreateTestLODEntities(size_t count) {
        PROFILE_FUNCTION();

        for (size_t i = 0; i < count; ++i) {
            float x = ((rand() % 2000) - 1000) / 1.0f;
            float y = ((rand() % 2000) - 1000) / 1.0f;
            float z = ((rand() % 2000) - 1000) / 1.0f;

            m_LODEntities.emplace_back(glm::vec3(x, y, z), 50.0f);
        }

        std::cout << "[Performance] Created " << count << " LOD entities" << std::endl;
    }

    void DemonstrateParticlePool() {
        PROFILE_SCOPE("Particle Pool Demo");

        std::cout << "\n========== Particle Pool Demonstration ==========\n" << std::endl;

        // Spawn particles
        std::vector<std::shared_ptr<Particle>> particles;
        for (int i = 0; i < 50; ++i) {
            auto particle = m_ParticlePool->Acquire();
            particle->position = glm::vec3(i * 10.0f, 0, 0);
            particle->velocity = glm::vec3(1, 1, 0);
            particles.push_back(particle);
        }

        std::cout << "Active particles: " << m_ParticlePool->GetActiveCount() << std::endl;
        std::cout << "Free particles: " << m_ParticlePool->GetFreeCount() << std::endl;

        // Release half
        particles.resize(25);

        std::cout << "After releasing 25:" << std::endl;
        std::cout << "Active particles: " << m_ParticlePool->GetActiveCount() << std::endl;
        std::cout << "Free particles: " << m_ParticlePool->GetFreeCount() << std::endl;

        std::cout << "\n==============================================\n" << std::endl;
    }

    void DemonstrateMultithreading() {
        PROFILE_SCOPE("Multithreading Demo");

        std::cout << "\n========== Multithreading Demonstration ==========\n" << std::endl;

        auto& jobSystem = Performance::JobSystem::GetInstance();

        // Schedule parallel tasks
        std::vector<std::future<int>> futures;

        for (int i = 0; i < 10; ++i) {
            auto future = jobSystem.Schedule([i]() {
                PROFILE_SCOPE("Async Task");
                // Simulate work
                int result = 0;
                for (int j = 0; j < 1000000; ++j) {
                    result += j % (i + 1);
                }
                return result;
            });

            futures.push_back(std::move(future));
        }

        std::cout << "Scheduled 10 async tasks" << std::endl;

        // Wait for all tasks
        for (auto& future : futures) {
            future.wait();
        }

        std::cout << "All tasks completed" << std::endl;

        std::cout << "\n==============================================\n" << std::endl;
    }

private:
    void UpdatePhysics(float deltaTime) {
        PROFILE_SCOPE("Physics");
        // Simulate physics work
        volatile int dummy = 0;
        for (int i = 0; i < 100000; ++i) {
            dummy += i;
        }
    }

    void UpdateAI(float deltaTime) {
        PROFILE_SCOPE("AI");
        // Simulate AI work
        volatile int dummy = 0;
        for (int i = 0; i < 80000; ++i) {
            dummy += i;
        }
    }

    void UpdateGameplay(float deltaTime) {
        PROFILE_SCOPE("Gameplay");
        // Simulate gameplay work
        volatile int dummy = 0;
        for (int i = 0; i < 50000; ++i) {
            dummy += i;
        }
    }

    void UpdateParticles(float deltaTime) {
        PROFILE_SCOPE("Particles");
        // Simulate particle updates
        volatile int dummy = 0;
        for (int i = 0; i < 30000; ++i) {
            dummy += i;
        }
    }

    std::unique_ptr<Performance::ObjectPool<Particle>> m_ParticlePool;
    std::unique_ptr<Performance::Octree> m_Octree;
    std::vector<std::unique_ptr<Entity>> m_Entities;
    std::vector<LODEntity> m_LODEntities;
};

// ========== Main Example ==========

int main() {
    std::cout << "Performance Optimization System Example\n" << std::endl;

    GameSystem game;

    // Initialize
    game.Initialize();

    // Create test content
    game.CreateTestEntities(1000);
    game.CreateTestLODEntities(100);

    // Demonstrate features
    game.DemonstrateParticlePool();
    game.DemonstrateMultithreading();

    // Simulate game frames
    std::cout << "\n========== Simulating Game Frames ==========\n" << std::endl;

    glm::mat4 viewProj = glm::mat4(1.0f); // Simplified
    glm::vec3 cameraPos(0, 0, 100);

    for (int frame = 0; frame < 180; ++frame) {
        game.SimulateFrame(1.0f / 60.0f, viewProj, cameraPos);

        // Print stats every 60 frames
        if (frame > 0 && frame % 60 == 0) {
            std::cout << "\nFrame " << frame << " - Stats:" << std::endl;
            auto stats = Performance::Profiler::GetInstance().GetAverageFrameStats(60);
            std::cout << "  FPS: " << stats.fps << std::endl;
            std::cout << "  Frame Time: " << stats.frameTimeMS << "ms" << std::endl;
            std::cout << "  Draw Calls: " << stats.drawCalls << std::endl;
            std::cout << "  Objects Rendered: " << stats.objectsRendered << std::endl;
            std::cout << "  Objects Culled: " << stats.objectsCulled << std::endl;
        }
    }

    std::cout << "\n===========================================\n" << std::endl;

    // Shutdown
    game.Shutdown();

    std::cout << "\nExample completed successfully!" << std::endl;
    std::cout << "Check 'profile_final.json' and 'frame_history.csv' for detailed data." << std::endl;
    std::cout << "Run: python tools/profile_analyzer.py profile_final.json" << std::endl;

    return 0;
}
