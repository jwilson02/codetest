#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

namespace Network {

// Network protocol types
enum class ProtocolType {
    TCP,  // Reliable, ordered - for important data
    UDP   // Fast, unreliable - for gameplay updates
};

// Network packet types
enum class PacketType : uint8_t {
    // Connection
    CONNECT_REQUEST = 0,
    CONNECT_RESPONSE = 1,
    DISCONNECT = 2,
    HEARTBEAT = 3,

    // Player sync
    PLAYER_POSITION = 10,
    PLAYER_ACTION = 11,
    PLAYER_STATS = 12,
    PLAYER_HEALTH = 13,
    PLAYER_ANIMATION = 14,

    // Party
    PARTY_INVITE = 20,
    PARTY_ACCEPT = 21,
    PARTY_DECLINE = 22,
    PARTY_LEAVE = 23,
    PARTY_KICK = 24,
    PARTY_UPDATE = 25,

    // Chat
    CHAT_MESSAGE = 30,
    CHAT_WHISPER = 31,
    CHAT_PARTY = 32,
    CHAT_GUILD = 33,

    // Combat
    COMBAT_ACTION = 40,
    COMBAT_DAMAGE = 41,
    COMBAT_HEAL = 42,
    COMBAT_BUFF = 43,
    COMBAT_DEBUFF = 44,

    // Trading
    TRADE_REQUEST = 50,
    TRADE_ACCEPT = 51,
    TRADE_DECLINE = 52,
    TRADE_ADD_ITEM = 53,
    TRADE_REMOVE_ITEM = 54,
    TRADE_CONFIRM = 55,
    TRADE_CANCEL = 56,

    // Quest
    QUEST_SYNC = 60,
    QUEST_UPDATE = 61,
    QUEST_COMPLETE = 62,
    QUEST_SHARE = 63,

    // World
    ENTITY_SPAWN = 70,
    ENTITY_DESPAWN = 71,
    WORLD_EVENT = 72,

    // Leaderboard
    LEADERBOARD_REQUEST = 80,
    LEADERBOARD_UPDATE = 81,

    // Friend system
    FRIEND_REQUEST = 90,
    FRIEND_ACCEPT = 91,
    FRIEND_DECLINE = 92,
    FRIEND_REMOVE = 93,
    FRIEND_STATUS = 94,

    // PvP
    PVP_CHALLENGE = 100,
    PVP_ACCEPT = 101,
    PVP_DECLINE = 102,
    PVP_RESULT = 103,

    // Misc
    PING = 254,
    PONG = 255
};

// Network packet structure
struct NetworkPacket {
    PacketType type;
    uint64_t timestamp;
    uint32_t sequenceNumber;
    uint16_t playerId;
    uint16_t dataSize;
    std::vector<uint8_t> data;

    NetworkPacket() : type(PacketType::HEARTBEAT), timestamp(0),
                     sequenceNumber(0), playerId(0), dataSize(0) {}

    // Serialize packet to bytes
    std::vector<uint8_t> serialize() const;

    // Deserialize bytes to packet
    static NetworkPacket deserialize(const std::vector<uint8_t>& bytes);
};

// Connection info
struct ConnectionInfo {
    uint16_t playerId;
    std::string playerName;
    std::string ipAddress;
    uint16_t port;
    uint64_t lastHeartbeat;
    uint32_t ping;
    bool isConnected;

    ConnectionInfo() : playerId(0), port(0), lastHeartbeat(0),
                      ping(0), isConnected(false) {}
};

// Network statistics
struct NetworkStats {
    uint64_t packetsSent;
    uint64_t packetsReceived;
    uint64_t bytessSent;
    uint64_t bytesReceived;
    uint64_t packetsLost;
    uint32_t averagePing;
    float packetLossRate;

