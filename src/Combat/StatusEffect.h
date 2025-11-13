#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "DamageCalculator.h"

namespace Combat {

// Forward declaration
class CombatEntity;

// Types of status effects
enum class StatusEffectType {
    // Damage over time
    BURN,
    POISON,
    BLEED,
    SHOCK,        // Lightning DOT
    FROSTBITE,    // Ice DOT
    CORRUPTION,   // Dark DOT

    // Control effects
    STUN,
    FREEZE,
    ROOT,         // Can't move but can attack
    SILENCE,      // Can't use abilities
    SLEEP,
    CHARM,        // Controlled by enemy
    FEAR,         // Run away uncontrollably
    SLOW,

    // Stat modifiers
    WEAKNESS,     // Reduced attack
    VULNERABILITY,// Increased damage taken
    BLIND,        // Reduced accuracy
    ARMOR_BREAK,  // Reduced defense
    MAGIC_BREAK,  // Reduced magic defense

    // Buffs
    STRENGTH,     // Increased attack
    FORTIFY,      // Increased defense
    HASTE,        // Increased attack speed
    REGENERATION, // Health over time
    BARRIER,      // Damage shield
    IMMUNITY,     // Immune to debuffs
    BERSERK,      // High damage, low defense

    // Special effects
    INVULNERABLE, // Cannot take damage
    INVISIBLE,    // Cannot be targeted
    TAUNT,        // Forces enemies to attack
    LIFESTEAL_BUFF,
    REFLECT,      // Reflects damage
    COUNTER,      // Counter attacks

    // Unique mechanics
    MARKED,       // Takes extra damage
    CURSED,       // Cannot be healed
    BLESSED,      // Gradual buff increase
    DOOM,         // Instant kill after duration
    PETRIFY,      // Turned to stone
    TRANSFORM     // Polymorphed
};

// How the effect stacks
enum class StackBehavior {
    NONE,         // Doesn't stack, refreshes duration
    INTENSITY,    // Stacks effect strength
    DURATION,     // Adds to duration
    INDEPENDENT   // Each application is separate
};

// Status effect data
struct StatusEffectData {
    std::string id;
    std::string name;
    std::string description;
    StatusEffectType type;
    Element element;

    float duration;           // In seconds
    float tickRate;           // How often it applies (for DOTs)
    int maxStacks;
    StackBehavior stackBehavior;

    // Damage/healing values
    float damagePerTick;
    float totalDamage;
    float healPerTick;

    // Stat modifiers (multipliers)
    float attackModifier;
    float defenseModifier;
    float magicAttackModifier;
    float magicDefenseModifier;
    float speedModifier;
    float accuracyModifier;
    float evasionModifier;
    float criticalChanceModifier;

    // Resistances
    ElementalResistance resistanceModifier;

    // Special properties
    bool removeOnDamage;
    bool removeOnMovement;
    bool preventActions;
    bool preventMovement;
    bool preventHealing;
    bool dispellable;
    bool isBuff;

    // Visual effects
    std::string visualEffect;
    std::string particleEffect;
    std::string soundEffect;

    // Chance to apply (for proc-based effects)
    float applicationChance;

    // Conditions
    std::vector<std::string> immunityTypes;  // Immune to these while active
    std::vector<std::string> clearsEffects;  // Removes these effects on application
};

// Active instance of a status effect on an entity
class StatusEffectInstance {
public:
    StatusEffectInstance(const StatusEffectData& data, CombatEntity* caster);
    ~StatusEffectInstance();

    // Update the effect (called each frame)
    void update(float deltaTime, CombatEntity* target);

    // Apply immediate effect
    void applyEffect(CombatEntity* target);

    // Stack management
    void addStack();
    bool canStack() const;
    int getStacks() const { return m_currentStacks; }

    // Duration management
    float getRemainingDuration() const { return m_remainingDuration; }
    void refreshDuration();
    void addDuration(float duration);
    bool isExpired() const { return m_remainingDuration <= 0.0f; }

