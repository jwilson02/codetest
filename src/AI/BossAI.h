#pragma once

#include "BehaviorTree.h"
#include "Pathfinding.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace AI {

// Forward declarations
class AISystem;
struct DifficultySettings;

// Boss phase states
enum class BossPhase {
    INTRO,      // Cinematic introduction
    PHASE_1,    // First combat phase
    PHASE_2,    // Second combat phase (usually at 66% HP)
    PHASE_3,    // Third combat phase (usually at 33% HP)
    ENRAGE,     // Berserk mode (low HP or timer)
    DEFEATED,   // Boss defeated
    TRANSITIONING // Between phases
};

// Special attack types
enum class SpecialAttackType {
    AREA_OF_EFFECT,     // Large AOE damage
    SUMMON_ADDS,        // Spawn additional enemies
    CHARGE_ATTACK,      // Powerful single-target
    GROUND_SLAM,        // Knockback AOE
    PROJECTILE_BARRAGE, // Multiple projectiles
    BEAM_ATTACK,        // Continuous damage beam
    TELEPORT_STRIKE,    // Teleport and attack
    ENVIRONMENT,        // Modify environment
    BUFF_SELF,          // Increase own stats
    DEBUFF_PLAYERS      // Weaken players
};

// Boss mechanic data
struct BossMechanic {
    std::string name;
    SpecialAttackType type;
    float damage;
    float range;
    float cooldown;
    float lastUsedTime;
    float castTime;
    float radius;
    bool isTelegraphed;
    bool canInterrupt;
    int maxUses; // -1 for unlimited
    int usesRemaining;
    std::vector<BossPhase> activePhases; // Which phases this mechanic is available

    BossMechanic()
        : damage(50.0f)
        , range(10.0f)
        , cooldown(10.0f)
        , lastUsedTime(0.0f)
        , castTime(2.0f)
        , radius(5.0f)
        , isTelegraphed(true)
        , canInterrupt(true)
        , maxUses(-1)
        , usesRemaining(-1)
    {}

    bool IsAvailableInPhase(BossPhase phase) const {
        if (activePhases.empty()) return true; // Available in all phases
        for (auto p : activePhases) {
            if (p == phase) return true;
        }
        return false;
    }
};

// Phase transition data
struct PhaseTransition {
    BossPhase fromPhase;
    BossPhase toPhase;
    float healthThreshold; // Trigger at this health %
    float timeThreshold;   // Or trigger after this many seconds (-1 to disable)
    bool hasCustomTrigger;
    std::function<bool()> customTrigger;
    std::string transitionAnimation;
    float transitionDuration;

    PhaseTransition()
        : fromPhase(BossPhase::PHASE_1)
        , toPhase(BossPhase::PHASE_2)
        , healthThreshold(0.66f)
        , timeThreshold(-1.0f)
        , hasCustomTrigger(false)
        , transitionDuration(3.0f)
    {}
};

// Add spawn data
struct AddSpawn {
    std::string enemyType;
    int count;
    WorldPos spawnLocation;
    float spawnDelay; // Time between spawns

    AddSpawn()
        : count(1)
        , spawnDelay(0.5f)
    {}
};

// Boss stats
struct BossStats {
    float maxHealth;
    float currentHealth;
    float baseSpeed;
    float currentSpeed;
    float baseDamage;
    float currentDamage;
    float armor;
    float magicResist;
    bool isInvulnerable; // During phase transitions

    BossStats()
        : maxHealth(1000.0f)
        , currentHealth(1000.0f)
        , baseSpeed(2.5f)
        , currentSpeed(2.5f)
        , baseDamage(25.0f)
        , currentDamage(25.0f)
        , armor(10.0f)
        , magicResist(10.0f)
        , isInvulnerable(false)
    {}

    float GetHealthPercent() const {
        return currentHealth / maxHealth;
    }
};

/**
 * BossAI - Advanced AI system for boss encounters with phases and mechanics
 */
class BossAI {
public:
    BossAI(int aiId, int entityId, const std::string& bossName);
    ~BossAI();

    // Initialization
    void Initialize();
    void SetAISystem(AISystem* aiSystem) { m_aiSystem = aiSystem; }

    // Update
    void Update(float deltaTime);

    // Boss identity
    void SetBossName(const std::string& name) { m_bossName = name; }
    const std::string& GetBossName() const { return m_bossName; }

    // Stats
    BossStats& GetStats() { return m_stats; }
    const BossStats& GetStats() const { return m_stats; }

    // Difficulty scaling
    void ApplyDifficultyScaling(const DifficultySettings& settings);

    // Phase management
    BossPhase GetCurrentPhase() const { return m_currentPhase; }
    void SetPhase(BossPhase phase);
    void AddPhaseTransition(const PhaseTransition& transition);
    bool CheckPhaseTransitions();
    float GetPhaseProgress() const; // 0.0 to 1.0

    // Mechanics
    void AddMechanic(const BossMechanic& mechanic);
    bool CanUseMechanic(const std::string& mechanicName, float currentTime) const;
    void UseMechanic(const std::string& mechanicName);
    BossMechanic* GetMechanic(const std::string& mechanicName);

    // Target management
    void SetPrimaryTarget(int targetId) { m_primaryTargetId = targetId; }
    int GetPrimaryTarget() const { return m_primaryTargetId; }
    void AddThreatTarget(int targetId, float threat);
    void UpdateTargetSelection();

    // Combat
    void TakeDamage(float damage, int attackerId);
    void Heal(float amount);
    bool IsDead() const { return m_currentPhase == BossPhase::DEFEATED; }
    bool IsDefeated() const { return IsDead(); }

