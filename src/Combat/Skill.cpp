#include "Skill.h"
#include <algorithm>
#include <iostream>

namespace Combat {

// ===== SkillInstance Implementation =====

SkillInstance::SkillInstance(const SkillData& data, CombatEntity* caster)
    : m_data(data)
    , m_caster(caster)
    , m_target(nullptr)
    , m_isCasting(false)
    , m_isChanneling(false)
    , m_castProgress(0.0f)
    , m_channelProgress(0.0f)
{
}

SkillInstance::~SkillInstance() {
}

bool SkillInstance::execute(CombatSystem* combatSystem, CombatEntity* target) {
    if (!m_caster || !combatSystem) {
        return false;
    }

    m_target = target;

    // Check cast type
    if (m_data.castType == CastType::CAST_TIME) {
        m_isCasting = true;
        m_castProgress = 0.0f;
        return true;
    }
    else if (m_data.castType == CastType::CHANNELED) {
        m_isChanneling = true;
        m_channelProgress = 0.0f;
        return true;
    }

    // Instant cast - execute immediately
    executeDamage(combatSystem, target);
    executeHealing(target);
    applyStatusEffects(target);
    executeMovement();
    executeSummon(combatSystem);

    return true;
}

void SkillInstance::update(float deltaTime) {
    // Update cast time
    if (m_isCasting) {
        m_castProgress += deltaTime;
        if (m_castProgress >= m_data.castTime) {
            m_isCasting = false;
            // Execute skill effect here
            // This would need CombatSystem context
        }
    }

    // Update channel
    if (m_isChanneling) {
        m_channelProgress += deltaTime;
        if (m_channelProgress >= m_data.channelDuration) {
            m_isChanneling = false;
        }
        // Apply channeled effects each tick
    }
}

void SkillInstance::interrupt() {
    m_isCasting = false;
    m_isChanneling = false;
    m_castProgress = 0.0f;
    m_channelProgress = 0.0f;
}

void SkillInstance::executeDamage(CombatSystem* combatSystem, CombatEntity* target) {
    if (!target || m_data.baseDamage <= 0.0f) {
        return;
    }

    // Calculate damage using caster's stats
    float damage = m_data.baseDamage;

    // Apply scaling
    if (m_data.damageType == DamageType::PHYSICAL) {
        damage += m_caster->getStats().attack * m_data.damageScaling;
    }
    else if (m_data.damageType == DamageType::MAGICAL) {
        damage += m_caster->getStats().magicAttack * m_data.damageScaling;
    }

    // Deal damage to target
    // This would use the CombatSystem's damage calculator
    target->takeDamage(damage, m_data.element);

    // Handle life steal
    if (m_data.lifeSteal && m_data.lifeStealAmount > 0.0f) {
        float healAmount = damage * m_data.lifeStealAmount;
        m_caster->heal(healAmount);
    }
}

void SkillInstance::executeHealing(CombatEntity* target) {
    if (!target || m_data.baseHealing <= 0.0f) {
        return;
    }

    float healing = m_data.baseHealing;

    // Scale with magic attack
    healing += m_caster->getStats().magicAttack * m_data.healingScaling;

    target->heal(healing);
}

void SkillInstance::applyStatusEffects(CombatEntity* target) {
    if (!target) return;

    // Apply all status effects
    for (size_t i = 0; i < m_data.statusEffectsToApply.size(); ++i) {
        float chance = 100.0f;
        if (i < m_data.statusEffectChances.size()) {
            chance = m_data.statusEffectChances[i];
        }

        // Roll for application
        float roll = static_cast<float>(rand() % 100);
        if (roll < chance) {
            // Get status effect from factory and apply
            auto* effectData = StatusEffectFactory::getInstance().getStatusEffect(
                m_data.statusEffectsToApply[i]
            );

            if (effectData) {
                // Apply through target's status effect manager
                // target->getStatusEffectManager()->applyStatusEffect(*effectData, m_caster, target);
            }
        }
    }
}

void SkillInstance::executeMovement() {
    if (m_data.dashDistance > 0.0f || m_data.teleport) {
        // Execute movement skill
        // This would move the caster in the facing direction
    }
}

void SkillInstance::executeSummon(CombatSystem* combatSystem) {
    if (!m_data.summon || m_data.summonId.empty()) {
        return;
    }

    // Summon entities through combat system
    // for (int i = 0; i < m_data.summonCount; ++i) {
    //     combatSystem->spawnSummon(m_data.summonId, m_caster);
    // }
}

// ===== SkillManager Implementation =====

SkillManager::SkillManager()
    : m_comboWindow(0.0f)
{
}

SkillManager::~SkillManager() {
}

void SkillManager::update(float deltaTime) {
    // Update active skills
    auto it = m_activeSkills.begin();
    while (it != m_activeSkills.end()) {
        (*it)->update(deltaTime);

        // Remove finished skills
        if (!(*it)->isCasting() && !(*it)->isChanneling()) {
            it = m_activeSkills.erase(it);
        }
        else {
            ++it;
        }
    }

    // Update cooldowns
    updateCooldowns(deltaTime);
    updateCharges(deltaTime);

    // Update combo window
    if (m_comboWindow > 0.0f) {
        m_comboWindow -= deltaTime;
        if (m_comboWindow <= 0.0f) {
            m_lastUsedSkill.clear();
        }
    }
}

bool SkillManager::learnSkill(const std::string& skillId) {
    auto* skillData = SkillFactory::getInstance().getSkill(skillId);
    if (!skillData) {
        return false;
    }

    // Check if already learned
    if (hasSkill(skillId)) {
        return false;
    }

    // Check prerequisites
    for (const auto& prereq : skillData->prerequisiteSkills) {
        if (!hasSkill(prereq)) {
            return false;
        }
    }

    // Learn the skill
    m_learnedSkills[skillId] = *skillData;
    m_skillLevels[skillId] = 1;

    // Initialize cooldown tracking
    CooldownInfo cooldownInfo;
    cooldownInfo.remainingTime = 0.0f;
    cooldownInfo.currentCharges = skillData->maxCharges > 0 ? skillData->maxCharges : 0;
    cooldownInfo.chargeRegenTime = 0.0f;
    m_cooldowns[skillId] = cooldownInfo;

    return true;
}

bool SkillManager::unlockSkill(const std::string& skillId) {
    return learnSkill(skillId);
}

bool SkillManager::upgradeSkill(const std::string& skillId) {
    if (!hasSkill(skillId)) {
        return false;
    }

    auto& skill = m_learnedSkills[skillId];
    int currentLevel = m_skillLevels[skillId];

    if (currentLevel >= skill.maxLevel) {
        return false;
    }

    m_skillLevels[skillId]++;

    // Apply upgrades
    if (currentLevel < static_cast<int>(skill.upgrades.size())) {
        // Apply upgrade modifications
        // This would modify the skill data based on upgrade definitions
    }

    return true;
}

bool SkillManager::canUseSkill(const std::string& skillId, CombatEntity* caster) {
    if (!hasSkill(skillId) || !caster) {
        return false;
    }

    // Check cooldown
    if (isOnCooldown(skillId)) {
        // Check charges
        if (m_cooldowns[skillId].currentCharges <= 0) {
            return false;
        }
    }

    // Check resources
    if (!hasResourcesForSkill(skillId, caster)) {
        return false;
    }

    // Check if already casting
    for (const auto& active : m_activeSkills) {
        if (active->isCasting() && !m_learnedSkills[skillId].comboCancelable) {
            return false;
        }
    }

    return true;
}

SkillInstance* SkillManager::useSkill(const std::string& skillId, CombatEntity* caster, CombatEntity* target) {
    if (!canUseSkill(skillId, caster)) {
        return nullptr;
    }

    auto& skillData = m_learnedSkills[skillId];

    // Create skill instance
    auto instance = std::make_shared<SkillInstance>(skillData, caster);
    m_activeSkills.push_back(instance);

    // Start cooldown
    if (skillData.maxCharges > 0) {
        // Use a charge
        m_cooldowns[skillId].currentCharges--;
        if (m_cooldowns[skillId].currentCharges == skillData.maxCharges - 1) {
            // Start charge regen
            m_cooldowns[skillId].chargeRegenTime = skillData.cooldown;
        }
    }
    else {
        // Start normal cooldown
        startCooldown(skillId, skillData.cooldown, 0);
    }

    // Update combo tracking
    setLastUsedSkill(skillId);
    m_comboWindow = skillData.comboWindow;

    return instance.get();
}

bool SkillManager::isOnCooldown(const std::string& skillId) const {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end()) {
        return it->second.remainingTime > 0.0f;
    }
    return false;
}

