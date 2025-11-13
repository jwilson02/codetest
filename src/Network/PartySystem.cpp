#include "PartySystem.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace Network {

PartySystem::PartySystem()
    : m_currentPartyId(0)
    , m_leaderId(0)
    , m_totalMonstersKilled(0)
    , m_totalQuestsCompleted(0)
    , m_totalLootCollected(0)
    , m_readyCheckActive(false)
    , m_readyCheckTimeout(READY_CHECK_TIMEOUT)
    , m_hasWaypoint(false)
    , m_waypointX(0), m_waypointY(0), m_waypointZ(0)
    , m_markedTarget(0)
    , m_networkManager(NetworkManager::getInstance())
    , m_debugMode(false)
{
}

PartySystem::~PartySystem() {
    shutdown();
}

void PartySystem::initialize() {
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::PARTY_INVITE,
        [this](const NetworkPacket& packet) { handlePartyInvite(packet); });

    m_networkManager.registerPacketHandler(PacketType::PARTY_ACCEPT,
        [this](const NetworkPacket& packet) { handlePartyAccept(packet); });

    m_networkManager.registerPacketHandler(PacketType::PARTY_DECLINE,
        [this](const NetworkPacket& packet) { handlePartyDecline(packet); });

    m_networkManager.registerPacketHandler(PacketType::PARTY_LEAVE,
        [this](const NetworkPacket& packet) { handlePartyLeave(packet); });

    m_networkManager.registerPacketHandler(PacketType::PARTY_KICK,
        [this](const NetworkPacket& packet) { handlePartyKick(packet); });

    m_networkManager.registerPacketHandler(PacketType::PARTY_UPDATE,
        [this](const NetworkPacket& packet) { handlePartyUpdate(packet); });

    std::cout << "[PartySystem] Initialized" << std::endl;
}

void PartySystem::shutdown() {
    if (isInParty()) {
        leaveParty();
    }
    m_members.clear();
    m_pendingInvites.clear();
}

bool PartySystem::createParty(const std::string& partyName) {
    if (isInParty()) {
        std::cerr << "[PartySystem] Already in a party" << std::endl;
        return false;
    }

    m_currentPartyId = m_networkManager.getLocalPlayerId();
    m_leaderId = m_networkManager.getLocalPlayerId();
    m_settings.partyName = partyName.empty() ? "Party" : partyName;
    m_createdTime = std::chrono::system_clock::now();

    // Add self as member
    PartyMember self;
    self.playerId = m_networkManager.getLocalPlayerId();
    self.playerName = "Player"; // Would get from player system
    self.isLeader = true;
    self.isOnline = true;
    self.joinedTime = m_createdTime;

    addMember(self);

    if (m_debugMode) {
        std::cout << "[PartySystem] Created party: " << m_settings.partyName << std::endl;
    }

    return true;
}

bool PartySystem::leaveParty() {
    if (!isInParty()) {
        return false;
    }

    NetworkPacket packet;
    packet.type = PacketType::PARTY_LEAVE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_debugMode) {
        std::cout << "[PartySystem] Left party" << std::endl;
    }

    m_currentPartyId = 0;
    m_leaderId = 0;
    m_members.clear();
    m_sharedQuests.clear();

    return true;
}

bool PartySystem::disbandParty() {
    if (!isLeader()) {
        std::cerr << "[PartySystem] Only leader can disband party" << std::endl;
        return false;
    }

    // Notify all members
    for (const auto& [playerId, member] : m_members) {
        if (playerId != m_networkManager.getLocalPlayerId()) {
            NetworkPacket packet;
            packet.type = PacketType::PARTY_LEAVE;
            packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            std::string reason = "DISBANDED";
            packet.data.assign(reason.begin(), reason.end());
            packet.dataSize = static_cast<uint16_t>(packet.data.size());

            m_networkManager.sendPacketTo(playerId, packet, ProtocolType::TCP);
        }
    }

    if (m_debugMode) {
        std::cout << "[PartySystem] Disbanded party" << std::endl;
    }

    m_currentPartyId = 0;
    m_leaderId = 0;
    m_members.clear();

    return true;
}

