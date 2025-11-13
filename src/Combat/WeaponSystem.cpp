#include "WeaponSystem.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>

namespace Combat {

static std::mt19937 weaponRng(static_cast<unsigned>(std::time(nullptr) + 1));

// ===== Weapon Implementation =====

Weapon::Weapon()
    : m_currentDurability(100.0f)
    , m_currentComboStep(0)
    , m_attackCooldown(0.0f)
    , m_comboTimer(0.0f)
{
}

Weapon::Weapon(const WeaponData& data)
    : m_data(data)
    , m_currentDurability(data.maxDurability)
    , m_currentComboStep(0)
    , m_attackCooldown(0.0f)
    , m_comboTimer(0.0f)
{
    recalculateStats();
}

Weapon::~Weapon() {
}

float Weapon::calculateDamage(const CombatStats& wielderStats) {
    // Base damage
    std::uniform_real_distribution<float> damageDist(m_data.minDamage, m_data.maxDamage);
    float baseDamage = damageDist(weaponRng);

    // Apply combo multiplier
    baseDamage *= getCurrentComboDamageMultiplier();

    // Scale with wielder stats
    if (m_data.primaryDamageType == DamageType::PHYSICAL) {
        baseDamage *= (wielderStats.attack / 100.0f);
    }
    else if (m_data.primaryDamageType == DamageType::MAGICAL) {
        baseDamage *= (wielderStats.magicAttack / 100.0f);
    }

    // Apply upgrade bonus
    float upgradeMultiplier = 1.0f + (m_data.upgradeLevel * 0.1f);
    baseDamage *= upgradeMultiplier;

    // Durability affects damage
    if (m_currentDurability < m_data.maxDurability * 0.25f) {
        baseDamage *= 0.75f; // 25% damage reduction when low durability
    }

    return baseDamage;
}

bool Weapon::canAttack() const {
    return m_attackCooldown <= 0.0f && !isBroken();
}

void Weapon::performAttack() {
    if (!canAttack()) return;

    // Set attack cooldown based on attack speed
    m_attackCooldown = 1.0f / m_data.attackSpeed;

    // Advance combo
    nextComboStep();

    // Reduce durability
    reduceDurability(m_data.durabilityLossPerHit);

    // Reset combo timer
    m_comboTimer = 1.5f; // 1.5 seconds to continue combo
}

void Weapon::nextComboStep() {
    m_currentComboStep++;
    if (m_currentComboStep >= m_data.attackComboLength) {
        m_currentComboStep = 0;
    }
}

void Weapon::resetCombo() {
    m_currentComboStep = 0;
}

float Weapon::getCurrentComboDamageMultiplier() const {
    if (m_currentComboStep < static_cast<int>(m_data.comboDamageMultipliers.size())) {
        return m_data.comboDamageMultipliers[m_currentComboStep];
    }
    return 1.0f;
}

void Weapon::reduceDurability(float amount) {
    m_currentDurability = std::max(0.0f, m_currentDurability - amount);
}

void Weapon::repair(float amount) {
    m_currentDurability = std::min(m_data.maxDurability, m_currentDurability + amount);
}

float Weapon::getDurabilityPercent() const {
    if (m_data.maxDurability <= 0.0f) return 100.0f;
    return (m_currentDurability / m_data.maxDurability) * 100.0f;
}

bool Weapon::upgrade() {
    if (!canUpgrade()) return false;

    m_data.upgradeLevel++;

    // Increase stats
    m_data.minDamage *= 1.1f;
    m_data.maxDamage *= 1.1f;
    m_data.criticalDamage += 5.0f;

    recalculateStats();
    return true;
}

bool Weapon::canUpgrade() const {
    return m_data.upgradeLevel < m_data.maxUpgradeLevel;
}

bool Weapon::addGem(const std::string& gemId) {
    if (static_cast<int>(m_data.socketedGems.size()) >= m_data.socketCount) {
        return false;
    }

    m_data.socketedGems.push_back(gemId);
    recalculateStats();
    return true;
}

bool Weapon::removeGem(int socketIndex) {
    if (socketIndex < 0 || socketIndex >= static_cast<int>(m_data.socketedGems.size())) {
        return false;
    }

    m_data.socketedGems.erase(m_data.socketedGems.begin() + socketIndex);
    recalculateStats();
    return true;
}

float Weapon::getMinDamage() const {
    return m_data.minDamage;
}

float Weapon::getMaxDamage() const {
    return m_data.maxDamage;
}

float Weapon::getAttackSpeed() const {
    return m_data.attackSpeed * m_data.attackSpeedMultiplier;
}

float Weapon::getCriticalChance() const {
    return m_data.criticalChance;
}

float Weapon::getCriticalDamage() const {
    return m_data.criticalDamage;
}

float Weapon::getRange() const {
    return m_data.range;
}

void Weapon::update(float deltaTime) {
    // Update attack cooldown
    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= deltaTime;
    }

