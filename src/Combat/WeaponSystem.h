#ifndef WEAPON_SYSTEM_H
#define WEAPON_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include "DamageCalculator.h"

namespace Combat {

// Weapon types
enum class WeaponType {
    // Melee
    SWORD,
    GREATSWORD,
    DAGGER,
    AXE,
    MACE,
    HAMMER,
    SPEAR,
    POLEARM,
    KATANA,
    SCYTHE,

    // Ranged
    BOW,
    CROSSBOW,
    GUN,
    RIFLE,

    // Magic
    STAFF,
    WAND,
    ORB,
    TOME,

    // Defensive
    SHIELD,

    // Special
    FIST,          // Unarmed/gauntlets
    DUAL_WIELD,
    WHIP,
    CHAIN
};

// Weapon rarity
enum class WeaponRarity {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY,
    MYTHIC
};

// Attack patterns
enum class AttackPattern {
    SINGLE_HIT,
    MULTI_HIT,
    RAPID_FIRE,
    CHARGE_ATTACK,
    SWEEP,         // Hits multiple enemies
    THRUST,        // Linear attack
    OVERHEAD,      // Slam attack
    SPIN           // 360 degree attack
};

// Weapon properties
struct WeaponData {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;
    std::string modelPath;

    WeaponType type;
    WeaponRarity rarity;

    // Base stats
    float minDamage;
    float maxDamage;
    float attackSpeed;      // Attacks per second
    float criticalChance;
    float criticalDamage;
    float range;
    float knockback;

    // Damage type distribution
    DamageType primaryDamageType;
    Element element;
    float elementalDamage;  // Percentage of damage as elemental

    // Special stats
    float armorPenetration;
    float magicPenetration;
    float lifeSteal;
    float attackSpeedMultiplier;

    // Stat requirements
    int strengthRequired;
    int dexterityRequired;
    int intelligenceRequired;
    int levelRequired;

    // Attack pattern
    AttackPattern attackPattern;
    int attackComboLength;   // Number of attacks in combo chain
    std::vector<float> comboTimings;    // Timing for each attack in combo
    std::vector<float> comboDamageMultipliers; // Damage multiplier per combo hit

    // Weapon-specific skills
    std::vector<std::string> weaponSkills;

    // Status effects on hit
    std::vector<std::string> onHitEffects;
    std::vector<float> effectChances;

    // Durability
    float maxDurability;
    float durabilityLossPerHit;

    // Upgrade
    int upgradeLevel;
    int maxUpgradeLevel;

    // Sockets
    int socketCount;
    std::vector<std::string> socketedGems;

    // Visual/audio
    std::string attackAnimation;
    std::string swingSound;
    std::string hitSound;
    std::string particleEffect;

    // Special properties
    bool twoHanded;
    bool dualWieldable;
    bool ranged;
    bool magical;
    std::vector<std::string> specialTags;
};

// Active weapon instance
class Weapon {
public:
    Weapon();
    Weapon(const WeaponData& data);
    ~Weapon();

    // Attack execution
    float calculateDamage(const CombatStats& wielderStats);
    bool canAttack() const;
    void performAttack();

    // Combo system
    void nextComboStep();
    void resetCombo();
    int getCurrentComboStep() const { return m_currentComboStep; }
    float getCurrentComboDamageMultiplier() const;

    // Durability
    void reduceDurability(float amount = 1.0f);
    void repair(float amount);
    bool isBroken() const { return m_currentDurability <= 0.0f; }
    float getDurabilityPercent() const;

    // Upgrade
    bool upgrade();
    bool canUpgrade() const;

    // Sockets
    bool addGem(const std::string& gemId);
    bool removeGem(int socketIndex);
    std::vector<std::string> getSocketedGems() const { return m_data.socketedGems; }

    // Stats
    float getMinDamage() const;
    float getMaxDamage() const;
    float getAttackSpeed() const;
    float getCriticalChance() const;
    float getCriticalDamage() const;
    float getRange() const;

    // Getters
    const WeaponData& getData() const { return m_data; }
    WeaponType getType() const { return m_data.type; }
    std::string getId() const { return m_data.id; }
    std::string getName() const { return m_data.name; }

    // Update
    void update(float deltaTime);

private:
    WeaponData m_data;
    float m_currentDurability;
    int m_currentComboStep;
    float m_attackCooldown;
    float m_comboTimer;

    // Calculate final stats with upgrades and gems
    void recalculateStats();
};

// Manages equipped weapons for an entity
class WeaponManager {
public:
    WeaponManager();
    ~WeaponManager();

