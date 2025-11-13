#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "DamageCalculator.h"
#include "StatusEffect.h"

namespace Combat {

// Forward declarations
class CombatEntity;
class CombatSystem;

// Skill categories
enum class SkillCategory {
    OFFENSIVE,
    DEFENSIVE,
    SUPPORT,
    CROWD_CONTROL,
    MOVEMENT,
    ULTIMATE
};

// Skill targeting types
enum class TargetType {
    SELF,
    SINGLE_ENEMY,
    SINGLE_ALLY,
    ALL_ENEMIES,
    ALL_ALLIES,
    AREA,
    LINE,
    CONE,
    CHAIN,        // Bounces between targets
    GROUND,       // Place at location
    DIRECTION     // Fires in a direction
};

// Resource cost types
enum class ResourceType {
    MANA,
    STAMINA,
    RAGE,
    ENERGY,
    HEALTH,
    COMBO_POINTS,
    NONE
};

// Skill animation/cast types
enum class CastType {
    INSTANT,
    CAST_TIME,
    CHANNELED
};

// Skill data definition
struct SkillData {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;

    SkillCategory category;
    TargetType targetType;
    CastType castType;

    // Requirements
    int levelRequired;
    std::vector<std::string> prerequisiteSkills;
    std::string weaponTypeRequired;

    // Resource costs
    ResourceType resourceType;
    float resourceCost;
    float healthCost;

    // Cooldown
    float cooldown;
    int maxCharges;  // For skills with multiple charges

    // Casting
    float castTime;
    float channelDuration;
    bool canMoveWhileCasting;
    bool interruptible;

    // Range and area
    float range;
    float areaOfEffect;
    float width;         // For line/cone skills
    int maxTargets;
    int chainCount;      // For chain skills

    // Damage properties
    float baseDamage;
    float damageScaling;     // Scales with attack/magic attack
    DamageType damageType;
    Element element;
    bool canCrit;
    float critBonus;         // Additional crit chance for this skill

    // Status effects to apply
    std::vector<std::string> statusEffectsToApply;
    std::vector<float> statusEffectChances;

    // Healing properties
    float baseHealing;
    float healingScaling;

    // Shield/barrier properties
    float shieldAmount;
    float shieldDuration;

    // Movement
    float dashDistance;
    float movementSpeed;
    bool teleport;

    // Special properties
    bool piercing;           // Hits through targets
    bool lifeSteal;
    float lifeStealAmount;
    bool areaOfDenial;       // Creates persistent area
    float areaOfDenialDuration;
    bool summon;
    std::string summonId;
    int summonCount;

    // Combo properties
    std::string comboFollowup; // Next skill in combo chain
    float comboWindow;         // Time to use followup
    bool comboCancelable;      // Can be cancelled into other skills

    // Visual/audio
    std::string animationName;
    std::string castEffect;
    std::string projectileEffect;
    std::string hitEffect;
    std::string soundEffect;

    // Skill tree
    int tier;                  // Skill tier/level
    int maxLevel;
    std::vector<std::string> upgrades;

    // Tags for special interactions
    std::vector<std::string> tags;
};

// Active skill instance (when a skill is being used)
class SkillInstance {
public:
    SkillInstance(const SkillData& data, CombatEntity* caster);
    ~SkillInstance();

    // Execute the skill
    bool execute(CombatSystem* combatSystem, CombatEntity* target = nullptr);

    // Update for channeled skills
    void update(float deltaTime);

    // Interrupt casting
    void interrupt();

    // Getters
    const SkillData& getData() const { return m_data; }
    CombatEntity* getCaster() const { return m_caster; }
    bool isChanneling() const { return m_isChanneling; }
    bool isCasting() const { return m_isCasting; }
    float getCastProgress() const { return m_castProgress; }

private:
    SkillData m_data;
    CombatEntity* m_caster;
    CombatEntity* m_target;

    bool m_isCasting;
    bool m_isChanneling;
    float m_castProgress;
    float m_channelProgress;

    // Execute damage
    void executeDamage(CombatSystem* combatSystem, CombatEntity* target);

    // Execute healing
    void executeHealing(CombatEntity* target);

    // Apply status effects
    void applyStatusEffects(CombatEntity* target);

    // Execute movement
    void executeMovement();

