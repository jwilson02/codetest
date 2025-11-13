#include "NetworkManager.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// Platform-specific socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

namespace Network {

// Helper to convert socket handle
inline SOCKET toSocket(void* ptr) {
    return ptr ? *static_cast<SOCKET*>(ptr) : INVALID_SOCKET;
}

// NetworkPacket serialization
std::vector<uint8_t> NetworkPacket::serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(sizeof(type) + sizeof(timestamp) + sizeof(sequenceNumber) +
                   sizeof(playerId) + sizeof(dataSize) + dataSize);

    // Write header
    buffer.push_back(static_cast<uint8_t>(type));

    // Write timestamp (8 bytes)
    for (int i = 0; i < 8; i++) {
        buffer.push_back((timestamp >> (i * 8)) & 0xFF);
    }

    // Write sequence number (4 bytes)
    for (int i = 0; i < 4; i++) {
        buffer.push_back((sequenceNumber >> (i * 8)) & 0xFF);
    }

    // Write player ID (2 bytes)
    buffer.push_back(playerId & 0xFF);
    buffer.push_back((playerId >> 8) & 0xFF);

    // Write data size (2 bytes)
    buffer.push_back(dataSize & 0xFF);
    buffer.push_back((dataSize >> 8) & 0xFF);

    // Write data
    buffer.insert(buffer.end(), data.begin(), data.end());

    return buffer;
}

NetworkPacket NetworkPacket::deserialize(const std::vector<uint8_t>& bytes) {
    NetworkPacket packet;

    if (bytes.size() < 17) { // Minimum packet size
        return packet;
    }

    size_t offset = 0;

    // Read type
    packet.type = static_cast<PacketType>(bytes[offset++]);

    // Read timestamp
    packet.timestamp = 0;
    for (int i = 0; i < 8; i++) {
        packet.timestamp |= (static_cast<uint64_t>(bytes[offset++]) << (i * 8));
    }

    // Read sequence number
    packet.sequenceNumber = 0;
    for (int i = 0; i < 4; i++) {
        packet.sequenceNumber |= (static_cast<uint32_t>(bytes[offset++]) << (i * 8));
    }

    // Read player ID
    packet.playerId = bytes[offset] | (bytes[offset + 1] << 8);
    offset += 2;

    // Read data size
    packet.dataSize = bytes[offset] | (bytes[offset + 1] << 8);
    offset += 2;

    // Read data
    if (packet.dataSize > 0 && offset + packet.dataSize <= bytes.size()) {
        packet.data.assign(bytes.begin() + offset, bytes.begin() + offset + packet.dataSize);
    }

    return packet;
}

// NetworkManager implementation

NetworkManager::NetworkManager()
    : m_initialized(false)
    , m_isServer(false)
    , m_connected(false)
    , m_localPlayerId(0)
    , m_tcpSocket(nullptr)
    , m_udpSocket(nullptr)
    , m_tcpPort(7777)
    , m_udpPort(7778)
    , m_running(false)
    , m_sequenceNumber(0)
    , m_heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL)
    , m_heartbeatTimer(0.0f)
    , m_connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
    , m_lagCompensationEnabled(true)
    , m_predictionEnabled(true)
    , m_interpolationDelay(DEFAULT_INTERPOLATION_DELAY)
    , m_maxBandwidth(1024 * 1024) // 1 MB/s default
    , m_currentBandwidthUsage(0)
    , m_debugMode(false)
{
}

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initialize(bool isServer, uint16_t tcpPort, uint16_t udpPort) {
    if (m_initialized) {
        handleError("NetworkManager already initialized");
        return false;
    }

    m_isServer = isServer;
    m_tcpPort = tcpPort;
    m_udpPort = udpPort;

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        handleError("Failed to initialize Winsock");
        return false;
    }
#endif

    if (!initializeTCP() || !initializeUDP()) {
        closeSockets();
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    m_initialized = true;
    m_running = true;

    // Start network threads
    m_tcpReceiveThread = std::thread(&NetworkManager::tcpReceiveThread, this);
    m_udpReceiveThread = std::thread(&NetworkManager::udpReceiveThread, this);
    m_processingThread = std::thread(&NetworkManager::processingThread, this);

    if (m_debugMode) {
        std::cout << "[NetworkManager] Initialized in " << (isServer ? "SERVER" : "CLIENT")
                  << " mode on TCP:" << tcpPort << " UDP:" << udpPort << std::endl;
    }

    return true;
}

void NetworkManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_debugMode) {
        std::cout << "[NetworkManager] Shutting down..." << std::endl;
    }

    m_running = false;
    m_connected = false;

    // Wait for threads to finish
    if (m_tcpReceiveThread.joinable()) m_tcpReceiveThread.join();
    if (m_udpReceiveThread.joinable()) m_udpReceiveThread.join();
    if (m_processingThread.joinable()) m_processingThread.join();

    closeSockets();

#ifdef _WIN32
    WSACleanup();
#endif

    m_connections.clear();
    m_packetHandlers.clear();

    m_initialized = false;
}

bool NetworkManager::initializeTCP() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        handleError("Failed to create TCP socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    if (m_isServer) {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(m_tcpPort);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            handleError("Failed to bind TCP socket");
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            return false;
        }

        if (listen(sock, MAX_CONNECTIONS) == SOCKET_ERROR) {
            handleError("Failed to listen on TCP socket");
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            return false;
        }
    }

    // Set non-blocking mode
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

    m_tcpSocket = new SOCKET(sock);
    return true;
}

bool NetworkManager::initializeUDP() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        handleError("Failed to create UDP socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_udpPort);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        handleError("Failed to bind UDP socket");
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return false;
    }

    // Set non-blocking mode
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

    m_udpSocket = new SOCKET(sock);
    return true;
}

void NetworkManager::closeSockets() {
    if (m_tcpSocket) {
        SOCKET sock = toSocket(m_tcpSocket);
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        delete static_cast<SOCKET*>(m_tcpSocket);
        m_tcpSocket = nullptr;
    }

    if (m_udpSocket) {
        SOCKET sock = toSocket(m_udpSocket);
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        delete static_cast<SOCKET*>(m_udpSocket);
        m_udpSocket = nullptr;
    }
}

bool NetworkManager::connectToServer(const std::string& serverIP, uint16_t tcpPort, uint16_t udpPort) {
    if (m_isServer) {
        handleError("Server cannot connect to another server");
        return false;
    }

    if (m_connected) {
        handleError("Already connected to a server");
        return false;
    }

    m_serverIP = serverIP;

    // Send connection request
    NetworkPacket packet;
    packet.type = PacketType::CONNECT_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.sequenceNumber = getNextSequenceNumber();
    packet.playerId = 0;

    sendPacket(packet, ProtocolType::TCP);

    m_connected = true;
    m_lastPingTime = std::chrono::steady_clock::now();

    if (m_debugMode) {
        std::cout << "[NetworkManager] Connected to server: " << serverIP << std::endl;
    }

    return true;
}

void NetworkManager::disconnect() {
    if (!m_connected) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::DISCONNECT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.sequenceNumber = getNextSequenceNumber();
    packet.playerId = m_localPlayerId;

    sendPacket(packet, ProtocolType::TCP);

    m_connected = false;

    if (m_debugMode) {
        std::cout << "[NetworkManager] Disconnected from server" << std::endl;
    }
}

void NetworkManager::sendPacket(const NetworkPacket& packet, ProtocolType protocol) {
    std::lock_guard<std::mutex> lock(m_sendQueueMutex);
    m_sendQueue.push({packet, protocol});
}

void NetworkManager::sendPacketTo(uint16_t playerId, const NetworkPacket& packet, ProtocolType protocol) {
    if (!m_isServer) {
        return;
    }

    // Implementation would send to specific player
    sendPacket(packet, protocol);
}

void NetworkManager::broadcastPacket(const NetworkPacket& packet, ProtocolType protocol) {
    if (!m_isServer) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    for (const auto& [playerId, conn] : m_connections) {
        if (conn.isConnected) {
            sendPacketTo(playerId, packet, protocol);
        }
    }
}

void NetworkManager::broadcastPacketExcept(uint16_t excludePlayerId, const NetworkPacket& packet, ProtocolType protocol) {
    if (!m_isServer) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    for (const auto& [playerId, conn] : m_connections) {
        if (conn.isConnected && playerId != excludePlayerId) {
            sendPacketTo(playerId, packet, protocol);
        }
    }
}

