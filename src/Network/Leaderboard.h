#pragma once

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace Network {

/**
 * @brief Leaderboard entry
 */
struct LeaderboardEntry {
    uint16_t playerId;
    std::string playerName;
    int rank;
    uint64_t score;
    int level;
    std::string characterClass;
    std::string guild;
    std::string region;

    // Specific stats based on category
    int kills;
    int deaths;
    int wins;
    int losses;
    uint64_t goldEarned;
    int questsCompleted;
    int achievementsUnlocked;
    float playtimeHours;

    std::chrono::system_clock::time_point lastUpdated;

    LeaderboardEntry()
        : playerId(0), rank(0), score(0), level(1)
        , kills(0), deaths(0), wins(0), losses(0)
        , goldEarned(0), questsCompleted(0), achievementsUnlocked(0)
        , playtimeHours(0.0f) {}

    float getKDRatio() const {
        return deaths > 0 ? static_cast<float>(kills) / deaths : static_cast<float>(kills);
    }

    float getWinRate() const {
        int total = wins + losses;
        return total > 0 ? static_cast<float>(wins) / total : 0.0f;
    }
};

/**
 * @brief Leaderboard categories
 */
enum class LeaderboardCategory {
    OVERALL,           // Overall score
    LEVEL,             // Highest level
    PVP_KILLS,         // PvP kills
    PVP_RATING,        // PvP rating/ELO
    ARENA_WINS,        // Arena victories
    DUNGEON_CLEARS,    // Dungeon completions
    BOSS_KILLS,        // Boss defeats
    GOLD_EARNED,       // Total gold accumulated
    QUESTS_COMPLETED,  // Quest completion count
    ACHIEVEMENTS,      // Achievement points
    PLAYTIME,          // Total playtime
    SPEED_RUN,         // Fastest dungeon/quest times
    CRAFTING,          // Crafting level/items
    TRADING,           // Trading volume
    GUILD_RATING       // Guild rankings
};

/**
 * @brief Leaderboard filters
 */
struct LeaderboardFilter {
    std::string region;
    std::string characterClass;
    int minLevel;
    int maxLevel;
    bool friendsOnly;
    bool guildOnly;

    LeaderboardFilter()
        : minLevel(1), maxLevel(999)
        , friendsOnly(false), guildOnly(false) {}
};

/**
 * @brief Season information
 */
struct Season {
    uint32_t seasonId;
    std::string name;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    bool isActive;

    bool isSeasonActive() const {
        auto now = std::chrono::system_clock::now();
        return isActive && now >= startTime && now < endTime;
    }
};

/**
 * @brief Achievement entry
 */
struct Achievement {
    uint32_t achievementId;
    std::string name;
    std::string description;
    int points;
    float completionPercentage;  // Global completion rate
    std::chrono::system_clock::time_point unlockedTime;
    bool isUnlocked;
    bool isSecret;

    Achievement()
        : achievementId(0), points(0)
        , completionPercentage(0.0f)
        , isUnlocked(false), isSecret(false) {}
};

/**
 * @brief Leaderboard and ranking system
 *
 * Tracks and displays player rankings across various categories.
 * Supports global, regional, and friend leaderboards.
 * Includes seasonal rankings and achievements.
 */
class Leaderboard {
public:
    static Leaderboard& getInstance() {
        static Leaderboard instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Leaderboard requests
    void requestLeaderboard(LeaderboardCategory category, int offset = 0, int limit = 100);
    void requestPlayerRank(LeaderboardCategory category);
    void requestNearbyRanks(LeaderboardCategory category, int range = 10);

    // Leaderboard data
    std::vector<LeaderboardEntry> getLeaderboard(LeaderboardCategory category) const;
    LeaderboardEntry getPlayerEntry(uint16_t playerId, LeaderboardCategory category) const;
    LeaderboardEntry getMyEntry(LeaderboardCategory category) const;
    int getMyRank(LeaderboardCategory category) const;
    int getTotalPlayers(LeaderboardCategory category) const;

    // Filtering
    void setFilter(const LeaderboardFilter& filter) { m_filter = filter; }
    LeaderboardFilter getFilter() const { return m_filter; }
    void clearFilter();
    void applyFilter();

    // Seasons
    void requestSeasonInfo();
    Season getCurrentSeason() const { return m_currentSeason; }
    std::vector<Season> getAllSeasons() const { return m_seasons; }
    std::vector<LeaderboardEntry> getSeasonalLeaderboard(uint32_t seasonId, LeaderboardCategory category) const;

    // Achievements
    void requestAchievements();
    std::vector<Achievement> getAchievements() const { return m_achievements; }
    std::vector<Achievement> getUnlockedAchievements() const;
    std::vector<Achievement> getLockedAchievements() const;
    int getTotalAchievementPoints() const;
    float getAchievementCompletion() const;

    // PvP Arena specific
    struct ArenaStats {
        int rating;
        int wins;
        int losses;
        int winStreak;
        int bestRating;
        std::string tier;  // Bronze, Silver, Gold, Platinum, Diamond, etc.
    };

    void requestArenaStats();
    ArenaStats getArenaStats() const { return m_arenaStats; }
    std::vector<LeaderboardEntry> getArenaLeaderboard(const std::string& tier = "") const;

    // Guild rankings
    struct GuildRanking {
        std::string guildName;
        int rank;
        uint64_t score;
        int memberCount;
        int level;
        std::string leader;
    };

    void requestGuildRankings();
    std::vector<GuildRanking> getGuildRankings() const { return m_guildRankings; }
    GuildRanking getMyGuildRanking() const;

    // Statistics submission (from client to server)
    void submitScore(LeaderboardCategory category, uint64_t score);
    void submitKill(bool isPvP = false);
    void submitDeath();
    void submitQuestComplete();
    void submitAchievement(uint32_t achievementId);

    // Refresh
    void refresh();
    void setAutoRefresh(bool enable, float intervalSeconds = 60.0f);
    bool isAutoRefreshEnabled() const { return m_autoRefreshEnabled; }

    // Caching
    void clearCache();
    void setCacheTimeout(float seconds) { m_cacheTimeout = seconds; }
    bool isCacheValid(LeaderboardCategory category) const;

    // Comparison
    std::vector<LeaderboardEntry> compareWithFriends(LeaderboardCategory category);
    std::vector<LeaderboardEntry> compareWithGuild(LeaderboardCategory category);

    // Update
    void update(float deltaTime);

    // Callbacks
    using LeaderboardUpdateCallback = std::function<void(LeaderboardCategory category)>;
    using RankChangeCallback = std::function<void(LeaderboardCategory category, int oldRank, int newRank)>;
    using AchievementUnlockedCallback = std::function<void(const Achievement& achievement)>;
    using SeasonEndCallback = std::function<void(const Season& season)>;

    void setOnLeaderboardUpdate(LeaderboardUpdateCallback callback) { m_onLeaderboardUpdate = callback; }
    void setOnRankChange(RankChangeCallback callback) { m_onRankChange = callback; }
    void setOnAchievementUnlocked(AchievementUnlockedCallback callback) { m_onAchievementUnlocked = callback; }
    void setOnSeasonEnd(SeasonEndCallback callback) { m_onSeasonEnd = callback; }

    // Utility
    static std::string getCategoryName(LeaderboardCategory category);
    static std::string getTierName(int rating);
    static int getRatingForTier(const std::string& tier);

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    void logLeaderboardInfo();

private:
    Leaderboard();
    ~Leaderboard();
    Leaderboard(const Leaderboard&) = delete;
    Leaderboard& operator=(const Leaderboard&) = delete;

    // Packet handlers
    void handleLeaderboardUpdate(const NetworkPacket& packet);
    void handleRankUpdate(const NetworkPacket& packet);
    void handleAchievementUnlock(const NetworkPacket& packet);
    void handleSeasonInfo(const NetworkPacket& packet);

    // Internal helpers
    void cacheLeaderboard(LeaderboardCategory category, const std::vector<LeaderboardEntry>& entries);
    std::vector<LeaderboardEntry> getCachedLeaderboard(LeaderboardCategory category) const;
    void updateRankings(LeaderboardCategory category);

    // Data members
    std::unordered_map<LeaderboardCategory, std::vector<LeaderboardEntry>> m_leaderboards;
    std::unordered_map<LeaderboardCategory, std::chrono::steady_clock::time_point> m_cacheTimestamps;
    std::unordered_map<LeaderboardCategory, int> m_playerRanks;

    LeaderboardFilter m_filter;

    // Seasons
    Season m_currentSeason;
    std::vector<Season> m_seasons;

    // Achievements
    std::vector<Achievement> m_achievements;

    // Arena
    ArenaStats m_arenaStats;

    // Guilds
    std::vector<GuildRanking> m_guildRankings;

    // Auto refresh
    bool m_autoRefreshEnabled;
    float m_autoRefreshInterval;
    float m_autoRefreshTimer;

    // Cache settings
    float m_cacheTimeout;

    // Callbacks
    LeaderboardUpdateCallback m_onLeaderboardUpdate;
    RankChangeCallback m_onRankChange;
    AchievementUnlockedCallback m_onAchievementUnlocked;
    SeasonEndCallback m_onSeasonEnd;

    // Network reference
    NetworkManager& m_networkManager;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr float DEFAULT_CACHE_TIMEOUT = 300.0f;  // 5 minutes
    static constexpr float DEFAULT_REFRESH_INTERVAL = 60.0f; // 1 minute
    static constexpr int DEFAULT_LEADERBOARD_SIZE = 100;
};

} // namespace Network
