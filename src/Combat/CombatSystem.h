#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "DamageCalculator.h"
#include "StatusEffect.h"
#include "Skill.h"
#include "WeaponSystem.h"
#include "ComboSystem.h"

namespace Combat {

// Forward declarations
struct Vector2 { float x, y; };
struct Vector3 { float x, y, z; };
struct Transform { Vector3 position; float rotation; };

// Combat entity state
enum class EntityState {
    IDLE,
    ATTACKING,
    CASTING,
    DODGING,
    BLOCKING,
    PARRYING,
    STUNNED,
    KNOCKED_BACK,
    DEAD
};

// Defensive action result
enum class DefenseResult {
    NONE,
    BLOCKED,
    PARRIED,
    DODGED,
    PERFECT_DODGE  // i-frames dodge
};

// Hit result for detailed combat feedback
struct HitResult {
    bool hit;
    DamageResult damageResult;
    DefenseResult defenseResult;
    bool interrupted;
    float knockbackDistance;
    Vector2 knockbackDirection;
    float hitstun;
    bool triggeredCombo;
    std::vector<std::string> appliedEffects;
};

// Combat entity - represents any entity in combat
class CombatEntity {
public:
    CombatEntity(const std::string& id);
    virtual ~CombatEntity();

    // Update
    virtual void update(float deltaTime);

    // Core combat actions
    virtual bool attack(CombatEntity* target);
    virtual bool useSkill(const std::string& skillId, CombatEntity* target = nullptr);
    virtual bool dodge();
    virtual bool block();
    virtual bool parry();

    // Damage/healing
    virtual void takeDamage(float damage, Element element = Element::NONE, CombatEntity* attacker = nullptr);
    virtual void heal(float amount);

    // State management
    EntityState getState() const { return m_state; }
    void setState(EntityState state);
    bool canAct() const;
    bool canMove() const;

    // Stats
    CombatStats& getStats() { return m_stats; }
    const CombatStats& getStats() const { return m_stats; }

    // Health/resources
    float getHealth() const { return m_currentHealth; }
    float getMaxHealth() const { return m_maxHealth; }
    float getHealthPercent() const { return (m_currentHealth / m_maxHealth) * 100.0f; }
    void setHealth(float health);
    void setMaxHealth(float maxHealth);
    bool isAlive() const { return m_currentHealth > 0.0f; }
    virtual void die();

    float getMana() const { return m_currentMana; }
    float getMaxMana() const { return m_maxMana; }
    void setMana(float mana);
    void consumeMana(float amount);

    float getStamina() const { return m_currentStamina; }
    float getMaxStamina() const { return m_maxStamina; }
    void setStamina(float stamina);
    void consumeStamina(float amount);

    // Position/transform
    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }
    float getDistanceTo(CombatEntity* other) const;
    bool isInRange(CombatEntity* target, float range) const;

    // Managers
    StatusEffectManager* getStatusEffectManager() { return m_statusEffectManager.get(); }
    SkillManager* getSkillManager() { return m_skillManager.get(); }
    WeaponManager* getWeaponManager() { return m_weaponManager.get(); }

    // Invincibility frames (i-frames)
    void setInvincible(float duration);
    bool isInvincible() const { return m_invincibilityTime > 0.0f; }
    float getInvincibilityTime() const { return m_invincibilityTime; }

    // Hitstun/knockback
    void applyHitstun(float duration);
    void applyKnockback(Vector2 direction, float force);
    bool isInHitstun() const { return m_hitstunTime > 0.0f; }

    // Defense mechanics
    void startBlocking();
    void stopBlocking();
    bool isBlocking() const { return m_isBlocking; }
    DefenseResult checkDefense(CombatEntity* attacker);

    // Parry window
    void triggerParry();
    bool isInParryWindow() const { return m_parryWindow > 0.0f; }

    // Level/experience
    int getLevel() const { return m_level; }
    void setLevel(int level);

    // ID
    std::string getId() const { return m_id; }

    // Callbacks for combat events
    void setOnDamageTaken(std::function<void(float, CombatEntity*)> callback) { m_onDamageTaken = callback; }
    void setOnHeal(std::function<void(float)> callback) { m_onHeal = callback; }
    void setOnAttack(std::function<void(CombatEntity*)> callback) { m_onAttack = callback; }
    void setOnDeath(std::function<void()> callback) { m_onDeath = callback; }

protected:
    std::string m_id;
    EntityState m_state;
    Transform m_transform;

    // Stats
    CombatStats m_stats;
    int m_level;

    // Health/resources
    float m_currentHealth;
    float m_maxHealth;
    float m_currentMana;
    float m_maxMana;
    float m_currentStamina;
    float m_maxStamina;
    float m_healthRegenRate;
    float m_manaRegenRate;
    float m_staminaRegenRate;

    // Combat state
    float m_invincibilityTime;
    float m_hitstunTime;
    float m_parryWindow;
    bool m_isBlocking;
    float m_blockValue;