void NetworkManager::registerPacketHandler(PacketType type, PacketCallback callback) {
    std::lock_guard<std::mutex> lock(m_handlersMutex);
    m_packetHandlers[type] = callback;
}

void NetworkManager::unregisterPacketHandler(PacketType type) {
    std::lock_guard<std::mutex> lock(m_handlersMutex);
    m_packetHandlers.erase(type);
}

void NetworkManager::update(float deltaTime) {
    if (!m_initialized) {
        return;
    }

    // Update heartbeat timer
    m_heartbeatTimer += deltaTime;
    if (m_heartbeatTimer >= m_heartbeatInterval) {
        m_heartbeatTimer = 0.0f;

        NetworkPacket heartbeat;
        heartbeat.type = PacketType::HEARTBEAT;
        heartbeat.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        heartbeat.sequenceNumber = getNextSequenceNumber();
        heartbeat.playerId = m_localPlayerId;

        sendPacket(heartbeat, ProtocolType::UDP);
    }

    // Process received packets
    std::queue<NetworkPacket> packetsToProcess;
    {
        std::lock_guard<std::mutex> lock(m_receiveQueueMutex);
        packetsToProcess.swap(m_receiveQueue);
    }

    while (!packetsToProcess.empty()) {
        processPacket(packetsToProcess.front());
        packetsToProcess.pop();
    }

    // Check connection timeouts
    if (m_isServer) {
        checkConnectionTimeouts();
    }

    // Process send queue
    processSendQueue();

    // Check reliable packet acknowledgments
    checkReliablePacketAcks();

    // Update bandwidth usage
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_bandwidthResetTime);
    if (elapsed.count() >= 1) {
        m_currentBandwidthUsage = 0;
        m_bandwidthResetTime = now;
    }
}

void NetworkManager::tcpReceiveThread() {
    // TCP receive loop implementation
    // Simplified for brevity
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void NetworkManager::udpReceiveThread() {
    // UDP receive loop implementation
    // Simplified for brevity
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void NetworkManager::processingThread() {
    while (m_running) {
        // Process queued operations
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void NetworkManager::processPacket(const NetworkPacket& packet) {
    m_stats.packetsReceived++;
    m_stats.bytesReceived += packet.dataSize + 17; // Header size

    if (m_debugMode) {
        logPacket(packet, false);
    }

    std::lock_guard<std::mutex> lock(m_handlersMutex);
    auto it = m_packetHandlers.find(packet.type);
    if (it != m_packetHandlers.end()) {
        it->second(packet);
    }
}

void NetworkManager::processSendQueue() {
    std::lock_guard<std::mutex> lock(m_sendQueueMutex);

    while (!m_sendQueue.empty()) {
        auto [packet, protocol] = m_sendQueue.front();
        m_sendQueue.pop();

        auto serialized = packet.serialize();
        size_t packetSize = serialized.size();

        if (!canSendPacket(packetSize)) {
            // Re-queue if bandwidth limit reached
            m_sendQueue.push({packet, protocol});
            break;
        }

        m_stats.packetsSent++;
        m_stats.bytessSent += packetSize;
        updateBandwidthUsage(packetSize);

        if (m_debugMode) {
            logPacket(packet, true);
        }

        // Actual socket send would go here
    }
}

uint32_t NetworkManager::getPing() const {
    return m_stats.averagePing;
}

float NetworkManager::getPacketLoss() const {
    return m_stats.packetLossRate;
}

std::vector<ConnectionInfo> NetworkManager::getConnectedPlayers() const {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    std::vector<ConnectionInfo> players;
    for (const auto& [id, conn] : m_connections) {
        if (conn.isConnected) {
            players.push_back(conn);
        }
    }
    return players;
}

ConnectionInfo NetworkManager::getPlayerInfo(uint16_t playerId) const {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    auto it = m_connections.find(playerId);
    if (it != m_connections.end()) {
        return it->second;
    }
    return ConnectionInfo();
}

void NetworkManager::checkConnectionTimeouts() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    std::vector<uint16_t> timedOutPlayers;

    for (const auto& [playerId, conn] : m_connections) {
        if (conn.isConnected) {
            auto timeSinceHeartbeat = (now - conn.lastHeartbeat) / 1000.0f;
            if (timeSinceHeartbeat > m_connectionTimeout) {
                timedOutPlayers.push_back(playerId);
            }
        }
    }

    for (uint16_t playerId : timedOutPlayers) {
        if (m_debugMode) {
            std::cout << "[NetworkManager] Player " << playerId << " timed out" << std::endl;
        }
        removeConnection(playerId);
    }
}

void NetworkManager::addConnection(uint16_t playerId, const std::string& ipAddress, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);

    ConnectionInfo info;
    info.playerId = playerId;
    info.ipAddress = ipAddress;
    info.port = port;
    info.isConnected = true;
    info.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    m_connections[playerId] = info;

    if (m_onConnectionChanged) {
        m_onConnectionChanged(playerId, true);
    }
}

void NetworkManager::removeConnection(uint16_t playerId) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);

    auto it = m_connections.find(playerId);
    if (it != m_connections.end()) {
        it->second.isConnected = false;
        m_connections.erase(it);

        if (m_onConnectionChanged) {
            m_onConnectionChanged(playerId, false);
        }
    }
}