    // Special mechanics
    void SpawnAdds(const AddSpawn& spawnData);
    void TeleportTo(const WorldPos& position);
    void CreateHazard(const WorldPos& position, float radius, float duration, float damagePerSecond);
    void ApplyBuff(const std::string& buffName, float multiplier, float duration);
    void RemoveBuff(const std::string& buffName);

    // Enrage system
    void SetEnrageTimer(float seconds);
    void TriggerEnrage();
    bool IsEnraged() const { return m_currentPhase == BossPhase::ENRAGE || m_isEnraged; }

    // Position and movement
    void SetPosition(const WorldPos& pos) { m_position = pos; }
    WorldPos GetPosition() const { return m_position; }
    void SetArenaCenter(const WorldPos& center) { m_arenaCenter = center; }
    void SetArenaRadius(float radius) { m_arenaRadius = radius; }

    // Pathfinding
    void SetPath(const PathResult& path);
    void ClearPath();
    bool HasPath() const { return !m_currentPath.worldPath.empty(); }
    void UpdatePathing(float deltaTime);

    // Telegraph system
    void StartTelegraph(const std::string& mechanicName, float duration);
    bool IsTelegraphing() const { return m_isTelegraphing; }
    float GetTelegraphProgress() const;
    const std::string& GetTelegraphMechanic() const { return m_telegraphMechanic; }

    // Invulnerability
    void SetInvulnerable(bool invulnerable) { m_stats.isInvulnerable = invulnerable; }
    bool IsInvulnerable() const { return m_stats.isInvulnerable; }

    // Crowd control immunity (bosses are usually immune to stuns, etc.)
    void SetCCImmune(bool immune) { m_ccImmune = immune; }
    bool IsCCImmune() const { return m_ccImmune; }

    // Event callbacks
    using PhaseChangeCallback = std::function<void(BossPhase oldPhase, BossPhase newPhase)>;
    void SetPhaseChangeCallback(PhaseChangeCallback callback) { m_phaseChangeCallback = callback; }

    using DeathCallback = std::function<void()>;
    void SetDeathCallback(DeathCallback callback) { m_deathCallback = callback; }

    // Animation state
    void SetAnimationState(const std::string& state) { m_animationState = state; }
    const std::string& GetAnimationState() const { return m_animationState; }

    // Debug
    int GetAIId() const { return m_aiId; }
    int GetEntityId() const { return m_entityId; }
    float GetTimeSinceEncounterStart() const { return m_encounterTime; }

private:
    // IDs
    int m_aiId;
    int m_entityId;
    int m_primaryTargetId;

    // AI system reference
    AISystem* m_aiSystem;

    // Boss identity
    std::string m_bossName;

    // Stats
    BossStats m_stats;

    // Phase system
    BossPhase m_currentPhase;
    BossPhase m_previousPhase;
    std::vector<PhaseTransition> m_phaseTransitions;
    bool m_isTransitioning;
    float m_transitionTime;
    float m_transitionDuration;

    // Mechanics
    std::vector<BossMechanic> m_mechanics;

    // Behavior tree (can switch between phases)
    std::shared_ptr<BehaviorTree> m_behaviorTree;

    // Position and movement
    WorldPos m_position;
    WorldPos m_velocity;
    PathResult m_currentPath;
    size_t m_currentWaypointIndex;
    WorldPos m_arenaCenter;
    float m_arenaRadius;

    // Telegraph system
    bool m_isTelegraphing;
    std::string m_telegraphMechanic;
    float m_telegraphDuration;
    float m_telegraphTime;

    // Enrage
    bool m_isEnraged;
    float m_enrageTimer;
    float m_enrageTime;

    // Timing
    float m_encounterTime;
    float m_phaseTime;

    // Buffs (buffName -> (multiplier, remainingDuration))
    std::unordered_map<std::string, std::pair<float, float>> m_activeBuffs;

    // Crowd control immunity
    bool m_ccImmune;

    // Difficulty modifiers
    float m_difficultyDamageMultiplier;
    float m_difficultyHealthMultiplier;

    // Callbacks
    PhaseChangeCallback m_phaseChangeCallback;
    DeathCallback m_deathCallback;

    // Animation
    std::string m_animationState;

    // Helper methods
    void BuildBehaviorTree();
    void UpdatePhase(float deltaTime);
    void UpdateEnrage(float deltaTime);
    void UpdateBuffs(float deltaTime);
    void UpdateTelegraph(float deltaTime);
    void ExecuteMechanic(BossMechanic& mechanic);
    void OnPhaseChange(BossPhase newPhase);

    // Behavior tree actions
    NodeStatus BT_SelectTarget(BehaviorTreeContext& context);
    NodeStatus BT_MoveToTarget(BehaviorTreeContext& context);
    NodeStatus BT_BasicAttack(BehaviorTreeContext& context);
    NodeStatus BT_UseSpecialMechanic(BehaviorTreeContext& context);
    NodeStatus BT_MoveToArenaCenter(BehaviorTreeContext& context);

    // Behavior tree conditions
    bool BT_HasTarget(BehaviorTreeContext& context);
    bool BT_IsTargetInRange(BehaviorTreeContext& context);
    bool BT_CanUseSpecialMechanic(BehaviorTreeContext& context);
    bool BT_ShouldMoveToCenter(BehaviorTreeContext& context);
};

/**
 * Factory function to create bosses by name
 */
std::shared_ptr<BossAI> CreateBossByName(int aiId, int entityId, const std::string& bossName);

/**
 * Load boss configurations from JSON
 */
bool LoadBossConfigFromJSON(const std::string& filepath);

} // namespace AI
