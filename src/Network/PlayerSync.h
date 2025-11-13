#pragma once

#include "NetworkManager.h"
#include "../RPG/CharacterStats.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <chrono>

namespace Network {

/**
 * @brief Player state snapshot for network synchronization
 */
struct PlayerState {
    uint16_t playerId;
    uint64_t timestamp;

    // Transform
    float posX, posY, posZ;
    float rotX, rotY, rotZ;
    float velocityX, velocityY, velocityZ;

    // Animation
    std::string currentAnimation;
    float animationTime;

    // Combat
    float health;
    float maxHealth;
    float mana;
    float maxMana;
    float stamina;
    float maxStamina;

    // Status
    bool isAlive;
    bool isMoving;
    bool isInCombat;
    bool isCasting;

    // Equipment (simplified - just IDs)
    uint32_t weaponId;
    uint32_t armorId;

    PlayerState()
        : playerId(0), timestamp(0)
        , posX(0), posY(0), posZ(0)
        , rotX(0), rotY(0), rotZ(0)
        , velocityX(0), velocityY(0), velocityZ(0)
        , animationTime(0)
        , health(100), maxHealth(100)
        , mana(100), maxMana(100)
        , stamina(100), maxStamina(100)
        , isAlive(true), isMoving(false)
        , isInCombat(false), isCasting(false)
        , weaponId(0), armorId(0) {}

    // Serialization
    std::vector<uint8_t> serialize() const;
    static PlayerState deserialize(const std::vector<uint8_t>& data);
};

/**
 * @brief Player action for network transmission
 */
struct PlayerAction {
    enum class ActionType {
        MOVE,
        ATTACK,
        USE_SKILL,
        USE_ITEM,
        INTERACT,
        JUMP,
        DODGE,
        BLOCK,
        CAST_SPELL,
        EMOTE
    };

    uint16_t playerId;
    ActionType type;
    uint64_t timestamp;

    // Action parameters
    float targetX, targetY, targetZ;
    uint32_t targetEntityId;
    uint32_t skillId;
    uint32_t itemId;
    std::string actionData;

    PlayerAction()
        : playerId(0), type(ActionType::MOVE), timestamp(0)
        , targetX(0), targetY(0), targetZ(0)
        , targetEntityId(0), skillId(0), itemId(0) {}

    std::vector<uint8_t> serialize() const;
    static PlayerAction deserialize(const std::vector<uint8_t>& data);
};

/**
 * @brief Remote player representation
 */
struct RemotePlayer {
    uint16_t playerId;
    std::string playerName;
    int level;
    std::string characterClass;

    PlayerState currentState;
    PlayerState previousState;

    // Interpolation
    float interpolationProgress;
    std::chrono::steady_clock::time_point lastUpdateTime;

    // Prediction
    PlayerState predictedState;
    bool usingPrediction;

    // Stats
    uint32_t packetsReceived;
    uint32_t packetsLost;
    float averagePing;

    RemotePlayer()
        : playerId(0), level(1)
        , interpolationProgress(0)
        , usingPrediction(false)
        , packetsReceived(0), packetsLost(0)
        , averagePing(0) {}
};

/**
 * @brief Handles player synchronization across the network
 *
 * Implements client-side prediction, server reconciliation, and entity interpolation
 * for smooth multiplayer gameplay with lag compensation.
 */
class PlayerSync {
public:
    static PlayerSync& getInstance() {
        static PlayerSync instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Local player state
    void setLocalPlayer(uint16_t playerId);
    uint16_t getLocalPlayerId() const { return m_localPlayerId; }
    void updateLocalState(const PlayerState& state);
    PlayerState getLocalState() const { return m_localState; }

    // Send updates
    void sendPositionUpdate();
    void sendActionUpdate(const PlayerAction& action);
    void sendStatsUpdate();
    void sendFullStateUpdate();

    // Remote players
    std::vector<RemotePlayer> getRemotePlayers() const;
    RemotePlayer* getRemotePlayer(uint16_t playerId);
    bool hasRemotePlayer(uint16_t playerId) const;
    int getRemotePlayerCount() const;

    // Update
    void update(float deltaTime);

    // Interpolation settings
    void setInterpolationDelay(float seconds) { m_interpolationDelay = seconds; }
    float getInterpolationDelay() const { return m_interpolationDelay; }
    void enableInterpolation(bool enable) { m_interpolationEnabled = enable; }
    bool isInterpolationEnabled() const { return m_interpolationEnabled; }

    // Prediction settings
    void enablePrediction(bool enable) { m_predictionEnabled = enable; }
    bool isPredictionEnabled() const { return m_predictionEnabled; }
    void setPredictionTime(float seconds) { m_predictionTime = seconds; }

    // Lag compensation
    void enableLagCompensation(bool enable) { m_lagCompensationEnabled = enable; }
    bool isLagCompensationEnabled() const { return m_lagCompensationEnabled; }
    PlayerState getHistoricalState(uint16_t playerId, uint64_t timestamp) const;

    // Update frequency
    void setPositionUpdateRate(float updatesPerSecond);
    void setStatsUpdateRate(float updatesPerSecond);
    float getPositionUpdateRate() const { return m_positionUpdateRate; }
    float getStatsUpdateRate() const { return m_statsUpdateRate; }

    // Callbacks
    using PlayerJoinCallback = std::function<void(uint16_t playerId, const std::string& name)>;
    using PlayerLeaveCallback = std::function<void(uint16_t playerId)>;
    using PlayerActionCallback = std::function<void(const PlayerAction& action)>;
    using PlayerStateCallback = std::function<void(uint16_t playerId, const PlayerState& state)>;

    void setOnPlayerJoin(PlayerJoinCallback callback) { m_onPlayerJoin = callback; }
    void setOnPlayerLeave(PlayerLeaveCallback callback) { m_onPlayerLeave = callback; }
    void setOnPlayerAction(PlayerActionCallback callback) { m_onPlayerAction = callback; }
    void setOnPlayerStateUpdate(PlayerStateCallback callback) { m_onPlayerStateUpdate = callback; }

    // Dead reckoning for movement prediction
    void enableDeadReckoning(bool enable) { m_deadReckoningEnabled = enable; }
    bool isDeadReckoningEnabled() const { return m_deadReckoningEnabled; }

    // Snapshot management (for lag compensation)
    void storeSnapshot(uint16_t playerId, const PlayerState& state);
    std::vector<PlayerState> getSnapshotHistory(uint16_t playerId) const;
    void clearSnapshotHistory(uint16_t playerId);

    // Bandwidth optimization
    void enableDeltaCompression(bool enable) { m_deltaCompressionEnabled = enable; }
    bool isDeltaCompressionEnabled() const { return m_deltaCompressionEnabled; }
    void setUpdateThreshold(float threshold) { m_updateThreshold = threshold; }

    // Priority system (prioritize nearby players)
    void setPriorityDistance(float distance) { m_priorityDistance = distance; }
    void updatePlayerPriorities(float playerX, float playerY, float playerZ);

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }
    void logSyncStats();

private:
    PlayerSync();
    ~PlayerSync();
    PlayerSync(const PlayerSync&) = delete;
    PlayerSync& operator=(const PlayerSync&) = delete;