void NetworkManager::updateConnectionHeartbeat(uint16_t playerId) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);

    auto it = m_connections.find(playerId);
    if (it != m_connections.end()) {
        it->second.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

bool NetworkManager::canSendPacket(size_t packetSize) {
    return (m_currentBandwidthUsage + packetSize) <= m_maxBandwidth;
}

void NetworkManager::updateBandwidthUsage(size_t bytes) {
    m_currentBandwidthUsage += bytes;
}

void NetworkManager::checkReliablePacketAcks() {
    std::lock_guard<std::mutex> lock(m_reliablePacketsMutex);

    auto now = std::chrono::steady_clock::now();
    std::vector<uint32_t> toResend;

    for (auto& [seqNum, info] : m_reliablePackets) {
        if (!info.acknowledged) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - info.lastSendTime).count() / 1000.0f;

            if (elapsed >= RELIABLE_RESEND_INTERVAL && info.sendAttempts < RELIABLE_RESEND_ATTEMPTS) {
                toResend.push_back(seqNum);
            }
        }
    }

    for (uint32_t seqNum : toResend) {
        auto& info = m_reliablePackets[seqNum];
        info.sendAttempts++;
        info.lastSendTime = now;
        sendPacket(info.packet, ProtocolType::UDP);
    }
}

void NetworkManager::handleError(const std::string& error) {
    if (m_debugMode) {
        std::cerr << "[NetworkManager] ERROR: " << error << std::endl;
    }

    if (m_onError) {
        m_onError(error);
    }
}

void NetworkManager::logPacket(const NetworkPacket& packet, bool sent) {
    std::cout << "[NetworkManager] " << (sent ? "SENT" : "RECV") << " Packet: "
              << "Type=" << static_cast<int>(packet.type)
              << " Seq=" << packet.sequenceNumber
              << " Player=" << packet.playerId
              << " Size=" << packet.dataSize << std::endl;
}

void NetworkManager::kickPlayer(uint16_t playerId, const std::string& reason) {
    if (!m_isServer) {
        return;
    }

    if (m_debugMode) {
        std::cout << "[NetworkManager] Kicking player " << playerId
                  << " Reason: " << reason << std::endl;
    }

    NetworkPacket packet;
    packet.type = PacketType::DISCONNECT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.sequenceNumber = getNextSequenceNumber();
    packet.playerId = playerId;

    sendPacketTo(playerId, packet, ProtocolType::TCP);
    removeConnection(playerId);
}

void NetworkManager::setMaxBandwidth(uint32_t bytesPerSecond) {
    m_maxBandwidth = bytesPerSecond;
}

uint32_t NetworkManager::getCurrentBandwidthUsage() const {
    return m_currentBandwidthUsage;
}

void NetworkManager::sendReliablePacket(const NetworkPacket& packet) {
    trackReliablePacket(packet);
    sendPacket(packet, ProtocolType::UDP);
}

void NetworkManager::trackReliablePacket(const NetworkPacket& packet) {
    std::lock_guard<std::mutex> lock(m_reliablePacketsMutex);

    ReliablePacketInfo info;
    info.packet = packet;
    info.sendAttempts = 1;
    info.lastSendTime = std::chrono::steady_clock::now();
    info.acknowledged = false;

    m_reliablePackets[packet.sequenceNumber] = info;
}

} // namespace Network
