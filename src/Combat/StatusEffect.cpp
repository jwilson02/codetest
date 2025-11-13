#include "StatusEffect.h"
#include <algorithm>
#include <fstream>
#include <iostream>

// Note: CombatEntity would be defined in CombatSystem.h
// For now, we'll use a forward declaration and basic structure

namespace Combat {

// Placeholder for CombatEntity (will be properly defined in CombatSystem)
class CombatEntity {
public:
    virtual void takeDamage(float damage, Element element) {}
    virtual void heal(float amount) {}
    virtual CombatStats& getStats() { static CombatStats stats; return stats; }
};

// ===== StatusEffectInstance Implementation =====

StatusEffectInstance::StatusEffectInstance(const StatusEffectData& data, CombatEntity* caster)
    : m_data(data)
    , m_caster(caster)
    , m_remainingDuration(data.duration)
    , m_tickTimer(0.0f)
    , m_currentStacks(1)
    , m_strengthMultiplier(1.0f)
    , m_hasAppliedInitialEffect(false)
{
}

StatusEffectInstance::~StatusEffectInstance() {
}

void StatusEffectInstance::update(float deltaTime, CombatEntity* target) {
    if (!target) return;

    // Update duration
    m_remainingDuration -= deltaTime;

    // Handle tick-based effects (DOTs)
    if (m_data.tickRate > 0.0f) {
        m_tickTimer += deltaTime;

        while (m_tickTimer >= m_data.tickRate) {
            m_tickTimer -= m_data.tickRate;
            applyEffect(target);
        }
    }

    // Apply continuous effects on first frame
    if (!m_hasAppliedInitialEffect) {
        applyEffect(target);
        m_hasAppliedInitialEffect = true;
    }
}

void StatusEffectInstance::applyEffect(CombatEntity* target) {
    if (!target) return;

    // Apply damage for DOT effects
    if (m_data.damagePerTick > 0.0f) {
        float damage = m_data.damagePerTick * m_strengthMultiplier * m_currentStacks;
        target->takeDamage(damage, m_data.element);
    }

    // Apply healing for HOT effects
    if (m_data.healPerTick > 0.0f) {
        float healing = m_data.healPerTick * m_strengthMultiplier * m_currentStacks;
        target->heal(healing);
    }

    // Special effect handling
    switch (m_data.type) {
        case StatusEffectType::DOOM:
            // Check if doom timer expired
            if (m_remainingDuration <= 0.0f) {
                target->takeDamage(999999.0f, Element::DARK); // Instant kill
            }
            break;

        case StatusEffectType::SHOCK:
            // Shock can chain to nearby enemies (would need combat system context)
            break;

        case StatusEffectType::BLESSED:
            // Gradually increases buff strength over time
            m_strengthMultiplier += 0.01f * m_currentStacks;
            break;

        default:
            break;
    }
}

void StatusEffectInstance::addStack() {
    if (m_currentStacks < m_data.maxStacks) {
        m_currentStacks++;
    }
}

bool StatusEffectInstance::canStack() const {
    return m_currentStacks < m_data.maxStacks &&
           m_data.stackBehavior != StackBehavior::NONE;
}

void StatusEffectInstance::refreshDuration() {
    m_remainingDuration = m_data.duration;
}

void StatusEffectInstance::addDuration(float duration) {
    m_remainingDuration += duration;
}

bool StatusEffectInstance::isControlEffect() const {
    return m_data.type == StatusEffectType::STUN ||
           m_data.type == StatusEffectType::FREEZE ||
           m_data.type == StatusEffectType::ROOT ||
           m_data.type == StatusEffectType::SILENCE ||
           m_data.type == StatusEffectType::SLEEP ||
           m_data.type == StatusEffectType::FEAR ||
           m_data.type == StatusEffectType::CHARM ||
           m_data.type == StatusEffectType::PETRIFY;
}

void StatusEffectInstance::modifyStrength(float multiplier) {
    m_strengthMultiplier *= multiplier;
}

// ===== StatusEffectManager Implementation =====

StatusEffectManager::StatusEffectManager() {
}

StatusEffectManager::~StatusEffectManager() {
    clearAll();
}

void StatusEffectManager::update(float deltaTime, CombatEntity* owner) {
    if (!owner) return;

    // Update all active effects
    auto it = m_activeEffects.begin();
    while (it != m_activeEffects.end()) {
        (*it)->update(deltaTime, owner);

        // Remove expired effects
        if ((*it)->isExpired()) {
            it = m_activeEffects.erase(it);
        }
        else {
            ++it;
        }
    }
}

bool StatusEffectManager::applyStatusEffect(const StatusEffectData& data, CombatEntity* caster, CombatEntity* target) {
    if (!target) return false;

    // Check if blocked
    if (shouldBlockEffect(data, target)) {
        return false;
    }

    // Check for existing effect
    auto existing = getEffect(data.id);
    if (existing) {
        handleStacking(existing, data, caster);
        return true;
    }

    // Create new effect instance
    auto newEffect = std::make_shared<StatusEffectInstance>(data, caster);
    m_activeEffects.push_back(newEffect);

    // Remove conflicting effects
    for (const auto& clearEffect : data.clearsEffects) {
        removeEffect(clearEffect);
    }

    return true;
}

void StatusEffectManager::removeEffect(const std::string& effectId) {
    m_activeEffects.erase(
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
            [&effectId](const std::shared_ptr<StatusEffectInstance>& effect) {
                return effect->getId() == effectId;
            }),
        m_activeEffects.end()
    );
}

