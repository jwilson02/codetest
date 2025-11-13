#pragma once

#include "BehaviorTree.h"
#include "Pathfinding.h"
#include <string>
#include <vector>
#include <memory>

namespace AI {

// Forward declarations
class AISystem;
struct DifficultySettings;

// Enemy archetypes
enum class EnemyArchetype {
    MELEE,      // Close-range physical attackers
    RANGED,     // Long-range physical attackers
    CASTER,     // Magic users with varied spells
    TANK,       // High health, defensive enemies
    SUPPORT,    // Buffs allies, debuffs enemies
    ASSASSIN,   // High damage, low health, stealth
    BERSERKER   // Aggressive, becomes stronger when damaged
};

// Enemy states
enum class EnemyState {
    IDLE,
    PATROL,
    ALERT,
    CHASE,
    ATTACK,
    RETREAT,
    FLEE,
    STUNNED,
    DEAD
};

// Attack data
struct AttackData {
    std::string name;
    float damage;
    float range;
    float cooldown;
    float lastUsedTime;
    float castTime;           // Time to telegraph/wind up
    float radius;            // AOE radius (0 if single target)
    bool isTelegraphed;      // Visual warning before attack
    bool canInterrupt;       // Can be interrupted during cast
    std::string animationName;

    AttackData()
        : damage(10.0f)
        , range(1.5f)
        , cooldown(1.0f)
        , lastUsedTime(0.0f)
        , castTime(0.3f)
        , radius(0.0f)
        , isTelegraphed(true)
        , canInterrupt(true)
    {}
};

// Ability data
struct AbilityData {
    std::string name;
    float cooldown;
    float lastUsedTime;
    float duration;
    float healthThreshold;   // Use when health below this %
    bool isDefensive;

    AbilityData()
        : cooldown(5.0f)
        , lastUsedTime(0.0f)
        , duration(2.0f)
        , healthThreshold(1.0f)
        , isDefensive(false)
    {}
};

// Enemy stats
struct EnemyStats {
    float maxHealth;
    float currentHealth;
    float baseSpeed;
    float currentSpeed;
    float baseDamage;
    float currentDamage;
    float aggroRange;
    float attackRange;
    float retreatHealthThreshold; // Retreat when health below this %
    float leashRange;            // Max distance from spawn before resetting

    EnemyStats()
        : maxHealth(100.0f)
        , currentHealth(100.0f)
        , baseSpeed(3.0f)
        , currentSpeed(3.0f)
        , baseDamage(10.0f)
        , currentDamage(10.0f)
        , aggroRange(10.0f)
        , attackRange(1.5f)
        , retreatHealthThreshold(0.2f)
        , leashRange(30.0f)
    {}

    float GetHealthPercent() const {
        return currentHealth / maxHealth;
    }
};

/**
 * EnemyAI - Manages enemy behaviors, combat, and decision making
 */
class EnemyAI {
public:
    EnemyAI(int aiId, int entityId, const std::string& archetypeName);
    ~EnemyAI();

    // Initialization
    void Initialize();
    void SetAISystem(AISystem* aiSystem) { m_aiSystem = aiSystem; }

    // Update
    void Update(float deltaTime);

    // Archetype setup
    void SetArchetype(EnemyArchetype archetype);
    EnemyArchetype GetArchetype() const { return m_archetype; }

    // Stats
    EnemyStats& GetStats() { return m_stats; }
    const EnemyStats& GetStats() const { return m_stats; }

    // Difficulty scaling
    void ApplyDifficultyScaling(const DifficultySettings& settings);

    // State
    EnemyState GetState() const { return m_currentState; }
    void SetState(EnemyState state) { m_currentState = state; }

    // Target management
    void SetTarget(int targetId) { m_targetId = targetId; }
    int GetTarget() const { return m_targetId; }
    bool HasTarget() const { return m_targetId >= 0; }
    void ClearTarget() { m_targetId = -1; }

    // Combat
    void TakeDamage(float damage, int attackerId);
    bool IsDead() const { return m_currentState == EnemyState::DEAD; }
    bool IsInCombat() const;

    // Attacks and abilities
    void AddAttack(const AttackData& attack);
    void AddAbility(const AbilityData& ability);
    bool CanUseAttack(const std::string& attackName, float currentTime) const;
    bool CanUseAbility(const std::string& abilityName, float currentTime) const;

    // Position and movement
    void SetPosition(const WorldPos& pos) { m_position = pos; }
    WorldPos GetPosition() const { return m_position; }
    void SetSpawnPosition(const WorldPos& pos) { m_spawnPosition = pos; }
    WorldPos GetSpawnPosition() const { return m_spawnPosition; }