    // Equip/unequip
    bool equipWeapon(Weapon* weapon, bool mainHand = true);
    void unequipMainHand();
    void unequipOffHand();
    void swapWeapons();

    // Get equipped weapons
    Weapon* getMainHandWeapon() { return m_mainHandWeapon; }
    Weapon* getOffHandWeapon() { return m_offHandWeapon; }
    bool hasMainHandWeapon() const { return m_mainHandWeapon != nullptr; }
    bool hasOffHandWeapon() const { return m_offHandWeapon != nullptr; }
    bool isDualWielding() const { return m_mainHandWeapon && m_offHandWeapon; }

    // Attack with current weapon
    bool canAttack() const;
    void performMainHandAttack();
    void performOffHandAttack();
    void performDualWieldAttack();

    // Get combined weapon stats
    float getTotalAttackSpeed() const;
    float getTotalDamage() const;
    float getTotalCriticalChance() const;
    float getTotalRange() const;

    // Update
    void update(float deltaTime);

    // Weapon switching/hotswap
    void setAlternateWeaponSet(Weapon* mainHand, Weapon* offHand);
    void switchToAlternateSet();
    bool hasAlternateSet() const { return m_alternateMainHand != nullptr; }

private:
    Weapon* m_mainHandWeapon;
    Weapon* m_offHandWeapon;

    // Alternate weapon set for quick switching
    Weapon* m_alternateMainHand;
    Weapon* m_alternateOffHand;

    bool m_usingAlternateSet;
};

// Weapon factory
class WeaponFactory {
public:
    static WeaponFactory& getInstance();

    // Load weapons from data
    bool loadWeapons(const std::string& jsonPath);

    // Create weapon
    Weapon* createWeapon(const std::string& weaponId);
    WeaponData* getWeaponData(const std::string& weaponId);

    // Get weapons by type
    std::vector<WeaponData*> getWeaponsByType(WeaponType type);
    std::vector<WeaponData*> getWeaponsByRarity(WeaponRarity rarity);
    std::vector<WeaponData*> getWeaponsByElement(Element element);

    // Register weapon
    void registerWeapon(const WeaponData& data);

    // Generate random weapon
    Weapon* generateRandomWeapon(WeaponRarity minRarity, WeaponRarity maxRarity);

    // Weapon builder
    class WeaponBuilder {
    public:
        WeaponBuilder(const std::string& id);

        WeaponBuilder& setName(const std::string& name);
        WeaponBuilder& setType(WeaponType type);
        WeaponBuilder& setRarity(WeaponRarity rarity);
        WeaponBuilder& setDamage(float min, float max);
        WeaponBuilder& setAttackSpeed(float speed);
        WeaponBuilder& setCritical(float chance, float damage);
        WeaponBuilder& setElement(Element element, float elementalDamage);
        WeaponBuilder& setDamageType(DamageType type);
        WeaponBuilder& setRange(float range);
        WeaponBuilder& setTwoHanded(bool twoHanded);
        WeaponBuilder& addWeaponSkill(const std::string& skillId);
        WeaponBuilder& addOnHitEffect(const std::string& effectId, float chance);
        WeaponBuilder& setSockets(int count);

        WeaponData build();

    private:
        WeaponData m_weaponData;
    };

private:
    WeaponFactory();
    ~WeaponFactory();

    std::map<std::string, WeaponData> m_weaponDatabase;

    // Helpers
    WeaponType parseWeaponType(const std::string& str);
    WeaponRarity parseWeaponRarity(const std::string& str);
    AttackPattern parseAttackPattern(const std::string& str);
};

// Weapon modifier/enchantment system
struct WeaponModifier {
    std::string id;
    std::string name;
    std::string description;

    // Stat bonuses
    float damageMultiplier;
    float attackSpeedBonus;
    float critChanceBonus;
    float critDamageBonus;
    float rangeBonus;

    // Special effects
    std::vector<std::string> addedEffects;
    Element addedElement;
    float elementalDamageBonus;

    // Visual
    std::string particleEffect;
    std::string glowColor;
};

class WeaponModifierSystem {
public:
    // Apply modifier to weapon
    static void applyModifier(Weapon* weapon, const WeaponModifier& modifier);
    static void removeModifier(Weapon* weapon, const std::string& modifierId);

    // Enchant weapon with element
    static void enchantWeapon(Weapon* weapon, Element element, float duration);

    // Temporary buffs
    static void applyTemporaryBuff(Weapon* weapon, const WeaponModifier& buff, float duration);
};

} // namespace Combat

#endif // WEAPON_SYSTEM_H
