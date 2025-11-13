#include "EnemyAI.h"
#include "AISystem.h"
#include <algorithm>
#include <cmath>

namespace AI {

EnemyAI::EnemyAI(int aiId, int entityId, const std::string& archetypeName)
    : m_aiId(aiId)
    , m_entityId(entityId)
    , m_targetId(-1)
    , m_aiSystem(nullptr)
    , m_archetype(EnemyArchetype::MELEE)
    , m_currentState(EnemyState::IDLE)
    , m_archetypeName(archetypeName)
    , m_currentWaypointIndex(0)
    , m_isTelegraphing(false)
    , m_telegraphDuration(0.0f)
    , m_telegraphTime(0.0f)
    , m_timeSinceLastAttack(0.0f)
    , m_timeSinceStateChange(0.0f)
    , m_stunTime(0.0f)
    , m_difficultyDamageMultiplier(1.0f)
    , m_difficultyHealthMultiplier(1.0f)
    , m_difficultySpeedMultiplier(1.0f)
{
}

EnemyAI::~EnemyAI() {
}

void EnemyAI::Initialize() {
    // Determine archetype from name
    if (m_archetypeName == "melee" || m_archetypeName == "Melee") {
        SetArchetype(EnemyArchetype::MELEE);
    } else if (m_archetypeName == "ranged" || m_archetypeName == "Ranged") {
        SetArchetype(EnemyArchetype::RANGED);
    } else if (m_archetypeName == "caster" || m_archetypeName == "Caster") {
        SetArchetype(EnemyArchetype::CASTER);
    } else if (m_archetypeName == "tank" || m_archetypeName == "Tank") {
        SetArchetype(EnemyArchetype::TANK);
    } else if (m_archetypeName == "support" || m_archetypeName == "Support") {
        SetArchetype(EnemyArchetype::SUPPORT);
    } else if (m_archetypeName == "assassin" || m_archetypeName == "Assassin") {
        SetArchetype(EnemyArchetype::ASSASSIN);
    } else if (m_archetypeName == "berserker" || m_archetypeName == "Berserker") {
        SetArchetype(EnemyArchetype::BERSERKER);
    } else {
        SetArchetype(EnemyArchetype::MELEE); // Default
    }

    // Build behavior tree
    BuildBehaviorTree();
}

void EnemyAI::Update(float deltaTime) {
    if (IsDead()) {
        return;
    }

    // Update timers
    m_timeSinceStateChange += deltaTime;
    m_timeSinceLastAttack += deltaTime;

    // Update stun
    UpdateStun(deltaTime);
    if (IsStunned()) {
        return; // Can't do anything while stunned
    }

    // Update telegraph
    UpdateTelegraph(deltaTime);

    // Update behavior tree
    if (m_behaviorTree) {
        m_behaviorTree->GetContext().SetValue("DeltaTime", deltaTime);
        m_behaviorTree->GetContext().SetValue("CurrentTime", m_timeSinceStateChange);
        m_behaviorTree->Execute();
    }

    // Update archetype-specific behavior
    switch (m_archetype) {
        case EnemyArchetype::MELEE:
            UpdateMeleeBehavior(deltaTime);
            break;
        case EnemyArchetype::RANGED:
            UpdateRangedBehavior(deltaTime);
            break;
        case EnemyArchetype::CASTER:
            UpdateCasterBehavior(deltaTime);
            break;
        case EnemyArchetype::TANK:
            UpdateTankBehavior(deltaTime);
            break;
        case EnemyArchetype::SUPPORT:
            UpdateSupportBehavior(deltaTime);
            break;
        case EnemyArchetype::ASSASSIN:
            UpdateAssassinBehavior(deltaTime);
            break;
        case EnemyArchetype::BERSERKER:
            UpdateBerserkerBehavior(deltaTime);
            break;
    }

    // Update pathfinding
    UpdatePathing(deltaTime);

    // Update threat
    UpdateThreat();
}

void EnemyAI::SetArchetype(EnemyArchetype archetype) {
    m_archetype = archetype;
    SetupArchetypeStats();
    SetupArchetypeAttacks();
}

void EnemyAI::ApplyDifficultyScaling(const DifficultySettings& settings) {
    m_difficultyDamageMultiplier = settings.damageMultiplier;
    m_difficultyHealthMultiplier = settings.healthMultiplier;
    m_difficultySpeedMultiplier = settings.speedMultiplier;

    // Apply to stats
    m_stats.maxHealth *= m_difficultyHealthMultiplier;
    m_stats.currentHealth = m_stats.maxHealth;
    m_stats.baseDamage *= m_difficultyDamageMultiplier;
    m_stats.currentDamage = m_stats.baseDamage;
    m_stats.baseSpeed *= m_difficultySpeedMultiplier;
    m_stats.currentSpeed = m_stats.baseSpeed;
    m_stats.aggroRange *= settings.aggroRangeMultiplier;

    // Apply to attacks
    for (auto& attack : m_attacks) {
        attack.damage *= m_difficultyDamageMultiplier;
        if (settings.enableAdvancedTactics) {
            attack.castTime *= 0.8f; // Faster attacks
        }
    }
}

void EnemyAI::TakeDamage(float damage, int attackerId) {
    if (IsDead()) {
        return;
    }

    m_stats.currentHealth -= damage;

    // Add threat
    if (m_aiSystem && attackerId >= 0) {
        m_aiSystem->AddThreat(m_aiId, attackerId, damage);
    }

    // Set target if we don't have one
    if (!HasTarget() && attackerId >= 0) {
        SetTarget(attackerId);
    }

    // Check for death
    if (m_stats.currentHealth <= 0.0f) {
        m_stats.currentHealth = 0.0f;
        SetState(EnemyState::DEAD);
    }
    // Check for retreat
    else if (m_stats.GetHealthPercent() < m_stats.retreatHealthThreshold) {
        SetState(EnemyState::RETREAT);
    }
    // Enter combat if not already
    else if (m_currentState == EnemyState::IDLE || m_currentState == EnemyState::PATROL) {
        SetState(EnemyState::ALERT);
    }
}

bool EnemyAI::IsInCombat() const {
    return m_currentState == EnemyState::CHASE ||
           m_currentState == EnemyState::ATTACK ||
           m_currentState == EnemyState::ALERT ||
           m_currentState == EnemyState::RETREAT;
}

void EnemyAI::AddAttack(const AttackData& attack) {
    m_attacks.push_back(attack);
}

void EnemyAI::AddAbility(const AbilityData& ability) {
    m_abilities.push_back(ability);
}

bool EnemyAI::CanUseAttack(const std::string& attackName, float currentTime) const {
    for (const auto& attack : m_attacks) {
        if (attack.name == attackName) {
            return (currentTime - attack.lastUsedTime) >= attack.cooldown;
        }
    }
    return false;
}

bool EnemyAI::CanUseAbility(const std::string& abilityName, float currentTime) const {
    for (const auto& ability : m_abilities) {
        if (ability.name == abilityName) {
            bool cooldownReady = (currentTime - ability.lastUsedTime) >= ability.cooldown;
            bool healthThresholdMet = m_stats.GetHealthPercent() <= ability.healthThreshold;
            return cooldownReady && healthThresholdMet;
        }
    }
    return false;
}

void EnemyAI::SetPath(const PathResult& path) {
    m_currentPath = path;
    m_currentWaypointIndex = 0;
}

void EnemyAI::ClearPath() {
    m_currentPath = PathResult();
    m_currentWaypointIndex = 0;
}

WorldPos EnemyAI::GetNextWaypoint() {
    if (m_currentWaypointIndex < m_currentPath.worldPath.size()) {
        return m_currentPath.worldPath[m_currentWaypointIndex];
    }
    return m_position;
}

void EnemyAI::UpdatePathing(float deltaTime) {
    if (!HasPath()) {
        return;
    }

    WorldPos target = GetNextWaypoint();
    float distanceToWaypoint = m_position.DistanceTo(target);

    if (distanceToWaypoint < 0.5f) {
        // Reached waypoint, move to next
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

float EnemyAI::GetDistanceToTarget() const {
    if (!HasTarget()) {
        return std::numeric_limits<float>::max();
    }

    // In a real implementation, you'd get the target's position from the game world
    // For now, we'll return a placeholder
    return 10.0f;
}

bool EnemyAI::IsTargetInRange(float range) const {
    return GetDistanceToTarget() <= range;
}

bool EnemyAI::IsTargetInLineOfSight() const {
    if (!HasTarget() || !m_aiSystem) {
        return false;
    }

    // In a real implementation, use pathfinding system to check LOS
    return true;
}

std::vector<int> EnemyAI::GetNearbyAllies(float radius) const {
    std::vector<int> allies;

    if (IsInGroup() && m_aiSystem) {
        auto groupMembers = m_aiSystem->GetGroupMembers(m_groupName);
        // Filter by distance (would need position data in real implementation)
        allies = groupMembers;
    }

    return allies;
}

std::vector<int> EnemyAI::GetNearbyEnemies(float radius) const {
    // In a real implementation, query the game world for nearby player entities
    return std::vector<int>();
}

void EnemyAI::UpdateMeleeBehavior(float deltaTime) {
    // Melee enemies are aggressive and always push forward
    if (HasTarget() && IsInCombat()) {
        if (!IsTargetInRange(m_stats.attackRange)) {
            // Chase target
            SetState(EnemyState::CHASE);
        } else {
            // Attack!
            SetState(EnemyState::ATTACK);
        }
    }
}

void EnemyAI::UpdateRangedBehavior(float deltaTime) {
    // Ranged enemies maintain distance
    if (HasTarget() && IsInCombat()) {
        float distance = GetDistanceToTarget();
        float optimalRange = m_stats.attackRange * 0.8f;

        if (distance < optimalRange * 0.5f) {
            // Too close, back up
            SetState(EnemyState::RETREAT);
        } else if (distance <= m_stats.attackRange) {
            // Perfect range, attack
            SetState(EnemyState::ATTACK);
        } else {
            // Too far, move closer
            SetState(EnemyState::CHASE);
        }
    }
}

void EnemyAI::UpdateCasterBehavior(float deltaTime) {
    // Casters use abilities more frequently
    if (HasTarget() && IsInCombat()) {
        // Check if we should use an ability
        for (auto& ability : m_abilities) {
            if (CanUseAbility(ability.name, m_timeSinceStateChange)) {
                // Use ability
                ability.lastUsedTime = m_timeSinceStateChange;
            }
        }

        // Maintain safe distance
        float distance = GetDistanceToTarget();
        if (distance < m_stats.attackRange * 0.6f) {
            SetState(EnemyState::RETREAT);
        } else if (distance <= m_stats.attackRange) {
            SetState(EnemyState::ATTACK);
        } else {
            SetState(EnemyState::CHASE);
        }
    }
}

void EnemyAI::UpdateTankBehavior(float deltaTime) {
    // Tanks protect allies and soak damage
    if (IsInCombat()) {
        // Try to position between target and allies
        auto allies = GetNearbyAllies(20.0f);

        if (!allies.empty() && HasTarget()) {
            // Position defensively
            SetState(EnemyState::CHASE);
        }

        // Use defensive abilities when health is low
        if (m_stats.GetHealthPercent() < 0.5f) {
            for (auto& ability : m_abilities) {
                if (ability.isDefensive && CanUseAbility(ability.name, m_timeSinceStateChange)) {
                    ability.lastUsedTime = m_timeSinceStateChange;
                }
            }
        }
    }
}

void EnemyAI::UpdateSupportBehavior(float deltaTime) {
    // Support enemies buff allies and debuff enemies
    auto allies = GetNearbyAllies(15.0f);

    // Maintain distance from enemies
    if (HasTarget()) {
        float distance = GetDistanceToTarget();
        if (distance < 8.0f) {
            SetState(EnemyState::RETREAT);
        }
    }

    // Use support abilities on cooldown
    if (!allies.empty()) {
        for (auto& ability : m_abilities) {
            if (CanUseAbility(ability.name, m_timeSinceStateChange)) {
                ability.lastUsedTime = m_timeSinceStateChange;
            }
        }
    }
}

void EnemyAI::UpdateAssassinBehavior(float deltaTime) {
    // Assassins strike quickly then retreat
    if (HasTarget() && IsInCombat()) {
        float distance = GetDistanceToTarget();

        // Strike when in range
        if (distance <= m_stats.attackRange) {
            SetState(EnemyState::ATTACK);

            // After attacking, retreat
            if (m_timeSinceLastAttack < 0.5f) {
                SetState(EnemyState::RETREAT);
            }
        } else {
            SetState(EnemyState::CHASE);
        }
    }
}

void EnemyAI::UpdateBerserkerBehavior(float deltaTime) {
    // Berserkers become stronger when damaged
    float healthPercent = m_stats.GetHealthPercent();

    // Increase damage and speed as health decreases
    m_stats.currentDamage = m_stats.baseDamage * (1.0f + (1.0f - healthPercent) * 0.5f);
    m_stats.currentSpeed = m_stats.baseSpeed * (1.0f + (1.0f - healthPercent) * 0.3f);

    // Always aggressive
    if (HasTarget() && IsInCombat()) {
        if (IsTargetInRange(m_stats.attackRange)) {
            SetState(EnemyState::ATTACK);
        } else {
            SetState(EnemyState::CHASE);
        }
    }
}

void EnemyAI::StartTelegraph(const std::string& attackName, float duration) {
    m_isTelegraphing = true;
    m_telegraphAttack = attackName;
    m_telegraphDuration = duration;
    m_telegraphTime = 0.0f;
}

float EnemyAI::GetTelegraphProgress() const {
    if (!m_isTelegraphing || m_telegraphDuration <= 0.0f) {
        return 0.0f;
    }
    return m_telegraphTime / m_telegraphDuration;
}

void EnemyAI::Stun(float duration) {
    m_stunTime = duration;
    SetState(EnemyState::STUNNED);
}

void EnemyAI::SetupArchetypeStats() {
    switch (m_archetype) {
        case EnemyArchetype::MELEE:
            m_stats.maxHealth = 100.0f;
            m_stats.baseDamage = 15.0f;
            m_stats.baseSpeed = 4.0f;
            m_stats.attackRange = 2.0f;
            m_stats.aggroRange = 12.0f;
            break;

        case EnemyArchetype::RANGED:
            m_stats.maxHealth = 70.0f;
            m_stats.baseDamage = 12.0f;
            m_stats.baseSpeed = 3.5f;
            m_stats.attackRange = 10.0f;
            m_stats.aggroRange = 15.0f;
            break;

        case EnemyArchetype::CASTER:
            m_stats.maxHealth = 60.0f;
            m_stats.baseDamage = 20.0f;
            m_stats.baseSpeed = 3.0f;
            m_stats.attackRange = 12.0f;
            m_stats.aggroRange = 15.0f;
            break;

        case EnemyArchetype::TANK:
            m_stats.maxHealth = 200.0f;
            m_stats.baseDamage = 10.0f;
            m_stats.baseSpeed = 2.5f;
            m_stats.attackRange = 2.5f;
            m_stats.aggroRange = 10.0f;
            m_stats.retreatHealthThreshold = 0.1f; // Tanks rarely retreat
            break;

        case EnemyArchetype::SUPPORT:
            m_stats.maxHealth = 50.0f;
            m_stats.baseDamage = 5.0f;
            m_stats.baseSpeed = 3.0f;
            m_stats.attackRange = 8.0f;
            m_stats.aggroRange = 12.0f;
            break;

        case EnemyArchetype::ASSASSIN:
            m_stats.maxHealth = 50.0f;
            m_stats.baseDamage = 30.0f;
            m_stats.baseSpeed = 5.0f;
            m_stats.attackRange = 1.5f;
            m_stats.aggroRange = 10.0f;
            break;

        case EnemyArchetype::BERSERKER:
            m_stats.maxHealth = 120.0f;
            m_stats.baseDamage = 18.0f;
            m_stats.baseSpeed = 3.5f;
            m_stats.attackRange = 2.0f;
            m_stats.aggroRange = 15.0f;
            m_stats.retreatHealthThreshold = 0.0f; // Never retreats
            break;
    }

    m_stats.currentHealth = m_stats.maxHealth;
    m_stats.currentDamage = m_stats.baseDamage;
    m_stats.currentSpeed = m_stats.baseSpeed;
}

void EnemyAI::SetupArchetypeAttacks() {
    m_attacks.clear();
    m_abilities.clear();

    switch (m_archetype) {
        case EnemyArchetype::MELEE: {
            AttackData slash;
            slash.name = "Slash";
            slash.damage = 15.0f;
            slash.range = 2.0f;
            slash.cooldown = 1.5f;
            slash.castTime = 0.3f;
            slash.isTelegraphed = true;
            AddAttack(slash);

            AttackData cleave;
            cleave.name = "Cleave";
            cleave.damage = 25.0f;
            cleave.range = 2.5f;
            cleave.cooldown = 3.0f;
            cleave.castTime = 0.5f;
            cleave.radius = 3.0f;
            cleave.isTelegraphed = true;
            AddAttack(cleave);
            break;
        }

        case EnemyArchetype::RANGED: {
            AttackData arrow;
            arrow.name = "Arrow Shot";
            arrow.damage = 12.0f;
            arrow.range = 10.0f;
            arrow.cooldown = 1.0f;
            arrow.castTime = 0.4f;
            arrow.isTelegraphed = true;
            AddAttack(arrow);

            AttackData volley;
            volley.name = "Arrow Volley";
            volley.damage = 8.0f;
            volley.range = 10.0f;
            volley.cooldown = 4.0f;
            volley.castTime = 0.6f;
            volley.radius = 4.0f;
            volley.isTelegraphed = true;
            AddAttack(volley);
            break;
        }

        case EnemyArchetype::CASTER: {
            AttackData fireball;
            fireball.name = "Fireball";
            fireball.damage = 20.0f;
            fireball.range = 12.0f;
            fireball.cooldown = 2.0f;
            fireball.castTime = 0.8f;
            fireball.radius = 2.0f;
            fireball.isTelegraphed = true;
            AddAttack(fireball);

            AbilityData teleport;
            teleport.name = "Teleport";
            teleport.cooldown = 8.0f;
            teleport.healthThreshold = 0.5f;
            teleport.isDefensive = true;
            AddAbility(teleport);
            break;
        }

        case EnemyArchetype::TANK: {
            AttackData slam;
            slam.name = "Ground Slam";
            slam.damage = 10.0f;
            slam.range = 3.0f;
            slam.cooldown = 2.0f;
            slam.castTime = 0.6f;
            slam.radius = 3.0f;
            slam.isTelegraphed = true;
            AddAttack(slam);

            AbilityData shield;
            shield.name = "Shield Wall";
            shield.cooldown = 10.0f;
            shield.duration = 5.0f;
            shield.healthThreshold = 0.4f;
            shield.isDefensive = true;
            AddAbility(shield);
            break;
        }

        case EnemyArchetype::SUPPORT: {
            AbilityData heal;
            heal.name = "Heal Allies";
            heal.cooldown = 6.0f;
            heal.healthThreshold = 1.0f;
            heal.isDefensive = true;
            AddAbility(heal);

            AbilityData buff;
            buff.name = "Damage Buff";
            buff.cooldown = 8.0f;
            buff.duration = 10.0f;
            buff.healthThreshold = 1.0f;
            AddAbility(buff);
            break;
        }

        case EnemyArchetype::ASSASSIN: {
            AttackData backstab;
            backstab.name = "Backstab";
            backstab.damage = 40.0f;
            backstab.range = 1.5f;
            backstab.cooldown = 0.8f;
            backstab.castTime = 0.2f;
            backstab.isTelegraphed = false; // Surprise attack!
            AddAttack(backstab);

            AbilityData vanish;
            vanish.name = "Vanish";
            vanish.cooldown = 12.0f;
            vanish.healthThreshold = 0.3f;
            vanish.isDefensive = true;
            AddAbility(vanish);
            break;
        }

        case EnemyArchetype::BERSERKER: {
            AttackData fury;
            fury.name = "Fury Strike";
            fury.damage = 18.0f;
            fury.range = 2.0f;
            fury.cooldown = 1.2f;
            fury.castTime = 0.3f;
            fury.isTelegraphed = true;
            AddAttack(fury);

            AbilityData enrage;
            enrage.name = "Enrage";
            enrage.cooldown = 15.0f;
            enrage.duration = 8.0f;
            enrage.healthThreshold = 0.5f;
            AddAbility(enrage);
            break;
        }
    }
}

void EnemyAI::BuildBehaviorTree() {
    BehaviorTreeBuilder builder(m_entityId, m_aiId);

    // Main behavior selector
    builder.Selector("MainBehavior")
        // Check if dead
        .Condition([this](BehaviorTreeContext& ctx) {
            return IsDead();
        }, "IsDead")
        // Check if stunned
        .Condition([this](BehaviorTreeContext& ctx) {
            return IsStunned();
        }, "IsStunned")
        // Combat behavior
        .Sequence("CombatBehavior")
            .Condition([this](BehaviorTreeContext& ctx) {
                return BT_HasTarget(ctx);
            }, "HasTarget")
            .Selector("CombatActions")
                // Retreat if low health
                .Sequence("RetreatSequence")
                    .Condition([this](BehaviorTreeContext& ctx) {
                        return BT_IsHealthLow(ctx);
                    }, "IsHealthLow")
                    .Action([this](BehaviorTreeContext& ctx) {
                        return BT_Retreat(ctx);
                    }, "Retreat")
                .End()
                // Use ability if available
                .Sequence("AbilitySequence")
                    .Condition([this](BehaviorTreeContext& ctx) {
                        return BT_ShouldUseAbility(ctx);
                    }, "ShouldUseAbility")
                    .Action([this](BehaviorTreeContext& ctx) {
                        return BT_UseAbility(ctx);
                    }, "UseAbility")
                .End()
                // Attack if in range
                .Sequence("AttackSequence")
                    .Condition([this](BehaviorTreeContext& ctx) {
                        return BT_IsTargetInRange(ctx);
                    }, "IsTargetInRange")
                    .Condition([this](BehaviorTreeContext& ctx) {
                        return BT_CanAttack(ctx);
                    }, "CanAttack")
                    .Action([this](BehaviorTreeContext& ctx) {
                        return BT_AttackTarget(ctx);
                    }, "Attack")
                .End()
                // Chase target
                .Action([this](BehaviorTreeContext& ctx) {
                    return BT_ChaseTarget(ctx);
                }, "Chase")
            .End()
        .End()
        // Return to spawn if too far
        .Sequence("ReturnToSpawn")
            .Condition([this](BehaviorTreeContext& ctx) {
                return BT_IsTooFarFromSpawn(ctx);
            }, "IsTooFarFromSpawn")
            .Action([this](BehaviorTreeContext& ctx) {
                return BT_ReturnToSpawn(ctx);
            }, "ReturnToSpawn")
        .End()
        // Idle/Patrol behavior
        .Selector("IdleBehavior")
            .Action([this](BehaviorTreeContext& ctx) {
                return BT_FindTarget(ctx);
            }, "FindTarget")
            .Action([this](BehaviorTreeContext& ctx) {
                return BT_Patrol(ctx);
            }, "Patrol")
        .End()
    .End();

    m_behaviorTree = builder.Build();
}

void EnemyAI::UpdateTelegraph(float deltaTime) {
    if (m_isTelegraphing) {
        m_telegraphTime += deltaTime;

        if (m_telegraphTime >= m_telegraphDuration) {
            // Telegraph complete, execute attack
            m_isTelegraphing = false;
            m_telegraphTime = 0.0f;

            // Find and execute the attack
            for (auto& attack : m_attacks) {
                if (attack.name == m_telegraphAttack) {
                    attack.lastUsedTime = m_timeSinceStateChange;
                    m_timeSinceLastAttack = 0.0f;
                    break;
                }
            }
        }
    }
}

void EnemyAI::UpdateStun(float deltaTime) {
    if (m_stunTime > 0.0f) {
        m_stunTime -= deltaTime;
        if (m_stunTime <= 0.0f) {
            m_stunTime = 0.0f;
            // Return to previous state or idle
            if (m_currentState == EnemyState::STUNNED) {
                SetState(HasTarget() ? EnemyState::CHASE : EnemyState::IDLE);
            }
        }
    }
}

void EnemyAI::UpdateThreat() {
    if (!m_aiSystem || !HasTarget()) {
        return;
    }

    // Get highest threat target
    int highestThreatTarget = m_aiSystem->GetHighestThreatTarget(m_aiId);
    if (highestThreatTarget >= 0 && highestThreatTarget != m_targetId) {
        SetTarget(highestThreatTarget);
    }
}

// Behavior tree actions and conditions
NodeStatus EnemyAI::BT_FindTarget(BehaviorTreeContext& context) {
    // In real implementation, search for targets in aggro range
    return NodeStatus::FAILURE;
}

NodeStatus EnemyAI::BT_ChaseTarget(BehaviorTreeContext& context) {
    SetState(EnemyState::CHASE);
    // In real implementation, pathfind to target
    return NodeStatus::RUNNING;
}

NodeStatus EnemyAI::BT_AttackTarget(BehaviorTreeContext& context) {
    if (m_attacks.empty()) {
        return NodeStatus::FAILURE;
    }

    // Use first available attack
    for (auto& attack : m_attacks) {
        if (CanUseAttack(attack.name, m_timeSinceStateChange)) {
            if (attack.isTelegraphed) {
                StartTelegraph(attack.name, attack.castTime);
            } else {
                attack.lastUsedTime = m_timeSinceStateChange;
                m_timeSinceLastAttack = 0.0f;
            }
            SetState(EnemyState::ATTACK);
            return NodeStatus::SUCCESS;
        }
    }

    return NodeStatus::FAILURE;
}

NodeStatus EnemyAI::BT_UseAbility(BehaviorTreeContext& context) {
    for (auto& ability : m_abilities) {
        if (CanUseAbility(ability.name, m_timeSinceStateChange)) {
            ability.lastUsedTime = m_timeSinceStateChange;
            return NodeStatus::SUCCESS;
        }
    }
    return NodeStatus::FAILURE;
}

NodeStatus EnemyAI::BT_Retreat(BehaviorTreeContext& context) {
    SetState(EnemyState::RETREAT);
    // In real implementation, pathfind away from target
    return NodeStatus::RUNNING;
}

NodeStatus EnemyAI::BT_ReturnToSpawn(BehaviorTreeContext& context) {
    ClearTarget();
    if (m_aiSystem) {
        m_aiSystem->ClearAllThreat(m_aiId);
    }
    // In real implementation, pathfind to spawn
    return NodeStatus::RUNNING;
}

NodeStatus EnemyAI::BT_Patrol(BehaviorTreeContext& context) {
    SetState(EnemyState::PATROL);
    return NodeStatus::RUNNING;
}

bool EnemyAI::BT_HasTarget(BehaviorTreeContext& context) {
    return HasTarget();
}

bool EnemyAI::BT_IsTargetInRange(BehaviorTreeContext& context) {
    return IsTargetInRange(m_stats.attackRange);
}

bool EnemyAI::BT_IsHealthLow(BehaviorTreeContext& context) {
    return m_stats.GetHealthPercent() < m_stats.retreatHealthThreshold;
}

bool EnemyAI::BT_IsTooFarFromSpawn(BehaviorTreeContext& context) {
    float distance = m_position.DistanceTo(m_spawnPosition);
    return distance > m_stats.leashRange;
}

bool EnemyAI::BT_CanAttack(BehaviorTreeContext& context) {
    if (m_isTelegraphing) {
        return false;
    }

    for (const auto& attack : m_attacks) {
        if (CanUseAttack(attack.name, m_timeSinceStateChange)) {
            return true;
        }
    }
    return false;
}

bool EnemyAI::BT_ShouldUseAbility(BehaviorTreeContext& context) {
    for (const auto& ability : m_abilities) {
        if (CanUseAbility(ability.name, m_timeSinceStateChange)) {
            return true;
        }
    }
    return false;
}

// Factory function
std::shared_ptr<EnemyAI> CreateEnemyByArchetype(int aiId, int entityId, const std::string& archetypeName) {
    auto enemy = std::make_shared<EnemyAI>(aiId, entityId, archetypeName);
    enemy->Initialize();
    return enemy;
}

// JSON loading (stub for now)
bool LoadEnemyArchetypesFromJSON(const std::string& filepath) {
    // Would parse JSON and create enemy templates
    return true;
}

} // namespace AI
