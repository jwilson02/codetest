#include "CharacterStats.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace RPG {

CharacterStats::CharacterStats()
    : m_availableStatPoints(0)
    , m_totalPointsSpent(0)
    , m_onStatChange(nullptr)
    , m_onDerivedStatChange(nullptr)
{
    // Initialize base stats to starting value
    for (int i = static_cast<int>(PrimaryStat::STRENGTH);
         i <= static_cast<int>(PrimaryStat::LUCK); ++i) {
        PrimaryStat stat = static_cast<PrimaryStat>(i);
        m_baseStats[stat] = STARTING_STAT_VALUE;
        m_finalStats[stat] = STARTING_STAT_VALUE;
    }
}

CharacterStats::~CharacterStats() {
}

void CharacterStats::initialize(int startingPoints) {
    m_availableStatPoints = startingPoints;
    m_totalPointsSpent = 0;
    recalculateAllStats();
}

void CharacterStats::setBaseStats(int str, int dex, int intelligence, int vit, int end, int luck) {
    m_baseStats[PrimaryStat::STRENGTH] = str;
    m_baseStats[PrimaryStat::DEXTERITY] = dex;
    m_baseStats[PrimaryStat::INTELLIGENCE] = intelligence;
    m_baseStats[PrimaryStat::VITALITY] = vit;
    m_baseStats[PrimaryStat::ENDURANCE] = end;
    m_baseStats[PrimaryStat::LUCK] = luck;
    recalculateAllStats();
}

int CharacterStats::getBaseStat(PrimaryStat stat) const {
    auto it = m_baseStats.find(stat);
    return (it != m_baseStats.end()) ? it->second : 0;
}

int CharacterStats::getFinalStat(PrimaryStat stat) const {
    auto it = m_finalStats.find(stat);
    return (it != m_finalStats.end()) ? it->second : 0;
}

bool CharacterStats::allocateStatPoint(PrimaryStat stat, int points) {
    if (points <= 0 || points > m_availableStatPoints) {
        return false;
    }

    int currentValue = m_baseStats[stat];
    if (currentValue + points > MAX_STAT_VALUE) {
        return false;
    }

    int oldValue = currentValue;
    m_baseStats[stat] = currentValue + points;
    m_availableStatPoints -= points;
    m_totalPointsSpent += points;

    calculateFinalStat(stat);
    recalculateAllStats();

    if (m_onStatChange) {
        m_onStatChange(stat, oldValue, m_baseStats[stat]);
    }

    return true;
}

void CharacterStats::resetStatPoints() {
    // Return all allocated points
    m_availableStatPoints = m_totalPointsSpent;
    m_totalPointsSpent = 0;

    // Reset all stats to starting value
    for (auto& pair : m_baseStats) {
        pair.second = STARTING_STAT_VALUE;
    }

    recalculateAllStats();
}

void CharacterStats::addStatModifier(PrimaryStat stat, const StatModifier& modifier) {
    m_statModifiers[stat].push_back(modifier);
    calculateFinalStat(stat);
    recalculateAllStats();
}

void CharacterStats::removeStatModifier(PrimaryStat stat, const std::string& source) {
    auto& modifiers = m_statModifiers[stat];
    modifiers.erase(
        std::remove_if(modifiers.begin(), modifiers.end(),
            [&source](const StatModifier& mod) { return mod.source == source; }),
        modifiers.end()
    );
    calculateFinalStat(stat);
    recalculateAllStats();
}

void CharacterStats::removeAllModifiersFromSource(const std::string& source) {
    for (auto& pair : m_statModifiers) {
        removeStatModifier(pair.first, source);
    }
    for (auto& pair : m_derivedStatModifiers) {
        removeDerivedStatModifier(pair.first, source);
    }
}

