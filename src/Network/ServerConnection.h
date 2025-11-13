#pragma once

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace Network {

/**
 * @brief Manages client-server connection lifecycle and authentication
 *
 * Handles connection establishment, authentication, session management,
 * and reconnection logic for robust server communication.
 */
class ServerConnection {
public:
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        CONNECTED,
        RECONNECTING,
        ERROR
    };

    enum class DisconnectReason {
        USER_REQUESTED,
        CONNECTION_LOST,
        KICKED,
        BANNED,
        SERVER_SHUTDOWN,
        TIMEOUT,
        AUTHENTICATION_FAILED,
        VERSION_MISMATCH,
        SERVER_FULL
    };

    struct ServerInfo {
        std::string name;
        std::string address;
        uint16_t tcpPort;
        uint16_t udpPort;
        std::string version;
        int currentPlayers;
        int maxPlayers;
        int ping;
        std::string region;
        std::string gameMode;
        bool passwordProtected;

        ServerInfo()
            : tcpPort(7777)
            , udpPort(7778)
            , currentPlayers(0)
            , maxPlayers(64)
            , ping(0)
            , passwordProtected(false) {}
    };

    struct AuthenticationData {
        std::string username;
        std::string password;
        std::string token;
        uint64_t sessionId;
        std::string clientVersion;
        std::chrono::system_clock::time_point tokenExpiry;

        AuthenticationData() : sessionId(0) {}

        bool isTokenValid() const {
            return std::chrono::system_clock::now() < tokenExpiry;
        }
    };

    using ConnectionStateCallback = std::function<void(ConnectionState oldState, ConnectionState newState)>;
    using DisconnectCallback = std::function<void(DisconnectReason reason)>;
    using AuthenticationCallback = std::function<void(bool success, const std::string& message)>;
    using ServerListCallback = std::function<void(const std::vector<ServerInfo>& servers)>;

    ServerConnection();
    ~ServerConnection();

    // Connection management
    bool connect(const ServerInfo& server, const AuthenticationData& authData);
    void disconnect();
    bool reconnect();
    bool isConnected() const { return m_state == ConnectionState::CONNECTED; }
    ConnectionState getState() const { return m_state; }

    // Authentication
    void authenticate(const AuthenticationData& authData);
    bool isAuthenticated() const { return m_authenticated; }
    const AuthenticationData& getAuthData() const { return m_authData; }
    void refreshAuthToken();

    // Server info
    const ServerInfo& getServerInfo() const { return m_serverInfo; }
    void setServerInfo(const ServerInfo& info) { m_serverInfo = info; }

    // Server browser
    void requestServerList(const std::string& masterServerAddress);
    std::vector<ServerInfo> getServerList() const { return m_serverList; }
    void refreshServerList();
    void pingServer(const std::string& address, uint16_t port);

    // Session management
    uint64_t getSessionId() const { return m_authData.sessionId; }
    std::chrono::system_clock::time_point getConnectedTime() const { return m_connectedTime; }
    float getConnectionDuration() const;

    // Reconnection settings
    void enableAutoReconnect(bool enable) { m_autoReconnect = enable; }
    bool isAutoReconnectEnabled() const { return m_autoReconnect; }
    void setReconnectAttempts(int attempts) { m_maxReconnectAttempts = attempts; }
    int getReconnectAttempts() const { return m_reconnectAttempts; }
    void setReconnectDelay(float seconds) { m_reconnectDelay = seconds; }

    // Connection quality
    int getPing() const;
    float getPacketLoss() const;
    float getConnectionQuality() const; // 0.0 - 1.0

    // Callbacks
    void setOnConnectionStateChanged(ConnectionStateCallback callback) {
        m_onConnectionStateChanged = callback;
    }
    void setOnDisconnect(DisconnectCallback callback) {
        m_onDisconnect = callback;
    }
    void setOnAuthenticated(AuthenticationCallback callback) {
        m_onAuthenticated = callback;
    }
    void setOnServerListReceived(ServerListCallback callback) {
        m_onServerListReceived = callback;
    }

    // Update
    void update(float deltaTime);

    // Server commands
    void sendServerCommand(const std::string& command, const std::vector<std::string>& args);

    // Heartbeat management
    void sendHeartbeat();
    void setHeartbeatInterval(float seconds) { m_heartbeatInterval = seconds; }

    // Version checking
    bool isVersionCompatible(const std::string& serverVersion) const;
    std::string getClientVersion() const { return m_clientVersion; }
    void setClientVersion(const std::string& version) { m_clientVersion = version; }

private:
    // State management
    void setState(ConnectionState newState);
    void handleDisconnect(DisconnectReason reason);

    // Network packet handlers
    void handleConnectResponse(const NetworkPacket& packet);
    void handleAuthResponse(const NetworkPacket& packet);
    void handleDisconnectPacket(const NetworkPacket& packet);
    void handleServerInfo(const NetworkPacket& packet);

    // Reconnection logic
    void attemptReconnect();
    void resetReconnectState();

    // Authentication helpers
    std::string hashPassword(const std::string& password) const;
    bool validateAuthToken(const std::string& token) const;

    // Data members
    ConnectionState m_state;
    bool m_authenticated;
    ServerInfo m_serverInfo;
    AuthenticationData m_authData;
    std::chrono::system_clock::time_point m_connectedTime;

    // Server list
    std::vector<ServerInfo> m_serverList;
    std::string m_masterServerAddress;

    // Reconnection
    bool m_autoReconnect;
    int m_maxReconnectAttempts;
    int m_reconnectAttempts;
    float m_reconnectDelay;
    float m_reconnectTimer;

    // Heartbeat
    float m_heartbeatInterval;
    float m_heartbeatTimer;

    // Callbacks
    ConnectionStateCallback m_onConnectionStateChanged;
    DisconnectCallback m_onDisconnect;
    AuthenticationCallback m_onAuthenticated;
    ServerListCallback m_onServerListReceived;

    // Client info
    std::string m_clientVersion;
    DisconnectReason m_lastDisconnectReason;

    // Network reference
    NetworkManager& m_networkManager;

    // Constants
    static constexpr float DEFAULT_HEARTBEAT_INTERVAL = 5.0f;
    static constexpr int DEFAULT_MAX_RECONNECT_ATTEMPTS = 3;
    static constexpr float DEFAULT_RECONNECT_DELAY = 5.0f;
    static constexpr int AUTH_TIMEOUT_SECONDS = 30;
};

