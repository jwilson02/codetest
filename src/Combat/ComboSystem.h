#ifndef COMBO_SYSTEM_H
#define COMBO_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "DamageCalculator.h"

namespace Combat {

// Forward declarations
class CombatEntity;
class Skill;

// Combo action types
enum class ComboActionType {
    LIGHT_ATTACK,
    HEAVY_ATTACK,
    SKILL,
    DODGE,
    PARRY,
    BLOCK,
    JUMP,
    DASH
};

// Combo state
enum class ComboState {
    NONE,
    ACTIVE,
    FINISHING,
    BROKEN
};

// Combo rank/grade
enum class ComboRank {
    D,
    C,
    B,
    A,
    S,
    SS,
    SSS
};

// Individual action in a combo
struct ComboAction {
    ComboActionType type;
    std::string skillId;       // If type is SKILL
    float timestamp;           // When the action was performed
    float damage;              // Damage dealt
    bool hitLanded;
};

// Combo chain definition (predefined combo sequences)
struct ComboChainDefinition {
    std::string id;
    std::string name;
    std::string description;

    // Required sequence
    std::vector<ComboActionType> sequence;

    // Timing windows
    float maxTimeBetweenActions;
    float totalExecutionTime;

    // Requirements
    int levelRequired;
    std::string weaponTypeRequired;

    // Bonuses
    float damageMultiplier;
    float criticalChanceBonus;
    std::string finisherSkillId;  // Special skill at the end
    std::vector<std::string> appliedEffects;

    // Visual
    std::string animationOverride;
    std::string particleEffect;
    std::string soundEffect;
};

// Active combo tracker
class ComboTracker {
public:
    ComboTracker();
    ~ComboTracker();

    // Start a new combo
    void startCombo(CombatEntity* performer);

    // Add action to combo
    void addAction(ComboActionType actionType, const std::string& skillId = "", float damage = 0.0f, bool hitLanded = true);

    // End combo
    void endCombo();

    // Break combo (missed attack, got hit, etc.)
    void breakCombo();

    // Update (handles timing and decay)
    void update(float deltaTime);

    // Query combo state
    int getComboCount() const { return m_comboCount; }
    ComboState getState() const { return m_state; }
    ComboRank getRank() const;
    float getComboMultiplier() const;
    float getTotalDamage() const { return m_totalDamage; }
    float getComboTime() const { return m_comboTime; }
    bool isActive() const { return m_state == ComboState::ACTIVE; }

    // Check for combo chains
    bool checkForComboChain(const ComboChainDefinition& chain);
    std::vector<ComboChainDefinition*> getActiveChains() const;

    // Get recent actions
    const std::vector<ComboAction>& getActions() const { return m_actions; }

    // Callbacks
    void setOnComboStart(std::function<void(int)> callback) { m_onComboStart = callback; }
    void setOnComboIncrement(std::function<void(int)> callback) { m_onComboIncrement = callback; }
    void setOnComboEnd(std::function<void(int, float)> callback) { m_onComboEnd = callback; }
    void setOnComboBreak(std::function<void(int)> callback) { m_onComboBreak = callback; }
    void setOnRankUp(std::function<void(ComboRank)> callback) { m_onRankUp = callback; }

private:
    CombatEntity* m_performer;
    ComboState m_state;

    int m_comboCount;
    float m_comboTime;
    float m_timeSinceLastAction;
    float m_decayTimer;
    float m_totalDamage;

    std::vector<ComboAction> m_actions;
    ComboRank m_currentRank;

    // Callbacks
    std::function<void(int)> m_onComboStart;
    std::function<void(int)> m_onComboIncrement;
    std::function<void(int, float)> m_onComboEnd;
    std::function<void(int)> m_onComboBreak;
    std::function<void(ComboRank)> m_onRankUp;

    // Configuration
    float m_maxTimeBetweenActions;
    float m_comboDecayRate;
    float m_damageScalingFactor;
    float m_maxComboMultiplier;

    // Helper functions
    void updateRank();
    void triggerCallbacks();
};

// Combo system manager
class ComboSystem {
public:
    ComboSystem();
    ~ComboSystem();

    // Update all active combos
    void update(float deltaTime);

    // Get combo tracker for entity
    ComboTracker* getComboTracker(CombatEntity* entity);

    // Create combo tracker
    ComboTracker* createComboTracker(CombatEntity* entity);

    // Remove combo tracker
    void removeComboTracker(CombatEntity* entity);

    // Register combo chain
    void registerComboChain(const ComboChainDefinition& chain);
    ComboChainDefinition* getComboChain(const std::string& chainId);
    std::vector<ComboChainDefinition*> getAllComboChains();

    // Check if action sequence matches any chain
    std::vector<ComboChainDefinition*> checkForMatchingChains(const std::vector<ComboActionType>& sequence);

    // Configuration
    void setMaxTimeBetweenActions(float time) { m_maxTimeBetweenActions = time; }
    void setComboDecayRate(float rate) { m_comboDecayRate = rate; }
    void setDamageScalingFactor(float factor) { m_damageScalingFactor = factor; }
    void setMaxComboMultiplier(float multiplier) { m_maxComboMultiplier = multiplier; }

    // Statistics
    struct ComboStats {
        int totalCombos;
        int longestCombo;
        float highestMultiplier;
        ComboRank highestRank;
        std::map<std::string, int> completedChains;
    };

    ComboStats getStats(CombatEntity* entity);
    void resetStats(CombatEntity* entity);

private:
    std::map<CombatEntity*, ComboTracker*> m_comboTrackers;
    std::map<std::string, ComboChainDefinition> m_comboChains;
    std::map<CombatEntity*, ComboStats> m_stats;

    // Configuration
    float m_maxTimeBetweenActions;
    float m_comboDecayRate;
    float m_damageScalingFactor;
    float m_maxComboMultiplier;
};

// Combo builder for creating custom chains
class ComboChainBuilder {
public:
    ComboChainBuilder(const std::string& id);

    ComboChainBuilder& setName(const std::string& name);
    ComboChainBuilder& setDescription(const std::string& desc);
    ComboChainBuilder& addAction(ComboActionType action);
    ComboChainBuilder& setTimingWindow(float maxTime);
    ComboChainBuilder& setDamageMultiplier(float multiplier);
    ComboChainBuilder& setFinisher(const std::string& skillId);
    ComboChainBuilder& addEffect(const std::string& effectId);
    ComboChainBuilder& setRequirement(int level, const std::string& weaponType);
    ComboChainBuilder& setVisuals(const std::string& anim, const std::string& particle);

    ComboChainDefinition build();

private:
    ComboChainDefinition m_chain;
};

// Utility functions
namespace ComboUtil {
    // Convert combo count to rank
    ComboRank calculateRank(int comboCount, float totalDamage, float comboTime);

    // Calculate damage multiplier from combo
    float calculateDamageMultiplier(int comboCount, float scalingFactor, float maxMultiplier);

    // Check if combo is still valid based on timing
    bool isComboValid(float timeSinceLastAction, float maxTime);

    // Convert rank to string
    std::string rankToString(ComboRank rank);

    // Get rank color (for UI)
    struct Color { float r, g, b, a; };
    Color getRankColor(ComboRank rank);
}

} // namespace Combat

#endif // COMBO_SYSTEM_H
