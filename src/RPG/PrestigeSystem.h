#ifndef PRESTIGE_SYSTEM_H
#define PRESTIGE_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace RPG {

/**
 * @brief Manages the prestige/rebirth endgame progression system
 *
 * Allows players to reset their character at max level for permanent bonuses
 * Provides escalating rewards and unlocks for repeated playthroughs
 * Includes prestige-only content, skills, and challenges
 */
class PrestigeSystem {
public:
    struct PrestigeReward {
        int permanentStatBonus; // Added to base stats
        float xpMultiplier; // Multiplier for future XP gains
        float goldMultiplier; // Multiplier for gold gains
        float dropRateMultiplier; // Better loot
        int skillPointBonus; // Extra skill points per level
        int talentPointBonus; // Extra talent points
        std::vector<std::string> unlockedFeatures; // New game mechanics
        std::vector<std::string> prestigeSkills; // Prestige-exclusive skills
        std::vector<std::string> prestigeItems; // Prestige-exclusive items
    };

    struct PrestigeTier {
        int tier;
        std::string name;
        std::string description;
        int prestigeRequired; // Number of prestiges to reach this tier
        PrestigeReward cumulativeRewards;
        std::string colorTheme; // For UI
    };

    struct PrestigeCurrency {
        std::string name;
        int amount;
        std::string description;
    };

    using PrestigeCallback = std::function<void(int newPrestigeLevel, const PrestigeReward& rewards)>;
    using CurrencyGainCallback = std::function<void(const std::string& currencyType, int amount)>;

    PrestigeSystem();
    ~PrestigeSystem();

    // Initialization
    void initialize();

    // Prestige Management
    bool canPrestige() const;
    bool performPrestige();
    int getPrestigeLevel() const { return m_prestigeLevel; }
    int getTotalPrestiges() const { return m_totalPrestiges; }

    // Requirements
    int getPrestigeRequiredLevel() const { return m_prestigeRequiredLevel; }
    void setPrestigeRequiredLevel(int level) { m_prestigeRequiredLevel = level; }

    // Prestige Rewards
    PrestigeReward getCurrentPrestigeReward() const;
    PrestigeReward getCumulativeRewards() const;
    PrestigeReward getNextPrestigeReward() const;

    // Prestige Tiers
    const PrestigeTier* getCurrentTier() const;
    const PrestigeTier* getNextTier() const;
    std::vector<PrestigeTier> getAllTiers() const { return m_prestigeTiers; }

    // Prestige Currency (earned through prestige activities)
    void addCurrency(const std::string& currencyType, int amount);
    int getCurrency(const std::string& currencyType) const;
    bool spendCurrency(const std::string& currencyType, int amount);
    std::map<std::string, int> getAllCurrencies() const { return m_currencies; }

    // Prestige Shop (buy permanent upgrades with prestige currency)
    struct PrestigeUpgrade {
        std::string id;
        std::string name;
        std::string description;
        std::string currencyType;
        int cost;
        int maxPurchases; // -1 for unlimited
        int currentPurchases;
        std::map<std::string, float> effects;
        bool isPurchased() const { return currentPurchases > 0; }
        bool isMaxed() const { return maxPurchases != -1 && currentPurchases >= maxPurchases; }
    };

    bool purchaseUpgrade(const std::string& upgradeId);
    const PrestigeUpgrade* getUpgrade(const std::string& upgradeId) const;
    std::vector<const PrestigeUpgrade*> getAvailableUpgrades() const;
    std::vector<const PrestigeUpgrade*> getPurchasedUpgrades() const;

    // Prestige Challenges (optional harder objectives for extra rewards)
    struct PrestigeChallenge {
        std::string id;
        std::string name;
        std::string description;
        int prestigeRequired;
        bool isActive;
        bool isCompleted;
        std::map<std::string, int> objectives; // Objective type -> target value
        std::map<std::string, int> progress; // Current progress
        PrestigeReward rewards;
    };

