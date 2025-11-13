#include "DamageCalculator.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>

namespace Combat {

static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));

DamageCalculator::DamageCalculator()
    : m_globalDamageMultiplier(1.0f)
    , m_maxCriticalChance(100.0f)
    , m_maxCriticalDamage(300.0f)
    , m_defenseSoftCap(200.0f)
    , m_defenseHardCap(500.0f)
    , m_comboScalingFactor(0.15f)
    , m_levelScalingFactor(0.05f)
{
    initializeElementalChart();
}

DamageCalculator::~DamageCalculator() {
}

void DamageCalculator::initializeElementalChart() {
    // Fire interactions
    m_elementalChart[Element::FIRE][Element::ICE] = 1.5f;      // Fire strong vs Ice
    m_elementalChart[Element::FIRE][Element::EARTH] = 1.25f;   // Fire strong vs Earth
    m_elementalChart[Element::FIRE][Element::FIRE] = 0.5f;     // Fire weak vs Fire
    m_elementalChart[Element::FIRE][Element::LIGHTNING] = 1.0f;
    m_elementalChart[Element::FIRE][Element::HOLY] = 1.0f;
    m_elementalChart[Element::FIRE][Element::DARK] = 1.0f;

    // Ice interactions
    m_elementalChart[Element::ICE][Element::FIRE] = 0.5f;      // Ice weak vs Fire
    m_elementalChart[Element::ICE][Element::EARTH] = 1.25f;    // Ice strong vs Earth
    m_elementalChart[Element::ICE][Element::LIGHTNING] = 1.25f; // Ice strong vs Lightning
    m_elementalChart[Element::ICE][Element::ICE] = 0.5f;       // Ice weak vs Ice
    m_elementalChart[Element::ICE][Element::HOLY] = 1.0f;
    m_elementalChart[Element::ICE][Element::DARK] = 1.0f;

    // Lightning interactions
    m_elementalChart[Element::LIGHTNING][Element::ICE] = 0.75f;
    m_elementalChart[Element::LIGHTNING][Element::EARTH] = 0.5f; // Lightning weak vs Earth
    m_elementalChart[Element::LIGHTNING][Element::FIRE] = 1.25f;
    m_elementalChart[Element::LIGHTNING][Element::LIGHTNING] = 0.5f;
    m_elementalChart[Element::LIGHTNING][Element::HOLY] = 1.0f;
    m_elementalChart[Element::LIGHTNING][Element::DARK] = 1.0f;

    // Earth interactions
    m_elementalChart[Element::EARTH][Element::FIRE] = 0.75f;
    m_elementalChart[Element::EARTH][Element::LIGHTNING] = 1.5f; // Earth strong vs Lightning
    m_elementalChart[Element::EARTH][Element::ICE] = 0.75f;
    m_elementalChart[Element::EARTH][Element::EARTH] = 0.5f;
    m_elementalChart[Element::EARTH][Element::HOLY] = 1.0f;
    m_elementalChart[Element::EARTH][Element::DARK] = 1.0f;

    // Holy interactions
    m_elementalChart[Element::HOLY][Element::DARK] = 1.5f;     // Holy strong vs Dark
    m_elementalChart[Element::HOLY][Element::HOLY] = 0.5f;
    m_elementalChart[Element::HOLY][Element::FIRE] = 1.0f;
    m_elementalChart[Element::HOLY][Element::ICE] = 1.0f;
    m_elementalChart[Element::HOLY][Element::LIGHTNING] = 1.0f;
    m_elementalChart[Element::HOLY][Element::EARTH] = 1.0f;

    // Dark interactions
    m_elementalChart[Element::DARK][Element::HOLY] = 1.5f;     // Dark strong vs Holy
    m_elementalChart[Element::DARK][Element::DARK] = 0.5f;
    m_elementalChart[Element::DARK][Element::FIRE] = 1.0f;
    m_elementalChart[Element::DARK][Element::ICE] = 1.0f;
    m_elementalChart[Element::DARK][Element::LIGHTNING] = 1.0f;
    m_elementalChart[Element::DARK][Element::EARTH] = 1.0f;
}