void CharacterStats::updateModifiers(float deltaTime) {
    bool needsRecalc = false;

    // Update primary stat modifiers
    for (auto& pair : m_statModifiers) {
        auto& modifiers = pair.second;
        for (auto it = modifiers.begin(); it != modifiers.end();) {
            if (!it->isPermanent()) {
                it->duration -= static_cast<int>(deltaTime);
                if (it->duration <= 0) {
                    it = modifiers.erase(it);
                    needsRecalc = true;
                    continue;
                }
            }
            ++it;
        }
    }

    // Update derived stat modifiers
    for (auto& pair : m_derivedStatModifiers) {
        auto& modifiers = pair.second;
        for (auto it = modifiers.begin(); it != modifiers.end();) {
            if (!it->isPermanent()) {
                it->duration -= static_cast<int>(deltaTime);
                if (it->duration <= 0) {
                    it = modifiers.erase(it);
                    needsRecalc = true;
                    continue;
                }
            }
            ++it;
        }
    }

    if (needsRecalc) {
        recalculateAllStats();
    }
}

std::vector<CharacterStats::StatModifier> CharacterStats::getActiveModifiers(PrimaryStat stat) const {
    auto it = m_statModifiers.find(stat);
    return (it != m_statModifiers.end()) ? it->second : std::vector<StatModifier>();
}

void CharacterStats::addDerivedStatModifier(DerivedStat stat, const StatModifier& modifier) {
    m_derivedStatModifiers[stat].push_back(modifier);
    recalculateAllStats();
}

void CharacterStats::removeDerivedStatModifier(DerivedStat stat, const std::string& source) {
    auto& modifiers = m_derivedStatModifiers[stat];
    modifiers.erase(
        std::remove_if(modifiers.begin(), modifiers.end(),
            [&source](const StatModifier& mod) { return mod.source == source; }),
        modifiers.end()
    );
    recalculateAllStats();
}

float CharacterStats::getDerivedStat(DerivedStat stat) const {
    auto it = m_derivedStats.find(stat);
    if (it != m_derivedStats.end()) {
        return it->second;
    }
    return calculateDerivedStat(stat);
}

void CharacterStats::calculateFinalStat(PrimaryStat stat) {
    int baseValue = m_baseStats[stat];
    float finalValue = applyModifiers(stat, static_cast<float>(baseValue));
    m_finalStats[stat] = static_cast<int>(std::round(finalValue));
}

float CharacterStats::applyModifiers(PrimaryStat stat, float baseValue) const {
    float additive = 0.0f;
    float multiplicative = 1.0f;

    auto it = m_statModifiers.find(stat);
    if (it != m_statModifiers.end()) {
        for (const auto& mod : it->second) {
            additive += mod.additive;
            multiplicative *= (1.0f + mod.multiplicative);
        }
    }

    return (baseValue + additive) * multiplicative;
}

float CharacterStats::applyDerivedModifiers(DerivedStat stat, float baseValue) const {
    float additive = 0.0f;
    float multiplicative = 1.0f;

    auto it = m_derivedStatModifiers.find(stat);
    if (it != m_derivedStatModifiers.end()) {
        for (const auto& mod : it->second) {
            additive += mod.additive;
            multiplicative *= (1.0f + mod.multiplicative);
        }
    }

    return (baseValue + additive) * multiplicative;
}

void CharacterStats::recalculateAllStats() {
    // Recalculate final primary stats
    for (auto& pair : m_baseStats) {
        calculateFinalStat(pair.first);
    }

    // Recalculate all derived stats
    for (int i = static_cast<int>(DerivedStat::MAX_HEALTH);
         i <= static_cast<int>(DerivedStat::EXPERIENCE_GAIN); ++i) {
        DerivedStat stat = static_cast<DerivedStat>(i);
        float oldValue = m_derivedStats[stat];
        float newValue = calculateDerivedStat(stat);
        m_derivedStats[stat] = newValue;

        if (m_onDerivedStatChange && oldValue != newValue) {
            m_onDerivedStatChange(stat, oldValue, newValue);
        }
    }
}

