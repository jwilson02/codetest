#include "PlayerSync.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Network {

// PlayerState serialization

std::vector<uint8_t> PlayerState::serialize() const {
    std::vector<uint8_t> data;
    data.reserve(128); // Reserve estimated size

    // Helper lambda to write floats
    auto writeFloat = [&data](float value) {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(float));
        for (int i = 0; i < 4; i++) {
            data.push_back((bits >> (i * 8)) & 0xFF);
        }
    };

    // Player ID (2 bytes)
    data.push_back(playerId & 0xFF);
    data.push_back((playerId >> 8) & 0xFF);

    // Timestamp (8 bytes)
    for (int i = 0; i < 8; i++) {
        data.push_back((timestamp >> (i * 8)) & 0xFF);
    }

    // Position (12 bytes)
    writeFloat(posX);
    writeFloat(posY);
    writeFloat(posZ);

    // Rotation (12 bytes)
    writeFloat(rotX);
    writeFloat(rotY);
    writeFloat(rotZ);

    // Velocity (12 bytes)
    writeFloat(velocityX);
    writeFloat(velocityY);
    writeFloat(velocityZ);

    // Health/Mana/Stamina (24 bytes)
    writeFloat(health);
    writeFloat(maxHealth);
    writeFloat(mana);
    writeFloat(maxMana);
    writeFloat(stamina);
    writeFloat(maxStamina);

    // Status flags (1 byte)
    uint8_t flags = 0;
    if (isAlive) flags |= 0x01;
    if (isMoving) flags |= 0x02;
    if (isInCombat) flags |= 0x04;
    if (isCasting) flags |= 0x08;
    data.push_back(flags);

    // Equipment IDs (8 bytes)
    for (int i = 0; i < 4; i++) {
        data.push_back((weaponId >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        data.push_back((armorId >> (i * 8)) & 0xFF);
    }

    // Animation name length and data
    uint16_t animLen = static_cast<uint16_t>(currentAnimation.length());
    data.push_back(animLen & 0xFF);
    data.push_back((animLen >> 8) & 0xFF);
    data.insert(data.end(), currentAnimation.begin(), currentAnimation.end());

    // Animation time
    writeFloat(animationTime);

    return data;
}

PlayerState PlayerState::deserialize(const std::vector<uint8_t>& data) {
    PlayerState state;

    if (data.size() < 70) {
        return state;
    }

    size_t offset = 0;

    auto readFloat = [&data, &offset]() -> float {
        uint32_t bits = 0;
        for (int i = 0; i < 4; i++) {
            bits |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
        }
        float value;
        std::memcpy(&value, &bits, sizeof(float));
        return value;
    };

    // Player ID
    state.playerId = data[offset] | (data[offset + 1] << 8);
    offset += 2;

    // Timestamp
    for (int i = 0; i < 8; i++) {
        state.timestamp |= (static_cast<uint64_t>(data[offset++]) << (i * 8));
    }

    // Position
    state.posX = readFloat();
    state.posY = readFloat();
    state.posZ = readFloat();

    // Rotation
    state.rotX = readFloat();
    state.rotY = readFloat();
    state.rotZ = readFloat();

    // Velocity
    state.velocityX = readFloat();
    state.velocityY = readFloat();
    state.velocityZ = readFloat();

    // Health/Mana/Stamina
    state.health = readFloat();
    state.maxHealth = readFloat();
    state.mana = readFloat();
    state.maxMana = readFloat();
    state.stamina = readFloat();
    state.maxStamina = readFloat();

    // Status flags
    uint8_t flags = data[offset++];
    state.isAlive = (flags & 0x01) != 0;
    state.isMoving = (flags & 0x02) != 0;
    state.isInCombat = (flags & 0x04) != 0;
    state.isCasting = (flags & 0x08) != 0;

    // Equipment IDs
    state.weaponId = 0;
    for (int i = 0; i < 4; i++) {
        state.weaponId |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
    }
    state.armorId = 0;
    for (int i = 0; i < 4; i++) {
        state.armorId |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
    }

    // Animation name
    uint16_t animLen = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    if (offset + animLen <= data.size()) {
        state.currentAnimation.assign(data.begin() + offset, data.begin() + offset + animLen);
        offset += animLen;
    }

    // Animation time
    if (offset + 4 <= data.size()) {
        state.animationTime = readFloat();
    }

    return state;
}

// PlayerAction serialization

std::vector<uint8_t> PlayerAction::serialize() const {
    std::vector<uint8_t> data;

    auto writeFloat = [&data](float value) {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(float));
        for (int i = 0; i < 4; i++) {
            data.push_back((bits >> (i * 8)) & 0xFF);
        }
    };

    // Player ID
    data.push_back(playerId & 0xFF);
    data.push_back((playerId >> 8) & 0xFF);

    // Action type
    data.push_back(static_cast<uint8_t>(type));

    // Timestamp
    for (int i = 0; i < 8; i++) {
        data.push_back((timestamp >> (i * 8)) & 0xFF);
    }

    // Target position
    writeFloat(targetX);
    writeFloat(targetY);
    writeFloat(targetZ);

    // IDs
    for (int i = 0; i < 4; i++) {
        data.push_back((targetEntityId >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        data.push_back((skillId >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        data.push_back((itemId >> (i * 8)) & 0xFF);
    }

    // Action data
    uint16_t dataLen = static_cast<uint16_t>(actionData.length());
    data.push_back(dataLen & 0xFF);
    data.push_back((dataLen >> 8) & 0xFF);
    data.insert(data.end(), actionData.begin(), actionData.end());

    return data;
}

PlayerAction PlayerAction::deserialize(const std::vector<uint8_t>& data) {
    PlayerAction action;

    if (data.size() < 35) {
        return action;
    }

    size_t offset = 0;

    auto readFloat = [&data, &offset]() -> float {
        uint32_t bits = 0;
        for (int i = 0; i < 4; i++) {
            bits |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
        }
        float value;
        std::memcpy(&value, &bits, sizeof(float));
        return value;
    };

    action.playerId = data[offset] | (data[offset + 1] << 8);
    offset += 2;

    action.type = static_cast<PlayerAction::ActionType>(data[offset++]);

    for (int i = 0; i < 8; i++) {
        action.timestamp |= (static_cast<uint64_t>(data[offset++]) << (i * 8));
    }

    action.targetX = readFloat();
    action.targetY = readFloat();
    action.targetZ = readFloat();

    for (int i = 0; i < 4; i++) {
        action.targetEntityId |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
    }
    for (int i = 0; i < 4; i++) {
        action.skillId |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
    }
    for (int i = 0; i < 4; i++) {
        action.itemId |= (static_cast<uint32_t>(data[offset++]) << (i * 8));
    }

    uint16_t dataLen = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    if (offset + dataLen <= data.size()) {
        action.actionData.assign(data.begin() + offset, data.begin() + offset + dataLen);
    }

    return action;
}

// PlayerSync implementation

PlayerSync::PlayerSync()
    : m_localPlayerId(0)
    , m_positionUpdateTimer(0)
    , m_statsUpdateTimer(0)
    , m_positionUpdateRate(DEFAULT_POSITION_UPDATE_RATE)
    , m_statsUpdateRate(DEFAULT_STATS_UPDATE_RATE)
    , m_interpolationEnabled(true)
    , m_interpolationDelay(DEFAULT_INTERPOLATION_DELAY)
    , m_predictionEnabled(true)
    , m_predictionTime(DEFAULT_PREDICTION_TIME)
    , m_lagCompensationEnabled(true)
    , m_snapshotHistorySize(MAX_SNAPSHOT_HISTORY)
    , m_deadReckoningEnabled(true)
    , m_deltaCompressionEnabled(true)
    , m_updateThreshold(DEFAULT_UPDATE_THRESHOLD)
    , m_priorityDistance(DEFAULT_PRIORITY_DISTANCE)
    , m_networkManager(NetworkManager::getInstance())
    , m_debugMode(false)
{
}

PlayerSync::~PlayerSync() {
    shutdown();
}

void PlayerSync::initialize() {
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::PLAYER_POSITION,
        [this](const NetworkPacket& packet) { handlePlayerPosition(packet); });

    m_networkManager.registerPacketHandler(PacketType::PLAYER_ACTION,
        [this](const NetworkPacket& packet) { handlePlayerAction(packet); });

    m_networkManager.registerPacketHandler(PacketType::PLAYER_STATS,
        [this](const NetworkPacket& packet) { handlePlayerStats(packet); });

    m_networkManager.registerPacketHandler(PacketType::PLAYER_ANIMATION,
        [this](const NetworkPacket& packet) { handlePlayerAnimation(packet); });

    m_networkManager.registerPacketHandler(PacketType::ENTITY_SPAWN,
        [this](const NetworkPacket& packet) { handlePlayerSpawn(packet); });

    m_networkManager.registerPacketHandler(PacketType::ENTITY_DESPAWN,
        [this](const NetworkPacket& packet) { handlePlayerDespawn(packet); });

    std::cout << "[PlayerSync] Initialized" << std::endl;
}

void PlayerSync::shutdown() {
    m_remotePlayers.clear();
    m_stateHistory.clear();
}

void PlayerSync::setLocalPlayer(uint16_t playerId) {
    m_localPlayerId = playerId;
    m_localState.playerId = playerId;

    if (m_debugMode) {
        std::cout << "[PlayerSync] Local player ID set to " << playerId << std::endl;
    }
}

void PlayerSync::updateLocalState(const PlayerState& state) {
    m_previousLocalState = m_localState;
    m_localState = state;
    m_localState.playerId = m_localPlayerId;
    m_localState.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

void PlayerSync::sendPositionUpdate() {
    NetworkPacket packet;
    packet.type = PacketType::PLAYER_POSITION;
    packet.timestamp = m_localState.timestamp;
    packet.playerId = m_localPlayerId;

    if (m_deltaCompressionEnabled) {
        packet.data = createDeltaUpdate(m_localState, m_previousLocalState);
    } else {
        packet.data = m_localState.serialize();
    }

    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);
}

void PlayerSync::sendActionUpdate(const PlayerAction& action) {
    NetworkPacket packet;
    packet.type = PacketType::PLAYER_ACTION;
    packet.timestamp = action.timestamp;
    packet.playerId = action.playerId;
    packet.data = action.serialize();
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);

    if (m_debugMode) {
        std::cout << "[PlayerSync] Sent action: " << static_cast<int>(action.type) << std::endl;
    }
}

void PlayerSync::sendStatsUpdate() {
    NetworkPacket packet;
    packet.type = PacketType::PLAYER_STATS;
    packet.timestamp = m_localState.timestamp;
    packet.playerId = m_localPlayerId;
    packet.data = m_localState.serialize();
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

void PlayerSync::sendFullStateUpdate() {
    NetworkPacket packet;
    packet.type = PacketType::PLAYER_POSITION;
    packet.timestamp = m_localState.timestamp;
    packet.playerId = m_localPlayerId;
    packet.data = m_localState.serialize();
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

std::vector<RemotePlayer> PlayerSync::getRemotePlayers() const {
    std::vector<RemotePlayer> players;
    players.reserve(m_remotePlayers.size());

    for (const auto& [id, player] : m_remotePlayers) {
        players.push_back(player);
    }

    return players;
}

RemotePlayer* PlayerSync::getRemotePlayer(uint16_t playerId) {
    auto it = m_remotePlayers.find(playerId);
    return (it != m_remotePlayers.end()) ? &it->second : nullptr;
}

bool PlayerSync::hasRemotePlayer(uint16_t playerId) const {
    return m_remotePlayers.find(playerId) != m_remotePlayers.end();
}

int PlayerSync::getRemotePlayerCount() const {
    return static_cast<int>(m_remotePlayers.size());
}

void PlayerSync::update(float deltaTime) {
    // Update position send timer
    m_positionUpdateTimer += deltaTime;
    if (m_positionUpdateTimer >= (1.0f / m_positionUpdateRate)) {
        m_positionUpdateTimer = 0.0f;

        // Check if position changed significantly
        float dx = m_localState.posX - m_previousLocalState.posX;
        float dy = m_localState.posY - m_previousLocalState.posY;
        float dz = m_localState.posZ - m_previousLocalState.posZ;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq > m_updateThreshold * m_updateThreshold) {
            sendPositionUpdate();
        }
    }

    // Update stats send timer
    m_statsUpdateTimer += deltaTime;
    if (m_statsUpdateTimer >= (1.0f / m_statsUpdateRate)) {
        m_statsUpdateTimer = 0.0f;
        sendStatsUpdate();
    }

    // Interpolate remote players
    if (m_interpolationEnabled) {
        interpolateRemotePlayers(deltaTime);
    }

    // Apply prediction to remote players
    if (m_predictionEnabled) {
        for (auto& [id, player] : m_remotePlayers) {
            predictRemotePlayer(player, deltaTime);
        }
    }

    // Prune old snapshots
    if (m_lagCompensationEnabled) {
        pruneOldSnapshots();
    }
}

void PlayerSync::handlePlayerPosition(const NetworkPacket& packet) {
    if (packet.playerId == m_localPlayerId) {
        return; // Ignore our own packets
    }

    PlayerState state;

    if (m_deltaCompressionEnabled && hasRemotePlayer(packet.playerId)) {
        RemotePlayer* player = getRemotePlayer(packet.playerId);
        state = applyDeltaUpdate(player->currentState, packet.data);
    } else {
        state = PlayerState::deserialize(packet.data);
    }

    updateRemotePlayer(packet.playerId, state);

    if (m_lagCompensationEnabled) {
        addToHistory(packet.playerId, state);
    }

    if (m_onPlayerStateUpdate) {
        m_onPlayerStateUpdate(packet.playerId, state);
    }
}

void PlayerSync::handlePlayerAction(const NetworkPacket& packet) {
    PlayerAction action = PlayerAction::deserialize(packet.data);

    if (m_debugMode) {
        std::cout << "[PlayerSync] Received action from player " << action.playerId
                  << ": " << static_cast<int>(action.type) << std::endl;
    }

    if (m_onPlayerAction) {
        m_onPlayerAction(action);
    }
}

void PlayerSync::handlePlayerStats(const NetworkPacket& packet) {
    PlayerState state = PlayerState::deserialize(packet.data);
    updateRemotePlayer(packet.playerId, state);
}

void PlayerSync::handlePlayerSpawn(const NetworkPacket& packet) {
    // Parse player info from packet
    std::string playerName(packet.data.begin(), packet.data.end());
    addRemotePlayer(packet.playerId, playerName);

    if (m_debugMode) {
        std::cout << "[PlayerSync] Player " << packet.playerId << " spawned: " << playerName << std::endl;
    }

    if (m_onPlayerJoin) {
        m_onPlayerJoin(packet.playerId, playerName);
    }
}

void PlayerSync::handlePlayerDespawn(const NetworkPacket& packet) {
    removeRemotePlayer(packet.playerId);

    if (m_debugMode) {
        std::cout << "[PlayerSync] Player " << packet.playerId << " despawned" << std::endl;
    }

    if (m_onPlayerLeave) {
        m_onPlayerLeave(packet.playerId);
    }
}

void PlayerSync::handlePlayerAnimation(const NetworkPacket& packet) {
    RemotePlayer* player = getRemotePlayer(packet.playerId);
    if (player) {
        // Parse animation data
        std::string animName(packet.data.begin(), packet.data.end());
        player->currentState.currentAnimation = animName;
    }
}

void PlayerSync::interpolateRemotePlayers(float deltaTime) {
    for (auto& [id, player] : m_remotePlayers) {
        player.interpolationProgress += deltaTime / m_interpolationDelay;

        if (player.interpolationProgress >= 1.0f) {
            player.interpolationProgress = 1.0f;
        }

        // Interpolate between previous and current state
        PlayerState interpolated = interpolateStates(
            player.previousState,
            player.currentState,
            player.interpolationProgress
        );

        // Update the player's rendered state (would be used by rendering system)
        if (m_predictionEnabled) {
            player.predictedState = interpolated;
        }
    }
}

PlayerState PlayerSync::interpolateStates(const PlayerState& from, const PlayerState& to, float t) const {
    PlayerState result;

    result.playerId = to.playerId;
    result.timestamp = to.timestamp;

    // Linear interpolation for position
    result.posX = from.posX + (to.posX - from.posX) * t;
    result.posY = from.posY + (to.posY - from.posY) * t;
    result.posZ = from.posZ + (to.posZ - from.posZ) * t;

    // Linear interpolation for rotation
    result.rotX = from.rotX + (to.rotX - from.rotX) * t;
    result.rotY = from.rotY + (to.rotY - from.rotY) * t;
    result.rotZ = from.rotZ + (to.rotZ - from.rotZ) * t;

    // Linear interpolation for velocity
    result.velocityX = from.velocityX + (to.velocityX - from.velocityX) * t;
    result.velocityY = from.velocityY + (to.velocityY - from.velocityY) * t;
    result.velocityZ = from.velocityZ + (to.velocityZ - from.velocityZ) * t;

    // Linear interpolation for health/mana/stamina
    result.health = from.health + (to.health - from.health) * t;
    result.mana = from.mana + (to.mana - from.mana) * t;
    result.stamina = from.stamina + (to.stamina - from.stamina) * t;

    // Copy other fields from target state
    result.maxHealth = to.maxHealth;
    result.maxMana = to.maxMana;
    result.maxStamina = to.maxStamina;
    result.isAlive = to.isAlive;
    result.isMoving = to.isMoving;
    result.isInCombat = to.isInCombat;
    result.isCasting = to.isCasting;
    result.weaponId = to.weaponId;
    result.armorId = to.armorId;
    result.currentAnimation = to.currentAnimation;
    result.animationTime = from.animationTime + (to.animationTime - from.animationTime) * t;

    return result;
}

void PlayerSync::predictRemotePlayer(RemotePlayer& player, float deltaTime) {
    if (!player.usingPrediction) {
        return;
    }

    player.predictedState = predictState(player.currentState, m_predictionTime);
}

PlayerState PlayerSync::predictState(const PlayerState& current, float deltaTime) const {
    PlayerState predicted = current;

    if (m_deadReckoningEnabled) {
        predicted = applyDeadReckoning(current, deltaTime);
    }

    return predicted;
}

PlayerState PlayerSync::applyDeadReckoning(const PlayerState& state, float deltaTime) const {
    PlayerState predicted = state;

    // Apply velocity to position
    predicted.posX += state.velocityX * deltaTime;
    predicted.posY += state.velocityY * deltaTime;
    predicted.posZ += state.velocityZ * deltaTime;

    return predicted;
}

std::vector<uint8_t> PlayerSync::createDeltaUpdate(const PlayerState& current, const PlayerState& previous) {
    // Simplified delta compression - only send changed values
    std::vector<uint8_t> delta;

    // Flags to indicate which fields changed
    uint32_t changeFlags = 0;

    if (std::abs(current.posX - previous.posX) > m_updateThreshold) changeFlags |= 0x01;
    if (std::abs(current.posY - previous.posY) > m_updateThreshold) changeFlags |= 0x02;
    if (std::abs(current.posZ - previous.posZ) > m_updateThreshold) changeFlags |= 0x04;

    // Write change flags
    for (int i = 0; i < 4; i++) {
        delta.push_back((changeFlags >> (i * 8)) & 0xFF);
    }

    // Write only changed values
    // (Full implementation would write each changed field based on flags)

    // For now, just return full state if anything changed
    if (changeFlags != 0) {
        return current.serialize();
    }

    return delta;
}

PlayerState PlayerSync::applyDeltaUpdate(const PlayerState& base, const std::vector<uint8_t>& delta) {
    // For simplified implementation, just deserialize the full state
    return PlayerState::deserialize(delta);
}

void PlayerSync::addRemotePlayer(uint16_t playerId, const std::string& name) {
    if (hasRemotePlayer(playerId)) {
        return;
    }

    RemotePlayer player;
    player.playerId = playerId;
    player.playerName = name;
    player.lastUpdateTime = std::chrono::steady_clock::now();

    m_remotePlayers[playerId] = player;
}

void PlayerSync::removeRemotePlayer(uint16_t playerId) {
    m_remotePlayers.erase(playerId);
    m_stateHistory.erase(playerId);
    m_playerPriorities.erase(playerId);
}

void PlayerSync::updateRemotePlayer(uint16_t playerId, const PlayerState& state) {
    if (!hasRemotePlayer(playerId)) {
        addRemotePlayer(playerId, "Player" + std::to_string(playerId));
    }

    RemotePlayer& player = m_remotePlayers[playerId];
    player.previousState = player.currentState;
    player.currentState = state;
    player.lastUpdateTime = std::chrono::steady_clock::now();
    player.interpolationProgress = 0.0f;
    player.packetsReceived++;
}

void PlayerSync::addToHistory(uint16_t playerId, const PlayerState& state) {
    auto& history = m_stateHistory[playerId];
    history.push_back(state);

    // Limit history size
    if (history.size() > m_snapshotHistorySize) {
        history.erase(history.begin());
    }
}

void PlayerSync::pruneOldSnapshots() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (auto& [playerId, history] : m_stateHistory) {
        history.erase(
            std::remove_if(history.begin(), history.end(),
                [now](const PlayerState& state) {
                    return (now - state.timestamp) > 3000; // Remove snapshots older than 3 seconds
                }),
            history.end()
        );
    }
}

void PlayerSync::setPositionUpdateRate(float updatesPerSecond) {
    m_positionUpdateRate = std::max(1.0f, std::min(60.0f, updatesPerSecond));
}

void PlayerSync::setStatsUpdateRate(float updatesPerSecond) {
    m_statsUpdateRate = std::max(0.1f, std::min(10.0f, updatesPerSecond));
}

void PlayerSync::storeSnapshot(uint16_t playerId, const PlayerState& state) {
    addToHistory(playerId, state);
}

std::vector<PlayerState> PlayerSync::getSnapshotHistory(uint16_t playerId) const {
    auto it = m_stateHistory.find(playerId);
    if (it != m_stateHistory.end()) {
        return it->second;
    }
    return std::vector<PlayerState>();
}

void PlayerSync::clearSnapshotHistory(uint16_t playerId) {
    m_stateHistory.erase(playerId);
}

PlayerState PlayerSync::getHistoricalState(uint16_t playerId, uint64_t timestamp) const {
    auto it = m_stateHistory.find(playerId);
    if (it == m_stateHistory.end() || it->second.empty()) {
        return PlayerState();
    }

    const auto& history = it->second;

    // Find closest snapshot to requested timestamp
    auto closest = std::min_element(history.begin(), history.end(),
        [timestamp](const PlayerState& a, const PlayerState& b) {
            return std::abs(static_cast<int64_t>(a.timestamp - timestamp)) <
                   std::abs(static_cast<int64_t>(b.timestamp - timestamp));
        });

    return *closest;
}

void PlayerSync::updatePlayerPriorities(float playerX, float playerY, float playerZ) {
    for (auto& [id, player] : m_remotePlayers) {
        float dx = player.currentState.posX - playerX;
        float dy = player.currentState.posY - playerY;
        float dz = player.currentState.posZ - playerZ;
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        m_playerPriorities[id] = distance;
    }
}

void PlayerSync::logSyncStats() {
    std::cout << "\n===== Player Sync Statistics =====" << std::endl;
    std::cout << "Local Player ID: " << m_localPlayerId << std::endl;
    std::cout << "Remote Players: " << m_remotePlayers.size() << std::endl;
    std::cout << "Position Update Rate: " << m_positionUpdateRate << " Hz" << std::endl;
    std::cout << "Stats Update Rate: " << m_statsUpdateRate << " Hz" << std::endl;
    std::cout << "Interpolation: " << (m_interpolationEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "Prediction: " << (m_predictionEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "Lag Compensation: " << (m_lagCompensationEnabled ? "ON" : "OFF") << std::endl;

    std::cout << "\nRemote Players:" << std::endl;
    for (const auto& [id, player] : m_remotePlayers) {
        std::cout << "  Player " << id << " (" << player.playerName << ")" << std::endl;
        std::cout << "    Packets Received: " << player.packetsReceived << std::endl;
        std::cout << "    Packets Lost: " << player.packetsLost << std::endl;
        std::cout << "    Position: (" << player.currentState.posX << ", "
                  << player.currentState.posY << ", " << player.currentState.posZ << ")" << std::endl;
    }

    std::cout << "==================================\n" << std::endl;
}

} // namespace Network