    // Managers
    std::unique_ptr<StatusEffectManager> m_statusEffectManager;
    std::unique_ptr<SkillManager> m_skillManager;
    std::unique_ptr<WeaponManager> m_weaponManager;

    // Callbacks
    std::function<void(float, CombatEntity*)> m_onDamageTaken;
    std::function<void(float)> m_onHeal;
    std::function<void(CombatEntity*)> m_onAttack;
    std::function<void()> m_onDeath;

    // Internal updates
    void updateTimers(float deltaTime);
    void updateResourceRegen(float deltaTime);
};

// Main combat system manager
class CombatSystem {
public:
    static CombatSystem& getInstance();

    // Initialize/shutdown
    void initialize();
    void shutdown();

    // Update
    void update(float deltaTime);

    // Entity management
    void registerEntity(CombatEntity* entity);
    void unregisterEntity(CombatEntity* entity);
    CombatEntity* getEntity(const std::string& id);
    std::vector<CombatEntity*> getAllEntities();
    std::vector<CombatEntity*> getEntitiesInRadius(const Vector3& center, float radius);

    // Combat execution
    HitResult executeAttack(CombatEntity* attacker, CombatEntity* target);
    HitResult executeSkill(CombatEntity* caster, const std::string& skillId, CombatEntity* target = nullptr);
    bool executeDodge(CombatEntity* entity);
    bool executeBlock(CombatEntity* entity);
    bool executeParry(CombatEntity* entity);

    // Damage dealing
    HitResult dealDamage(
        CombatEntity* attacker,
        CombatEntity* target,
        float baseDamage,
        Element element,
        DamageType damageType,
        float skillMultiplier = 1.0f
    );

    // Damage calculation
    DamageCalculator* getDamageCalculator() { return &m_damageCalculator; }

    // Combo system
    ComboSystem* getComboSystem() { return &m_comboSystem; }

    // Hit detection
    bool checkHit(CombatEntity* attacker, CombatEntity* target, float range);
    std::vector<CombatEntity*> getTargetsInArea(const Vector3& center, float radius, CombatEntity* ignore = nullptr);
    std::vector<CombatEntity*> getTargetsInCone(const Vector3& origin, Vector3 direction, float angle, float range, CombatEntity* ignore = nullptr);
    std::vector<CombatEntity*> getTargetsInLine(const Vector3& start, Vector3 end, float width, CombatEntity* ignore = nullptr);

    // Collision/physics
    bool hasLineOfSight(CombatEntity* from, CombatEntity* to);
    void applyKnockback(CombatEntity* entity, Vector2 direction, float force);

    // Team/faction system
    void setEntityTeam(CombatEntity* entity, int teamId);
    int getEntityTeam(CombatEntity* entity);
    bool areEnemies(CombatEntity* a, CombatEntity* b);
    bool areAllies(CombatEntity* a, CombatEntity* b);

    // Combat settings
    void setGlobalDamageMultiplier(float multiplier);
    void setInvincibilityFrameDuration(float duration);
    void setHitstunEnabled(bool enabled);
    void setKnockbackEnabled(bool enabled);

    // Statistics
    struct CombatStats {
        int totalDamageDealt;
        int totalDamageTaken;
        int totalKills;
        int totalDeaths;
        int totalHitsLanded;
        int totalHitsMissed;
        int totalCriticalHits;
        int totalDodges;
        int totalBlocks;
        int totalParries;
    };

    CombatStats getEntityStats(CombatEntity* entity);
    void resetEntityStats(CombatEntity* entity);

    // Events
    void subscribeToAttackEvent(std::function<void(CombatEntity*, CombatEntity*, HitResult)> callback);
    void subscribeToDamageEvent(std::function<void(CombatEntity*, CombatEntity*, float)> callback);
    void subscribeToDeathEvent(std::function<void(CombatEntity*, CombatEntity*)> callback);

private:
    CombatSystem();
    ~CombatSystem();

    // Components
    DamageCalculator m_damageCalculator;
    ComboSystem m_comboSystem;

    // Entity tracking
    std::map<std::string, CombatEntity*> m_entities;
    std::map<CombatEntity*, int> m_entityTeams;
    std::map<CombatEntity*, CombatStats> m_entityStats;

    // Settings
    float m_globalDamageMultiplier;
    float m_defaultIFrameDuration;
    bool m_hitstunEnabled;
    bool m_knockbackEnabled;

    // Event callbacks
    std::vector<std::function<void(CombatEntity*, CombatEntity*, HitResult)>> m_attackCallbacks;
    std::vector<std::function<void(CombatEntity*, CombatEntity*, float)>> m_damageCallbacks;
    std::vector<std::function<void(CombatEntity*, CombatEntity*)>> m_deathCallbacks;

    // Helper functions
    void processHitResult(CombatEntity* attacker, CombatEntity* target, const HitResult& result);
    void updateStats(CombatEntity* attacker, CombatEntity* target, const HitResult& result);
};

} // namespace Combat

#endif // COMBAT_SYSTEM_H