    NetworkStats() : packetsSent(0), packetsReceived(0), bytessSent(0),
                    bytesReceived(0), packetsLost(0), averagePing(0),
                    packetLossRate(0.0f) {}
};

// Callback types
using PacketCallback = std::function<void(const NetworkPacket&)>;
using ConnectionCallback = std::function<void(uint16_t playerId, bool connected)>;
using ErrorCallback = std::function<void(const std::string& error)>;

/**
 * @brief Core network manager handling client-server architecture
 *
 * Manages UDP for fast gameplay updates and TCP for reliable important data.
 * Implements lag compensation, prediction, and robust error handling.
 */
class NetworkManager {
public:
    static NetworkManager& getInstance() {
        static NetworkManager instance;
        return instance;
    }

    // Initialization
    bool initialize(bool isServer = false, uint16_t tcpPort = 7777, uint16_t udpPort = 7778);
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    bool isServer() const { return m_isServer; }

    // Connection management
    bool connectToServer(const std::string& serverIP, uint16_t tcpPort, uint16_t udpPort);
    void disconnect();
    bool isConnected() const { return m_connected; }

    // Packet sending
    void sendPacket(const NetworkPacket& packet, ProtocolType protocol = ProtocolType::UDP);
    void sendPacketTo(uint16_t playerId, const NetworkPacket& packet, ProtocolType protocol = ProtocolType::UDP);
    void broadcastPacket(const NetworkPacket& packet, ProtocolType protocol = ProtocolType::UDP);
    void broadcastPacketExcept(uint16_t excludePlayerId, const NetworkPacket& packet, ProtocolType protocol = ProtocolType::UDP);

    // Packet handling
    void registerPacketHandler(PacketType type, PacketCallback callback);
    void unregisterPacketHandler(PacketType type);

    // Update loop
    void update(float deltaTime);

    // Player management
    uint16_t getLocalPlayerId() const { return m_localPlayerId; }
    std::vector<ConnectionInfo> getConnectedPlayers() const;
    ConnectionInfo getPlayerInfo(uint16_t playerId) const;
    int getPlayerCount() const { return static_cast<int>(m_connections.size()); }

    // Network statistics
    NetworkStats getNetworkStats() const { return m_stats; }
    uint32_t getPing() const;
    float getPacketLoss() const;

    // Lag compensation
    void enableLagCompensation(bool enable) { m_lagCompensationEnabled = enable; }
    bool isLagCompensationEnabled() const { return m_lagCompensationEnabled; }
    void setInterpolationDelay(float delay) { m_interpolationDelay = delay; }
    float getInterpolationDelay() const { return m_interpolationDelay; }

    // Prediction settings
    void enableClientPrediction(bool enable) { m_predictionEnabled = enable; }
    bool isClientPredictionEnabled() const { return m_predictionEnabled; }

    // Callbacks
    void setOnConnectionChanged(ConnectionCallback callback) { m_onConnectionChanged = callback; }
    void setOnError(ErrorCallback callback) { m_onError = callback; }

    // Server management (server-side only)
    void kickPlayer(uint16_t playerId, const std::string& reason = "");
    void banPlayer(const std::string& ipAddress);
    bool isPlayerBanned(const std::string& ipAddress) const;

    // Heartbeat
    void setHeartbeatInterval(float seconds) { m_heartbeatInterval = seconds; }
    float getHeartbeatInterval() const { return m_heartbeatInterval; }

    // Timeout settings
    void setConnectionTimeout(float seconds) { m_connectionTimeout = seconds; }
    float getConnectionTimeout() const { return m_connectionTimeout; }

    // Bandwidth management
    void setMaxBandwidth(uint32_t bytesPerSecond);
    uint32_t getMaxBandwidth() const { return m_maxBandwidth; }
    uint32_t getCurrentBandwidthUsage() const;

    // Packet reliability (for UDP packets that need reliability)
    void sendReliablePacket(const NetworkPacket& packet);

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }
    void logPacket(const NetworkPacket& packet, bool sent);

private:
    NetworkManager();
    ~NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    // Socket management
    bool initializeTCP();
    bool initializeUDP();
    void closeSockets();

    // Thread functions
    void tcpReceiveThread();
    void udpReceiveThread();
    void processingThread();

