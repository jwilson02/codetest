#include "Leaderboard.h"
#include <iostream>
#include <algorithm>

namespace Network {

Leaderboard::Leaderboard()
    : m_autoRefreshEnabled(false)
    , m_autoRefreshInterval(DEFAULT_REFRESH_INTERVAL)
    , m_autoRefreshTimer(0.0f)
    , m_cacheTimeout(DEFAULT_CACHE_TIMEOUT)
    , m_networkManager(NetworkManager::getInstance())
    , m_debugMode(false)
{
    m_arenaStats.rating = 1000;
    m_arenaStats.wins = 0;
    m_arenaStats.losses = 0;
    m_arenaStats.winStreak = 0;
    m_arenaStats.bestRating = 1000;
    m_arenaStats.tier = "Bronze";
}

Leaderboard::~Leaderboard() {
    shutdown();
}

void Leaderboard::initialize() {
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::LEADERBOARD_UPDATE,
        [this](const NetworkPacket& packet) { handleLeaderboardUpdate(packet); });

    m_networkManager.registerPacketHandler(PacketType::LEADERBOARD_REQUEST,
        [this](const NetworkPacket& packet) { handleRankUpdate(packet); });

    std::cout << "[Leaderboard] Initialized" << std::endl;
}

void Leaderboard::shutdown() {
    m_leaderboards.clear();
    m_achievements.clear();
    m_seasons.clear();
    m_guildRankings.clear();
}

void Leaderboard::requestLeaderboard(LeaderboardCategory category, int offset, int limit) {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    // Pack request data
    std::string data = std::to_string(static_cast<int>(category)) + ":" +
                      std::to_string(offset) + ":" + std::to_string(limit);
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_debugMode) {
        std::cout << "[Leaderboard] Requested " << getCategoryName(category)
                  << " leaderboard" << std::endl;
    }
}

