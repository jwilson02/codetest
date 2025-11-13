#include "BossAI.h"
#include "AISystem.h"
#include <algorithm>
#include <cmath>

namespace AI {

BossAI::BossAI(int aiId, int entityId, const std::string& bossName)
    : m_aiId(aiId)
    , m_entityId(entityId)
    , m_primaryTargetId(-1)
    , m_aiSystem(nullptr)
    , m_bossName(bossName)
    , m_currentPhase(BossPhase::INTRO)
    , m_previousPhase(BossPhase::INTRO)
    , m_isTransitioning(false)
    , m_transitionTime(0.0f)
    , m_transitionDuration(3.0f)
    , m_currentWaypointIndex(0)
    , m_arenaRadius(50.0f)
    , m_isTelegraphing(false)
    , m_telegraphDuration(0.0f)
    , m_telegraphTime(0.0f)
    , m_isEnraged(false)
    , m_enrageTimer(-1.0f)
    , m_enrageTime(0.0f)
    , m_encounterTime(0.0f)
    , m_phaseTime(0.0f)
    , m_ccImmune(true)
    , m_difficultyDamageMultiplier(1.0f)
    , m_difficultyHealthMultiplier(1.0f)
    , m_animationState("idle")
{
}

BossAI::~BossAI() {
}

void BossAI::Initialize() {
    // Setup default boss stats
    m_stats.maxHealth = 1000.0f;
    m_stats.currentHealth = m_stats.maxHealth;
    m_stats.baseDamage = 25.0f;
    m_stats.currentDamage = m_stats.baseDamage;
    m_stats.baseSpeed = 2.5f;
    m_stats.currentSpeed = m_stats.baseSpeed;

    // Start in intro phase
    m_currentPhase = BossPhase::INTRO;

    // Build behavior tree
    BuildBehaviorTree();
}

void BossAI::Update(float deltaTime) {
    if (IsDead()) {
        return;
    }

    // Update encounter time
    m_encounterTime += deltaTime;
    m_phaseTime += deltaTime;

    // Update enrage timer
    UpdateEnrage(deltaTime);

    // Update active buffs
    UpdateBuffs(deltaTime);

    // Update telegraph
    UpdateTelegraph(deltaTime);

    // Check for phase transitions
    if (!m_isTransitioning) {
        if (CheckPhaseTransitions()) {
            // Transitioning to new phase
            m_isTransitioning = true;
            m_transitionTime = 0.0f;
            SetInvulnerable(true);
        }
    }

    // Update phase transition
    if (m_isTransitioning) {
        UpdatePhase(deltaTime);
        return; // Don't process behavior tree during transition
    }

    // Update behavior tree
    if (m_behaviorTree) {
        m_behaviorTree->GetContext().SetValue("DeltaTime", deltaTime);
        m_behaviorTree->GetContext().SetValue("CurrentTime", m_encounterTime);
        m_behaviorTree->GetContext().SetValue("PhaseTime", m_phaseTime);
        m_behaviorTree->Execute();
    }

    // Update pathfinding
    UpdatePathing(deltaTime);

    // Update target selection
    UpdateTargetSelection();
}

void BossAI::SetPhase(BossPhase phase) {
    if (m_currentPhase == phase) {
        return;
    }

    OnPhaseChange(phase);
}

void BossAI::AddPhaseTransition(const PhaseTransition& transition) {
    m_phaseTransitions.push_back(transition);
}

bool BossAI::CheckPhaseTransitions() {
    float healthPercent = m_stats.GetHealthPercent();

    for (const auto& transition : m_phaseTransitions) {
        if (transition.fromPhase != m_currentPhase) {
            continue;
        }

        bool shouldTransition = false;

        // Check health threshold
        if (healthPercent <= transition.healthThreshold) {
            shouldTransition = true;
        }

        // Check time threshold
        if (transition.timeThreshold > 0.0f && m_phaseTime >= transition.timeThreshold) {
            shouldTransition = true;
        }

        // Check custom trigger
        if (transition.hasCustomTrigger && transition.customTrigger) {
            if (transition.customTrigger()) {
                shouldTransition = true;
            }
        }

        if (shouldTransition) {
            m_previousPhase = m_currentPhase;
            m_currentPhase = transition.toPhase;
            m_transitionDuration = transition.transitionDuration;
            m_animationState = transition.transitionAnimation;
            return true;
        }
    }

    return false;
}

float BossAI::GetPhaseProgress() const {
    // Calculate progress through current phase based on health thresholds
    float healthPercent = m_stats.GetHealthPercent();

    switch (m_currentPhase) {
        case BossPhase::INTRO:
            return 0.0f;
        case BossPhase::PHASE_1:
            return 1.0f - (healthPercent / 1.0f);
        case BossPhase::PHASE_2:
            return 1.0f - (healthPercent / 0.66f);
        case BossPhase::PHASE_3:
            return 1.0f - (healthPercent / 0.33f);
        case BossPhase::ENRAGE:
        case BossPhase::DEFEATED:
            return 1.0f;
        default:
            return 0.0f;
    }
}

void BossAI::AddMechanic(const BossMechanic& mechanic) {
    m_mechanics.push_back(mechanic);
}

bool BossAI::CanUseMechanic(const std::string& mechanicName, float currentTime) const {
    for (const auto& mechanic : m_mechanics) {
        if (mechanic.name == mechanicName) {
            // Check if available in current phase
            if (!mechanic.IsAvailableInPhase(m_currentPhase)) {
                return false;
            }

            // Check cooldown
            if ((currentTime - mechanic.lastUsedTime) < mechanic.cooldown) {
                return false;
            }

            // Check max uses
            if (mechanic.maxUses > 0 && mechanic.usesRemaining <= 0) {
                return false;
            }

            return true;
        }
    }
    return false;
}

void BossAI::UseMechanic(const std::string& mechanicName) {
    for (auto& mechanic : m_mechanics) {
        if (mechanic.name == mechanicName) {
            if (mechanic.isTelegraphed) {
                StartTelegraph(mechanicName, mechanic.castTime);
            } else {
                ExecuteMechanic(mechanic);
            }
            break;
        }
    }
}

BossMechanic* BossAI::GetMechanic(const std::string& mechanicName) {
    for (auto& mechanic : m_mechanics) {
        if (mechanic.name == mechanicName) {
            return &mechanic;
        }
    }
    return nullptr;
}

void BossAI::ApplyDifficultyScaling(const DifficultySettings& settings) {
    m_difficultyDamageMultiplier = settings.damageMultiplier;
    m_difficultyHealthMultiplier = settings.healthMultiplier;

    // Apply to stats
    m_stats.maxHealth *= m_difficultyHealthMultiplier;
    m_stats.currentHealth = m_stats.maxHealth;
    m_stats.baseDamage *= m_difficultyDamageMultiplier;
    m_stats.currentDamage = m_stats.baseDamage;

    // Apply to mechanics
    for (auto& mechanic : m_mechanics) {
        mechanic.damage *= m_difficultyDamageMultiplier;
        if (settings.enableAdvancedTactics) {
            mechanic.cooldown *= 0.8f; // Faster cooldowns on higher difficulty
        }
    }

    // Add mechanics on higher difficulties
    if (settings.additionalAbilities > 0) {
        // Could add extra mechanics here
    }
}

void BossAI::AddThreatTarget(int targetId, float threat) {
    if (m_aiSystem) {
        m_aiSystem->AddThreat(m_aiId, targetId, threat);
    }
}

void BossAI::UpdateTargetSelection() {
    if (!m_aiSystem) {
        return;
    }

    // Get highest threat target
    int highestThreat = m_aiSystem->GetHighestThreatTarget(m_aiId);
    if (highestThreat >= 0) {
        m_primaryTargetId = highestThreat;
    }
}

void BossAI::TakeDamage(float damage, int attackerId) {
    if (IsDead() || IsInvulnerable()) {
        return;
    }

    // Apply armor/resist reduction
    float actualDamage = damage - m_stats.armor;
    if (actualDamage < 0.0f) {
        actualDamage = damage * 0.1f; // Minimum 10% damage
    }

    m_stats.currentHealth -= actualDamage;

    // Add threat
    if (attackerId >= 0) {
        AddThreatTarget(attackerId, actualDamage);
    }

    // Set target if we don't have one
    if (m_primaryTargetId < 0 && attackerId >= 0) {
        m_primaryTargetId = attackerId;
    }

    // Check for death
    if (m_stats.currentHealth <= 0.0f) {
        m_stats.currentHealth = 0.0f;
        SetPhase(BossPhase::DEFEATED);
        if (m_deathCallback) {
            m_deathCallback();
        }
    }
}

void BossAI::Heal(float amount) {
    m_stats.currentHealth += amount;
    if (m_stats.currentHealth > m_stats.maxHealth) {
        m_stats.currentHealth = m_stats.maxHealth;
    }
}

void BossAI::SpawnAdds(const AddSpawn& spawnData) {
    // In real implementation, would spawn enemy entities
    // For now, just a placeholder
    if (m_aiSystem) {
        // m_aiSystem->SpawnEnemy(spawnData.enemyType, spawnData.spawnLocation);
    }
}

void BossAI::TeleportTo(const WorldPos& position) {
    m_position = position;
    ClearPath();
    m_animationState = "teleport";
}

void BossAI::CreateHazard(const WorldPos& position, float radius, float duration, float damagePerSecond) {
    // Would create a hazard zone in the game world
    // Placeholder for now
}

void BossAI::ApplyBuff(const std::string& buffName, float multiplier, float duration) {
    m_activeBuffs[buffName] = std::make_pair(multiplier, duration);

    // Apply buff effects
    if (buffName == "damage") {
        m_stats.currentDamage = m_stats.baseDamage * multiplier;
    } else if (buffName == "speed") {
        m_stats.currentSpeed = m_stats.baseSpeed * multiplier;
    } else if (buffName == "armor") {
        m_stats.armor *= multiplier;
    }
}

void BossAI::RemoveBuff(const std::string& buffName) {
    auto it = m_activeBuffs.find(buffName);
    if (it != m_activeBuffs.end()) {
        // Reset stat
        if (buffName == "damage") {
            m_stats.currentDamage = m_stats.baseDamage;
        } else if (buffName == "speed") {
            m_stats.currentSpeed = m_stats.baseSpeed;
        }

        m_activeBuffs.erase(it);
    }
}

void BossAI::SetEnrageTimer(float seconds) {
    m_enrageTimer = seconds;
    m_enrageTime = 0.0f;
}

void BossAI::TriggerEnrage() {
    if (m_isEnraged) {
        return;
    }

    m_isEnraged = true;
    m_currentPhase = BossPhase::ENRAGE;

    // Enrage buffs
    ApplyBuff("damage", 1.5f, -1.0f); // Permanent until death
    ApplyBuff("speed", 1.3f, -1.0f);

    m_animationState = "enrage";

    if (m_phaseChangeCallback) {
        m_phaseChangeCallback(m_previousPhase, BossPhase::ENRAGE);
    }
}

void BossAI::SetPath(const PathResult& path) {
    m_currentPath = path;
    m_currentWaypointIndex = 0;
}

void BossAI::ClearPath() {
    m_currentPath = PathResult();
    m_currentWaypointIndex = 0;
}

void BossAI::UpdatePathing(float deltaTime) {
    if (!HasPath()) {
        return;
    }

    if (m_currentWaypointIndex >= m_currentPath.worldPath.size()) {
        ClearPath();
        return;
    }

    WorldPos target = m_currentPath.worldPath[m_currentWaypointIndex];
    float distanceToWaypoint = m_position.DistanceTo(target);

    if (distanceToWaypoint < 0.5f) {
        m_currentWaypointIndex++;
        if (m_currentWaypointIndex >= m_currentPath.worldPath.size()) {
            ClearPath();
        }
    } else {
        // Move towards waypoint
        float dx = target.x - m_position.x;
        float dy = target.y - m_position.y;
        float length = std::sqrt(dx * dx + dy * dy);

        if (length > 0.0f) {
            m_velocity.x = (dx / length) * m_stats.currentSpeed;
            m_velocity.y = (dy / length) * m_stats.currentSpeed;

            m_position.x += m_velocity.x * deltaTime;
            m_position.y += m_velocity.y * deltaTime;
        }
    }
}

void BossAI::StartTelegraph(const std::string& mechanicName, float duration) {
    m_isTelegraphing = true;
    m_telegraphMechanic = mechanicName;
    m_telegraphDuration = duration;
    m_telegraphTime = 0.0f;
}

float BossAI::GetTelegraphProgress() const {
    if (!m_isTelegraphing || m_telegraphDuration <= 0.0f) {
        return 0.0f;
    }
    return m_telegraphTime / m_telegraphDuration;
}

void BossAI::BuildBehaviorTree() {
    BehaviorTreeBuilder builder(m_entityId, m_aiId);

    // Boss behavior is more complex than regular enemies
    builder.Selector("BossBehavior")
        // Check if defeated
        .Condition([this](BehaviorTreeContext& ctx) {
            return IsDead();
        }, "IsDefeated")
        // Use special mechanic if available
        .Sequence("SpecialMechanic")
            .Condition([this](BehaviorTreeContext& ctx) {
                return BT_CanUseSpecialMechanic(ctx);
            }, "CanUseSpecialMechanic")
            .Action([this](BehaviorTreeContext& ctx) {
                return BT_UseSpecialMechanic(ctx);
            }, "UseSpecialMechanic")
        .End()
        // Combat sequence
        .Sequence("Combat")
            .Condition([this](BehaviorTreeContext& ctx) {
                return BT_HasTarget(ctx);
            }, "HasTarget")
            .Selector("CombatActions")
                // Attack if in range
                .Sequence("Attack")
                    .Condition([this](BehaviorTreeContext& ctx) {
                        return BT_IsTargetInRange(ctx);
                    }, "IsTargetInRange")
                    .Action([this](BehaviorTreeContext& ctx) {
                        return BT_BasicAttack(ctx);
                    }, "BasicAttack")
                .End()
                // Move to target
                .Action([this](BehaviorTreeContext& ctx) {
                    return BT_MoveToTarget(ctx);
                }, "MoveToTarget")
            .End()
        .End()
        // Select new target
        .Action([this](BehaviorTreeContext& ctx) {
            return BT_SelectTarget(ctx);
        }, "SelectTarget")
    .End();

    m_behaviorTree = builder.Build();
}

void BossAI::UpdatePhase(float deltaTime) {
    m_transitionTime += deltaTime;

    if (m_transitionTime >= m_transitionDuration) {
        m_isTransitioning = false;
        m_transitionTime = 0.0f;
        SetInvulnerable(false);

        OnPhaseChange(m_currentPhase);
    }
}

void BossAI::UpdateEnrage(float deltaTime) {
    if (m_enrageTimer > 0.0f && !m_isEnraged) {
        m_enrageTime += deltaTime;
        if (m_enrageTime >= m_enrageTimer) {
            TriggerEnrage();
        }
    }
}

void BossAI::UpdateBuffs(float deltaTime) {
    std::vector<std::string> buffsToRemove;

    for (auto& [buffName, buffData] : m_activeBuffs) {
        float& duration = buffData.second;
        if (duration > 0.0f) {
            duration -= deltaTime;
            if (duration <= 0.0f) {
                buffsToRemove.push_back(buffName);
            }
        }
    }

    for (const auto& buffName : buffsToRemove) {
        RemoveBuff(buffName);
    }
}

void BossAI::UpdateTelegraph(float deltaTime) {
    if (m_isTelegraphing) {
        m_telegraphTime += deltaTime;

        if (m_telegraphTime >= m_telegraphDuration) {
            // Telegraph complete, execute mechanic
            m_isTelegraphing = false;
            m_telegraphTime = 0.0f;

            BossMechanic* mechanic = GetMechanic(m_telegraphMechanic);
            if (mechanic) {
                ExecuteMechanic(*mechanic);
            }
        }
    }
}

void BossAI::ExecuteMechanic(BossMechanic& mechanic) {
    mechanic.lastUsedTime = m_encounterTime;

    if (mechanic.maxUses > 0) {
        mechanic.usesRemaining--;
    }

    // Execute based on type
    switch (mechanic.type) {
        case SpecialAttackType::AREA_OF_EFFECT:
            // Create AOE damage zone
            CreateHazard(m_position, mechanic.radius, 2.0f, mechanic.damage / 2.0f);
            break;

        case SpecialAttackType::SUMMON_ADDS: {
            AddSpawn spawn;
            spawn.enemyType = "minion";
            spawn.count = 3;
            spawn.spawnLocation = m_position;
            SpawnAdds(spawn);
            break;
        }

        case SpecialAttackType::CHARGE_ATTACK:
            // Charge at target
            if (m_primaryTargetId >= 0) {
                // Would dash to target position
            }
            break;

        case SpecialAttackType::GROUND_SLAM:
            // Knockback AOE
            CreateHazard(m_position, mechanic.radius, 0.5f, mechanic.damage);
            break;

        case SpecialAttackType::PROJECTILE_BARRAGE:
            // Fire multiple projectiles
            break;

        case SpecialAttackType::BEAM_ATTACK:
            // Continuous damage beam
            break;

        case SpecialAttackType::TELEPORT_STRIKE:
            // Teleport and attack
            if (m_primaryTargetId >= 0) {
                // Would teleport to target
            }
            break;

        case SpecialAttackType::ENVIRONMENT:
            // Modify arena
            break;

        case SpecialAttackType::BUFF_SELF:
            ApplyBuff("damage", 1.3f, 10.0f);
            break;

        case SpecialAttackType::DEBUFF_PLAYERS:
            // Apply debuff to all players
            break;
    }

    m_animationState = mechanic.name;
}

void BossAI::OnPhaseChange(BossPhase newPhase) {
    m_phaseTime = 0.0f;

    // Trigger callback
    if (m_phaseChangeCallback) {
        m_phaseChangeCallback(m_previousPhase, newPhase);
    }

    // Phase-specific logic
    switch (newPhase) {
        case BossPhase::INTRO:
            // Play intro animation
            SetInvulnerable(true);
            break;

        case BossPhase::PHASE_1:
            SetInvulnerable(false);
            break;

        case BossPhase::PHASE_2:
            // Become more aggressive
            m_stats.currentSpeed = m_stats.baseSpeed * 1.2f;
            break;

        case BossPhase::PHASE_3:
            // Even more aggressive
            m_stats.currentSpeed = m_stats.baseSpeed * 1.4f;
            break;

        case BossPhase::ENRAGE:
            TriggerEnrage();
            break;

        case BossPhase::DEFEATED:
            m_animationState = "death";
            break;

        default:
            break;
    }
}

// Behavior tree implementations
NodeStatus BossAI::BT_SelectTarget(BehaviorTreeContext& context) {
    // Select highest threat target
    UpdateTargetSelection();
    return m_primaryTargetId >= 0 ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus BossAI::BT_MoveToTarget(BehaviorTreeContext& context) {
    if (m_primaryTargetId < 0) {
        return NodeStatus::FAILURE;
    }

    // In real implementation, pathfind to target
    m_animationState = "walking";
    return NodeStatus::RUNNING;
}

NodeStatus BossAI::BT_BasicAttack(BehaviorTreeContext& context) {
    if (m_primaryTargetId < 0) {
        return NodeStatus::FAILURE;
    }

    // Perform basic attack
    m_animationState = "attack";
    return NodeStatus::SUCCESS;
}

NodeStatus BossAI::BT_UseSpecialMechanic(BehaviorTreeContext& context) {
    // Find and use an available mechanic
    for (auto& mechanic : m_mechanics) {
        if (CanUseMechanic(mechanic.name, m_encounterTime)) {
            UseMechanic(mechanic.name);
            return NodeStatus::SUCCESS;
        }
    }
    return NodeStatus::FAILURE;
}

NodeStatus BossAI::BT_MoveToArenaCenter(BehaviorTreeContext& context) {
    // Move to center of arena
    if (m_position.DistanceTo(m_arenaCenter) < 2.0f) {
        return NodeStatus::SUCCESS;
    }

    // Create path to center
    PathResult path;
    path.worldPath.push_back(m_position);
    path.worldPath.push_back(m_arenaCenter);
    path.success = true;
    SetPath(path);

    return NodeStatus::RUNNING;
}

bool BossAI::BT_HasTarget(BehaviorTreeContext& context) {
    return m_primaryTargetId >= 0;
}

bool BossAI::BT_IsTargetInRange(BehaviorTreeContext& context) {
    if (m_primaryTargetId < 0) {
        return false;
    }

    // In real implementation, check actual distance to target
    return true;
}

bool BossAI::BT_CanUseSpecialMechanic(BehaviorTreeContext& context) {
    for (const auto& mechanic : m_mechanics) {
        if (CanUseMechanic(mechanic.name, m_encounterTime)) {
            return true;
        }
    }
    return false;
}

bool BossAI::BT_ShouldMoveToCenter(BehaviorTreeContext& context) {
    // Check if boss should return to center (e.g., for a specific mechanic)
    return false;
}

// Factory function
std::shared_ptr<BossAI> CreateBossByName(int aiId, int entityId, const std::string& bossName) {
    auto boss = std::make_shared<BossAI>(aiId, entityId, bossName);
    boss->Initialize();
    return boss;
}

// JSON loading (stub)
bool LoadBossConfigFromJSON(const std::string& filepath) {
    // Would parse JSON and create boss templates
    return true;
}

} // namespace AI