void StatusEffectManager::removeEffectByType(StatusEffectType type) {
    m_activeEffects.erase(
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
            [type](const std::shared_ptr<StatusEffectInstance>& effect) {
                return effect->getType() == type;
            }),
        m_activeEffects.end()
    );
}

void StatusEffectManager::removeAllDebuffs() {
    m_activeEffects.erase(
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
            [](const std::shared_ptr<StatusEffectInstance>& effect) {
                return effect->isDebuff();
            }),
        m_activeEffects.end()
    );
}

void StatusEffectManager::removeAllBuffs() {
    m_activeEffects.erase(
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(),
            [](const std::shared_ptr<StatusEffectInstance>& effect) {
                return effect->isBuff();
            }),
        m_activeEffects.end()
    );
}

void StatusEffectManager::clearAll() {
    m_activeEffects.clear();
}

bool StatusEffectManager::hasEffect(const std::string& effectId) const {
    return std::any_of(m_activeEffects.begin(), m_activeEffects.end(),
        [&effectId](const std::shared_ptr<StatusEffectInstance>& effect) {
            return effect->getId() == effectId;
        });
}

bool StatusEffectManager::hasEffectType(StatusEffectType type) const {
    return std::any_of(m_activeEffects.begin(), m_activeEffects.end(),
        [type](const std::shared_ptr<StatusEffectInstance>& effect) {
            return effect->getType() == type;
        });
}

StatusEffectInstance* StatusEffectManager::getEffect(const std::string& effectId) {
    auto it = std::find_if(m_activeEffects.begin(), m_activeEffects.end(),
        [&effectId](const std::shared_ptr<StatusEffectInstance>& effect) {
            return effect->getId() == effectId;
        });

    return (it != m_activeEffects.end()) ? it->get() : nullptr;
}

std::vector<StatusEffectInstance*> StatusEffectManager::getEffectsByType(StatusEffectType type) {
    std::vector<StatusEffectInstance*> results;
    for (auto& effect : m_activeEffects) {
        if (effect->getType() == type) {
            results.push_back(effect.get());
        }
    }
    return results;
}

std::vector<StatusEffectInstance*> StatusEffectManager::getAllEffects() {
    std::vector<StatusEffectInstance*> results;
    for (auto& effect : m_activeEffects) {
        results.push_back(effect.get());
    }
    return results;
}

