#ifndef CHARACTER_STATS_H
#define CHARACTER_STATS_H

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace RPG {

/**
 * @brief Manages all character statistics and attributes
 *
 * Handles primary stats, derived stats, stat modifiers, and attribute point allocation
 * Supports dynamic stat calculation with buffs, debuffs, and equipment bonuses
 */
class CharacterStats {
public:
    // Primary Stats (player can allocate points to these)
    enum class PrimaryStat {
        STRENGTH,      // Affects: Physical damage, carry weight, melee crit
        DEXTERITY,     // Affects: Attack speed, dodge, ranged damage, crit chance
        INTELLIGENCE,  // Affects: Magic damage, mana pool, spell crit
        VITALITY,      // Affects: Health, health regen, resistances
        ENDURANCE,     // Affects: Stamina, stamina regen, armor
        LUCK           // Affects: Crit damage, loot quality, rare drops
    };

    // Derived Stats (calculated from primary stats and modifiers)
    enum class DerivedStat {
        MAX_HEALTH,
        MAX_MANA,
        MAX_STAMINA,
        HEALTH_REGEN,
        MANA_REGEN,
        STAMINA_REGEN,
        PHYSICAL_DAMAGE,
        MAGIC_DAMAGE,
        ATTACK_SPEED,
        CAST_SPEED,
        CRITICAL_CHANCE,
        CRITICAL_DAMAGE,
        ARMOR,
        MAGIC_RESIST,
        DODGE_CHANCE,
        BLOCK_CHANCE,
        MOVEMENT_SPEED,
        CARRY_WEIGHT,
        LOOT_FIND,
        EXPERIENCE_GAIN
    };

    struct StatModifier {
        std::string source;
        float additive;      // Flat bonus (e.g., +10 Strength)
        float multiplicative; // Percentage bonus (e.g., +25% Strength)
        int duration;        // -1 for permanent, >0 for temporary (in seconds)
        bool isPermanent() const { return duration == -1; }
    };

    using StatChangeCallback = std::function<void(PrimaryStat stat, int oldValue, int newValue)>;
    using DerivedStatChangeCallback = std::function<void(DerivedStat stat, float oldValue, float newValue)>;

    CharacterStats();
    ~CharacterStats();

    // Initialization
    void initialize(int startingPoints = 0);
    void setBaseStats(int str, int dex, int intelligence, int vit, int end, int luck);

    // Primary Stat Management
    int getBaseStat(PrimaryStat stat) const;
    int getFinalStat(PrimaryStat stat) const; // Base + all modifiers
    bool allocateStatPoint(PrimaryStat stat, int points = 1);
    bool canAllocateStatPoint() const { return m_availableStatPoints > 0; }
    int getAvailableStatPoints() const { return m_availableStatPoints; }
    void addStatPoints(int points) { m_availableStatPoints += points; }
    void resetStatPoints(); // Returns all allocated points

    // Stat Modifiers
    void addStatModifier(PrimaryStat stat, const StatModifier& modifier);
    void removeStatModifier(PrimaryStat stat, const std::string& source);
    void removeAllModifiersFromSource(const std::string& source);
    void updateModifiers(float deltaTime); // Update temporary modifiers
    std::vector<StatModifier> getActiveModifiers(PrimaryStat stat) const;

    // Derived Stats
    float getDerivedStat(DerivedStat stat) const;
    void addDerivedStatModifier(DerivedStat stat, const StatModifier& modifier);
    void removeDerivedStatModifier(DerivedStat stat, const std::string& source);

    // Stat Calculations
    void recalculateAllStats();
    float calculateDerivedStat(DerivedStat stat) const;

    // Stat Requirements (for equipment, skills, etc.)
    struct StatRequirement {
        PrimaryStat stat;
        int value;
    };
    bool meetsRequirements(const std::vector<StatRequirement>& requirements) const;

    // Callbacks
    void setStatChangeCallback(StatChangeCallback callback) { m_onStatChange = callback; }
    void setDerivedStatChangeCallback(DerivedStatChangeCallback callback) { m_onDerivedStatChange = callback; }

    // Stat Presets (for different character builds)
    void applyStatPreset(const std::string& presetName);
    static std::map<std::string, std::map<PrimaryStat, int>> getStatPresets();

    // Stat Conversion Names
    static std::string getStatName(PrimaryStat stat);
    static std::string getDerivedStatName(DerivedStat stat);
    static std::string getStatDescription(PrimaryStat stat);
    static std::string getDerivedStatDescription(DerivedStat stat);

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Statistics
    int getTotalStatsAllocated() const;
    std::map<PrimaryStat, int> getAllBaseStats() const;
    std::map<DerivedStat, float> getAllDerivedStats() const;

private:
    void calculateFinalStat(PrimaryStat stat);
    float applyModifiers(PrimaryStat stat, float baseValue) const;
    float applyDerivedModifiers(DerivedStat stat, float baseValue) const;

    // Derived stat calculation formulas
    float calculateMaxHealth() const;
    float calculateMaxMana() const;
    float calculateMaxStamina() const;
    float calculateHealthRegen() const;
    float calculateManaRegen() const;
    float calculateStaminaRegen() const;
    float calculatePhysicalDamage() const;
    float calculateMagicDamage() const;
    float calculateAttackSpeed() const;
    float calculateCastSpeed() const;
    float calculateCriticalChance() const;
    float calculateCriticalDamage() const;
    float calculateArmor() const;
    float calculateMagicResist() const;
    float calculateDodgeChance() const;
    float calculateBlockChance() const;
    float calculateMovementSpeed() const;
    float calculateCarryWeight() const;
    float calculateLootFind() const;
    float calculateExperienceGain() const;

    // Primary stats
    std::map<PrimaryStat, int> m_baseStats;
    std::map<PrimaryStat, int> m_finalStats;
    std::map<PrimaryStat, std::vector<StatModifier>> m_statModifiers;
    int m_availableStatPoints;
    int m_totalPointsSpent;

    // Derived stats
    mutable std::map<DerivedStat, float> m_derivedStats;
    std::map<DerivedStat, std::vector<StatModifier>> m_derivedStatModifiers;

    // Callbacks
    StatChangeCallback m_onStatChange;
    DerivedStatChangeCallback m_onDerivedStatChange;

    // Constants
    static constexpr int STARTING_STAT_VALUE = 10;
    static constexpr int MAX_STAT_VALUE = 999;
};

} // namespace RPG

#endif // CHARACTER_STATS_H