    // Update combo timer
    if (m_comboTimer > 0.0f) {
        m_comboTimer -= deltaTime;
        if (m_comboTimer <= 0.0f) {
            resetCombo();
        }
    }
}

void Weapon::recalculateStats() {
    // Apply gem bonuses
    for (const auto& gemId : m_data.socketedGems) {
        // Look up gem and apply bonuses
        // This would integrate with a gem system
    }
}

// ===== WeaponManager Implementation =====

WeaponManager::WeaponManager()
    : m_mainHandWeapon(nullptr)
    , m_offHandWeapon(nullptr)
    , m_alternateMainHand(nullptr)
    , m_alternateOffHand(nullptr)
    , m_usingAlternateSet(false)
{
}

WeaponManager::~WeaponManager() {
}

bool WeaponManager::equipWeapon(Weapon* weapon, bool mainHand) {
    if (!weapon) return false;

    // Check if weapon can be equipped
    const auto& data = weapon->getData();

    if (mainHand) {
        // Unequip current main hand
        unequipMainHand();

        // If two-handed, also unequip off-hand
        if (data.twoHanded) {
            unequipOffHand();
        }

        m_mainHandWeapon = weapon;
        return true;
    }
    else {
        // Cannot equip two-handed weapon in off-hand
        if (data.twoHanded) {
            return false;
        }

        // Cannot equip off-hand if main hand is two-handed
        if (m_mainHandWeapon && m_mainHandWeapon->getData().twoHanded) {
            return false;
        }

        // Cannot dual wield if weapon is not dual-wieldable
        if (!data.dualWieldable && m_mainHandWeapon) {
            return false;
        }

        unequipOffHand();
        m_offHandWeapon = weapon;
        return true;
    }
}

void WeaponManager::unequipMainHand() {
    m_mainHandWeapon = nullptr;
}

void WeaponManager::unequipOffHand() {
    m_offHandWeapon = nullptr;
}

void WeaponManager::swapWeapons() {
    std::swap(m_mainHandWeapon, m_offHandWeapon);
}

bool WeaponManager::canAttack() const {
    if (!m_mainHandWeapon) return false;
    return m_mainHandWeapon->canAttack();
}

void WeaponManager::performMainHandAttack() {
    if (m_mainHandWeapon && m_mainHandWeapon->canAttack()) {
        m_mainHandWeapon->performAttack();
    }
}

void WeaponManager::performOffHandAttack() {
    if (m_offHandWeapon && m_offHandWeapon->canAttack()) {
        m_offHandWeapon->performAttack();
    }
}

void WeaponManager::performDualWieldAttack() {
    performMainHandAttack();

    // Off-hand attack with slight delay and reduced damage
    if (m_offHandWeapon) {
        performOffHandAttack();
    }
}