    // Packet handlers
    void handlePlayerPosition(const NetworkPacket& packet);
    void handlePlayerAction(const NetworkPacket& packet);
    void handlePlayerStats(const NetworkPacket& packet);
    void handlePlayerSpawn(const NetworkPacket& packet);
    void handlePlayerDespawn(const NetworkPacket& packet);
    void handlePlayerAnimation(const NetworkPacket& packet);

    // Interpolation
    void interpolateRemotePlayers(float deltaTime);
    PlayerState interpolateStates(const PlayerState& from, const PlayerState& to, float t) const;

    // Prediction
    void predictRemotePlayer(RemotePlayer& player, float deltaTime);
    PlayerState predictState(const PlayerState& current, float deltaTime) const;

    // Dead reckoning
    PlayerState applyDeadReckoning(const PlayerState& state, float deltaTime) const;

    // Delta compression
    std::vector<uint8_t> createDeltaUpdate(const PlayerState& current, const PlayerState& previous);
    PlayerState applyDeltaUpdate(const PlayerState& base, const std::vector<uint8_t>& delta);

    // Snapshot history management
    void addToHistory(uint16_t playerId, const PlayerState& state);
    void pruneOldSnapshots();

    // Remote player management
    void addRemotePlayer(uint16_t playerId, const std::string& name);
    void removeRemotePlayer(uint16_t playerId);
    void updateRemotePlayer(uint16_t playerId, const PlayerState& state);

    // Data members
    uint16_t m_localPlayerId;
    PlayerState m_localState;
    PlayerState m_previousLocalState;

    std::unordered_map<uint16_t, RemotePlayer> m_remotePlayers;
    std::unordered_map<uint16_t, std::vector<PlayerState>> m_stateHistory;

    // Update timing
    float m_positionUpdateTimer;
    float m_statsUpdateTimer;
    float m_positionUpdateRate;     // Updates per second
    float m_statsUpdateRate;        // Updates per second

    // Interpolation
    bool m_interpolationEnabled;
    float m_interpolationDelay;

    // Prediction
    bool m_predictionEnabled;
    float m_predictionTime;

    // Lag compensation
    bool m_lagCompensationEnabled;
    int m_snapshotHistorySize;

    // Dead reckoning
    bool m_deadReckoningEnabled;

    // Delta compression
    bool m_deltaCompressionEnabled;
    float m_updateThreshold;

    // Priority system
    float m_priorityDistance;
    std::unordered_map<uint16_t, float> m_playerPriorities;

    // Callbacks
    PlayerJoinCallback m_onPlayerJoin;
    PlayerLeaveCallback m_onPlayerLeave;
    PlayerActionCallback m_onPlayerAction;
    PlayerStateCallback m_onPlayerStateUpdate;

    // Network reference
    NetworkManager& m_networkManager;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr float DEFAULT_POSITION_UPDATE_RATE = 20.0f;  // 20 updates/sec
    static constexpr float DEFAULT_STATS_UPDATE_RATE = 5.0f;      // 5 updates/sec
    static constexpr float DEFAULT_INTERPOLATION_DELAY = 0.1f;    // 100ms
    static constexpr float DEFAULT_PREDICTION_TIME = 0.05f;       // 50ms
    static constexpr int MAX_SNAPSHOT_HISTORY = 60;               // ~3 seconds at 20Hz
    static constexpr float DEFAULT_UPDATE_THRESHOLD = 0.01f;      // 1cm movement threshold
    static constexpr float DEFAULT_PRIORITY_DISTANCE = 50.0f;     // 50 units
};

} // namespace Network