DamageResult DamageCalculator::calculateDamage(
    const CombatStats& attacker,
    const CombatStats& defender,
    float baseDamage,
    Element element,
    DamageType damageType,
    float skillMultiplier
) {
    DamageResult result;
    result.baseDamage = baseDamage;
    result.element = element;
    result.damageType = damageType;

    // Check if attack hits
    result.isEvaded = !rollHit(attacker.accuracy, defender.evasion);
    if (result.isEvaded) {
        result.damageFlags.push_back("EVADED");
        return result;
    }

    float damage = baseDamage;

    // Apply skill multiplier
    damage *= skillMultiplier;

    // Calculate base damage based on type
    if (damageType == DamageType::PHYSICAL) {
        damage = calculatePhysicalDamage(
            attacker.attack,
            defender.defense,
            attacker.armorPenetration,
            damage
        );
    }
    else if (damageType == DamageType::MAGICAL) {
        damage = calculateMagicalDamage(
            attacker.magicAttack,
            defender.magicDefense,
            attacker.magicPenetration,
            damage
        );
    }
    else if (damageType == DamageType::TRUE_DAMAGE) {
        // True damage ignores defense but still uses attack stat
        damage *= (attacker.attack / 100.0f);
    }
    // PURE damage type stays as is

    result.modifiedDamage = damage;

    // Apply elemental multiplier
    if (element != Element::NONE) {
        result.elementalMultiplier = getElementalMultiplier(element, defender.resistance);
        damage *= result.elementalMultiplier;

        if (result.elementalMultiplier > 1.0f) {
            result.damageFlags.push_back("SUPER_EFFECTIVE");
        }
        else if (result.elementalMultiplier < 1.0f) {
            result.damageFlags.push_back("RESISTED");
        }
    }

    // Check for critical hit
    result.isCritical = rollCritical(attacker.criticalChance);
    if (result.isCritical) {
        damage = applyCriticalDamage(damage, attacker.criticalDamage);
        result.damageFlags.push_back("CRITICAL");
    }

    // Apply random variance for natural feel
    damage = applyRandomVariance(damage, 5.0f);

    // Apply global damage multiplier
    damage *= m_globalDamageMultiplier;

    // Calculate life steal
    if (attacker.lifeSteal > 0.0f) {
        result.lifeStealAmount = calculateLifeSteal(damage, attacker.lifeSteal);
    }

    // Calculate reflected damage
    if (defender.damageReflection > 0.0f) {
        result.reflectedDamage = calculateReflectedDamage(damage, defender.damageReflection);
    }

    // Ensure damage is not negative
    result.totalDamage = std::max(0.0f, damage);

    return result;
}

float DamageCalculator::calculatePhysicalDamage(
    float attack,
    float defense,
    float armorPenetration,
    float baseDamage
) {
    float effectiveDefense = calculateArmorReduction(defense, armorPenetration);

    // Damage formula: BaseDamage * (Attack / 100) * (100 / (100 + EffectiveDefense))
    float attackMultiplier = attack / 100.0f;
    float defenseReduction = 100.0f / (100.0f + effectiveDefense);

    return baseDamage * attackMultiplier * defenseReduction;
}

float DamageCalculator::calculateMagicalDamage(
    float magicAttack,
    float magicDefense,
    float magicPenetration,
    float baseDamage
) {
    float effectiveMagicDefense = calculateMagicReduction(magicDefense, magicPenetration);

    // Similar formula to physical damage
    float magicAttackMultiplier = magicAttack / 100.0f;
    float magicDefenseReduction = 100.0f / (100.0f + effectiveMagicDefense);

    return baseDamage * magicAttackMultiplier * magicDefenseReduction;
}

bool DamageCalculator::rollCritical(float criticalChance) {
    float clampedChance = clamp(criticalChance, 0.0f, m_maxCriticalChance);
    float roll = randomFloat(0.0f, 100.0f);
    return roll < clampedChance;
}

float DamageCalculator::applyCriticalDamage(float damage, float criticalMultiplier) {
    float clampedMultiplier = clamp(criticalMultiplier, 100.0f, m_maxCriticalDamage);
    return damage * (clampedMultiplier / 100.0f);
}

bool DamageCalculator::rollHit(float accuracy, float evasion) {
    float hitChance = clamp(accuracy - evasion, 5.0f, 100.0f); // Min 5%, Max 100%
    float roll = randomFloat(0.0f, 100.0f);
    return roll < hitChance;
}

float DamageCalculator::getElementalMultiplier(Element attackElement, const ElementalResistance& resistance) {
    float baseResistance = 0.0f;

    switch (attackElement) {
        case Element::FIRE:
            baseResistance = resistance.fire;
            break;
        case Element::ICE:
            baseResistance = resistance.ice;
            break;
        case Element::LIGHTNING:
            baseResistance = resistance.lightning;
            break;
        case Element::EARTH:
            baseResistance = resistance.earth;
            break;
        case Element::HOLY:
            baseResistance = resistance.holy;
            break;
        case Element::DARK:
            baseResistance = resistance.dark;
            break;
        case Element::PHYSICAL:
            baseResistance = resistance.physical;
            break;
        default:
            baseResistance = 0.0f;
    }

    // Resistance formula: 1 - (resistance / 100)
    // Positive resistance reduces damage, negative increases it
    return clamp(1.0f - (baseResistance / 100.0f), 0.1f, 2.0f);
}

void DamageCalculator::setElementalWeakness(Element attacker, Element defender, float multiplier) {
    m_elementalChart[attacker][defender] = multiplier;
}

float DamageCalculator::getWeaknessMultiplier(Element attacker, Element defender) {
    if (m_elementalChart.find(attacker) != m_elementalChart.end()) {
        if (m_elementalChart[attacker].find(defender) != m_elementalChart[attacker].end()) {
            return m_elementalChart[attacker][defender];
        }
    }
    return 1.0f;
}