float WeaponManager::getTotalAttackSpeed() const {
    float speed = 0.0f;

    if (m_mainHandWeapon) {
        speed += m_mainHandWeapon->getAttackSpeed();
    }

    if (isDualWielding()) {
        // Dual wielding gives bonus attack speed but not full double
        speed += m_offHandWeapon->getAttackSpeed() * 0.5f;
    }

    return speed;
}

float WeaponManager::getTotalDamage() const {
    float damage = 0.0f;

    if (m_mainHandWeapon) {
        damage += (m_mainHandWeapon->getMinDamage() + m_mainHandWeapon->getMaxDamage()) / 2.0f;
    }

    if (isDualWielding()) {
        // Off-hand deals reduced damage
        damage += ((m_offHandWeapon->getMinDamage() + m_offHandWeapon->getMaxDamage()) / 2.0f) * 0.6f;
    }

    return damage;
}

float WeaponManager::getTotalCriticalChance() const {
    float critChance = 0.0f;

    if (m_mainHandWeapon) {
        critChance = m_mainHandWeapon->getCriticalChance();
    }

    if (isDualWielding()) {
        // Take average of both weapons
        critChance = (critChance + m_offHandWeapon->getCriticalChance()) / 2.0f;
    }

    return critChance;
}

float WeaponManager::getTotalRange() const {
    float range = 0.0f;

    if (m_mainHandWeapon) {
        range = m_mainHandWeapon->getRange();
    }

    // Range is determined by longest weapon
    if (m_offHandWeapon) {
        range = std::max(range, m_offHandWeapon->getRange());
    }

    return range;
}

void WeaponManager::update(float deltaTime) {
    if (m_mainHandWeapon) {
        m_mainHandWeapon->update(deltaTime);
    }

    if (m_offHandWeapon) {
        m_offHandWeapon->update(deltaTime);
    }
}

void WeaponManager::setAlternateWeaponSet(Weapon* mainHand, Weapon* offHand) {
    m_alternateMainHand = mainHand;
    m_alternateOffHand = offHand;
}

void WeaponManager::switchToAlternateSet() {
    std::swap(m_mainHandWeapon, m_alternateMainHand);
    std::swap(m_offHandWeapon, m_alternateOffHand);
    m_usingAlternateSet = !m_usingAlternateSet;
}

// ===== WeaponFactory Implementation =====

WeaponFactory& WeaponFactory::getInstance() {
    static WeaponFactory instance;
    return instance;
}

WeaponFactory::WeaponFactory() {
}

WeaponFactory::~WeaponFactory() {
}

bool WeaponFactory::loadWeapons(const std::string& jsonPath) {
    // TODO: Implement JSON parsing
    std::cout << "Loading weapons from: " << jsonPath << std::endl;
    return true;
}

Weapon* WeaponFactory::createWeapon(const std::string& weaponId) {
    auto* data = getWeaponData(weaponId);
    if (!data) return nullptr;

    return new Weapon(*data);
}