/**
 * @brief Server browser for finding and connecting to game servers
 */
class ServerBrowser {
public:
    enum class SortBy {
        PING,
        PLAYERS,
        NAME,
        REGION
    };

    ServerBrowser();

    // Server list management
    void addServer(const ServerConnection::ServerInfo& server);
    void removeServer(const std::string& address);
    void clearServers();
    std::vector<ServerConnection::ServerInfo> getServers() const { return m_servers; }

    // Filtering
    void filterByRegion(const std::string& region);
    void filterByGameMode(const std::string& gameMode);
    void filterByPlayerCount(int min, int max);
    void filterByPasswordProtected(bool passwordProtected);
    void clearFilters();

    // Sorting
    void sortServers(SortBy sortBy, bool ascending = true);

    // Favorites
    void addFavorite(const std::string& address);
    void removeFavorite(const std::string& address);
    bool isFavorite(const std::string& address) const;
    std::vector<ServerConnection::ServerInfo> getFavorites() const;

    // Recent servers
    void addRecentServer(const ServerConnection::ServerInfo& server);
    std::vector<ServerConnection::ServerInfo> getRecentServers() const;

    // Server pinging
    void pingAllServers();
    void pingServer(const std::string& address);

    // Quick connect
    ServerConnection::ServerInfo quickConnect(const std::string& address, uint16_t port);

private:
    std::vector<ServerConnection::ServerInfo> m_servers;
    std::vector<std::string> m_favoriteAddresses;
    std::vector<ServerConnection::ServerInfo> m_recentServers;

    // Filter state
    std::string m_filterRegion;
    std::string m_filterGameMode;
    int m_filterMinPlayers;
    int m_filterMaxPlayers;
    bool m_filterPasswordProtected;
    bool m_hasFilters;

    static constexpr int MAX_RECENT_SERVERS = 10;
};

} // namespace Network