    // Getters
    const StatusEffectData& getData() const { return m_data; }
    StatusEffectType getType() const { return m_data.type; }
    std::string getId() const { return m_data.id; }
    CombatEntity* getCaster() const { return m_caster; }

    // Check if effect is control type
    bool isControlEffect() const;
    bool isDebuff() const { return !m_data.isBuff; }
    bool isBuff() const { return m_data.isBuff; }

    // Modify strength (for stacking intensity)
    void modifyStrength(float multiplier);
    float getStrength() const { return m_strengthMultiplier; }

private:
    StatusEffectData m_data;
    CombatEntity* m_caster;

    float m_remainingDuration;
    float m_tickTimer;
    int m_currentStacks;
    float m_strengthMultiplier;

    bool m_hasAppliedInitialEffect;
};

// Manager for all status effects on an entity
class StatusEffectManager {
public:
    StatusEffectManager();
    ~StatusEffectManager();

    // Update all active effects
    void update(float deltaTime, CombatEntity* owner);

    // Apply a new status effect
    bool applyStatusEffect(const StatusEffectData& data, CombatEntity* caster, CombatEntity* target);

    // Remove effects
    void removeEffect(const std::string& effectId);
    void removeEffectByType(StatusEffectType type);
    void removeAllDebuffs();
    void removeAllBuffs();
    void clearAll();

    // Query effects
    bool hasEffect(const std::string& effectId) const;
    bool hasEffectType(StatusEffectType type) const;
    StatusEffectInstance* getEffect(const std::string& effectId);
    std::vector<StatusEffectInstance*> getEffectsByType(StatusEffectType type);
    std::vector<StatusEffectInstance*> getAllEffects();

    // Check immunities
    bool isImmuneToType(StatusEffectType type) const;
    void addImmunity(StatusEffectType type);
    void removeImmunity(StatusEffectType type);

    // Get stat modifiers from all active effects
    float getTotalAttackModifier() const;
    float getTotalDefenseModifier() const;
    float getTotalMagicAttackModifier() const;
    float getTotalMagicDefenseModifier() const;
    float getTotalSpeedModifier() const;
    float getTotalAccuracyModifier() const;
    float getTotalEvasionModifier() const;
    ElementalResistance getTotalResistanceModifier() const;

    // Check control states
    bool isStunned() const;
    bool isFrozen() const;
    bool isSilenced() const;
    bool isRooted() const;
    bool canAct() const;
    bool canMove() const;
    bool canCast() const;
    bool canBeHealed() const;

    // Count effects
    int getActiveEffectCount() const { return static_cast<int>(m_activeEffects.size()); }
    int getDebuffCount() const;
    int getBuffCount() const;
    int getControlEffectCount() const;

    // Dispel effects
    int dispelBuffs(int count);
    int dispelDebuffs(int count);

private:
    std::vector<std::shared_ptr<StatusEffectInstance>> m_activeEffects;
    std::vector<StatusEffectType> m_immunities;

    // Helper to check if effect should be blocked
    bool shouldBlockEffect(const StatusEffectData& data, CombatEntity* target);

    // Handle stacking
    void handleStacking(StatusEffectInstance* existing, const StatusEffectData& newData, CombatEntity* caster);
};

// Status effect factory - loads and creates effects from data
class StatusEffectFactory {
public:
    static StatusEffectFactory& getInstance();

    // Load status effects from JSON
    bool loadStatusEffects(const std::string& jsonPath);

    // Create effect from ID
    StatusEffectData* getStatusEffect(const std::string& effectId);

    // Get all effects of a type
    std::vector<StatusEffectData*> getEffectsByType(StatusEffectType type);

    // Register custom effect
    void registerEffect(const StatusEffectData& data);

private:
    StatusEffectFactory();
    ~StatusEffectFactory();

    std::map<std::string, StatusEffectData> m_effectDatabase;

    // Helper to parse JSON
    StatusEffectType parseEffectType(const std::string& typeStr);
    StackBehavior parseStackBehavior(const std::string& behaviorStr);
};

} // namespace Combat

#endif // STATUS_EFFECT_H
