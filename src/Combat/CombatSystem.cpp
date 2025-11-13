#include "CombatSystem.h"
#include <cmath>
#include <algorithm>

namespace Combat {

// ===== CombatEntity Implementation =====

CombatEntity::CombatEntity(const std::string& id)
    : m_id(id)
    , m_state(EntityState::IDLE)
    , m_level(1)
    , m_currentHealth(100.0f)
    , m_maxHealth(100.0f)
    , m_currentMana(100.0f)
    , m_maxMana(100.0f)
    , m_currentStamina(100.0f)
    , m_maxStamina(100.0f)
    , m_healthRegenRate(1.0f)
    , m_manaRegenRate(2.0f)
    , m_staminaRegenRate(5.0f)
    , m_invincibilityTime(0.0f)
    , m_hitstunTime(0.0f)
    , m_parryWindow(0.0f)
    , m_isBlocking(false)
    , m_blockValue(50.0f)
{
    m_transform.position = {0.0f, 0.0f, 0.0f};
    m_transform.rotation = 0.0f;

    m_statusEffectManager = std::make_unique<StatusEffectManager>();
    m_skillManager = std::make_unique<SkillManager>();
    m_weaponManager = std::make_unique<WeaponManager>();
}

CombatEntity::~CombatEntity() {
}

void CombatEntity::update(float deltaTime) {
    // Update managers
    if (m_statusEffectManager) {
        m_statusEffectManager->update(deltaTime, this);
    }

    if (m_skillManager) {
        m_skillManager->update(deltaTime);
    }

    if (m_weaponManager) {
        m_weaponManager->update(deltaTime);
    }

    // Update timers
    updateTimers(deltaTime);

    // Update resource regeneration
    updateResourceRegen(deltaTime);
}

bool CombatEntity::attack(CombatEntity* target) {
    if (!canAct() || !target) {
        return false;
    }

    // Use weapon manager to attack
    if (m_weaponManager && m_weaponManager->hasMainHandWeapon()) {
        m_weaponManager->performMainHandAttack();
        setState(EntityState::ATTACKING);

        // Trigger callback
        if (m_onAttack) {
            m_onAttack(target);
        }

        return true;
    }

    return false;
}

bool CombatEntity::useSkill(const std::string& skillId, CombatEntity* target) {
    if (!canAct() || !m_skillManager) {
        return false;
    }

    // Check if can use skill
    if (!m_skillManager->canUseSkill(skillId, this)) {
        return false;
    }

    // Use skill
    auto* skillInstance = m_skillManager->useSkill(skillId, this, target);
    if (skillInstance) {
        setState(EntityState::CASTING);
        return true;
    }

    return false;
}

bool CombatEntity::dodge() {
    if (!canMove()) {
        return false;
    }

    // Check stamina
    if (m_currentStamina < 25.0f) {
        return false;
    }

    // Consume stamina
    consumeStamina(25.0f);

    // Set invincibility frames
    setInvincible(0.3f); // 300ms i-frames

    setState(EntityState::DODGING);
    return true;
}

bool CombatEntity::block() {
    if (!canAct()) {
        return false;
    }

    startBlocking();
    setState(EntityState::BLOCKING);
    return true;
}

bool CombatEntity::parry() {
    if (!canAct()) {
        return false;
    }

    triggerParry();
    setState(EntityState::PARRYING);
    return true;
}

void CombatEntity::takeDamage(float damage, Element element, CombatEntity* attacker) {
    // Check invincibility
    if (isInvincible()) {
        return;
    }

    // Apply status effect modifiers
    if (m_statusEffectManager) {
        // Get total defense modifier
        float defenseMultiplier = m_statusEffectManager->getTotalDefenseModifier();
        m_stats.defense *= defenseMultiplier;
    }

    // Reduce health
    m_currentHealth -= damage;

    // Clamp health
    if (m_currentHealth < 0.0f) {
        m_currentHealth = 0.0f;
        die();
    }

    // Trigger callback
    if (m_onDamageTaken) {
        m_onDamageTaken(damage, attacker);
    }
}

void CombatEntity::heal(float amount) {
    // Check if can be healed
    if (m_statusEffectManager && !m_statusEffectManager->canBeHealed()) {
        return;
    }

    m_currentHealth += amount;

    // Clamp to max health
    if (m_currentHealth > m_maxHealth) {
        m_currentHealth = m_maxHealth;
    }

    // Trigger callback
    if (m_onHeal) {
        m_onHeal(amount);
    }
}

void CombatEntity::setState(EntityState state) {
    m_state = state;
}

bool CombatEntity::canAct() const {
    if (!isAlive()) return false;
    if (isInHitstun()) return false;
    if (m_state == EntityState::STUNNED) return false;

    if (m_statusEffectManager) {
        return m_statusEffectManager->canAct();
    }

    return true;
}

bool CombatEntity::canMove() const {
    if (!canAct()) return false;

    if (m_statusEffectManager) {
        return m_statusEffectManager->canMove();
    }

    return true;
}

void CombatEntity::setHealth(float health) {
    m_currentHealth = std::min(health, m_maxHealth);
}

void CombatEntity::setMaxHealth(float maxHealth) {
    m_maxHealth = maxHealth;
    if (m_currentHealth > m_maxHealth) {
        m_currentHealth = m_maxHealth;
    }
}

void CombatEntity::die() {
    m_currentHealth = 0.0f;
    setState(EntityState::DEAD);

    if (m_onDeath) {
        m_onDeath();
    }
}

void CombatEntity::setMana(float mana) {
    m_currentMana = std::min(mana, m_maxMana);
}

void CombatEntity::consumeMana(float amount) {
    m_currentMana = std::max(0.0f, m_currentMana - amount);
}

void CombatEntity::setStamina(float stamina) {
    m_currentStamina = std::min(stamina, m_maxStamina);
}

void CombatEntity::consumeStamina(float amount) {
    m_currentStamina = std::max(0.0f, m_currentStamina - amount);
}

float CombatEntity::getDistanceTo(CombatEntity* other) const {
    if (!other) return 99999.0f;

    float dx = m_transform.position.x - other->m_transform.position.x;
    float dy = m_transform.position.y - other->m_transform.position.y;
    float dz = m_transform.position.z - other->m_transform.position.z;

    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool CombatEntity::isInRange(CombatEntity* target, float range) const {
    return getDistanceTo(target) <= range;
}

void CombatEntity::setInvincible(float duration) {
    m_invincibilityTime = duration;
}

void CombatEntity::applyHitstun(float duration) {
    m_hitstunTime = duration;
}

void CombatEntity::applyKnockback(Vector2 direction, float force) {
    // Apply knockback physics
    // This would integrate with a physics system
}

void CombatEntity::startBlocking() {
    m_isBlocking = true;
}

void CombatEntity::stopBlocking() {
    m_isBlocking = false;
}

DefenseResult CombatEntity::checkDefense(CombatEntity* attacker) {
    // Check parry first (highest priority)
    if (isInParryWindow()) {
        return DefenseResult::PARRIED;
    }

    // Check block
    if (isBlocking()) {
        return DefenseResult::BLOCKED;
    }

    // Check i-frame dodge
    if (isInvincible()) {
        return DefenseResult::PERFECT_DODGE;
    }

    return DefenseResult::NONE;
}

void CombatEntity::triggerParry() {
    m_parryWindow = 0.2f; // 200ms parry window
}

void CombatEntity::setLevel(int level) {
    m_level = level;
}

void CombatEntity::updateTimers(float deltaTime) {
    // Update invincibility
    if (m_invincibilityTime > 0.0f) {
        m_invincibilityTime -= deltaTime;
    }

    // Update hitstun
    if (m_hitstunTime > 0.0f) {
        m_hitstunTime -= deltaTime;
        if (m_hitstunTime <= 0.0f) {
            setState(EntityState::IDLE);
        }
    }

    // Update parry window
    if (m_parryWindow > 0.0f) {
        m_parryWindow -= deltaTime;
    }
}

void CombatEntity::updateResourceRegen(float deltaTime) {
    // Health regeneration
    if (m_currentHealth < m_maxHealth && m_healthRegenRate > 0.0f) {
        heal(m_healthRegenRate * deltaTime);
    }

    // Mana regeneration
    if (m_currentMana < m_maxMana && m_manaRegenRate > 0.0f) {
        setMana(m_currentMana + m_manaRegenRate * deltaTime);
    }

    // Stamina regeneration (faster when not blocking)
    if (m_currentStamina < m_maxStamina && m_staminaRegenRate > 0.0f) {
        float regenRate = m_isBlocking ? m_staminaRegenRate * 0.5f : m_staminaRegenRate;
        setStamina(m_currentStamina + regenRate * deltaTime);
    }
}

// ===== CombatSystem Implementation =====

CombatSystem& CombatSystem::getInstance() {
    static CombatSystem instance;
    return instance;
}

CombatSystem::CombatSystem()
    : m_globalDamageMultiplier(1.0f)
    , m_defaultIFrameDuration(0.3f)
    , m_hitstunEnabled(true)
    , m_knockbackEnabled(true)
{
}

CombatSystem::~CombatSystem() {
}

void CombatSystem::initialize() {
    // Load combat data
    StatusEffectFactory::getInstance().loadStatusEffects("data/status_effects.json");
    SkillFactory::getInstance().loadSkills("data/skills.json");
}

void CombatSystem::shutdown() {
    m_entities.clear();
    m_entityTeams.clear();
    m_entityStats.clear();
}

void CombatSystem::update(float deltaTime) {
    // Update all entities
    for (auto& pair : m_entities) {
        if (pair.second) {
            pair.second->update(deltaTime);
        }
    }

    // Update combo system
    m_comboSystem.update(deltaTime);
}

void CombatSystem::registerEntity(CombatEntity* entity) {
    if (!entity) return;
    m_entities[entity->getId()] = entity;

    // Initialize stats
    CombatStats stats;
    stats.totalDamageDealt = 0;
    stats.totalDamageTaken = 0;
    stats.totalKills = 0;
    stats.totalDeaths = 0;
    stats.totalHitsLanded = 0;
    stats.totalHitsMissed = 0;
    stats.totalCriticalHits = 0;
    stats.totalDodges = 0;
    stats.totalBlocks = 0;
    stats.totalParries = 0;
    m_entityStats[entity] = stats;
}

void CombatSystem::unregisterEntity(CombatEntity* entity) {
    if (!entity) return;

    m_entities.erase(entity->getId());
    m_entityTeams.erase(entity);
    m_entityStats.erase(entity);
    m_comboSystem.removeComboTracker(entity);
}

CombatEntity* CombatSystem::getEntity(const std::string& id) {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<CombatEntity*> CombatSystem::getAllEntities() {
    std::vector<CombatEntity*> entities;
    for (auto& pair : m_entities) {
        entities.push_back(pair.second);
    }
    return entities;
}

std::vector<CombatEntity*> CombatSystem::getEntitiesInRadius(const Vector3& center, float radius) {
    std::vector<CombatEntity*> result;

    for (auto& pair : m_entities) {
        auto* entity = pair.second;
        if (!entity) continue;

        float dx = entity->getTransform().position.x - center.x;
        float dy = entity->getTransform().position.y - center.y;
        float dz = entity->getTransform().position.z - center.z;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (distance <= radius) {
            result.push_back(entity);
        }
    }

    return result;
}

HitResult CombatSystem::executeAttack(CombatEntity* attacker, CombatEntity* target) {
    HitResult result;
    result.hit = false;

    if (!attacker || !target) {
        return result;
    }

    // Check range
    auto* weaponManager = attacker->getWeaponManager();
    if (!weaponManager || !weaponManager->hasMainHandWeapon()) {
        return result;
    }

    float range = weaponManager->getTotalRange();
    if (!attacker->isInRange(target, range)) {
        return result;
    }

    // Get weapon
    auto* weapon = weaponManager->getMainHandWeapon();
    if (!weapon) {
        return result;
    }

    // Check defense
    DefenseResult defenseResult = target->checkDefense(attacker);
    result.defenseResult = defenseResult;

    if (defenseResult == DefenseResult::PERFECT_DODGE) {
        // Perfect dodge - no damage
        result.hit = false;
        return result;
    }

    // Calculate damage
    float damage = weapon->calculateDamage(attacker->getStats());

    // Get weapon data
    const auto& weaponData = weapon->getData();

    // Deal damage
    result.damageResult = dealDamage(
        attacker,
        target,
        damage,
        weaponData.element,
        weaponData.primaryDamageType,
        1.0f
    ).damageResult;

    result.hit = !result.damageResult.isEvaded;

    // Apply defense effects
    if (defenseResult == DefenseResult::BLOCKED) {
        result.damageResult.totalDamage *= 0.5f; // Block reduces damage by 50%
        result.damageResult.isBlocked = true;
    }
    else if (defenseResult == DefenseResult::PARRIED) {
        result.damageResult.totalDamage *= 0.1f; // Parry reduces damage by 90%
        result.damageResult.isParried = true;
        // Parry leaves attacker vulnerable
        attacker->applyHitstun(0.5f);
    }

    // Apply hitstun
    if (result.hit && m_hitstunEnabled && defenseResult == DefenseResult::NONE) {
        target->applyHitstun(0.2f);
        result.hitstun = 0.2f;
    }

    // Apply knockback
    if (result.hit && m_knockbackEnabled && weaponData.knockback > 0.0f) {
        Vector2 direction = {
            target->getTransform().position.x - attacker->getTransform().position.x,
            target->getTransform().position.y - attacker->getTransform().position.y
        };
        target->applyKnockback(direction, weaponData.knockback);
        result.knockbackDistance = weaponData.knockback;
    }

    // Update combo
    auto* comboTracker = m_comboSystem.getComboTracker(attacker);
    if (!comboTracker) {
        comboTracker = m_comboSystem.createComboTracker(attacker);
    }
    comboTracker->addAction(ComboActionType::LIGHT_ATTACK, "", result.damageResult.totalDamage, result.hit);
    result.triggeredCombo = true;

    // Process hit result
    processHitResult(attacker, target, result);

    return result;
}

HitResult CombatSystem::executeSkill(CombatEntity* caster, const std::string& skillId, CombatEntity* target) {
    HitResult result;
    result.hit = false;

    if (!caster || !caster->useSkill(skillId, target)) {
        return result;
    }

    // Skill execution is handled by SkillManager
    // This would need more integration with skill system

    return result;
}

bool CombatSystem::executeDodge(CombatEntity* entity) {
    if (!entity) return false;
    return entity->dodge();
}

bool CombatSystem::executeBlock(CombatEntity* entity) {
    if (!entity) return false;
    return entity->block();
}

bool CombatSystem::executeParry(CombatEntity* entity) {
    if (!entity) return false;
    return entity->parry();
}

HitResult CombatSystem::dealDamage(
    CombatEntity* attacker,
    CombatEntity* target,
    float baseDamage,
    Element element,
    DamageType damageType,
    float skillMultiplier
) {
    HitResult result;

    if (!attacker || !target) {
        return result;
    }

    // Calculate damage
    result.damageResult = m_damageCalculator.calculateDamage(
        attacker->getStats(),
        target->getStats(),
        baseDamage,
        element,
        damageType,
        skillMultiplier
    );

    result.hit = !result.damageResult.isEvaded;

    // Apply damage to target
    if (result.hit) {
        target->takeDamage(result.damageResult.totalDamage, element, attacker);

        // Apply life steal to attacker
        if (result.damageResult.lifeStealAmount > 0.0f) {
            attacker->heal(result.damageResult.lifeStealAmount);
        }

        // Apply reflected damage to attacker
        if (result.damageResult.reflectedDamage > 0.0f) {
            attacker->takeDamage(result.damageResult.reflectedDamage, element, target);
        }
    }

    return result;
}

bool CombatSystem::checkHit(CombatEntity* attacker, CombatEntity* target, float range) {
    if (!attacker || !target) return false;
    return attacker->isInRange(target, range);
}

std::vector<CombatEntity*> CombatSystem::getTargetsInArea(const Vector3& center, float radius, CombatEntity* ignore) {
    auto entities = getEntitiesInRadius(center, radius);

    // Remove ignore entity
    if (ignore) {
        entities.erase(
            std::remove(entities.begin(), entities.end(), ignore),
            entities.end()
        );
    }

    return entities;
}

std::vector<CombatEntity*> CombatSystem::getTargetsInCone(const Vector3& origin, Vector3 direction, float angle, float range, CombatEntity* ignore) {
    std::vector<CombatEntity*> result;
    // Implementation would check entities within cone
    return result;
}

std::vector<CombatEntity*> CombatSystem::getTargetsInLine(const Vector3& start, Vector3 end, float width, CombatEntity* ignore) {
    std::vector<CombatEntity*> result;
    // Implementation would check entities within line
    return result;
}

bool CombatSystem::hasLineOfSight(CombatEntity* from, CombatEntity* to) {
    // Raycast implementation
    return true;
}

void CombatSystem::applyKnockback(CombatEntity* entity, Vector2 direction, float force) {
    if (!entity || !m_knockbackEnabled) return;
    entity->applyKnockback(direction, force);
}

void CombatSystem::setEntityTeam(CombatEntity* entity, int teamId) {
    if (!entity) return;
    m_entityTeams[entity] = teamId;
}

int CombatSystem::getEntityTeam(CombatEntity* entity) {
    auto it = m_entityTeams.find(entity);
    if (it != m_entityTeams.end()) {
        return it->second;
    }
    return 0; // Default team
}

bool CombatSystem::areEnemies(CombatEntity* a, CombatEntity* b) {
    if (!a || !b) return false;
    return getEntityTeam(a) != getEntityTeam(b);
}

bool CombatSystem::areAllies(CombatEntity* a, CombatEntity* b) {
    if (!a || !b) return false;
    return getEntityTeam(a) == getEntityTeam(b);
}

void CombatSystem::setGlobalDamageMultiplier(float multiplier) {
    m_globalDamageMultiplier = multiplier;
    m_damageCalculator.setGlobalDamageMultiplier(multiplier);
}

void CombatSystem::setInvincibilityFrameDuration(float duration) {
    m_defaultIFrameDuration = duration;
}

void CombatSystem::setHitstunEnabled(bool enabled) {
    m_hitstunEnabled = enabled;
}

void CombatSystem::setKnockbackEnabled(bool enabled) {
    m_knockbackEnabled = enabled;
}

CombatSystem::CombatStats CombatSystem::getEntityStats(CombatEntity* entity) {
    auto it = m_entityStats.find(entity);
    if (it != m_entityStats.end()) {
        return it->second;
    }
    return CombatStats();
}

void CombatSystem::resetEntityStats(CombatEntity* entity) {
    auto it = m_entityStats.find(entity);
    if (it != m_entityStats.end()) {
        it->second = CombatStats();
    }
}

void CombatSystem::subscribeToAttackEvent(std::function<void(CombatEntity*, CombatEntity*, HitResult)> callback) {
    m_attackCallbacks.push_back(callback);
}

void CombatSystem::subscribeToDamageEvent(std::function<void(CombatEntity*, CombatEntity*, float)> callback) {
    m_damageCallbacks.push_back(callback);
}

void CombatSystem::subscribeToDeathEvent(std::function<void(CombatEntity*, CombatEntity*)> callback) {
    m_deathCallbacks.push_back(callback);
}

void CombatSystem::processHitResult(CombatEntity* attacker, CombatEntity* target, const HitResult& result) {
    // Trigger attack callbacks
    for (auto& callback : m_attackCallbacks) {
        callback(attacker, target, result);
    }

    // Trigger damage callbacks
    if (result.hit) {
        for (auto& callback : m_damageCallbacks) {
            callback(attacker, target, result.damageResult.totalDamage);
        }
    }

    // Update statistics
    updateStats(attacker, target, result);
}

void CombatSystem::updateStats(CombatEntity* attacker, CombatEntity* target, const HitResult& result) {
    auto& attackerStats = m_entityStats[attacker];
    auto& targetStats = m_entityStats[target];

    if (result.hit) {
        attackerStats.totalHitsLanded++;
        attackerStats.totalDamageDealt += static_cast<int>(result.damageResult.totalDamage);
        targetStats.totalDamageTaken += static_cast<int>(result.damageResult.totalDamage);

        if (result.damageResult.isCritical) {
            attackerStats.totalCriticalHits++;
        }
    }
    else {
        attackerStats.totalHitsMissed++;
    }

    if (result.defenseResult == DefenseResult::DODGED || result.defenseResult == DefenseResult::PERFECT_DODGE) {
        targetStats.totalDodges++;
    }
    else if (result.defenseResult == DefenseResult::BLOCKED) {
        targetStats.totalBlocks++;
    }
    else if (result.defenseResult == DefenseResult::PARRIED) {
        targetStats.totalParries++;
    }
}

} // namespace Combat
