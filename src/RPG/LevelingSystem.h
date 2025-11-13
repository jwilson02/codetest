#ifndef LEVELING_SYSTEM_H
#define LEVELING_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace RPG {

/**
 * @brief Manages character leveling, XP gain, and progression
 *
 * This system handles XP accumulation, level-ups, and rewards
 * Supports exponential XP curves for balanced progression
 * Max level: 100
 */
class LevelingSystem {
public:
    struct LevelReward {
        int attributePoints;
        int skillPoints;
        int talentPoints;
        std::vector<std::string> unlocks; // Special abilities, items, or features
    };

    struct LevelData {
        int level;
        unsigned long long xpRequired;
        unsigned long long totalXp;
        LevelReward rewards;
    };

    // Callback types
    using LevelUpCallback = std::function<void(int newLevel, const LevelReward& rewards)>;
    using XPGainCallback = std::function<void(unsigned long long xpGained, unsigned long long totalXp)>;

    LevelingSystem();
    ~LevelingSystem();

    // Core leveling functions
    bool loadLevelCurve(const std::string& filepath);
    void initialize(int startLevel = 1);

    // XP Management
    void gainXP(unsigned long long amount);
    void setXP(unsigned long long amount);
    unsigned long long getCurrentXP() const { return m_currentXP; }
    unsigned long long getXPForNextLevel() const;
    unsigned long long getXPToNextLevel() const;
    float getProgressToNextLevel() const;

    // Level Management
    int getCurrentLevel() const { return m_currentLevel; }
    int getMaxLevel() const { return m_maxLevel; }
    bool canLevelUp() const;
    bool levelUp();
    void setLevel(int level); // For debugging/admin

    // Level Data
    const LevelData* getLevelData(int level) const;
    const LevelData* getCurrentLevelData() const;
    const LevelData* getNextLevelData() const;

    // Rewards
    LevelReward getPendingRewards() const { return m_pendingRewards; }
    void claimRewards();
    bool hasPendingRewards() const;

    // Callbacks
    void setLevelUpCallback(LevelUpCallback callback) { m_onLevelUp = callback; }
    void setXPGainCallback(XPGainCallback callback) { m_onXPGain = callback; }

    // XP Sources (for tracking and achievements)
    void addXPSource(const std::string& source, unsigned long long amount);
    unsigned long long getXPFromSource(const std::string& source) const;
    std::map<std::string, unsigned long long> getAllXPSources() const { return m_xpSources; }

    // Statistics
    unsigned long long getTotalXPEarned() const { return m_totalXPEarned; }
    int getTotalLevelUps() const { return m_totalLevelUps; }

    // XP Modifiers (for prestige bonuses, items, etc.)
    void addXPModifier(const std::string& name, float multiplier);
    void removeXPModifier(const std::string& name);
    float getTotalXPModifier() const;
    void clearXPModifiers();

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

private:
    void calculateLevelFromXP();
    void processLevelUp(int newLevel);
    unsigned long long applyXPModifiers(unsigned long long baseXP) const;
    void generateDefaultLevelCurve();

    // Level curve data
    std::vector<LevelData> m_levelCurve;
    int m_maxLevel;

    // Current state
    int m_currentLevel;
    unsigned long long m_currentXP;
    unsigned long long m_totalXPEarned;
    int m_totalLevelUps;
    LevelReward m_pendingRewards;

    // XP tracking
    std::map<std::string, unsigned long long> m_xpSources;
    std::map<std::string, float> m_xpModifiers;

    // Callbacks
    LevelUpCallback m_onLevelUp;
    XPGainCallback m_onXPGain;

    // Constants
    static constexpr int DEFAULT_MAX_LEVEL = 100;
    static constexpr float BASE_XP_MULTIPLIER = 1.15f;
    static constexpr unsigned long long BASE_XP_REQUIREMENT = 100;
};

} // namespace RPG

#endif // LEVELING_SYSTEM_H