float CharacterStats::calculateDerivedStat(DerivedStat stat) const {
    float baseValue = 0.0f;

    switch (stat) {
        case DerivedStat::MAX_HEALTH: baseValue = calculateMaxHealth(); break;
        case DerivedStat::MAX_MANA: baseValue = calculateMaxMana(); break;
        case DerivedStat::MAX_STAMINA: baseValue = calculateMaxStamina(); break;
        case DerivedStat::HEALTH_REGEN: baseValue = calculateHealthRegen(); break;
        case DerivedStat::MANA_REGEN: baseValue = calculateManaRegen(); break;
        case DerivedStat::STAMINA_REGEN: baseValue = calculateStaminaRegen(); break;
        case DerivedStat::PHYSICAL_DAMAGE: baseValue = calculatePhysicalDamage(); break;
        case DerivedStat::MAGIC_DAMAGE: baseValue = calculateMagicDamage(); break;
        case DerivedStat::ATTACK_SPEED: baseValue = calculateAttackSpeed(); break;
        case DerivedStat::CAST_SPEED: baseValue = calculateCastSpeed(); break;
        case DerivedStat::CRITICAL_CHANCE: baseValue = calculateCriticalChance(); break;
        case DerivedStat::CRITICAL_DAMAGE: baseValue = calculateCriticalDamage(); break;
        case DerivedStat::ARMOR: baseValue = calculateArmor(); break;
        case DerivedStat::MAGIC_RESIST: baseValue = calculateMagicResist(); break;
        case DerivedStat::DODGE_CHANCE: baseValue = calculateDodgeChance(); break;
        case DerivedStat::BLOCK_CHANCE: baseValue = calculateBlockChance(); break;
        case DerivedStat::MOVEMENT_SPEED: baseValue = calculateMovementSpeed(); break;
        case DerivedStat::CARRY_WEIGHT: baseValue = calculateCarryWeight(); break;
        case DerivedStat::LOOT_FIND: baseValue = calculateLootFind(); break;
        case DerivedStat::EXPERIENCE_GAIN: baseValue = calculateExperienceGain(); break;
    }

    return applyDerivedModifiers(stat, baseValue);
}

// Derived stat calculation formulas
float CharacterStats::calculateMaxHealth() const {
    int vit = getFinalStat(PrimaryStat::VITALITY);
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    return 100.0f + (vit * 10.0f) + (end * 5.0f);
}

float CharacterStats::calculateMaxMana() const {
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    return 100.0f + (intelligence * 10.0f);
}

float CharacterStats::calculateMaxStamina() const {
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    int str = getFinalStat(PrimaryStat::STRENGTH);
    return 100.0f + (end * 8.0f) + (str * 2.0f);
}

float CharacterStats::calculateHealthRegen() const {
    int vit = getFinalStat(PrimaryStat::VITALITY);
    return 1.0f + (vit * 0.1f);
}

float CharacterStats::calculateManaRegen() const {
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    return 1.0f + (intelligence * 0.1f);
}

float CharacterStats::calculateStaminaRegen() const {
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    return 2.0f + (end * 0.15f);
}

float CharacterStats::calculatePhysicalDamage() const {
    int str = getFinalStat(PrimaryStat::STRENGTH);
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    return 10.0f + (str * 2.0f) + (dex * 0.5f);
}

float CharacterStats::calculateMagicDamage() const {
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    return 10.0f + (intelligence * 2.5f);
}

float CharacterStats::calculateAttackSpeed() const {
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    return 1.0f + (dex * 0.01f); // 1% per dex point
}

float CharacterStats::calculateCastSpeed() const {
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    return 1.0f + (dex * 0.005f) + (intelligence * 0.005f);
}

float CharacterStats::calculateCriticalChance() const {
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    int luck = getFinalStat(PrimaryStat::LUCK);
    return 5.0f + (dex * 0.2f) + (luck * 0.3f); // Percentage
}

float CharacterStats::calculateCriticalDamage() const {
    int str = getFinalStat(PrimaryStat::STRENGTH);
    int luck = getFinalStat(PrimaryStat::LUCK);
    return 150.0f + (str * 0.5f) + (luck * 1.0f); // Percentage
}

float CharacterStats::calculateArmor() const {
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    int str = getFinalStat(PrimaryStat::STRENGTH);
    return 10.0f + (end * 3.0f) + (str * 0.5f);
}

float CharacterStats::calculateMagicResist() const {
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    int vit = getFinalStat(PrimaryStat::VITALITY);
    return 10.0f + (intelligence * 1.5f) + (vit * 1.0f);
}