void Leaderboard::requestPlayerRank(LeaderboardCategory category) {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "RANK:" + std::to_string(static_cast<int>(category));
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

void Leaderboard::requestNearbyRanks(LeaderboardCategory category, int range) {
    // Request ranks around player's rank
    int myRank = getMyRank(category);
    int offset = std::max(0, myRank - range);
    requestLeaderboard(category, offset, range * 2);
}

std::vector<LeaderboardEntry> Leaderboard::getLeaderboard(LeaderboardCategory category) const {
    auto it = m_leaderboards.find(category);
    if (it != m_leaderboards.end()) {
        return it->second;
    }
    return std::vector<LeaderboardEntry>();
}

LeaderboardEntry Leaderboard::getPlayerEntry(uint16_t playerId, LeaderboardCategory category) const {
    auto leaderboard = getLeaderboard(category);

    auto it = std::find_if(leaderboard.begin(), leaderboard.end(),
        [playerId](const LeaderboardEntry& entry) {
            return entry.playerId == playerId;
        });

    if (it != leaderboard.end()) {
        return *it;
    }

    return LeaderboardEntry();
}

LeaderboardEntry Leaderboard::getMyEntry(LeaderboardCategory category) const {
    return getPlayerEntry(m_networkManager.getLocalPlayerId(), category);
}

int Leaderboard::getMyRank(LeaderboardCategory category) const {
    auto it = m_playerRanks.find(category);
    if (it != m_playerRanks.end()) {
        return it->second;
    }

    // If not cached, try to find in leaderboard
    auto entry = getMyEntry(category);
    return entry.rank;
}

int Leaderboard::getTotalPlayers(LeaderboardCategory category) const {
    auto leaderboard = getLeaderboard(category);
    return static_cast<int>(leaderboard.size());
}

void Leaderboard::clearFilter() {
    m_filter = LeaderboardFilter();
}

void Leaderboard::applyFilter() {
    // Re-request leaderboards with new filter
    // Would send filter parameters to server
}

void Leaderboard::requestSeasonInfo() {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "SEASON_INFO";
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

std::vector<LeaderboardEntry> Leaderboard::getSeasonalLeaderboard(uint32_t seasonId, LeaderboardCategory category) const {
    // Would request/cache seasonal data
    return getLeaderboard(category);
}

void Leaderboard::requestAchievements() {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "ACHIEVEMENTS";
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

std::vector<Achievement> Leaderboard::getUnlockedAchievements() const {
    std::vector<Achievement> unlocked;

    for (const auto& achievement : m_achievements) {
        if (achievement.isUnlocked) {
            unlocked.push_back(achievement);
        }
    }

    return unlocked;
}

std::vector<Achievement> Leaderboard::getLockedAchievements() const {
    std::vector<Achievement> locked;

    for (const auto& achievement : m_achievements) {
        if (!achievement.isUnlocked && !achievement.isSecret) {
            locked.push_back(achievement);
        }
    }

    return locked;
}

int Leaderboard::getTotalAchievementPoints() const {
    int total = 0;

    for (const auto& achievement : m_achievements) {
        if (achievement.isUnlocked) {
            total += achievement.points;
        }
    }

    return total;
}

float Leaderboard::getAchievementCompletion() const {
    if (m_achievements.empty()) {
        return 0.0f;
    }

    int unlocked = 0;
    for (const auto& achievement : m_achievements) {
        if (achievement.isUnlocked) {
            unlocked++;
        }
    }

    return static_cast<float>(unlocked) / m_achievements.size();
}

void Leaderboard::requestArenaStats() {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "ARENA_STATS";
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

std::vector<LeaderboardEntry> Leaderboard::getArenaLeaderboard(const std::string& tier) const {
    auto leaderboard = getLeaderboard(LeaderboardCategory::ARENA_WINS);

    if (tier.empty()) {
        return leaderboard;
    }

    // Filter by tier
    std::vector<LeaderboardEntry> filtered;
    for (const auto& entry : leaderboard) {
        // Would check tier based on rating
        filtered.push_back(entry);
    }

    return filtered;
}

void Leaderboard::requestGuildRankings() {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "GUILD_RANKINGS";
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

Leaderboard::GuildRanking Leaderboard::getMyGuildRanking() const {
    // Would look up player's guild and find its ranking
    return GuildRanking();
}

void Leaderboard::submitScore(LeaderboardCategory category, uint64_t score) {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_UPDATE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = std::to_string(static_cast<int>(category)) + ":" + std::to_string(score);
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);

    if (m_debugMode) {
        std::cout << "[Leaderboard] Submitted score " << score << " for "
                  << getCategoryName(category) << std::endl;
    }
}

void Leaderboard::submitKill(bool isPvP) {
    if (isPvP) {
        submitScore(LeaderboardCategory::PVP_KILLS, 1);
    }
}

void Leaderboard::submitDeath() {
    // Record death for statistics
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_UPDATE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "DEATH";
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);
}

void Leaderboard::submitQuestComplete() {
    submitScore(LeaderboardCategory::QUESTS_COMPLETED, 1);
}

void Leaderboard::submitAchievement(uint32_t achievementId) {
    NetworkPacket packet;
    packet.type = PacketType::LEADERBOARD_UPDATE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = "ACHIEVEMENT:" + std::to_string(achievementId);
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

void Leaderboard::refresh() {
    // Refresh all cached leaderboards
    for (int i = 0; i <= static_cast<int>(LeaderboardCategory::GUILD_RATING); i++) {
        auto category = static_cast<LeaderboardCategory>(i);
        requestLeaderboard(category);
    }

    requestArenaStats();
    requestGuildRankings();
    requestAchievements();
}

void Leaderboard::setAutoRefresh(bool enable, float intervalSeconds) {
    m_autoRefreshEnabled = enable;
    m_autoRefreshInterval = intervalSeconds;
    m_autoRefreshTimer = 0.0f;
}

void Leaderboard::clearCache() {
    m_leaderboards.clear();
    m_cacheTimestamps.clear();
}

bool Leaderboard::isCacheValid(LeaderboardCategory category) const {
    auto it = m_cacheTimestamps.find(category);
    if (it == m_cacheTimestamps.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();

    return elapsed < m_cacheTimeout;
}

std::vector<LeaderboardEntry> Leaderboard::compareWithFriends(LeaderboardCategory category) {
    // Would filter leaderboard for friend IDs
    return getLeaderboard(category);
}

std::vector<LeaderboardEntry> Leaderboard::compareWithGuild(LeaderboardCategory category) {
    // Would filter leaderboard for guild members
    return getLeaderboard(category);
}

void Leaderboard::update(float deltaTime) {
    // Auto-refresh timer
    if (m_autoRefreshEnabled) {
        m_autoRefreshTimer += deltaTime;

        if (m_autoRefreshTimer >= m_autoRefreshInterval) {
            m_autoRefreshTimer = 0.0f;
            refresh();
        }
    }

    // Check for season end
    if (m_currentSeason.isActive && !m_currentSeason.isSeasonActive()) {
        if (m_onSeasonEnd) {
            m_onSeasonEnd(m_currentSeason);
        }
    }
}

void Leaderboard::handleLeaderboardUpdate(const NetworkPacket& packet) {
    // Parse leaderboard data from packet
    // Simplified implementation

    if (m_debugMode) {
        std::cout << "[Leaderboard] Received leaderboard update" << std::endl;
    }

    // Would deserialize and cache leaderboard entries
    // Notify callback
    if (m_onLeaderboardUpdate) {
        m_onLeaderboardUpdate(LeaderboardCategory::OVERALL);
    }
}

void Leaderboard::handleRankUpdate(const NetworkPacket& packet) {
    // Parse rank update
    if (packet.data.empty()) {
        return;
    }

    // Would parse category and new rank
    // Check if rank changed and notify

    if (m_onRankChange) {
        m_onRankChange(LeaderboardCategory::OVERALL, 0, 0);
    }

    if (m_debugMode) {
        std::cout << "[Leaderboard] Rank updated" << std::endl;
    }
}

void Leaderboard::handleAchievementUnlock(const NetworkPacket& packet) {
    // Parse achievement data
    Achievement achievement;
    achievement.achievementId = 1;
    achievement.name = "New Achievement";
    achievement.isUnlocked = true;
    achievement.unlockedTime = std::chrono::system_clock::now();

    // Add to list
    m_achievements.push_back(achievement);

    if (m_onAchievementUnlocked) {
        m_onAchievementUnlocked(achievement);
    }

    if (m_debugMode) {
        std::cout << "[Leaderboard] Achievement unlocked: " << achievement.name << std::endl;
    }
}

void Leaderboard::handleSeasonInfo(const NetworkPacket& packet) {
    // Parse season information
    if (m_debugMode) {
        std::cout << "[Leaderboard] Received season info" << std::endl;
    }
}

void Leaderboard::cacheLeaderboard(LeaderboardCategory category, const std::vector<LeaderboardEntry>& entries) {
    m_leaderboards[category] = entries;
    m_cacheTimestamps[category] = std::chrono::steady_clock::now();
}

std::vector<LeaderboardEntry> Leaderboard::getCachedLeaderboard(LeaderboardCategory category) const {
    if (isCacheValid(category)) {
        return getLeaderboard(category);
    }
    return std::vector<LeaderboardEntry>();
}

void Leaderboard::updateRankings(LeaderboardCategory category) {
    auto& leaderboard = m_leaderboards[category];

    // Sort by score
    std::sort(leaderboard.begin(), leaderboard.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score;
        });

    // Update ranks
    for (size_t i = 0; i < leaderboard.size(); i++) {
        leaderboard[i].rank = static_cast<int>(i + 1);
    }
}

std::string Leaderboard::getCategoryName(LeaderboardCategory category) {
    switch (category) {
        case LeaderboardCategory::OVERALL: return "Overall";
        case LeaderboardCategory::LEVEL: return "Level";
        case LeaderboardCategory::PVP_KILLS: return "PvP Kills";
        case LeaderboardCategory::PVP_RATING: return "PvP Rating";
        case LeaderboardCategory::ARENA_WINS: return "Arena Wins";
        case LeaderboardCategory::DUNGEON_CLEARS: return "Dungeon Clears";
        case LeaderboardCategory::BOSS_KILLS: return "Boss Kills";
        case LeaderboardCategory::GOLD_EARNED: return "Gold Earned";
        case LeaderboardCategory::QUESTS_COMPLETED: return "Quests Completed";
        case LeaderboardCategory::ACHIEVEMENTS: return "Achievements";
        case LeaderboardCategory::PLAYTIME: return "Playtime";
        case LeaderboardCategory::SPEED_RUN: return "Speed Run";
        case LeaderboardCategory::CRAFTING: return "Crafting";
        case LeaderboardCategory::TRADING: return "Trading";
        case LeaderboardCategory::GUILD_RATING: return "Guild Rating";
        default: return "Unknown";
    }
}

std::string Leaderboard::getTierName(int rating) {
    if (rating < 1000) return "Bronze";
    else if (rating < 1500) return "Silver";
    else if (rating < 2000) return "Gold";
    else if (rating < 2500) return "Platinum";
    else if (rating < 3000) return "Diamond";
    else return "Master";
}

int Leaderboard::getRatingForTier(const std::string& tier) {
    if (tier == "Bronze") return 0;
    else if (tier == "Silver") return 1000;
    else if (tier == "Gold") return 1500;
    else if (tier == "Platinum") return 2000;
    else if (tier == "Diamond") return 2500;
    else if (tier == "Master") return 3000;
    return 0;
}

void Leaderboard::logLeaderboardInfo() {
    std::cout << "\n===== Leaderboard Information =====" << std::endl;
    std::cout << "Cached Leaderboards: " << m_leaderboards.size() << std::endl;
    std::cout << "Total Achievements: " << m_achievements.size() << std::endl;
    std::cout << "Unlocked Achievements: " << getUnlockedAchievements().size() << std::endl;
    std::cout << "Achievement Points: " << getTotalAchievementPoints() << std::endl;
    std::cout << "Achievement Completion: " << (getAchievementCompletion() * 100.0f) << "%" << std::endl;

    std::cout << "\nArena Stats:" << std::endl;
    std::cout << "  Rating: " << m_arenaStats.rating << std::endl;
    std::cout << "  Tier: " << m_arenaStats.tier << std::endl;
    std::cout << "  W/L: " << m_arenaStats.wins << "/" << m_arenaStats.losses << std::endl;
    std::cout << "  Win Streak: " << m_arenaStats.winStreak << std::endl;
    std::cout << "  Best Rating: " << m_arenaStats.bestRating << std::endl;

    if (m_currentSeason.isActive) {
        std::cout << "\nCurrent Season:" << std::endl;
        std::cout << "  Name: " << m_currentSeason.name << std::endl;
        std::cout << "  Status: " << (m_currentSeason.isSeasonActive() ? "Active" : "Ended") << std::endl;
    }

    std::cout << "===================================\n" << std::endl;
}

} // namespace Network
