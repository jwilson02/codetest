#include "ServerConnection.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

// For password hashing (simplified - would use proper crypto library in production)
#include <cstring>

namespace Network {

ServerConnection::ServerConnection()
    : m_state(ConnectionState::DISCONNECTED)
    , m_authenticated(false)
    , m_autoReconnect(true)
    , m_maxReconnectAttempts(DEFAULT_MAX_RECONNECT_ATTEMPTS)
    , m_reconnectAttempts(0)
    , m_reconnectDelay(DEFAULT_RECONNECT_DELAY)
    , m_reconnectTimer(0.0f)
    , m_heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL)
    , m_heartbeatTimer(0.0f)
    , m_clientVersion("1.0.0")
    , m_lastDisconnectReason(DisconnectReason::USER_REQUESTED)
    , m_networkManager(NetworkManager::getInstance())
{
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::CONNECT_RESPONSE,
        [this](const NetworkPacket& packet) { handleConnectResponse(packet); });

    m_networkManager.registerPacketHandler(PacketType::DISCONNECT,
        [this](const NetworkPacket& packet) { handleDisconnectPacket(packet); });
}

ServerConnection::~ServerConnection() {
    disconnect();
}

bool ServerConnection::connect(const ServerInfo& server, const AuthenticationData& authData) {
    if (m_state == ConnectionState::CONNECTED || m_state == ConnectionState::CONNECTING) {
        std::cerr << "[ServerConnection] Already connected or connecting" << std::endl;
        return false;
    }

    std::cout << "[ServerConnection] Connecting to " << server.name
              << " (" << server.address << ":" << server.tcpPort << ")" << std::endl;

    m_serverInfo = server;
    m_authData = authData;
    m_authData.clientVersion = m_clientVersion;

    setState(ConnectionState::CONNECTING);

    // Attempt to connect via NetworkManager
    bool success = m_networkManager.connectToServer(
        server.address, server.tcpPort, server.udpPort);

    if (success) {
        setState(ConnectionState::AUTHENTICATING);
        authenticate(authData);
    } else {
        setState(ConnectionState::ERROR);
        handleDisconnect(DisconnectReason::CONNECTION_LOST);
        return false;
    }

    return true;
}

void ServerConnection::disconnect() {
    if (m_state == ConnectionState::DISCONNECTED) {
        return;
    }

    std::cout << "[ServerConnection] Disconnecting from server" << std::endl;

    m_networkManager.disconnect();
    handleDisconnect(DisconnectReason::USER_REQUESTED);
    setState(ConnectionState::DISCONNECTED);
    m_authenticated = false;
}

bool ServerConnection::reconnect() {
    if (m_state == ConnectionState::CONNECTED) {
        return true;
    }

    std::cout << "[ServerConnection] Attempting to reconnect..." << std::endl;

    setState(ConnectionState::RECONNECTING);
    return connect(m_serverInfo, m_authData);
}