    bool activateChallenge(const std::string& challengeId);
    bool completeChallenge(const std::string& challengeId);
    void updateChallengeProgress(const std::string& challengeId, const std::string& objective, int amount);
    const PrestigeChallenge* getChallenge(const std::string& challengeId) const;
    std::vector<const PrestigeChallenge*> getActiveChallenges() const;
    std::vector<const PrestigeChallenge*> getCompletedChallenges() const;

    // Prestige Milestones (achievements for prestige level)
    struct PrestigeMilestone {
        int prestigeLevel;
        std::string title;
        std::string description;
        PrestigeReward rewards;
        bool isUnlocked;
    };

    const PrestigeMilestone* getMilestone(int prestigeLevel) const;
    std::vector<const PrestigeMilestone*> getUnlockedMilestones() const;

    // Stat calculations with prestige bonuses
    float getStatMultiplier() const;
    float getXPMultiplier() const;
    float getGoldMultiplier() const;
    float getDropRateMultiplier() const;
    int getExtraSkillPoints() const;
    int getExtraTalentPoints() const;

    // What carries over after prestige
    struct CarryoverData {
        std::vector<std::string> items; // Specific items that carry over
        std::map<std::string, int> currencies;
        std::vector<std::string> unlockedFeatures;
        bool achievementsCarryOver;
        bool skillTreesReset;
        bool statsReset;
    };
    CarryoverData getCarryoverData() const;

    // Prestige points (alternative progression currency)
    void addPrestigePoints(int points);
    int getPrestigePoints() const { return m_prestigePoints; }
    int getTotalPrestigePointsEarned() const { return m_totalPrestigePoints; }

    // Legacy bonuses (small permanent bonuses for each prestige)
    struct LegacyBonus {
        std::string type;
        float value;
        int stackCount;
    };
    std::vector<LegacyBonus> getLegacyBonuses() const;
    float getLegacyBonusTotal(const std::string& type) const;

    // Callbacks
    void setPrestigeCallback(PrestigeCallback callback) { m_onPrestige = callback; }
    void setCurrencyGainCallback(CurrencyGainCallback callback) { m_onCurrencyGain = callback; }

    // Statistics
    long long getTotalPrestigeTime() const { return m_totalPrestigeTime; }
    int getFastestPrestige() const { return m_fastestPrestige; }
    std::map<std::string, int> getPrestigeStats() const;

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

private:
    void initializePrestigeTiers();
    void initializePrestigeUpgrades();
    void initializePrestigeChallenges();
    void initializePrestigeMilestones();

    void applyPrestigeRewards(const PrestigeReward& rewards);
    void calculateCumulativeRewards();
    PrestigeReward getRewardForPrestigeLevel(int level) const;

    // Prestige state
    int m_prestigeLevel;
    int m_totalPrestiges;
    int m_prestigeRequiredLevel;
    int m_prestigePoints;
    int m_totalPrestigePoints;

    // Prestige data
    std::vector<PrestigeTier> m_prestigeTiers;
    std::map<std::string, PrestigeUpgrade> m_upgrades;
    std::map<std::string, PrestigeChallenge> m_challenges;
    std::map<int, PrestigeMilestone> m_milestones;

    // Currencies
    std::map<std::string, int> m_currencies;

    // Legacy bonuses
    std::map<std::string, LegacyBonus> m_legacyBonuses;

    // Statistics
    long long m_totalPrestigeTime;
    int m_fastestPrestige;
    long long m_currentRunStartTime;

    // Callbacks
    PrestigeCallback m_onPrestige;
    CurrencyGainCallback m_onCurrencyGain;

    // Constants
    static constexpr int DEFAULT_PRESTIGE_LEVEL = 100;
    static constexpr int MAX_PRESTIGE_LEVEL = 999;
};

} // namespace RPG

#endif // PRESTIGE_SYSTEM_H
