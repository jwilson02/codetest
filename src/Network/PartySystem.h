#pragma once

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace Network {

/**
 * @brief Party member information
 */
struct PartyMember {
    uint16_t playerId;
    std::string playerName;
    int level;
    std::string characterClass;
    float health;
    float maxHealth;
    float mana;
    float maxMana;
    bool isOnline;
    bool isLeader;
    bool isReady;
    std::chrono::system_clock::time_point joinedTime;

    PartyMember()
        : playerId(0), level(1)
        , health(100), maxHealth(100)
        , mana(100), maxMana(100)
        , isOnline(true), isLeader(false)
        , isReady(false) {}
};

/**
 * @brief Party settings and configuration
 */
struct PartySettings {
    enum class LootMode {
        FREE_FOR_ALL,     // Everyone can loot
        ROUND_ROBIN,      // Loot distributed in turns
        MASTER_LOOTER,    // Leader assigns loot
        NEED_BEFORE_GREED // Need/Greed system
    };

    enum class ExperienceShare {
        EQUAL,            // Split equally
        LEVEL_WEIGHTED,   // Based on level contribution
        DAMAGE_BASED      // Based on damage dealt
    };

    std::string partyName;
    LootMode lootMode;
    ExperienceShare expShare;
    bool allowJoinRequests;
    bool isPublic;
    int maxMembers;
    int minLevel;
    int maxLevel;

    PartySettings()
        : lootMode(LootMode::FREE_FOR_ALL)
        , expShare(ExperienceShare::EQUAL)
        , allowJoinRequests(true)
        , isPublic(false)
        , maxMembers(4)
        , minLevel(1)
        , maxLevel(99) {}
};

/**
 * @brief Party invitation
 */
struct PartyInvite {
    uint16_t fromPlayerId;
    std::string fromPlayerName;
    uint16_t toPlayerId;
    std::chrono::system_clock::time_point sentTime;
    std::chrono::system_clock::time_point expiryTime;
    bool isExpired() const {
        return std::chrono::system_clock::now() > expiryTime;
    }
};

/**
 * @brief Co-op party system for up to 4 players
 *
 * Manages party creation, invitations, member management, loot distribution,
 * and shared quest progression.
 */
class PartySystem {
public:
    static PartySystem& getInstance() {
        static PartySystem instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Party creation and management
    bool createParty(const std::string& partyName = "");
    bool joinParty(uint16_t partyId);
    bool leaveParty();
    bool disbandParty();
    bool isInParty() const { return m_currentPartyId != 0; }
    uint16_t getPartyId() const { return m_currentPartyId; }

    // Invitations
    void invitePlayer(uint16_t playerId);
    void acceptInvite(uint16_t fromPlayerId);
    void declineInvite(uint16_t fromPlayerId);
    std::vector<PartyInvite> getPendingInvites() const;
    void clearExpiredInvites();

    // Member management
    void kickMember(uint16_t playerId);
    void promoteMember(uint16_t playerId);
    void transferLeadership(uint16_t playerId);
    std::vector<PartyMember> getMembers() const;
    PartyMember* getMember(uint16_t playerId);
    int getMemberCount() const { return static_cast<int>(m_members.size()); }
    int getMaxMembers() const { return m_settings.maxMembers; }
    bool isLeader() const;
    bool isLeader(uint16_t playerId) const;

    // Ready check
    void setReady(bool ready);
    bool isReady(uint16_t playerId) const;
    bool allMembersReady() const;
    void initiateReadyCheck();
    void cancelReadyCheck();

    // Party settings
    void updateSettings(const PartySettings& settings);
    PartySettings getSettings() const { return m_settings; }
    void setLootMode(PartySettings::LootMode mode);
    void setExperienceShare(PartySettings::ExperienceShare mode);
    void setMaxMembers(int max);

    // Member updates
    void updateMemberHealth(uint16_t playerId, float health, float maxHealth);
    void updateMemberMana(uint16_t playerId, float mana, float maxMana);
    void updateMemberPosition(uint16_t playerId, float x, float y, float z);
    void updateMemberLevel(uint16_t playerId, int level);

    // Quest sharing
    void shareQuest(const std::string& questId);
    void acceptSharedQuest(const std::string& questId);
    void declineSharedQuest(const std::string& questId);
    std::vector<std::string> getSharedQuests() const { return m_sharedQuests; }

    // Loot distribution
    void registerLootDrop(uint32_t lootId, const std::vector<uint32_t>& itemIds);
    void requestLootRoll(uint32_t lootId, uint32_t itemId, int rollType); // 0=pass, 1=need, 2=greed
    void distributeLoot();

    // Party chat
    void sendPartyMessage(const std::string& message);

    // Party markers and waypoints
    void setWaypoint(float x, float y, float z);
    void clearWaypoint();
    void markTarget(uint32_t entityId);

    // Party finder
    void listParty(); // List party in party finder
    void unlistParty();
    std::vector<PartySettings> findParties(); // Search for parties

    // Update
    void update(float deltaTime);

    // Callbacks
    using PartyInviteCallback = std::function<void(const PartyInvite& invite)>;
    using MemberJoinCallback = std::function<void(uint16_t playerId, const std::string& name)>;
    using MemberLeaveCallback = std::function<void(uint16_t playerId, const std::string& reason)>;
    using LeaderChangeCallback = std::function<void(uint16_t newLeaderId)>;
    using ReadyCheckCallback = std::function<void(uint16_t playerId, bool ready)>;
    using QuestShareCallback = std::function<void(const std::string& questId, uint16_t fromPlayerId)>;
    using LootRollCallback = std::function<void(uint32_t lootId, uint32_t itemId)>;

    void setOnInviteReceived(PartyInviteCallback callback) { m_onInviteReceived = callback; }
    void setOnMemberJoin(MemberJoinCallback callback) { m_onMemberJoin = callback; }
    void setOnMemberLeave(MemberLeaveCallback callback) { m_onMemberLeave = callback; }
    void setOnLeaderChange(LeaderChangeCallback callback) { m_onLeaderChange = callback; }
    void setOnReadyCheck(ReadyCheckCallback callback) { m_onReadyCheck = callback; }
    void setOnQuestShare(QuestShareCallback callback) { m_onQuestShare = callback; }
    void setOnLootRoll(LootRollCallback callback) { m_onLootRoll = callback; }

    // Statistics
    float getPartyDuration() const;
    int getTotalMonstersKilled() const { return m_totalMonstersKilled; }
    int getTotalQuestsCompleted() const { return m_totalQuestsCompleted; }
    int getTotalLootCollected() const { return m_totalLootCollected; }

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    void logPartyInfo();

private:
    PartySystem();
    ~PartySystem();
    PartySystem(const PartySystem&) = delete;
    PartySystem& operator=(const PartySystem&) = delete;

    // Packet handlers
    void handlePartyInvite(const NetworkPacket& packet);
    void handlePartyAccept(const NetworkPacket& packet);
    void handlePartyDecline(const NetworkPacket& packet);
    void handlePartyLeave(const NetworkPacket& packet);
    void handlePartyKick(const NetworkPacket& packet);
    void handlePartyUpdate(const NetworkPacket& packet);

    // Internal helpers
    void addMember(const PartyMember& member);
    void removeMember(uint16_t playerId);
    void notifyMembersOfChange();
    bool canInvite(uint16_t playerId) const;
    bool canJoin(uint16_t playerId) const;

    // Loot system helpers
    struct LootRoll {
        uint16_t playerId;
        int rollType;  // 0=pass, 1=need, 2=greed
        int rollValue;
        std::chrono::system_clock::time_point rollTime;
    };

    struct LootDrop {
        uint32_t lootId;
        std::vector<uint32_t> itemIds;
        std::unordered_map<uint32_t, std::vector<LootRoll>> rolls;
        std::chrono::system_clock::time_point dropTime;
        bool distributed;
    };

    void processLootRolls(LootDrop& drop);
    uint16_t determineWinner(const std::vector<LootRoll>& rolls);

    // Data members
    uint16_t m_currentPartyId;
    uint16_t m_leaderId;
    std::unordered_map<uint16_t, PartyMember> m_members;
    PartySettings m_settings;

    std::vector<PartyInvite> m_pendingInvites;
    std::vector<std::string> m_sharedQuests;
    std::unordered_map<uint32_t, LootDrop> m_lootDrops;

    // Party stats
    std::chrono::system_clock::time_point m_createdTime;
    int m_totalMonstersKilled;
    int m_totalQuestsCompleted;
    int m_totalLootCollected;

    // Ready check
    bool m_readyCheckActive;
    std::chrono::system_clock::time_point m_readyCheckStartTime;
    float m_readyCheckTimeout;

    // Waypoint
    bool m_hasWaypoint;
    float m_waypointX, m_waypointY, m_waypointZ;
    uint32_t m_markedTarget;

    // Callbacks
    PartyInviteCallback m_onInviteReceived;
    MemberJoinCallback m_onMemberJoin;
    MemberLeaveCallback m_onMemberLeave;
    LeaderChangeCallback m_onLeaderChange;
    ReadyCheckCallback m_onReadyCheck;
    QuestShareCallback m_onQuestShare;
    LootRollCallback m_onLootRoll;

    // Network reference
    NetworkManager& m_networkManager;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr int MAX_PARTY_SIZE = 4;
    static constexpr float INVITE_TIMEOUT_SECONDS = 60.0f;
    static constexpr float READY_CHECK_TIMEOUT = 30.0f;
    static constexpr float LOOT_ROLL_TIMEOUT = 30.0f;
};

} // namespace Network
