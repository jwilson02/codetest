#ifndef SKILL_TREE_H
#define SKILL_TREE_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include "CharacterStats.h"

namespace RPG {

/**
 * @brief Manages skill trees, skill unlocking, and talent progression
 *
 * Supports multiple skill trees per class with dependencies, synergies,
 * and complex unlock conditions. Includes talent/perk system for endgame customization
 */
class SkillTree {
public:
    // Skill tiers
    enum class SkillTier {
        BASIC,      // Available from start
        ADVANCED,   // Unlocked at level 10
        EXPERT,     // Unlocked at level 25
        MASTER,     // Unlocked at level 50
        LEGENDARY,  // Unlocked at level 80
        TRANSCENDENT // Unlocked at level 100 or prestige
    };

    // Skill types
    enum class SkillType {
        PASSIVE,    // Always active
        ACTIVE,     // Manually activated
        TOGGLE,     // Can be turned on/off
        ULTIMATE    // Powerful active skill with cooldown
    };

    struct SkillEffect {
        std::string effectType; // "damage", "heal", "buff", "debuff", "summon", etc.
        std::map<std::string, float> values; // Effect parameters
        float duration;
        float cooldown;
    };

    struct Skill {
        std::string id;
        std::string name;
        std::string description;
        std::string treeName; // Which skill tree this belongs to
        SkillTier tier;
        SkillType type;
        int maxRank;
        int currentRank;

        // Requirements
        std::vector<std::string> prerequisiteSkills; // Must have these skills
        std::vector<CharacterStats::StatRequirement> statRequirements;
        int levelRequirement;
        int skillPointCost; // Cost per rank

        // Effects scale with rank
        std::vector<SkillEffect> effects;

        // Synergies with other skills
        std::map<std::string, float> synergies; // skillId -> bonus multiplier

        // Visual/UI data
        int posX, posY; // Position in skill tree UI
        std::string iconPath;

        bool isUnlocked;
        bool isMaxRank() const { return currentRank >= maxRank; }
    };

    struct Talent {
        std::string id;
        std::string name;
        std::string description;
        int maxRank;
        int currentRank;
        int talentPointCost;

        // Effects can modify stats, skills, or game mechanics
        std::map<std::string, float> statModifiers;
        std::map<std::string, float> skillModifiers; // Modifies specific skills
        std::vector<std::string> unlocks; // Special abilities or mechanics

        bool isUnlocked;
        bool isMaxRank() const { return currentRank >= maxRank; }
    };

    using SkillUnlockCallback = std::function<void(const Skill& skill)>;
    using SkillRankUpCallback = std::function<void(const Skill& skill, int newRank)>;
    using TalentUnlockCallback = std::function<void(const Talent& talent)>;

    SkillTree();
    ~SkillTree();

    // Initialization
    bool loadSkillTrees(const std::string& filepath);
    void initialize();

    // Skill Management
    bool unlockSkill(const std::string& skillId);
    bool rankUpSkill(const std::string& skillId, int ranks = 1);
    bool canUnlockSkill(const std::string& skillId) const;
    bool canRankUpSkill(const std::string& skillId) const;
    const Skill* getSkill(const std::string& skillId) const;
    Skill* getSkillMutable(const std::string& skillId);

    // Skill Points
    int getAvailableSkillPoints() const { return m_availableSkillPoints; }
    void addSkillPoints(int points) { m_availableSkillPoints += points; }
    int getTotalSkillPointsSpent() const;
    int getSkillPointsSpentInTree(const std::string& treeName) const;

    // Skill Tree queries
    std::vector<Skill*> getSkillsInTree(const std::string& treeName);
    std::vector<const Skill*> getSkillsInTree(const std::string& treeName) const;
    std::vector<Skill*> getUnlockedSkills();
    std::vector<Skill*> getAvailableSkills(); // Can be unlocked right now
    std::vector<std::string> getAllTreeNames() const;

    // Skill effects and synergies
    std::vector<SkillEffect> getActiveEffects() const;
    float calculateSkillDamage(const std::string& skillId) const;
    float calculateSynergies(const std::string& skillId) const;

    // Talent System
    bool unlockTalent(const std::string& talentId);
    bool rankUpTalent(const std::string& talentId, int ranks = 1);
    bool canUnlockTalent(const std::string& talentId) const;
    const Talent* getTalent(const std::string& talentId) const;

    // Talent Points
    int getAvailableTalentPoints() const { return m_availableTalentPoints; }
    void addTalentPoints(int points) { m_availableTalentPoints += points; }
    int getTotalTalentPointsSpent() const;

    // Talent queries
    std::vector<const Talent*> getUnlockedTalents() const;
    std::vector<const Talent*> getAvailableTalents() const;
    std::map<std::string, float> getAllTalentStatModifiers() const;

    // Reset/Respec
    void resetSkills(); // Refund all skill points
    void resetTalents(); // Refund all talent points
    void resetSkillTree(const std::string& treeName); // Reset specific tree
    int calculateRespecCost() const; // Cost in gold/currency

    // Skill tree validation
    bool validateSkillTreeIntegrity() const;
    std::vector<std::string> getUnmetPrerequisites(const std::string& skillId) const;

    // Callbacks
    void setSkillUnlockCallback(SkillUnlockCallback callback) { m_onSkillUnlock = callback; }
    void setSkillRankUpCallback(SkillRankUpCallback callback) { m_onSkillRankUp = callback; }
    void setTalentUnlockCallback(TalentUnlockCallback callback) { m_onTalentUnlock = callback; }

    // Statistics
    int getTotalSkillsUnlocked() const;
    int getTotalTalentsUnlocked() const;
    std::map<std::string, int> getSkillRankDistribution() const;

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Helper for building skill trees programmatically
    void addSkill(const Skill& skill);
    void addTalent(const Talent& talent);
    void createSkillTree(const std::string& treeName);

private:
    bool meetsRequirements(const Skill& skill) const;
    bool hasPrerequisites(const Skill& skill) const;
    void generateDefaultSkillTrees();
    void generateDefaultTalents();

    // Skill data
    std::map<std::string, Skill> m_skills;
    std::map<std::string, Talent> m_talents;
    std::set<std::string> m_treeNames;

    // Points
    int m_availableSkillPoints;
    int m_availableTalentPoints;

    // Callbacks
    SkillUnlockCallback m_onSkillUnlock;
    SkillRankUpCallback m_onSkillRankUp;
    TalentUnlockCallback m_onTalentUnlock;

    // Constants
    static constexpr int MAX_SKILLS_PER_TREE = 30;
    static constexpr int MAX_TALENT_RANKS = 5;
};

} // namespace RPG

#endif // SKILL_TREE_H