float SkillManager::getCooldownRemaining(const std::string& skillId) const {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end()) {
        return it->second.remainingTime;
    }
    return 0.0f;
}

void SkillManager::setCooldown(const std::string& skillId, float duration) {
    startCooldown(skillId, duration, 0);
}

void SkillManager::reduceCooldown(const std::string& skillId, float reduction) {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end()) {
        it->second.remainingTime = std::max(0.0f, it->second.remainingTime - reduction);
    }
}

void SkillManager::resetCooldown(const std::string& skillId) {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end()) {
        it->second.remainingTime = 0.0f;

        // Reset charges if applicable
        if (hasSkill(skillId)) {
            auto& skill = m_learnedSkills[skillId];
            if (skill.maxCharges > 0) {
                it->second.currentCharges = skill.maxCharges;
                it->second.chargeRegenTime = 0.0f;
            }
        }
    }
}

void SkillManager::resetAllCooldowns() {
    for (auto& pair : m_cooldowns) {
        resetCooldown(pair.first);
    }
}

int SkillManager::getCharges(const std::string& skillId) const {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end()) {
        return it->second.currentCharges;
    }
    return 0;
}

void SkillManager::addCharge(const std::string& skillId) {
    auto it = m_cooldowns.find(skillId);
    if (it != m_cooldowns.end() && hasSkill(skillId)) {
        auto& skill = m_learnedSkills[skillId];
        if (it->second.currentCharges < skill.maxCharges) {
            it->second.currentCharges++;
        }
    }
}