    // Packet processing
    void processPacket(const NetworkPacket& packet);
    void handleConnectionRequest(const NetworkPacket& packet);
    void handleHeartbeat(const NetworkPacket& packet);
    void handlePing(const NetworkPacket& packet);

    // Connection management
    void addConnection(uint16_t playerId, const std::string& ipAddress, uint16_t port);
    void removeConnection(uint16_t playerId);
    void updateConnectionHeartbeat(uint16_t playerId);
    void checkConnectionTimeouts();

    // Packet queue management
    void queuePacketForSending(const NetworkPacket& packet, ProtocolType protocol);
    void processSendQueue();

    // Sequence number management
    uint32_t getNextSequenceNumber() { return m_sequenceNumber++; }
    bool isSequenceNewer(uint32_t s1, uint32_t s2);

    // Reliability system for UDP
    void trackReliablePacket(const NetworkPacket& packet);
    void checkReliablePacketAcks();
    void resendUnacknowledgedPackets();

    // Bandwidth throttling
    bool canSendPacket(size_t packetSize);
    void updateBandwidthUsage(size_t bytes);

    // Error handling
    void handleError(const std::string& error);

    // Data members
    bool m_initialized;
    bool m_isServer;
    bool m_connected;
    uint16_t m_localPlayerId;

    // Sockets (platform-specific handles as void*)
    void* m_tcpSocket;
    void* m_udpSocket;
    uint16_t m_tcpPort;
    uint16_t m_udpPort;

    std::string m_serverIP;

    // Connections
    std::unordered_map<uint16_t, ConnectionInfo> m_connections;
    std::vector<std::string> m_bannedIPs;
    mutable std::mutex m_connectionsMutex;

    // Packet handlers
    std::unordered_map<PacketType, PacketCallback> m_packetHandlers;
    std::mutex m_handlersMutex;

    // Packet queues
    std::queue<std::pair<NetworkPacket, ProtocolType>> m_sendQueue;
    std::queue<NetworkPacket> m_receiveQueue;
    std::mutex m_sendQueueMutex;
    std::mutex m_receiveQueueMutex;

    // Threads
    std::thread m_tcpReceiveThread;
    std::thread m_udpReceiveThread;
    std::thread m_processingThread;
    std::atomic<bool> m_running;

    // Network state
    uint32_t m_sequenceNumber;
    NetworkStats m_stats;

    // Timing
    float m_heartbeatInterval;
    float m_heartbeatTimer;
    float m_connectionTimeout;
    std::chrono::steady_clock::time_point m_lastPingTime;

    // Lag compensation & prediction
    bool m_lagCompensationEnabled;
    bool m_predictionEnabled;
    float m_interpolationDelay;

    // Bandwidth management
    uint32_t m_maxBandwidth;
    uint32_t m_currentBandwidthUsage;
    std::chrono::steady_clock::time_point m_bandwidthResetTime;

    // Reliability tracking
    struct ReliablePacketInfo {
        NetworkPacket packet;
        uint32_t sendAttempts;
        std::chrono::steady_clock::time_point lastSendTime;
        bool acknowledged;
    };
    std::unordered_map<uint32_t, ReliablePacketInfo> m_reliablePackets;
    std::mutex m_reliablePacketsMutex;

    // Callbacks
    ConnectionCallback m_onConnectionChanged;
    ErrorCallback m_onError;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr int MAX_PACKET_SIZE = 1400;  // Safe UDP packet size
    static constexpr int MAX_CONNECTIONS = 64;     // Max concurrent connections
    static constexpr float DEFAULT_HEARTBEAT_INTERVAL = 1.0f;
    static constexpr float DEFAULT_CONNECTION_TIMEOUT = 10.0f;
    static constexpr float DEFAULT_INTERPOLATION_DELAY = 0.1f;
    static constexpr int RELIABLE_RESEND_ATTEMPTS = 5;
    static constexpr float RELIABLE_RESEND_INTERVAL = 0.5f;
};

} // namespace Network