    // Pathfinding
    void SetPath(const PathResult& path);
    void ClearPath();
    bool HasPath() const { return !m_currentPath.worldPath.empty(); }
    WorldPos GetNextWaypoint();
    void UpdatePathing(float deltaTime);

    // Group behavior
    void SetGroup(const std::string& groupName) { m_groupName = groupName; }
    const std::string& GetGroup() const { return m_groupName; }
    bool IsInGroup() const { return !m_groupName.empty(); }

    // Tactical awareness
    float GetDistanceToTarget() const;
    bool IsTargetInRange(float range) const;
    bool IsTargetInLineOfSight() const;
    std::vector<int> GetNearbyAllies(float radius) const;
    std::vector<int> GetNearbyEnemies(float radius) const;

    // Special behaviors for specific archetypes
    void UpdateMeleeBehavior(float deltaTime);
    void UpdateRangedBehavior(float deltaTime);
    void UpdateCasterBehavior(float deltaTime);
    void UpdateTankBehavior(float deltaTime);
    void UpdateSupportBehavior(float deltaTime);
    void UpdateAssassinBehavior(float deltaTime);
    void UpdateBerserkerBehavior(float deltaTime);

    // Telegraphed attacks
    void StartTelegraph(const std::string& attackName, float duration);
    bool IsTelegraphing() const { return m_isTelegraphing; }
    float GetTelegraphProgress() const;

    // Stun and crowd control
    void Stun(float duration);
    bool IsStunned() const { return m_stunTime > 0.0f; }

    // Debug
    int GetAIId() const { return m_aiId; }
    int GetEntityId() const { return m_entityId; }

private:
    // IDs
    int m_aiId;
    int m_entityId;
    int m_targetId;

    // AI system reference
    AISystem* m_aiSystem;

    // Archetype and state
    EnemyArchetype m_archetype;
    EnemyState m_currentState;
    std::string m_archetypeName;

    // Stats
    EnemyStats m_stats;

    // Behavior tree
    std::shared_ptr<BehaviorTree> m_behaviorTree;

    // Position and movement
    WorldPos m_position;
    WorldPos m_spawnPosition;
    WorldPos m_velocity;
    PathResult m_currentPath;
    size_t m_currentWaypointIndex;

    // Combat data
    std::vector<AttackData> m_attacks;
    std::vector<AbilityData> m_abilities;

    // Telegraph system
    bool m_isTelegraphing;
    std::string m_telegraphAttack;
    float m_telegraphDuration;
    float m_telegraphTime;

    // Timing
    float m_timeSinceLastAttack;
    float m_timeSinceStateChange;
    float m_stunTime;

    // Group
    std::string m_groupName;

    // Difficulty modifiers
    float m_difficultyDamageMultiplier;
    float m_difficultyHealthMultiplier;
    float m_difficultySpeedMultiplier;

    // Helper methods
    void BuildBehaviorTree();
    void SetupArchetypeStats();
    void SetupArchetypeAttacks();
    void UpdateTelegraph(float deltaTime);
    void UpdateStun(float deltaTime);
    void UpdateThreat();

    // Behavior tree actions
    NodeStatus BT_FindTarget(BehaviorTreeContext& context);
    NodeStatus BT_ChaseTarget(BehaviorTreeContext& context);
    NodeStatus BT_AttackTarget(BehaviorTreeContext& context);
    NodeStatus BT_UseAbility(BehaviorTreeContext& context);
    NodeStatus BT_Retreat(BehaviorTreeContext& context);
    NodeStatus BT_ReturnToSpawn(BehaviorTreeContext& context);
    NodeStatus BT_Patrol(BehaviorTreeContext& context);

    // Behavior tree conditions
    bool BT_HasTarget(BehaviorTreeContext& context);
    bool BT_IsTargetInRange(BehaviorTreeContext& context);
    bool BT_IsHealthLow(BehaviorTreeContext& context);
    bool BT_IsTooFarFromSpawn(BehaviorTreeContext& context);
    bool BT_CanAttack(BehaviorTreeContext& context);
    bool BT_ShouldUseAbility(BehaviorTreeContext& context);
};

/**
 * Factory function to create enemies by archetype name
 */
std::shared_ptr<EnemyAI> CreateEnemyByArchetype(int aiId, int entityId, const std::string& archetypeName);

/**
 * Load enemy archetypes from JSON configuration
 */
bool LoadEnemyArchetypesFromJSON(const std::string& filepath);

} // namespace AI