WeaponData* WeaponFactory::getWeaponData(const std::string& weaponId) {
    auto it = m_weaponDatabase.find(weaponId);
    if (it != m_weaponDatabase.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<WeaponData*> WeaponFactory::getWeaponsByType(WeaponType type) {
    std::vector<WeaponData*> weapons;
    for (auto& pair : m_weaponDatabase) {
        if (pair.second.type == type) {
            weapons.push_back(&pair.second);
        }
    }
    return weapons;
}

std::vector<WeaponData*> WeaponFactory::getWeaponsByRarity(WeaponRarity rarity) {
    std::vector<WeaponData*> weapons;
    for (auto& pair : m_weaponDatabase) {
        if (pair.second.rarity == rarity) {
            weapons.push_back(&pair.second);
        }
    }
    return weapons;
}

std::vector<WeaponData*> WeaponFactory::getWeaponsByElement(Element element) {
    std::vector<WeaponData*> weapons;
    for (auto& pair : m_weaponDatabase) {
        if (pair.second.element == element) {
            weapons.push_back(&pair.second);
        }
    }
    return weapons;
}

void WeaponFactory::registerWeapon(const WeaponData& data) {
    m_weaponDatabase[data.id] = data;
}

Weapon* WeaponFactory::generateRandomWeapon(WeaponRarity minRarity, WeaponRarity maxRarity) {
    // Filter weapons by rarity range
    std::vector<WeaponData*> validWeapons;
    for (auto& pair : m_weaponDatabase) {
        int rarity = static_cast<int>(pair.second.rarity);
        if (rarity >= static_cast<int>(minRarity) && rarity <= static_cast<int>(maxRarity)) {
            validWeapons.push_back(&pair.second);
        }
    }

    if (validWeapons.empty()) return nullptr;

    // Pick random weapon
    std::uniform_int_distribution<size_t> dist(0, validWeapons.size() - 1);
    size_t index = dist(weaponRng);

    return new Weapon(*validWeapons[index]);
}

WeaponType WeaponFactory::parseWeaponType(const std::string& str) {
    if (str == "SWORD") return WeaponType::SWORD;
    if (str == "GREATSWORD") return WeaponType::GREATSWORD;
    if (str == "DAGGER") return WeaponType::DAGGER;
    if (str == "AXE") return WeaponType::AXE;
    if (str == "MACE") return WeaponType::MACE;
    if (str == "HAMMER") return WeaponType::HAMMER;
    if (str == "SPEAR") return WeaponType::SPEAR;
    if (str == "POLEARM") return WeaponType::POLEARM;
    if (str == "KATANA") return WeaponType::KATANA;
    if (str == "SCYTHE") return WeaponType::SCYTHE;
    if (str == "BOW") return WeaponType::BOW;
    if (str == "CROSSBOW") return WeaponType::CROSSBOW;
    if (str == "GUN") return WeaponType::GUN;
    if (str == "RIFLE") return WeaponType::RIFLE;
    if (str == "STAFF") return WeaponType::STAFF;
    if (str == "WAND") return WeaponType::WAND;
    if (str == "ORB") return WeaponType::ORB;
    if (str == "TOME") return WeaponType::TOME;
    if (str == "SHIELD") return WeaponType::SHIELD;
    if (str == "FIST") return WeaponType::FIST;
    if (str == "WHIP") return WeaponType::WHIP;
    if (str == "CHAIN") return WeaponType::CHAIN;
    return WeaponType::SWORD;
}

WeaponRarity WeaponFactory::parseWeaponRarity(const std::string& str) {
    if (str == "COMMON") return WeaponRarity::COMMON;
    if (str == "UNCOMMON") return WeaponRarity::UNCOMMON;
    if (str == "RARE") return WeaponRarity::RARE;
    if (str == "EPIC") return WeaponRarity::EPIC;
    if (str == "LEGENDARY") return WeaponRarity::LEGENDARY;
    if (str == "MYTHIC") return WeaponRarity::MYTHIC;
    return WeaponRarity::COMMON;
}

AttackPattern WeaponFactory::parseAttackPattern(const std::string& str) {
    if (str == "SINGLE_HIT") return AttackPattern::SINGLE_HIT;
    if (str == "MULTI_HIT") return AttackPattern::MULTI_HIT;
    if (str == "RAPID_FIRE") return AttackPattern::RAPID_FIRE;
    if (str == "CHARGE_ATTACK") return AttackPattern::CHARGE_ATTACK;
    if (str == "SWEEP") return AttackPattern::SWEEP;
    if (str == "THRUST") return AttackPattern::THRUST;
    if (str == "OVERHEAD") return AttackPattern::OVERHEAD;
    if (str == "SPIN") return AttackPattern::SPIN;
    return AttackPattern::SINGLE_HIT;
}

// ===== WeaponBuilder Implementation =====

WeaponFactory::WeaponBuilder::WeaponBuilder(const std::string& id) {
    m_weaponData.id = id;
    m_weaponData.name = id;
    m_weaponData.type = WeaponType::SWORD;
    m_weaponData.rarity = WeaponRarity::COMMON;
    m_weaponData.minDamage = 10.0f;
    m_weaponData.maxDamage = 20.0f;
    m_weaponData.attackSpeed = 1.0f;
    m_weaponData.criticalChance = 5.0f;
    m_weaponData.criticalDamage = 150.0f;
    m_weaponData.range = 2.0f;
    m_weaponData.primaryDamageType = DamageType::PHYSICAL;
    m_weaponData.element = Element::NONE;
    m_weaponData.attackPattern = AttackPattern::SINGLE_HIT;
    m_weaponData.attackComboLength = 3;
    m_weaponData.maxDurability = 100.0f;
    m_weaponData.durabilityLossPerHit = 0.1f;
    m_weaponData.upgradeLevel = 0;
    m_weaponData.maxUpgradeLevel = 10;
    m_weaponData.socketCount = 0;
    m_weaponData.twoHanded = false;
    m_weaponData.dualWieldable = true;
    m_weaponData.ranged = false;
    m_weaponData.magical = false;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setName(const std::string& name) {
    m_weaponData.name = name;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setType(WeaponType type) {
    m_weaponData.type = type;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setRarity(WeaponRarity rarity) {
    m_weaponData.rarity = rarity;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setDamage(float min, float max) {
    m_weaponData.minDamage = min;
    m_weaponData.maxDamage = max;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setAttackSpeed(float speed) {
    m_weaponData.attackSpeed = speed;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setCritical(float chance, float damage) {
    m_weaponData.criticalChance = chance;
    m_weaponData.criticalDamage = damage;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setElement(Element element, float elementalDamage) {
    m_weaponData.element = element;
    m_weaponData.elementalDamage = elementalDamage;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setDamageType(DamageType type) {
    m_weaponData.primaryDamageType = type;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setRange(float range) {
    m_weaponData.range = range;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setTwoHanded(bool twoHanded) {
    m_weaponData.twoHanded = twoHanded;
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::addWeaponSkill(const std::string& skillId) {
    m_weaponData.weaponSkills.push_back(skillId);
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::addOnHitEffect(const std::string& effectId, float chance) {
    m_weaponData.onHitEffects.push_back(effectId);
    m_weaponData.effectChances.push_back(chance);
    return *this;
}

WeaponFactory::WeaponBuilder& WeaponFactory::WeaponBuilder::setSockets(int count) {
    m_weaponData.socketCount = count;
    return *this;
}

WeaponData WeaponFactory::WeaponBuilder::build() {
    // Set default combo multipliers if not set
    if (m_weaponData.comboDamageMultipliers.empty()) {
        for (int i = 0; i < m_weaponData.attackComboLength; ++i) {
            m_weaponData.comboDamageMultipliers.push_back(1.0f + i * 0.1f);
        }
    }

    return m_weaponData;
}

// ===== WeaponModifierSystem Implementation =====

void WeaponModifierSystem::applyModifier(Weapon* weapon, const WeaponModifier& modifier) {
    if (!weapon) return;

    // This would modify the weapon's stats
    // Implementation would depend on how modifiers are stored on weapons
}

void WeaponModifierSystem::removeModifier(Weapon* weapon, const std::string& modifierId) {
    if (!weapon) return;

    // Remove specific modifier
}

void WeaponModifierSystem::enchantWeapon(Weapon* weapon, Element element, float duration) {
    if (!weapon) return;

    // Temporarily add elemental damage
}

void WeaponModifierSystem::applyTemporaryBuff(Weapon* weapon, const WeaponModifier& buff, float duration) {
    if (!weapon) return;

    // Apply buff with timer
}

} // namespace Combat