float CharacterStats::calculateDodgeChance() const {
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    int luck = getFinalStat(PrimaryStat::LUCK);
    return 5.0f + (dex * 0.3f) + (luck * 0.1f); // Percentage
}

float CharacterStats::calculateBlockChance() const {
    int str = getFinalStat(PrimaryStat::STRENGTH);
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    return 5.0f + (str * 0.1f) + (end * 0.2f); // Percentage
}

float CharacterStats::calculateMovementSpeed() const {
    int dex = getFinalStat(PrimaryStat::DEXTERITY);
    return 100.0f + (dex * 0.5f); // Percentage of base speed
}

float CharacterStats::calculateCarryWeight() const {
    int str = getFinalStat(PrimaryStat::STRENGTH);
    int end = getFinalStat(PrimaryStat::ENDURANCE);
    return 50.0f + (str * 5.0f) + (end * 2.0f);
}

float CharacterStats::calculateLootFind() const {
    int luck = getFinalStat(PrimaryStat::LUCK);
    return luck * 0.5f; // 0.5% per luck point
}

float CharacterStats::calculateExperienceGain() const {
    int intelligence = getFinalStat(PrimaryStat::INTELLIGENCE);
    return 100.0f + (intelligence * 0.2f); // Percentage
}

bool CharacterStats::meetsRequirements(const std::vector<StatRequirement>& requirements) const {
    for (const auto& req : requirements) {
        if (getFinalStat(req.stat) < req.value) {
            return false;
        }
    }
    return true;
}

void CharacterStats::applyStatPreset(const std::string& presetName) {
    auto presets = getStatPresets();
    auto it = presets.find(presetName);
    if (it != presets.end()) {
        for (const auto& pair : it->second) {
            m_baseStats[pair.first] = pair.second;
        }
        recalculateAllStats();
    }
}

std::map<std::string, std::map<CharacterStats::PrimaryStat, int>> CharacterStats::getStatPresets() {
    std::map<std::string, std::map<PrimaryStat, int>> presets;

    // Balanced
    presets["Balanced"] = {
        {PrimaryStat::STRENGTH, 15},
        {PrimaryStat::DEXTERITY, 15},
        {PrimaryStat::INTELLIGENCE, 15},
        {PrimaryStat::VITALITY, 15},
        {PrimaryStat::ENDURANCE, 15},
        {PrimaryStat::LUCK, 15}
    };

    // Warrior
    presets["Warrior"] = {
        {PrimaryStat::STRENGTH, 25},
        {PrimaryStat::DEXTERITY, 10},
        {PrimaryStat::INTELLIGENCE, 5},
        {PrimaryStat::VITALITY, 20},
        {PrimaryStat::ENDURANCE, 25},
        {PrimaryStat::LUCK, 5}
    };

    // Mage
    presets["Mage"] = {
        {PrimaryStat::STRENGTH, 5},
        {PrimaryStat::DEXTERITY, 10},
        {PrimaryStat::INTELLIGENCE, 30},
        {PrimaryStat::VITALITY, 15},
        {PrimaryStat::ENDURANCE, 10},
        {PrimaryStat::LUCK, 20}
    };

    // Ranger
    presets["Ranger"] = {
        {PrimaryStat::STRENGTH, 10},
        {PrimaryStat::DEXTERITY, 30},
        {PrimaryStat::INTELLIGENCE, 10},
        {PrimaryStat::VITALITY, 15},
        {PrimaryStat::ENDURANCE, 15},
        {PrimaryStat::LUCK, 10}
    };

    // Rogue
    presets["Rogue"] = {
        {PrimaryStat::STRENGTH, 10},
        {PrimaryStat::DEXTERITY, 25},
        {PrimaryStat::INTELLIGENCE, 10},
        {PrimaryStat::VITALITY, 10},
        {PrimaryStat::ENDURANCE, 15},
        {PrimaryStat::LUCK, 20}
    };

    // Paladin
    presets["Paladin"] = {
        {PrimaryStat::STRENGTH, 20},
        {PrimaryStat::DEXTERITY, 10},
        {PrimaryStat::INTELLIGENCE, 15},
        {PrimaryStat::VITALITY, 20},
        {PrimaryStat::ENDURANCE, 20},
        {PrimaryStat::LUCK, 5}
    };

    return presets;
}