void PartySystem::invitePlayer(uint16_t playerId) {
    if (!isInParty() || !canInvite(playerId)) {
        return;
    }

    PartyInvite invite;
    invite.fromPlayerId = m_networkManager.getLocalPlayerId();
    invite.fromPlayerName = "Player"; // Would get from player system
    invite.toPlayerId = playerId;
    invite.sentTime = std::chrono::system_clock::now();
    invite.expiryTime = invite.sentTime + std::chrono::seconds(static_cast<int>(INVITE_TIMEOUT_SECONDS));

    m_pendingInvites.push_back(invite);

    // Send invite packet
    NetworkPacket packet;
    packet.type = PacketType::PARTY_INVITE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        invite.sentTime.time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string inviteData = std::to_string(playerId) + ":" + m_settings.partyName;
    packet.data.assign(inviteData.begin(), inviteData.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_debugMode) {
        std::cout << "[PartySystem] Sent invite to player " << playerId << std::endl;
    }
}

void PartySystem::acceptInvite(uint16_t fromPlayerId) {
    auto it = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(),
        [fromPlayerId](const PartyInvite& inv) {
            return inv.fromPlayerId == fromPlayerId && !inv.isExpired();
        });

    if (it == m_pendingInvites.end()) {
        std::cerr << "[PartySystem] No valid invite from player " << fromPlayerId << std::endl;
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::PARTY_ACCEPT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    m_pendingInvites.erase(it);

    if (m_debugMode) {
        std::cout << "[PartySystem] Accepted invite from player " << fromPlayerId << std::endl;
    }
}

void PartySystem::declineInvite(uint16_t fromPlayerId) {
    auto it = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(),
        [fromPlayerId](const PartyInvite& inv) {
            return inv.fromPlayerId == fromPlayerId;
        });

    if (it != m_pendingInvites.end()) {
        NetworkPacket packet;
        packet.type = PacketType::PARTY_DECLINE;
        packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        packet.playerId = m_networkManager.getLocalPlayerId();

        m_networkManager.sendPacket(packet, ProtocolType::TCP);

        m_pendingInvites.erase(it);

        if (m_debugMode) {
            std::cout << "[PartySystem] Declined invite from player " << fromPlayerId << std::endl;
        }
    }
}

std::vector<PartyInvite> PartySystem::getPendingInvites() const {
    std::vector<PartyInvite> valid;
    for (const auto& invite : m_pendingInvites) {
        if (!invite.isExpired()) {
            valid.push_back(invite);
        }
    }
    return valid;
}

void PartySystem::clearExpiredInvites() {
    m_pendingInvites.erase(
        std::remove_if(m_pendingInvites.begin(), m_pendingInvites.end(),
            [](const PartyInvite& inv) { return inv.isExpired(); }),
        m_pendingInvites.end()
    );
}