bool StatusEffectManager::isImmuneToType(StatusEffectType type) const {
    return std::find(m_immunities.begin(), m_immunities.end(), type) != m_immunities.end();
}

void StatusEffectManager::addImmunity(StatusEffectType type) {
    if (!isImmuneToType(type)) {
        m_immunities.push_back(type);
    }
}

void StatusEffectManager::removeImmunity(StatusEffectType type) {
    m_immunities.erase(
        std::remove(m_immunities.begin(), m_immunities.end(), type),
        m_immunities.end()
    );
}

float StatusEffectManager::getTotalAttackModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().attackModifier;
    }
    return total;
}

float StatusEffectManager::getTotalDefenseModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().defenseModifier;
    }
    return total;
}

float StatusEffectManager::getTotalMagicAttackModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().magicAttackModifier;
    }
    return total;
}

float StatusEffectManager::getTotalMagicDefenseModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().magicDefenseModifier;
    }
    return total;
}

float StatusEffectManager::getTotalSpeedModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().speedModifier;
    }
    return total;
}

float StatusEffectManager::getTotalAccuracyModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().accuracyModifier;
    }
    return total;
}

float StatusEffectManager::getTotalEvasionModifier() const {
    float total = 1.0f;
    for (const auto& effect : m_activeEffects) {
        total *= effect->getData().evasionModifier;
    }
    return total;
}

ElementalResistance StatusEffectManager::getTotalResistanceModifier() const {
    ElementalResistance total;
    for (const auto& effect : m_activeEffects) {
        const auto& res = effect->getData().resistanceModifier;
        total.fire += res.fire;
        total.ice += res.ice;
        total.lightning += res.lightning;
        total.earth += res.earth;
        total.holy += res.holy;
        total.dark += res.dark;
        total.physical += res.physical;
    }
    return total;
}

bool StatusEffectManager::isStunned() const {
    return hasEffectType(StatusEffectType::STUN) ||
           hasEffectType(StatusEffectType::PETRIFY);
}

bool StatusEffectManager::isFrozen() const {
    return hasEffectType(StatusEffectType::FREEZE);
}

bool StatusEffectManager::isSilenced() const {
    return hasEffectType(StatusEffectType::SILENCE);
}

bool StatusEffectManager::isRooted() const {
    return hasEffectType(StatusEffectType::ROOT);
}

bool StatusEffectManager::canAct() const {
    return !isStunned() && !isFrozen() &&
           !hasEffectType(StatusEffectType::SLEEP) &&
           !hasEffectType(StatusEffectType::FEAR);
}

bool StatusEffectManager::canMove() const {
    return !isStunned() && !isFrozen() && !isRooted() &&
           !hasEffectType(StatusEffectType::SLEEP);
}

bool StatusEffectManager::canCast() const {
    return canAct() && !isSilenced();
}

bool StatusEffectManager::canBeHealed() const {
    return !hasEffectType(StatusEffectType::CURSED);
}

int StatusEffectManager::getDebuffCount() const {
    int count = 0;
    for (const auto& effect : m_activeEffects) {
        if (effect->isDebuff()) count++;
    }
    return count;
}

int StatusEffectManager::getBuffCount() const {
    int count = 0;
    for (const auto& effect : m_activeEffects) {
        if (effect->isBuff()) count++;
    }
    return count;
}

int StatusEffectManager::getControlEffectCount() const {
    int count = 0;
    for (const auto& effect : m_activeEffects) {
        if (effect->isControlEffect()) count++;
    }
    return count;
}

int StatusEffectManager::dispelBuffs(int count) {
    int dispelled = 0;
    auto it = m_activeEffects.begin();

    while (it != m_activeEffects.end() && dispelled < count) {
        if ((*it)->isBuff() && (*it)->getData().dispellable) {
            it = m_activeEffects.erase(it);
            dispelled++;
        }
        else {
            ++it;
        }
    }

    return dispelled;
}