std::string CharacterStats::getStatName(PrimaryStat stat) {
    switch (stat) {
        case PrimaryStat::STRENGTH: return "Strength";
        case PrimaryStat::DEXTERITY: return "Dexterity";
        case PrimaryStat::INTELLIGENCE: return "Intelligence";
        case PrimaryStat::VITALITY: return "Vitality";
        case PrimaryStat::ENDURANCE: return "Endurance";
        case PrimaryStat::LUCK: return "Luck";
        default: return "Unknown";
    }
}

std::string CharacterStats::getDerivedStatName(DerivedStat stat) {
    switch (stat) {
        case DerivedStat::MAX_HEALTH: return "Max Health";
        case DerivedStat::MAX_MANA: return "Max Mana";
        case DerivedStat::MAX_STAMINA: return "Max Stamina";
        case DerivedStat::HEALTH_REGEN: return "Health Regen";
        case DerivedStat::MANA_REGEN: return "Mana Regen";
        case DerivedStat::STAMINA_REGEN: return "Stamina Regen";
        case DerivedStat::PHYSICAL_DAMAGE: return "Physical Damage";
        case DerivedStat::MAGIC_DAMAGE: return "Magic Damage";
        case DerivedStat::ATTACK_SPEED: return "Attack Speed";
        case DerivedStat::CAST_SPEED: return "Cast Speed";
        case DerivedStat::CRITICAL_CHANCE: return "Critical Chance";
        case DerivedStat::CRITICAL_DAMAGE: return "Critical Damage";
        case DerivedStat::ARMOR: return "Armor";
        case DerivedStat::MAGIC_RESIST: return "Magic Resist";
        case DerivedStat::DODGE_CHANCE: return "Dodge Chance";
        case DerivedStat::BLOCK_CHANCE: return "Block Chance";
        case DerivedStat::MOVEMENT_SPEED: return "Movement Speed";
        case DerivedStat::CARRY_WEIGHT: return "Carry Weight";
        case DerivedStat::LOOT_FIND: return "Loot Find";
        case DerivedStat::EXPERIENCE_GAIN: return "Experience Gain";
        default: return "Unknown";
    }
}

std::string CharacterStats::getStatDescription(PrimaryStat stat) {
    switch (stat) {
        case PrimaryStat::STRENGTH:
            return "Increases physical damage, carry weight, and melee critical damage";
        case PrimaryStat::DEXTERITY:
            return "Increases attack speed, dodge chance, and critical chance";
        case PrimaryStat::INTELLIGENCE:
            return "Increases magic damage, mana pool, and experience gain";
        case PrimaryStat::VITALITY:
            return "Increases health, health regeneration, and resistances";
        case PrimaryStat::ENDURANCE:
            return "Increases stamina, armor, and stamina regeneration";
        case PrimaryStat::LUCK:
            return "Increases critical damage, loot quality, and rare item drops";
        default:
            return "Unknown stat";
    }
}

std::string CharacterStats::getDerivedStatDescription(DerivedStat stat) {
    // Implementation similar to above
    return getDerivedStatName(stat);
}

int CharacterStats::getTotalStatsAllocated() const {
    return m_totalPointsSpent;
}

std::map<CharacterStats::PrimaryStat, int> CharacterStats::getAllBaseStats() const {
    return m_baseStats;
}

std::map<CharacterStats::DerivedStat, float> CharacterStats::getAllDerivedStats() const {
    return m_derivedStats;
}

std::string CharacterStats::serialize() const {
    std::stringstream ss;
    ss << m_availableStatPoints << "," << m_totalPointsSpent << ",";
    for (const auto& pair : m_baseStats) {
        ss << static_cast<int>(pair.first) << ":" << pair.second << ";";
    }
    return ss.str();
}

bool CharacterStats::deserialize(const std::string& data) {
    // Implementation for deserialization
    return true;
}

} // namespace RPG