void PartySystem::kickMember(uint16_t playerId) {
    if (!isLeader()) {
        std::cerr << "[PartySystem] Only leader can kick members" << std::endl;
        return;
    }

    if (playerId == m_networkManager.getLocalPlayerId()) {
        std::cerr << "[PartySystem] Cannot kick yourself" << std::endl;
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::PARTY_KICK;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = playerId;

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    removeMember(playerId);

    if (m_debugMode) {
        std::cout << "[PartySystem] Kicked player " << playerId << std::endl;
    }
}

void PartySystem::transferLeadership(uint16_t playerId) {
    if (!isLeader()) {
        std::cerr << "[PartySystem] Only leader can transfer leadership" << std::endl;
        return;
    }

    auto it = m_members.find(playerId);
    if (it == m_members.end()) {
        std::cerr << "[PartySystem] Player not in party" << std::endl;
        return;
    }

    // Update leader status
    if (auto oldLeader = m_members.find(m_leaderId); oldLeader != m_members.end()) {
        oldLeader->second.isLeader = false;
    }

    m_leaderId = playerId;
    it->second.isLeader = true;

    notifyMembersOfChange();

    if (m_onLeaderChange) {
        m_onLeaderChange(playerId);
    }

    if (m_debugMode) {
        std::cout << "[PartySystem] Transferred leadership to player " << playerId << std::endl;
    }
}

std::vector<PartyMember> PartySystem::getMembers() const {
    std::vector<PartyMember> members;
    for (const auto& [id, member] : m_members) {
        members.push_back(member);
    }
    return members;
}

PartyMember* PartySystem::getMember(uint16_t playerId) {
    auto it = m_members.find(playerId);
    return (it != m_members.end()) ? &it->second : nullptr;
}

bool PartySystem::isLeader() const {
    return isLeader(m_networkManager.getLocalPlayerId());
}

bool PartySystem::isLeader(uint16_t playerId) const {
    return playerId == m_leaderId;
}

void PartySystem::setReady(bool ready) {
    auto* member = getMember(m_networkManager.getLocalPlayerId());
    if (member) {
        member->isReady = ready;
        notifyMembersOfChange();

        if (m_onReadyCheck) {
            m_onReadyCheck(m_networkManager.getLocalPlayerId(), ready);
        }
    }
}

bool PartySystem::isReady(uint16_t playerId) const {
    auto it = m_members.find(playerId);
    return (it != m_members.end()) ? it->second.isReady : false;
}

bool PartySystem::allMembersReady() const {
    for (const auto& [id, member] : m_members) {
        if (!member.isReady) {
            return false;
        }
    }
    return !m_members.empty();
}

void PartySystem::initiateReadyCheck() {
    if (!isLeader()) {
        return;
    }

    m_readyCheckActive = true;
    m_readyCheckStartTime = std::chrono::system_clock::now();

    // Reset all ready states
    for (auto& [id, member] : m_members) {
        member.isReady = false;
    }

    notifyMembersOfChange();

    if (m_debugMode) {
        std::cout << "[PartySystem] Initiated ready check" << std::endl;
    }
}

void PartySystem::cancelReadyCheck() {
    m_readyCheckActive = false;

    if (m_debugMode) {
        std::cout << "[PartySystem] Cancelled ready check" << std::endl;
    }
}

void PartySystem::updateSettings(const PartySettings& settings) {
    if (!isLeader()) {
        std::cerr << "[PartySystem] Only leader can update settings" << std::endl;
        return;
    }

    m_settings = settings;
    notifyMembersOfChange();
}

void PartySystem::setLootMode(PartySettings::LootMode mode) {
    if (isLeader()) {
        m_settings.lootMode = mode;
        notifyMembersOfChange();
    }
}

void PartySystem::setExperienceShare(PartySettings::ExperienceShare mode) {
    if (isLeader()) {
        m_settings.expShare = mode;
        notifyMembersOfChange();
    }
}

void PartySystem::setMaxMembers(int max) {
    if (isLeader()) {
        m_settings.maxMembers = std::min(max, MAX_PARTY_SIZE);
        notifyMembersOfChange();
    }
}

void PartySystem::shareQuest(const std::string& questId) {
    if (!isInParty()) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::QUEST_SHARE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();
    packet.data.assign(questId.begin(), questId.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_debugMode) {
        std::cout << "[PartySystem] Shared quest: " << questId << std::endl;
    }
}

void PartySystem::sendPartyMessage(const std::string& message) {
    if (!isInParty()) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::CHAT_PARTY;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();
    packet.data.assign(message.begin(), message.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);
}

void PartySystem::setWaypoint(float x, float y, float z) {
    m_hasWaypoint = true;
    m_waypointX = x;
    m_waypointY = y;
    m_waypointZ = z;

    notifyMembersOfChange();
}

void PartySystem::clearWaypoint() {
    m_hasWaypoint = false;
}

void PartySystem::update(float deltaTime) {
    // Clear expired invites
    clearExpiredInvites();

    // Check ready check timeout
    if (m_readyCheckActive) {
        auto elapsed = std::chrono::system_clock::now() - m_readyCheckStartTime;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > m_readyCheckTimeout) {
            cancelReadyCheck();
        }
    }

    // Process loot rolls
    distributeLoot();
}

void PartySystem::distributeLoot() {
    for (auto& [lootId, drop] : m_lootDrops) {
        if (!drop.distributed) {
            processLootRolls(drop);
        }
    }
}

void PartySystem::processLootRolls(LootDrop& drop) {
    auto elapsed = std::chrono::system_clock::now() - drop.dropTime;
    if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() < LOOT_ROLL_TIMEOUT) {
        return; // Still waiting for rolls
    }

    // Distribute items based on rolls
    for (const auto& [itemId, rolls] : drop.rolls) {
        if (!rolls.empty()) {
            uint16_t winner = determineWinner(rolls);
            if (m_debugMode) {
                std::cout << "[PartySystem] Player " << winner << " won item " << itemId << std::endl;
            }
        }
    }

    drop.distributed = true;
}

uint16_t PartySystem::determineWinner(const std::vector<LootRoll>& rolls) {
    // Separate need and greed rolls
    std::vector<LootRoll> needs, greeds;

    for (const auto& roll : rolls) {
        if (roll.rollType == 1) {
            needs.push_back(roll);
        } else if (roll.rollType == 2) {
            greeds.push_back(roll);
        }
    }

    // Need rolls take priority
    const auto& winningRolls = needs.empty() ? greeds : needs;

    if (winningRolls.empty()) {
        return 0;
    }

    // Find highest roll
    auto winner = std::max_element(winningRolls.begin(), winningRolls.end(),
        [](const LootRoll& a, const LootRoll& b) {
            return a.rollValue < b.rollValue;
        });

    return winner->playerId;
}