float DamageCalculator::calculateBlockReduction(float blockValue, float baseDamage) {
    // Block reduces damage by a percentage based on block value
    float blockPercentage = clamp(blockValue / 10.0f, 0.0f, 80.0f); // Max 80% reduction
    return baseDamage * (1.0f - (blockPercentage / 100.0f));
}

float DamageCalculator::calculateParryMultiplier(float parrySkill) {
    // Successful parry reduces damage and can even counter
    float reduction = clamp(parrySkill / 5.0f, 50.0f, 90.0f); // 50-90% reduction
    return 1.0f - (reduction / 100.0f);
}

float DamageCalculator::calculateArmorReduction(float defense, float armorPenetration) {
    float effectiveArmor = std::max(0.0f, defense - armorPenetration);

    // Apply soft cap
    if (effectiveArmor > m_defenseSoftCap) {
        float excess = effectiveArmor - m_defenseSoftCap;
        effectiveArmor = m_defenseSoftCap + (excess * 0.5f);
    }

    // Apply hard cap
    effectiveArmor = std::min(effectiveArmor, m_defenseHardCap);

    return effectiveArmor;
}

float DamageCalculator::calculateMagicReduction(float magicDefense, float magicPenetration) {
    float effectiveMagicDefense = std::max(0.0f, magicDefense - magicPenetration);

    // Apply soft cap
    if (effectiveMagicDefense > m_defenseSoftCap) {
        float excess = effectiveMagicDefense - m_defenseSoftCap;
        effectiveMagicDefense = m_defenseSoftCap + (excess * 0.5f);
    }

    // Apply hard cap
    effectiveMagicDefense = std::min(effectiveMagicDefense, m_defenseHardCap);

    return effectiveMagicDefense;
}

float DamageCalculator::calculateLifeSteal(float damage, float lifeStealPercent) {
    return damage * (lifeStealPercent / 100.0f);
}

float DamageCalculator::calculateReflectedDamage(float incomingDamage, float reflectionPercent) {
    return incomingDamage * (reflectionPercent / 100.0f);
}

float DamageCalculator::applyStatusModifiers(float baseDamage, const std::vector<std::string>& statusEffects) {
    float multiplier = 1.0f;

    for (const auto& status : statusEffects) {
        if (status == "WEAKENED") {
            multiplier *= 0.7f; // 30% damage reduction
        }
        else if (status == "STRENGTHENED") {
            multiplier *= 1.3f; // 30% damage increase
        }
        else if (status == "BERSERK") {
            multiplier *= 1.5f; // 50% damage increase
        }
        else if (status == "FRAGILE") {
            multiplier *= 1.25f; // 25% more damage taken
        }
    }

    return baseDamage * multiplier;
}

float DamageCalculator::applyComboScaling(float baseDamage, int comboCount) {
    if (comboCount <= 1) {
        return baseDamage;
    }

    // Each combo hit increases damage by scaling factor
    float comboBonus = 1.0f + (comboCount - 1) * m_comboScalingFactor;
    comboBonus = std::min(comboBonus, 3.0f); // Cap at 300% damage

    return baseDamage * comboBonus;
}

float DamageCalculator::applyLevelScaling(float baseDamage, int attackerLevel, int defenderLevel) {
    int levelDifference = attackerLevel - defenderLevel;

    // +/- 5% damage per level difference
    float scalingMultiplier = 1.0f + (levelDifference * m_levelScalingFactor);

    // Clamp between 0.5x and 2.0x
    scalingMultiplier = clamp(scalingMultiplier, 0.5f, 2.0f);

    return baseDamage * scalingMultiplier;
}

float DamageCalculator::applyRandomVariance(float damage, float variancePercent) {
    float variance = variancePercent / 100.0f;
    float minMultiplier = 1.0f - variance;
    float maxMultiplier = 1.0f + variance;

    float randomMultiplier = randomFloat(minMultiplier, maxMultiplier);
    return damage * randomMultiplier;
}

float DamageCalculator::calculateDOTDamage(
    float baseDamage,
    float duration,
    float tickRate,
    const CombatStats& caster
) {
    int totalTicks = static_cast<int>(duration / tickRate);
    float damagePerTick = baseDamage / totalTicks;

    // Scale with magic attack for most DOTs
    damagePerTick *= (caster.magicAttack / 100.0f);

    return damagePerTick;
}

void DamageCalculator::setGlobalDamageMultiplier(float multiplier) {
    m_globalDamageMultiplier = multiplier;
}

void DamageCalculator::setCriticalCapacity(float maxCritChance, float maxCritDamage) {
    m_maxCriticalChance = maxCritChance;
    m_maxCriticalDamage = maxCritDamage;
}

void DamageCalculator::setDefenseFormula(float softCap, float hardCap) {
    m_defenseSoftCap = softCap;
    m_defenseHardCap = hardCap;
}

float DamageCalculator::clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}

float DamageCalculator::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int DamageCalculator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

} // namespace Combat
