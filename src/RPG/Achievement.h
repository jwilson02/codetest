#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>

namespace RPG {

/**
 * @brief Comprehensive achievement tracking system
 *
 * Tracks 100+ achievements across multiple categories
 * Supports progress tracking, rewards, and secret achievements
 */
class Achievement {
public:
    enum class Category {
        COMBAT,         // Kill enemies, deal damage, etc.
        EXPLORATION,    // Discover locations, travel distance
        PROGRESSION,    // Reach levels, unlock skills
        COLLECTION,     // Gather items, materials
        MASTERY,        // Master skills, complete builds
        SOCIAL,         // Party achievements, trading
        CHALLENGE,      // Complete difficult content
        SECRET,         // Hidden achievements
        PRESTIGE,       // Endgame achievements
        GENERAL         // Miscellaneous
    };

    enum class Rarity {
        COMMON,         // Easy to obtain
        UNCOMMON,       // Moderate difficulty
        RARE,           // Difficult
        EPIC,           // Very difficult
        LEGENDARY,      // Extremely difficult
        MYTHIC          // Nearly impossible
    };

    struct AchievementData {
        std::string id;
        std::string name;
        std::string description;
        std::string hiddenDescription; // Shown after unlock for secret achievements
        Category category;
        Rarity rarity;

        // Progress tracking
        bool isProgressive; // Has progress (e.g., "Kill 100 enemies")
        int maxProgress;
        int currentProgress;

        // Requirements
        std::vector<std::string> prerequisiteAchievements;
        int levelRequirement;
        std::map<std::string, int> requirements; // Generic requirement tracking

        // Rewards
        int goldReward;
        int xpReward;
        std::vector<std::string> itemRewards;
        std::vector<std::string> titleRewards;
        std::map<std::string, float> statBonuses; // Permanent bonuses

        // Status
        bool isUnlocked;
        bool isSecret;
        long long unlockTimestamp; // Unix timestamp
        float completionPercentage; // For tracking purposes

        // UI/Display
        std::string iconPath;
        int points; // Achievement points for leaderboards
    };

    using AchievementUnlockCallback = std::function<void(const AchievementData& achievement)>;
    using ProgressUpdateCallback = std::function<void(const std::string& achievementId, int progress, int max)>;

    Achievement();
    ~Achievement();

    // Initialization
    bool loadAchievements(const std::string& filepath);
    void initialize();

    // Achievement Management
    bool unlockAchievement(const std::string& achievementId);
    bool incrementProgress(const std::string& achievementId, int amount = 1);
    bool setProgress(const std::string& achievementId, int progress);
    const AchievementData* getAchievement(const std::string& achievementId) const;
    AchievementData* getAchievementMutable(const std::string& achievementId);

    // Queries
    std::vector<const AchievementData*> getAllAchievements() const;
    std::vector<const AchievementData*> getUnlockedAchievements() const;
    std::vector<const AchievementData*> getAchievementsByCategory(Category category) const;
    std::vector<const AchievementData*> getAchievementsByRarity(Rarity rarity) const;
    std::vector<const AchievementData*> getInProgressAchievements() const;
    std::vector<const AchievementData*> getAvailableAchievements() const; // Can work towards

    // Statistics
    int getTotalAchievements() const { return static_cast<int>(m_achievements.size()); }
    int getUnlockedCount() const;
    int getAchievementPoints() const;
    float getCompletionPercentage() const;
    std::map<Category, int> getAchievementCountByCategory() const;
    std::map<Rarity, int> getAchievementCountByRarity() const;

    // Tracking specific achievement types
    void trackKill(const std::string& enemyType, int count = 1);
    void trackDamageDealt(int damage);
    void trackLocationDiscovered(const std::string& location);
    void trackItemCollected(const std::string& itemType, int count = 1);
    void trackSkillUnlocked(const std::string& skillId);
    void trackQuestCompleted(const std::string& questId);
    void trackDistanceTraveled(float distance);
    void trackGoldEarned(int amount);
    void trackDeaths(int count = 1);

    // Generic stat tracking
    void incrementStat(const std::string& statName, int amount = 1);
    void setStat(const std::string& statName, int value);
    int getStat(const std::string& statName) const;

    // Rewards
    struct RewardBundle {
        int gold;
        int xp;
        std::vector<std::string> items;
        std::vector<std::string> titles;
        std::map<std::string, float> statBonuses;
    };
    RewardBundle getPendingRewards() const { return m_pendingRewards; }
    void claimRewards();
    bool hasPendingRewards() const;

    // Titles
    std::vector<std::string> getUnlockedTitles() const { return m_unlockedTitles; }
    void setActiveTitle(const std::string& title);
    std::string getActiveTitle() const { return m_activeTitle; }

    // Callbacks
    void setUnlockCallback(AchievementUnlockCallback callback) { m_onUnlock = callback; }
    void setProgressCallback(ProgressUpdateCallback callback) { m_onProgress = callback; }

    // Category/Rarity names
    static std::string getCategoryName(Category category);
    static std::string getRarityName(Rarity rarity);
    static int getRarityPoints(Rarity rarity);

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Helper for adding achievements programmatically
    void addAchievement(const AchievementData& achievement);

private:
    void checkAchievement(const std::string& achievementId);
    bool meetsRequirements(const AchievementData& achievement) const;
    void grantRewards(const AchievementData& achievement);
    void generateDefaultAchievements();

    // Achievement data
    std::map<std::string, AchievementData> m_achievements;

    // Stat tracking for achievements
    std::map<std::string, int> m_stats;

    // Rewards
    RewardBundle m_pendingRewards;

    // Titles
    std::vector<std::string> m_unlockedTitles;
    std::string m_activeTitle;

    // Callbacks
    AchievementUnlockCallback m_onUnlock;
    ProgressUpdateCallback m_onProgress;

    // Achievement categories for auto-tracking
    std::map<std::string, std::vector<std::string>> m_trackedAchievements;

    static constexpr int TOTAL_ACHIEVEMENTS = 100;
};

} // namespace RPG

#endif // ACHIEVEMENT_H
