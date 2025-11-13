#ifndef DAMAGE_CALCULATOR_H
#define DAMAGE_CALCULATOR_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>

namespace Combat {

// Elemental types in the game
enum class Element {
    NONE,
    FIRE,
    ICE,
    LIGHTNING,
    EARTH,
    HOLY,
    DARK,
    PHYSICAL
};

// Damage types for different attack categories
enum class DamageType {
    PHYSICAL,
    MAGICAL,
    TRUE_DAMAGE,  // Ignores defense
    PURE          // Fixed damage
};

// Resistance/weakness multipliers
struct ElementalResistance {
    float fire = 0.0f;
    float ice = 0.0f;
    float lightning = 0.0f;
    float earth = 0.0f;
    float holy = 0.0f;
    float dark = 0.0f;
    float physical = 0.0f;
};

// Stats for damage calculation
struct CombatStats {
    float attack = 100.0f;
    float defense = 50.0f;
    float magicAttack = 100.0f;
    float magicDefense = 50.0f;
    float criticalChance = 5.0f;      // Percentage
    float criticalDamage = 150.0f;    // Percentage multiplier
    float accuracy = 95.0f;            // Percentage
    float evasion = 5.0f;              // Percentage
    float armorPenetration = 0.0f;     // Flat value
    float magicPenetration = 0.0f;     // Flat value
    float lifeSteal = 0.0f;            // Percentage
    float damageReflection = 0.0f;     // Percentage

    ElementalResistance resistance;
};

// Result of a damage calculation
struct DamageResult {
    float totalDamage = 0.0f;
    float baseDamage = 0.0f;
    float modifiedDamage = 0.0f;
    bool isCritical = false;
    bool isEvaded = false;
    bool isBlocked = false;
    bool isParried = false;
    Element element = Element::NONE;
    DamageType damageType = DamageType::PHYSICAL;
    float elementalMultiplier = 1.0f;
    float defenseReduction = 0.0f;
    float lifeStealAmount = 0.0f;
    float reflectedDamage = 0.0f;

    std::vector<std::string> damageFlags;  // For special effects
};

class DamageCalculator {
public:
    DamageCalculator();
    ~DamageCalculator();

    // Main damage calculation function
    DamageResult calculateDamage(
        const CombatStats& attacker,
        const CombatStats& defender,
        float baseDamage,
        Element element,
        DamageType damageType,
        float skillMultiplier = 1.0f
    );

    // Specific damage calculations
    float calculatePhysicalDamage(
        float attack,
        float defense,
        float armorPenetration,
        float baseDamage
    );

    float calculateMagicalDamage(
        float magicAttack,
        float magicDefense,
        float magicPenetration,
        float baseDamage
    );

    // Critical hit calculation
    bool rollCritical(float criticalChance);
    float applyCriticalDamage(float damage, float criticalMultiplier);

    // Hit chance calculation
    bool rollHit(float accuracy, float evasion);

    // Elemental calculations
    float getElementalMultiplier(Element attackElement, const ElementalResistance& resistance);
    void setElementalWeakness(Element attacker, Element defender, float multiplier);
    float getWeaknessMultiplier(Element attacker, Element defender);

    // Block/Parry calculations
    float calculateBlockReduction(float blockValue, float baseDamage);
    float calculateParryMultiplier(float parrySkill);

    // Advanced mechanics
    float calculateArmorReduction(float defense, float armorPenetration);
    float calculateMagicReduction(float magicDefense, float magicPenetration);
    float calculateLifeSteal(float damage, float lifeStealPercent);
    float calculateReflectedDamage(float incomingDamage, float reflectionPercent);

    // Status effect damage modifiers
    float applyStatusModifiers(float baseDamage, const std::vector<std::string>& statusEffects);

    // Combo damage scaling
    float applyComboScaling(float baseDamage, int comboCount);

    // Level difference scaling
    float applyLevelScaling(float baseDamage, int attackerLevel, int defenderLevel);

    // Random variance
    float applyRandomVariance(float damage, float variancePercent = 10.0f);

    // Damage over time calculations
    float calculateDOTDamage(
        float baseDamage,
        float duration,
        float tickRate,
        const CombatStats& caster
    );

    // Set balancing parameters
    void setGlobalDamageMultiplier(float multiplier);
    void setCriticalCapacity(float maxCritChance, float maxCritDamage);
    void setDefenseFormula(float softCap, float hardCap);

private:
    // Balancing parameters
    float m_globalDamageMultiplier;
    float m_maxCriticalChance;
    float m_maxCriticalDamage;
    float m_defenseSoftCap;
    float m_defenseHardCap;
    float m_comboScalingFactor;
    float m_levelScalingFactor;

    // Elemental weakness/resistance chart
    std::map<Element, std::map<Element, float>> m_elementalChart;

    // Initialize elemental interactions
    void initializeElementalChart();

    // Utility functions
    float clamp(float value, float min, float max);
    float randomFloat(float min, float max);
    int randomInt(int min, int max);
};

} // namespace Combat

#endif // DAMAGE_CALCULATOR_H