void ServerConnection::authenticate(const AuthenticationData& authData) {
    std::cout << "[ServerConnection] Authenticating as " << authData.username << std::endl;

    // Create authentication packet
    NetworkPacket packet;
    packet.type = PacketType::CONNECT_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.sequenceNumber = 0;
    packet.playerId = 0;

    // Pack authentication data
    std::string authString = authData.username + ":" +
                            hashPassword(authData.password) + ":" +
                            authData.clientVersion;

    packet.data.assign(authString.begin(), authString.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

void ServerConnection::setState(ConnectionState newState) {
    if (m_state == newState) {
        return;
    }

    ConnectionState oldState = m_state;
    m_state = newState;

    std::cout << "[ServerConnection] State changed: " << static_cast<int>(oldState)
              << " -> " << static_cast<int>(newState) << std::endl;

    if (newState == ConnectionState::CONNECTED) {
        m_connectedTime = std::chrono::system_clock::now();
        resetReconnectState();
    }

    if (m_onConnectionStateChanged) {
        m_onConnectionStateChanged(oldState, newState);
    }
}

void ServerConnection::handleDisconnect(DisconnectReason reason) {
    m_lastDisconnectReason = reason;

    std::cout << "[ServerConnection] Disconnected. Reason: " << static_cast<int>(reason) << std::endl;

    if (m_onDisconnect) {
        m_onDisconnect(reason);
    }

    // Auto-reconnect if enabled
    if (m_autoReconnect && reason != DisconnectReason::USER_REQUESTED &&
        reason != DisconnectReason::BANNED && reason != DisconnectReason::KICKED) {

        if (m_reconnectAttempts < m_maxReconnectAttempts) {
            setState(ConnectionState::RECONNECTING);
            m_reconnectTimer = m_reconnectDelay;
        } else {
            setState(ConnectionState::ERROR);
        }
    }
}

void ServerConnection::handleConnectResponse(const NetworkPacket& packet) {
    if (packet.data.empty()) {
        std::cerr << "[ServerConnection] Received empty connect response" << std::endl;
        handleDisconnect(DisconnectReason::AUTHENTICATION_FAILED);
        return;
    }

    // Parse response (simplified)
    std::string response(packet.data.begin(), packet.data.end());

    if (response.substr(0, 7) == "SUCCESS") {
        m_authenticated = true;
        setState(ConnectionState::CONNECTED);

        // Extract player ID from response
        size_t idPos = response.find("ID:");
        if (idPos != std::string::npos) {
            // Player ID would be parsed here
        }

        std::cout << "[ServerConnection] Authentication successful!" << std::endl;

        if (m_onAuthenticated) {
            m_onAuthenticated(true, "Connected successfully");
        }
    } else {
        std::cerr << "[ServerConnection] Authentication failed: " << response << std::endl;
        handleDisconnect(DisconnectReason::AUTHENTICATION_FAILED);

        if (m_onAuthenticated) {
            m_onAuthenticated(false, response);
        }
    }
}

void ServerConnection::handleDisconnectPacket(const NetworkPacket& packet) {
    DisconnectReason reason = DisconnectReason::CONNECTION_LOST;

    if (!packet.data.empty()) {
        std::string reasonStr(packet.data.begin(), packet.data.end());

        if (reasonStr == "KICKED") {
            reason = DisconnectReason::KICKED;
        } else if (reasonStr == "BANNED") {
            reason = DisconnectReason::BANNED;
        } else if (reasonStr == "SERVER_SHUTDOWN") {
            reason = DisconnectReason::SERVER_SHUTDOWN;
        } else if (reasonStr == "SERVER_FULL") {
            reason = DisconnectReason::SERVER_FULL;
        }
    }

    handleDisconnect(reason);
    setState(ConnectionState::DISCONNECTED);
}

void ServerConnection::handleServerInfo(const NetworkPacket& packet) {
    // Parse server info from packet
    // Implementation would deserialize ServerInfo struct
}

void ServerConnection::update(float deltaTime) {
    // Update heartbeat timer
    if (m_state == ConnectionState::CONNECTED) {
        m_heartbeatTimer += deltaTime;
        if (m_heartbeatTimer >= m_heartbeatInterval) {
            m_heartbeatTimer = 0.0f;
            sendHeartbeat();
        }
    }

    // Handle reconnection timer
    if (m_state == ConnectionState::RECONNECTING) {
        m_reconnectTimer -= deltaTime;
        if (m_reconnectTimer <= 0.0f) {
            attemptReconnect();
        }
    }

    // Check token expiry
    if (m_authenticated && !m_authData.isTokenValid()) {
        std::cout << "[ServerConnection] Auth token expired, refreshing..." << std::endl;
        refreshAuthToken();
    }
}

void ServerConnection::sendHeartbeat() {
    NetworkPacket packet;
    packet.type = PacketType::HEARTBEAT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::UDP);
}

void ServerConnection::attemptReconnect() {
    m_reconnectAttempts++;

    std::cout << "[ServerConnection] Reconnect attempt " << m_reconnectAttempts
              << "/" << m_maxReconnectAttempts << std::endl;

    if (!reconnect()) {
        if (m_reconnectAttempts >= m_maxReconnectAttempts) {
            std::cerr << "[ServerConnection] Max reconnect attempts reached" << std::endl;
            setState(ConnectionState::ERROR);
        } else {
            m_reconnectTimer = m_reconnectDelay;
        }
    }
}

void ServerConnection::resetReconnectState() {
    m_reconnectAttempts = 0;
    m_reconnectTimer = 0.0f;
}

std::string ServerConnection::hashPassword(const std::string& password) const {
    // Simplified hash - in production use proper crypto library (bcrypt, argon2, etc.)
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    size_t hash = 0;
    for (char c : password) {
        hash = hash * 31 + c;
    }

    ss << std::setw(16) << hash;
    return ss.str();
}

bool ServerConnection::validateAuthToken(const std::string& token) const {
    return !token.empty() && m_authData.isTokenValid();
}

void ServerConnection::refreshAuthToken() {
    // Send token refresh request
    NetworkPacket packet;
    packet.type = PacketType::CONNECT_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::string refreshData = "REFRESH:" + m_authData.token;
    packet.data.assign(refreshData.begin(), refreshData.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

int ServerConnection::getPing() const {
    return m_networkManager.getPing();
}

float ServerConnection::getPacketLoss() const {
    return m_networkManager.getPacketLoss();
}

float ServerConnection::getConnectionQuality() const {
    int ping = getPing();
    float packetLoss = getPacketLoss();

    // Calculate quality score (0.0 - 1.0)
    float pingScore = std::max(0.0f, 1.0f - (ping / 500.0f));
    float lossScore = 1.0f - packetLoss;

    return (pingScore * 0.7f + lossScore * 0.3f);
}

float ServerConnection::getConnectionDuration() const {
    if (m_state != ConnectionState::CONNECTED) {
        return 0.0f;
    }

    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_connectedTime);
    return static_cast<float>(duration.count());
}

bool ServerConnection::isVersionCompatible(const std::string& serverVersion) const {
    // Simple version check - in production use proper semantic versioning
    return serverVersion == m_clientVersion;
}

void ServerConnection::requestServerList(const std::string& masterServerAddress) {
    m_masterServerAddress = masterServerAddress;

    std::cout << "[ServerConnection] Requesting server list from " << masterServerAddress << std::endl;

    // Implementation would query master server
    // For now, just trigger callback with empty list
    if (m_onServerListReceived) {
        m_onServerListReceived(m_serverList);
    }
}

void ServerConnection::refreshServerList() {
    if (!m_masterServerAddress.empty()) {
        requestServerList(m_masterServerAddress);
    }
}

void ServerConnection::pingServer(const std::string& address, uint16_t port) {
    // Send ping packet to server
    NetworkPacket packet;
    packet.type = PacketType::PING;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Implementation would send to specific address
}

void ServerConnection::sendServerCommand(const std::string& command, const std::vector<std::string>& args) {
    if (!isConnected()) {
        return;
    }

    // Pack command and arguments into packet
    std::string cmdString = command;
    for (const auto& arg : args) {
        cmdString += " " + arg;
    }

    NetworkPacket packet;
    packet.type = PacketType::CHAT_MESSAGE; // Could use custom command type
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();
    packet.data.assign(cmdString.begin(), cmdString.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);
}

// ServerBrowser implementation

ServerBrowser::ServerBrowser()
    : m_filterMinPlayers(0)
    , m_filterMaxPlayers(999)
    , m_filterPasswordProtected(false)
    , m_hasFilters(false)
{
}

void ServerBrowser::addServer(const ServerConnection::ServerInfo& server) {
    // Check if server already exists
    auto it = std::find_if(m_servers.begin(), m_servers.end(),
        [&server](const ServerConnection::ServerInfo& s) {
            return s.address == server.address;
        });

    if (it != m_servers.end()) {
        *it = server; // Update existing
    } else {
        m_servers.push_back(server);
    }
}

void ServerBrowser::removeServer(const std::string& address) {
    m_servers.erase(
        std::remove_if(m_servers.begin(), m_servers.end(),
            [&address](const ServerConnection::ServerInfo& s) {
                return s.address == address;
            }),
        m_servers.end()
    );
}

void ServerBrowser::clearServers() {
    m_servers.clear();
}

void ServerBrowser::filterByRegion(const std::string& region) {
    m_filterRegion = region;
    m_hasFilters = true;
}

void ServerBrowser::filterByGameMode(const std::string& gameMode) {
    m_filterGameMode = gameMode;
    m_hasFilters = true;
}

void ServerBrowser::filterByPlayerCount(int min, int max) {
    m_filterMinPlayers = min;
    m_filterMaxPlayers = max;
    m_hasFilters = true;
}

void ServerBrowser::filterByPasswordProtected(bool passwordProtected) {
    m_filterPasswordProtected = passwordProtected;
    m_hasFilters = true;
}

void ServerBrowser::clearFilters() {
    m_filterRegion.clear();
    m_filterGameMode.clear();
    m_filterMinPlayers = 0;
    m_filterMaxPlayers = 999;
    m_filterPasswordProtected = false;
    m_hasFilters = false;
}

void ServerBrowser::sortServers(SortBy sortBy, bool ascending) {
    std::sort(m_servers.begin(), m_servers.end(),
        [sortBy, ascending](const ServerConnection::ServerInfo& a, const ServerConnection::ServerInfo& b) {
            bool result = false;
            switch (sortBy) {
                case SortBy::PING:
                    result = a.ping < b.ping;
                    break;
                case SortBy::PLAYERS:
                    result = a.currentPlayers < b.currentPlayers;
                    break;
                case SortBy::NAME:
                    result = a.name < b.name;
                    break;
                case SortBy::REGION:
                    result = a.region < b.region;
                    break;
            }
            return ascending ? result : !result;
        });
}

void ServerBrowser::addFavorite(const std::string& address) {
    if (!isFavorite(address)) {
        m_favoriteAddresses.push_back(address);
    }
}

void ServerBrowser::removeFavorite(const std::string& address) {
    m_favoriteAddresses.erase(
        std::remove(m_favoriteAddresses.begin(), m_favoriteAddresses.end(), address),
        m_favoriteAddresses.end()
    );
}

bool ServerBrowser::isFavorite(const std::string& address) const {
    return std::find(m_favoriteAddresses.begin(), m_favoriteAddresses.end(), address)
           != m_favoriteAddresses.end();
}

std::vector<ServerConnection::ServerInfo> ServerBrowser::getFavorites() const {
    std::vector<ServerConnection::ServerInfo> favorites;

    for (const auto& address : m_favoriteAddresses) {
        auto it = std::find_if(m_servers.begin(), m_servers.end(),
            [&address](const ServerConnection::ServerInfo& s) {
                return s.address == address;
            });

        if (it != m_servers.end()) {
            favorites.push_back(*it);
        }
    }

    return favorites;
}

void ServerBrowser::addRecentServer(const ServerConnection::ServerInfo& server) {
    // Remove if already exists
    m_recentServers.erase(
        std::remove_if(m_recentServers.begin(), m_recentServers.end(),
            [&server](const ServerConnection::ServerInfo& s) {
                return s.address == server.address;
            }),
        m_recentServers.end()
    );

    // Add to front
    m_recentServers.insert(m_recentServers.begin(), server);

    // Limit size
    if (m_recentServers.size() > MAX_RECENT_SERVERS) {
        m_recentServers.resize(MAX_RECENT_SERVERS);
    }
}

std::vector<ServerConnection::ServerInfo> ServerBrowser::getRecentServers() const {
    return m_recentServers;
}

void ServerBrowser::pingAllServers() {
    for (auto& server : m_servers) {
        // Implementation would ping each server
        // For now, just set a dummy value
        server.ping = 50 + (rand() % 100);
    }
}

void ServerBrowser::pingServer(const std::string& address) {
    auto it = std::find_if(m_servers.begin(), m_servers.end(),
        [&address](const ServerConnection::ServerInfo& s) {
            return s.address == address;
        });

    if (it != m_servers.end()) {
        // Implementation would ping specific server
        it->ping = 50 + (rand() % 100);
    }
}

ServerConnection::ServerInfo ServerBrowser::quickConnect(const std::string& address, uint16_t port) {
    ServerConnection::ServerInfo server;
    server.name = "Quick Connect";
    server.address = address;
    server.tcpPort = port;
    server.udpPort = port + 1;

    return server;
}

} // namespace Network