void PartySystem::handlePartyInvite(const NetworkPacket& packet) {
    PartyInvite invite;
    invite.fromPlayerId = packet.playerId;
    invite.sentTime = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(packet.timestamp));
    invite.expiryTime = invite.sentTime + std::chrono::seconds(static_cast<int>(INVITE_TIMEOUT_SECONDS));

    m_pendingInvites.push_back(invite);

    if (m_onInviteReceived) {
        m_onInviteReceived(invite);
    }

    if (m_debugMode) {
        std::cout << "[PartySystem] Received invite from player " << packet.playerId << std::endl;
    }
}

void PartySystem::handlePartyAccept(const NetworkPacket& packet) {
    // Player accepted our invite
    PartyMember member;
    member.playerId = packet.playerId;
    member.playerName = "Player" + std::to_string(packet.playerId);
    member.isOnline = true;
    member.joinedTime = std::chrono::system_clock::now();

    addMember(member);
}

void PartySystem::handlePartyDecline(const NetworkPacket& packet) {
    if (m_debugMode) {
        std::cout << "[PartySystem] Player " << packet.playerId << " declined invite" << std::endl;
    }
}

void PartySystem::handlePartyLeave(const NetworkPacket& packet) {
    removeMember(packet.playerId);
}

void PartySystem::handlePartyKick(const NetworkPacket& packet) {
    if (packet.playerId == m_networkManager.getLocalPlayerId()) {
        // We were kicked
        m_currentPartyId = 0;
        m_members.clear();

        if (m_onMemberLeave) {
            m_onMemberLeave(packet.playerId, "KICKED");
        }
    } else {
        removeMember(packet.playerId);
    }
}

void PartySystem::handlePartyUpdate(const NetworkPacket& packet) {
    // Update party state from server
}

void PartySystem::addMember(const PartyMember& member) {
    m_members[member.playerId] = member;

    if (m_onMemberJoin) {
        m_onMemberJoin(member.playerId, member.playerName);
    }

    if (m_debugMode) {
        std::cout << "[PartySystem] Player " << member.playerId << " joined party" << std::endl;
    }
}

void PartySystem::removeMember(uint16_t playerId) {
    auto it = m_members.find(playerId);
    if (it != m_members.end()) {
        if (m_onMemberLeave) {
            m_onMemberLeave(playerId, "LEFT");
        }

        m_members.erase(it);

        if (m_debugMode) {
            std::cout << "[PartySystem] Player " << playerId << " left party" << std::endl;
        }
    }
}

void PartySystem::notifyMembersOfChange() {
    NetworkPacket packet;
    packet.type = PacketType::PARTY_UPDATE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

bool PartySystem::canInvite(uint16_t playerId) const {
    return isInParty() && getMemberCount() < m_settings.maxMembers &&
           m_members.find(playerId) == m_members.end();
}

bool PartySystem::canJoin(uint16_t playerId) const {
    return getMemberCount() < m_settings.maxMembers;
}

float PartySystem::getPartyDuration() const {
    if (!isInParty()) {
        return 0.0f;
    }

    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_createdTime);
    return static_cast<float>(duration.count());
}

void PartySystem::logPartyInfo() {
    std::cout << "\n===== Party Information =====" << std::endl;
    std::cout << "Party ID: " << m_currentPartyId << std::endl;
    std::cout << "Party Name: " << m_settings.partyName << std::endl;
    std::cout << "Leader: " << m_leaderId << std::endl;
    std::cout << "Members: " << getMemberCount() << "/" << getMaxMembers() << std::endl;
    std::cout << "Duration: " << getPartyDuration() << " seconds" << std::endl;

    std::cout << "\nMembers:" << std::endl;
    for (const auto& [id, member] : m_members) {
        std::cout << "  " << member.playerName << " (ID: " << id << ")"
                  << " [" << member.characterClass << " Lvl " << member.level << "]"
                  << (member.isLeader ? " (Leader)" : "")
                  << (member.isReady ? " (Ready)" : "")
                  << std::endl;
    }

    std::cout << "============================\n" << std::endl;
}

} // namespace Network