    // Execute summon
    void executeSummon(CombatSystem* combatSystem);
};

// Skill manager for an entity
class SkillManager {
public:
    SkillManager();
    ~SkillManager();

    // Update active skills and cooldowns
    void update(float deltaTime);

    // Learn/unlock skills
    bool learnSkill(const std::string& skillId);
    bool unlockSkill(const std::string& skillId);
    bool upgradeSkill(const std::string& skillId);

    // Use skills
    bool canUseSkill(const std::string& skillId, CombatEntity* caster);
    SkillInstance* useSkill(const std::string& skillId, CombatEntity* caster, CombatEntity* target = nullptr);

    // Cooldown management
    bool isOnCooldown(const std::string& skillId) const;
    float getCooldownRemaining(const std::string& skillId) const;
    void setCooldown(const std::string& skillId, float duration);
    void reduceCooldown(const std::string& skillId, float reduction);
    void resetCooldown(const std::string& skillId);
    void resetAllCooldowns();

    // Charge management (for multi-charge skills)
    int getCharges(const std::string& skillId) const;
    void addCharge(const std::string& skillId);

    // Skill queries
    bool hasSkill(const std::string& skillId) const;
    SkillData* getSkill(const std::string& skillId);
    std::vector<SkillData*> getAllSkills();
    std::vector<SkillData*> getSkillsByCategory(SkillCategory category);

    // Combo system integration
    void setLastUsedSkill(const std::string& skillId);
    std::string getLastUsedSkill() const { return m_lastUsedSkill; }
    bool canComboInto(const std::string& skillId) const;

    // Resource checking
    bool hasResourcesForSkill(const std::string& skillId, CombatEntity* caster);

    // Skill levels
    int getSkillLevel(const std::string& skillId) const;
    void setSkillLevel(const std::string& skillId, int level);

private:
    // Learned skills
    std::map<std::string, SkillData> m_learnedSkills;

    // Cooldown tracking
    struct CooldownInfo {
        float remainingTime;
        int currentCharges;
        float chargeRegenTime;
    };
    std::map<std::string, CooldownInfo> m_cooldowns;

    // Skill levels
    std::map<std::string, int> m_skillLevels;

    // Active instances
    std::vector<std::shared_ptr<SkillInstance>> m_activeSkills;

    // Combo tracking
    std::string m_lastUsedSkill;
    float m_comboWindow;

    // Helper functions
    void startCooldown(const std::string& skillId, float duration, int maxCharges);
    void updateCooldowns(float deltaTime);
    void updateCharges(float deltaTime);
};

// Skill factory - loads and creates skills from data
class SkillFactory {
public:
    static SkillFactory& getInstance();

    // Load skills from JSON
    bool loadSkills(const std::string& jsonPath);

    // Get skill data
    SkillData* getSkill(const std::string& skillId);

    // Get skills by category
    std::vector<SkillData*> getSkillsByCategory(SkillCategory category);

    // Get skills by element
    std::vector<SkillData*> getSkillsByElement(Element element);

    // Register custom skill
    void registerSkill(const SkillData& data);

    // Skill builder for runtime creation
    class SkillBuilder {
    public:
        SkillBuilder(const std::string& id);

        SkillBuilder& setName(const std::string& name);
        SkillBuilder& setDescription(const std::string& desc);
        SkillBuilder& setCategory(SkillCategory category);
        SkillBuilder& setTargetType(TargetType targetType);
        SkillBuilder& setCastType(CastType castType);
        SkillBuilder& setDamage(float baseDamage, float scaling, DamageType type);
        SkillBuilder& setElement(Element element);
        SkillBuilder& setCooldown(float cooldown);
        SkillBuilder& setRange(float range);
        SkillBuilder& setResourceCost(ResourceType type, float cost);
        SkillBuilder& addStatusEffect(const std::string& effectId, float chance);
        SkillBuilder& setAnimation(const std::string& animName);

        SkillData build();

    private:
        SkillData m_skillData;
    };

private:
    SkillFactory();
    ~SkillFactory();

    std::map<std::string, SkillData> m_skillDatabase;

    // Helper parsers
    SkillCategory parseCategory(const std::string& str);
    TargetType parseTargetType(const std::string& str);
    CastType parseCastType(const std::string& str);
    ResourceType parseResourceType(const std::string& str);
};

} // namespace Combat

#endif // SKILL_H
