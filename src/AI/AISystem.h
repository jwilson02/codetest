#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace AI {

// Forward declarations
class BehaviorTree;
class StateMachine;
class Pathfinding;
class EnemyAI;
class NPCAI;
class BossAI;

// AI Entity types
enum class AIEntityType {
    ENEMY,
    NPC,
    BOSS
};

// Difficulty scaling settings
struct DifficultySettings {
    float damageMultiplier = 1.0f;
    float healthMultiplier = 1.0f;
    float speedMultiplier = 1.0f;
    float aggroRangeMultiplier = 1.0f;
    float xpMultiplier = 1.0f;
    int additionalAbilities = 0;
    bool enableAdvancedTactics = false;
};

// AI Entity base structure
struct AIEntity {
    int entityId;
    AIEntityType type;
    void* aiComponent; // Pointer to specific AI (EnemyAI, NPCAI, or BossAI)
    bool isActive;
    float updateInterval; // How often to update this AI (in seconds)
    float timeSinceLastUpdate;

    AIEntity(int id, AIEntityType t)
        : entityId(id), type(t), aiComponent(nullptr),
          isActive(true), updateInterval(0.1f), timeSinceLastUpdate(0.0f) {}
};

/**
 * AISystem - Central manager for all AI entities in the game
 *
 * Responsibilities:
 * - Register and manage all AI entities (enemies, NPCs, bosses)
 * - Update AI behaviors based on update intervals
 * - Manage pathfinding grid and navigation
 * - Apply difficulty scaling
 * - Coordinate group behaviors and flocking
 * - Handle AI events and callbacks
 */
class AISystem {
public:
    AISystem();
    ~AISystem();

    // Initialization
    void Initialize(int worldWidth, int worldHeight);
    void Shutdown();

    // Entity management
    int RegisterEnemy(int entityId, const std::string& archetypeName);
    int RegisterNPC(int entityId, const std::string& npcType);
    int RegisterBoss(int entityId, const std::string& bossName);
    void UnregisterEntity(int aiId);

    // Update
    void Update(float deltaTime);
    void UpdateEntity(int aiId, float deltaTime);

    // Difficulty scaling
    void SetDifficulty(float difficultyLevel); // 0.0 = Easy, 1.0 = Normal, 2.0 = Hard, 3.0+ = Extreme
    const DifficultySettings& GetDifficultySettings() const { return m_difficultySettings; }
    void ApplyDifficultyScaling(int aiId);

    // Pathfinding access
    std::shared_ptr<Pathfinding> GetPathfinding() { return m_pathfinding; }

    // Aggro and threat management
    void AddThreat(int enemyId, int targetId, float threatValue);
    void RemoveThreat(int enemyId, int targetId);
    void ClearAllThreat(int enemyId);
    int GetHighestThreatTarget(int enemyId);

    // Group behavior coordination
    void RegisterToGroup(int aiId, const std::string& groupName);
    void UnregisterFromGroup(int aiId, const std::string& groupName);
    std::vector<int> GetGroupMembers(const std::string& groupName);

    // AI State queries
    bool IsEntityActive(int aiId) const;
    void SetEntityActive(int aiId, bool active);
    AIEntityType GetEntityType(int aiId) const;

    // Event callbacks
    using AIEventCallback = std::function<void(int aiId, const std::string& eventName, void* eventData)>;
    void RegisterEventCallback(const std::string& eventName, AIEventCallback callback);
    void TriggerEvent(int aiId, const std::string& eventName, void* eventData = nullptr);

    // Debug and statistics
    int GetActiveAICount() const;
    int GetAICountByType(AIEntityType type) const;
    void EnableDebugDrawing(bool enable) { m_debugDraw = enable; }
    bool IsDebugDrawingEnabled() const { return m_debugDraw; }

private:
    // AI entities storage
    std::unordered_map<int, std::unique_ptr<AIEntity>> m_aiEntities;
    int m_nextAIId;

    // Subsystems
    std::shared_ptr<Pathfinding> m_pathfinding;

    // Difficulty
    DifficultySettings m_difficultySettings;
    float m_currentDifficultyLevel;

    // Threat tables (enemyId -> (targetId -> threatValue))
    std::unordered_map<int, std::unordered_map<int, float>> m_threatTables;

    // Group management (groupName -> list of aiIds)
    std::unordered_map<std::string, std::vector<int>> m_groups;

    // Event system
    std::unordered_map<std::string, std::vector<AIEventCallback>> m_eventCallbacks;

    // Debug
    bool m_debugDraw;

    // Helper methods
    void UpdateDifficultySettings();
    void CleanupInactiveEntities();
    AIEntity* GetAIEntity(int aiId);
    const AIEntity* GetAIEntity(int aiId) const;
};

} // namespace AI