int StatusEffectManager::dispelDebuffs(int count) {
    int dispelled = 0;
    auto it = m_activeEffects.begin();

    while (it != m_activeEffects.end() && dispelled < count) {
        if ((*it)->isDebuff() && (*it)->getData().dispellable) {
            it = m_activeEffects.erase(it);
            dispelled++;
        }
        else {
            ++it;
        }
    }

    return dispelled;
}

bool StatusEffectManager::shouldBlockEffect(const StatusEffectData& data, CombatEntity* target) {
    // Check immunity
    if (isImmuneToType(data.type)) {
        return true;
    }

    // Check if already has immunity effect
    if (hasEffectType(StatusEffectType::IMMUNITY) && !data.isBuff) {
        return true;
    }

    // Check application chance
    if (data.applicationChance < 100.0f) {
        float roll = static_cast<float>(rand() % 100);
        if (roll >= data.applicationChance) {
            return true;
        }
    }

    return false;
}

void StatusEffectManager::handleStacking(StatusEffectInstance* existing, const StatusEffectData& newData, CombatEntity* caster) {
    if (!existing) return;

    switch (newData.stackBehavior) {
        case StackBehavior::NONE:
            // Just refresh duration
            existing->refreshDuration();
            break;

        case StackBehavior::INTENSITY:
            // Add stack and refresh duration
            if (existing->canStack()) {
                existing->addStack();
            }
            existing->refreshDuration();
            break;

        case StackBehavior::DURATION:
            // Add to duration
            existing->addDuration(newData.duration);
            break;

        case StackBehavior::INDEPENDENT:
            // Create a new independent instance (handled by caller)
            break;
    }
}

// ===== StatusEffectFactory Implementation =====

StatusEffectFactory& StatusEffectFactory::getInstance() {
    static StatusEffectFactory instance;
    return instance;
}

StatusEffectFactory::StatusEffectFactory() {
}

StatusEffectFactory::~StatusEffectFactory() {
}

bool StatusEffectFactory::loadStatusEffects(const std::string& jsonPath) {
    // TODO: Implement JSON parsing
    // For now, this is a placeholder
    std::cout << "Loading status effects from: " << jsonPath << std::endl;
    return true;
}

StatusEffectData* StatusEffectFactory::getStatusEffect(const std::string& effectId) {
    auto it = m_effectDatabase.find(effectId);
    if (it != m_effectDatabase.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<StatusEffectData*> StatusEffectFactory::getEffectsByType(StatusEffectType type) {
    std::vector<StatusEffectData*> results;
    for (auto& pair : m_effectDatabase) {
        if (pair.second.type == type) {
            results.push_back(&pair.second);
        }
    }
    return results;
}

void StatusEffectFactory::registerEffect(const StatusEffectData& data) {
    m_effectDatabase[data.id] = data;
}

StatusEffectType StatusEffectFactory::parseEffectType(const std::string& typeStr) {
    // Simple string to enum conversion
    if (typeStr == "BURN") return StatusEffectType::BURN;
    if (typeStr == "POISON") return StatusEffectType::POISON;
    if (typeStr == "BLEED") return StatusEffectType::BLEED;
    if (typeStr == "STUN") return StatusEffectType::STUN;
    if (typeStr == "FREEZE") return StatusEffectType::FREEZE;
    // ... add more mappings
    return StatusEffectType::BURN; // Default
}

StackBehavior StatusEffectFactory::parseStackBehavior(const std::string& behaviorStr) {
    if (behaviorStr == "NONE") return StackBehavior::NONE;
    if (behaviorStr == "INTENSITY") return StackBehavior::INTENSITY;
    if (behaviorStr == "DURATION") return StackBehavior::DURATION;
    if (behaviorStr == "INDEPENDENT") return StackBehavior::INDEPENDENT;
    return StackBehavior::NONE; // Default
}

} // namespace Combat