bool SkillManager::hasSkill(const std::string& skillId) const {
    return m_learnedSkills.find(skillId) != m_learnedSkills.end();
}

SkillData* SkillManager::getSkill(const std::string& skillId) {
    auto it = m_learnedSkills.find(skillId);
    if (it != m_learnedSkills.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<SkillData*> SkillManager::getAllSkills() {
    std::vector<SkillData*> skills;
    for (auto& pair : m_learnedSkills) {
        skills.push_back(&pair.second);
    }
    return skills;
}

std::vector<SkillData*> SkillManager::getSkillsByCategory(SkillCategory category) {
    std::vector<SkillData*> skills;
    for (auto& pair : m_learnedSkills) {
        if (pair.second.category == category) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

void SkillManager::setLastUsedSkill(const std::string& skillId) {
    m_lastUsedSkill = skillId;
}

bool SkillManager::canComboInto(const std::string& skillId) const {
    if (m_lastUsedSkill.empty() || m_comboWindow <= 0.0f) {
        return false;
    }

    auto it = m_learnedSkills.find(m_lastUsedSkill);
    if (it != m_learnedSkills.end()) {
        return it->second.comboFollowup == skillId;
    }

    return false;
}

bool SkillManager::hasResourcesForSkill(const std::string& skillId, CombatEntity* caster) {
    if (!hasSkill(skillId) || !caster) {
        return false;
    }

    auto& skill = m_learnedSkills[skillId];

    // Resource checks would go here
    // For now, just return true
    return true;
}

int SkillManager::getSkillLevel(const std::string& skillId) const {
    auto it = m_skillLevels.find(skillId);
    if (it != m_skillLevels.end()) {
        return it->second;
    }
    return 0;
}

void SkillManager::setSkillLevel(const std::string& skillId, int level) {
    if (hasSkill(skillId)) {
        m_skillLevels[skillId] = level;
    }
}

void SkillManager::startCooldown(const std::string& skillId, float duration, int maxCharges) {
    CooldownInfo info;
    info.remainingTime = duration;
    info.currentCharges = maxCharges;
    info.chargeRegenTime = 0.0f;
    m_cooldowns[skillId] = info;
}

void SkillManager::updateCooldowns(float deltaTime) {
    for (auto& pair : m_cooldowns) {
        if (pair.second.remainingTime > 0.0f) {
            pair.second.remainingTime -= deltaTime;
            if (pair.second.remainingTime < 0.0f) {
                pair.second.remainingTime = 0.0f;
            }
        }
    }
}

void SkillManager::updateCharges(float deltaTime) {
    for (auto& pair : m_cooldowns) {
        if (pair.second.chargeRegenTime > 0.0f) {
            pair.second.chargeRegenTime -= deltaTime;

            if (pair.second.chargeRegenTime <= 0.0f) {
                // Regenerate a charge
                if (hasSkill(pair.first)) {
                    auto& skill = m_learnedSkills[pair.first];
                    if (pair.second.currentCharges < skill.maxCharges) {
                        pair.second.currentCharges++;

                        // Continue regenerating if not at max
                        if (pair.second.currentCharges < skill.maxCharges) {
                            pair.second.chargeRegenTime = skill.cooldown;
                        }
                    }
                }
            }
        }
    }
}

// ===== SkillFactory Implementation =====

SkillFactory& SkillFactory::getInstance() {
    static SkillFactory instance;
    return instance;
}

SkillFactory::SkillFactory() {
}

SkillFactory::~SkillFactory() {
}

bool SkillFactory::loadSkills(const std::string& jsonPath) {
    // TODO: Implement JSON parsing
    std::cout << "Loading skills from: " << jsonPath << std::endl;
    return true;
}

SkillData* SkillFactory::getSkill(const std::string& skillId) {
    auto it = m_skillDatabase.find(skillId);
    if (it != m_skillDatabase.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<SkillData*> SkillFactory::getSkillsByCategory(SkillCategory category) {
    std::vector<SkillData*> skills;
    for (auto& pair : m_skillDatabase) {
        if (pair.second.category == category) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

std::vector<SkillData*> SkillFactory::getSkillsByElement(Element element) {
    std::vector<SkillData*> skills;
    for (auto& pair : m_skillDatabase) {
        if (pair.second.element == element) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

void SkillFactory::registerSkill(const SkillData& data) {
    m_skillDatabase[data.id] = data;
}

SkillCategory SkillFactory::parseCategory(const std::string& str) {
    if (str == "OFFENSIVE") return SkillCategory::OFFENSIVE;
    if (str == "DEFENSIVE") return SkillCategory::DEFENSIVE;
    if (str == "SUPPORT") return SkillCategory::SUPPORT;
    if (str == "CROWD_CONTROL") return SkillCategory::CROWD_CONTROL;
    if (str == "MOVEMENT") return SkillCategory::MOVEMENT;
    if (str == "ULTIMATE") return SkillCategory::ULTIMATE;
    return SkillCategory::OFFENSIVE;
}

TargetType SkillFactory::parseTargetType(const std::string& str) {
    if (str == "SELF") return TargetType::SELF;
    if (str == "SINGLE_ENEMY") return TargetType::SINGLE_ENEMY;
    if (str == "SINGLE_ALLY") return TargetType::SINGLE_ALLY;
    if (str == "ALL_ENEMIES") return TargetType::ALL_ENEMIES;
    if (str == "ALL_ALLIES") return TargetType::ALL_ALLIES;
    if (str == "AREA") return TargetType::AREA;
    if (str == "LINE") return TargetType::LINE;
    if (str == "CONE") return TargetType::CONE;
    if (str == "CHAIN") return TargetType::CHAIN;
    if (str == "GROUND") return TargetType::GROUND;
    if (str == "DIRECTION") return TargetType::DIRECTION;
    return TargetType::SINGLE_ENEMY;
}

CastType SkillFactory::parseCastType(const std::string& str) {
    if (str == "INSTANT") return CastType::INSTANT;
    if (str == "CAST_TIME") return CastType::CAST_TIME;
    if (str == "CHANNELED") return CastType::CHANNELED;
    return CastType::INSTANT;
}

ResourceType SkillFactory::parseResourceType(const std::string& str) {
    if (str == "MANA") return ResourceType::MANA;
    if (str == "STAMINA") return ResourceType::STAMINA;
    if (str == "RAGE") return ResourceType::RAGE;
    if (str == "ENERGY") return ResourceType::ENERGY;
    if (str == "HEALTH") return ResourceType::HEALTH;
    if (str == "COMBO_POINTS") return ResourceType::COMBO_POINTS;
    if (str == "NONE") return ResourceType::NONE;
    return ResourceType::MANA;
}

// ===== SkillBuilder Implementation =====

SkillFactory::SkillBuilder::SkillBuilder(const std::string& id) {
    m_skillData.id = id;
    m_skillData.name = id;
    m_skillData.category = SkillCategory::OFFENSIVE;
    m_skillData.targetType = TargetType::SINGLE_ENEMY;
    m_skillData.castType = CastType::INSTANT;
    m_skillData.damageType = DamageType::PHYSICAL;
    m_skillData.element = Element::NONE;
    m_skillData.resourceType = ResourceType::MANA;
    m_skillData.resourceCost = 0.0f;
    m_skillData.cooldown = 0.0f;
    m_skillData.range = 5.0f;
    m_skillData.baseDamage = 0.0f;
    m_skillData.damageScaling = 1.0f;
    m_skillData.canCrit = true;
    m_skillData.maxLevel = 5;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setName(const std::string& name) {
    m_skillData.name = name;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setDescription(const std::string& desc) {
    m_skillData.description = desc;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setCategory(SkillCategory category) {
    m_skillData.category = category;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setTargetType(TargetType targetType) {
    m_skillData.targetType = targetType;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setCastType(CastType castType) {
    m_skillData.castType = castType;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setDamage(float baseDamage, float scaling, DamageType type) {
    m_skillData.baseDamage = baseDamage;
    m_skillData.damageScaling = scaling;
    m_skillData.damageType = type;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setElement(Element element) {
    m_skillData.element = element;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setCooldown(float cooldown) {
    m_skillData.cooldown = cooldown;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setRange(float range) {
    m_skillData.range = range;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setResourceCost(ResourceType type, float cost) {
    m_skillData.resourceType = type;
    m_skillData.resourceCost = cost;
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::addStatusEffect(const std::string& effectId, float chance) {
    m_skillData.statusEffectsToApply.push_back(effectId);
    m_skillData.statusEffectChances.push_back(chance);
    return *this;
}

SkillFactory::SkillBuilder& SkillFactory::SkillBuilder::setAnimation(const std::string& animName) {
    m_skillData.animationName = animName;
    return *this;
}

SkillData SkillFactory::SkillBuilder::build() {
    return m_skillData;
}

} // namespace Combat
