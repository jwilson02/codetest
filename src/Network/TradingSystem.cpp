#include "TradingSystem.h"
#include <iostream>
#include <algorithm>

namespace Network {

TradingSystem::TradingSystem()
    : m_currentOffer(nullptr)
    , m_tradeRequestsEnabled(true)
    , m_verificationEnabled(true)
    , m_tradeTimeout(DEFAULT_TRADE_TIMEOUT)
    , m_totalGoldTraded(0)
    , m_totalItemsTraded(0)
    , m_totalSuccessfulTrades(0)
    , m_totalCancelledTrades(0)
    , m_networkManager(NetworkManager::getInstance())
    , m_debugMode(false)
{
}

TradingSystem::~TradingSystem() {
    shutdown();
}

void TradingSystem::initialize() {
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::TRADE_REQUEST,
        [this](const NetworkPacket& packet) { handleTradeRequest(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_ACCEPT,
        [this](const NetworkPacket& packet) { handleTradeAccept(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_DECLINE,
        [this](const NetworkPacket& packet) { handleTradeDecline(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_ADD_ITEM,
        [this](const NetworkPacket& packet) { handleTradeAddItem(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_REMOVE_ITEM,
        [this](const NetworkPacket& packet) { handleTradeRemoveItem(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_CONFIRM,
        [this](const NetworkPacket& packet) { handleTradeConfirm(packet); });

    m_networkManager.registerPacketHandler(PacketType::TRADE_CANCEL,
        [this](const NetworkPacket& packet) { handleTradeCancel(packet); });

    std::cout << "[TradingSystem] Initialized" << std::endl;
}

void TradingSystem::shutdown() {
    if (m_currentOffer) {
        delete m_currentOffer;
        m_currentOffer = nullptr;
    }
    m_tradeHistory.clear();
    m_pendingRequests.clear();
}

void TradingSystem::requestTrade(uint16_t playerId) {
    if (!canTrade() || isBlacklisted(playerId)) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::TRADE_REQUEST;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    std::string data = std::to_string(playerId);
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    m_pendingRequests[playerId] = std::chrono::system_clock::now();

    if (m_debugMode) {
        std::cout << "[TradingSystem] Sent trade request to player " << playerId << std::endl;
    }
}

void TradingSystem::acceptTradeRequest(uint16_t fromPlayerId) {
    auto it = m_pendingRequests.find(fromPlayerId);
    if (it == m_pendingRequests.end()) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::TRADE_ACCEPT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    createTrade(m_networkManager.getLocalPlayerId(), fromPlayerId);
    m_pendingRequests.erase(it);

    if (m_onTradeAccepted) {
        m_onTradeAccepted(fromPlayerId);
    }

    if (m_debugMode) {
        std::cout << "[TradingSystem] Accepted trade request from player " << fromPlayerId << std::endl;
    }
}

void TradingSystem::declineTradeRequest(uint16_t fromPlayerId) {
    auto it = m_pendingRequests.find(fromPlayerId);
    if (it != m_pendingRequests.end()) {
        NetworkPacket packet;
        packet.type = PacketType::TRADE_DECLINE;
        packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        packet.playerId = m_networkManager.getLocalPlayerId();

        m_networkManager.sendPacket(packet, ProtocolType::TCP);

        m_pendingRequests.erase(it);

        if (m_debugMode) {
            std::cout << "[TradingSystem] Declined trade request from player " << fromPlayerId << std::endl;
        }
    }
}

void TradingSystem::cancelTrade() {
    if (!isInTrade()) {
        return;
    }

    NetworkPacket packet;
    packet.type = PacketType::TRADE_CANCEL;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_onTradeCancelled) {
        m_onTradeCancelled("Trade cancelled by player");
    }

    recordTradeHistory(*m_currentOffer, false);
    delete m_currentOffer;
    m_currentOffer = nullptr;
    m_totalCancelledTrades++;

    if (m_debugMode) {
        std::cout << "[TradingSystem] Cancelled trade" << std::endl;
    }
}

void TradingSystem::addItem(uint32_t itemId, int quantity) {
    if (!isInTrade() || !canAddItem(itemId) || !hasItem(itemId, quantity)) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    auto& items = (m_currentOffer->playerId1 == localId) ?
                  m_currentOffer->player1Items : m_currentOffer->player2Items;

    TradeItem item;
    item.itemId = itemId;
    item.quantity = quantity;
    item.itemName = "Item" + std::to_string(itemId);
    item.value = calculateItemValue(itemId) * quantity;

    items.push_back(item);

    // Reset ready status when items change
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Ready = false;
    } else {
        m_currentOffer->player2Ready = false;
    }

    notifyTradeUpdate();

    if (m_debugMode) {
        std::cout << "[TradingSystem] Added item " << itemId << " x" << quantity << std::endl;
    }
}

void TradingSystem::removeItem(uint32_t itemId) {
    if (!isInTrade()) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    auto& items = (m_currentOffer->playerId1 == localId) ?
                  m_currentOffer->player1Items : m_currentOffer->player2Items;

    items.erase(
        std::remove_if(items.begin(), items.end(),
            [itemId](const TradeItem& item) { return item.itemId == itemId; }),
        items.end()
    );

    // Reset ready status
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Ready = false;
    } else {
        m_currentOffer->player2Ready = false;
    }

    notifyTradeUpdate();
}

void TradingSystem::clearItems() {
    if (!isInTrade()) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Items.clear();
        m_currentOffer->player1Ready = false;
    } else {
        m_currentOffer->player2Items.clear();
        m_currentOffer->player2Ready = false;
    }

    notifyTradeUpdate();
}

void TradingSystem::setGoldOffer(uint64_t amount) {
    if (!isInTrade() || !hasGold(amount)) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Gold = amount;
        m_currentOffer->player1Ready = false;
    } else {
        m_currentOffer->player2Gold = amount;
        m_currentOffer->player2Ready = false;
    }

    notifyTradeUpdate();
}

std::vector<TradeItem> TradingSystem::getMyOfferedItems() const {
    if (!isInTrade()) {
        return std::vector<TradeItem>();
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    return (m_currentOffer->playerId1 == localId) ?
           m_currentOffer->player1Items : m_currentOffer->player2Items;
}

uint64_t TradingSystem::getMyOfferedGold() const {
    if (!isInTrade()) {
        return 0;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    return (m_currentOffer->playerId1 == localId) ?
           m_currentOffer->player1Gold : m_currentOffer->player2Gold;
}

void TradingSystem::setReady(bool ready) {
    if (!isInTrade()) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Ready = ready;
    } else {
        m_currentOffer->player2Ready = ready;
    }

    notifyTradeUpdate();

    if (m_currentOffer->isBothReady()) {
        if (m_debugMode) {
            std::cout << "[TradingSystem] Both players ready, awaiting final confirmation" << std::endl;
        }
    }
}

void TradingSystem::confirmTrade() {
    if (!isInTrade() || !m_currentOffer->isBothReady()) {
        return;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    if (m_currentOffer->playerId1 == localId) {
        m_currentOffer->player1Confirmed = true;
    } else {
        m_currentOffer->player2Confirmed = true;
    }

    NetworkPacket packet;
    packet.type = PacketType::TRADE_CONFIRM;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = localId;

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_currentOffer->isBothConfirmed()) {
        executeTrade();
    }
}

void TradingSystem::executeTrade() {
    if (!isInTrade() || !m_currentOffer->isBothConfirmed()) {
        return;
    }

    // Verify trade before execution
    if (m_verificationEnabled && !verifyTrade(*m_currentOffer)) {
        cancelTrade();
        return;
    }

    // Execute the trade (would actually transfer items/gold here)
    completeTrade();

    if (m_debugMode) {
        std::cout << "[TradingSystem] Trade executed successfully!" << std::endl;
    }
}

bool TradingSystem::isReady() const {
    if (!isInTrade()) {
        return false;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    return (m_currentOffer->playerId1 == localId) ?
           m_currentOffer->player1Ready : m_currentOffer->player2Ready;
}

bool TradingSystem::isOtherPlayerReady() const {
    if (!isInTrade()) {
        return false;
    }

    uint16_t localId = m_networkManager.getLocalPlayerId();
    return (m_currentOffer->playerId1 == localId) ?
           m_currentOffer->player2Ready : m_currentOffer->player1Ready;
}

void TradingSystem::clearTradeHistory() {
    m_tradeHistory.clear();
}

int TradingSystem::getSuccessfulTradeCount() const {
    return m_totalSuccessfulTrades;
}

bool TradingSystem::canTrade() const {
    return m_tradeRequestsEnabled && !isInTrade();
}

bool TradingSystem::canAddItem(uint32_t itemId) const {
    if (!isInTrade()) {
        return false;
    }

    auto& items = getMyOfferedItems();
    return items.size() < MAX_TRADE_ITEMS && isItemTradeable(itemId);
}

bool TradingSystem::hasItem(uint32_t itemId, int quantity) const {
    // Would check player's inventory
    return true;
}

bool TradingSystem::hasGold(uint64_t amount) const {
    // Would check player's gold
    return true;
}

bool TradingSystem::isItemTradeable(uint32_t itemId) const {
    // Check if item can be traded (not quest item, not bound, etc.)
    return true;
}

void TradingSystem::addToBlacklist(uint16_t playerId) {
    if (!isBlacklisted(playerId)) {
        m_blacklistedPlayers.push_back(playerId);
    }
}

void TradingSystem::removeFromBlacklist(uint16_t playerId) {
    m_blacklistedPlayers.erase(
        std::remove(m_blacklistedPlayers.begin(), m_blacklistedPlayers.end(), playerId),
        m_blacklistedPlayers.end()
    );
}

bool TradingSystem::isBlacklisted(uint16_t playerId) const {
    return std::find(m_blacklistedPlayers.begin(), m_blacklistedPlayers.end(), playerId)
           != m_blacklistedPlayers.end();
}

bool TradingSystem::verifyTrade(const TradeOffer& offer) const {
    // Check for suspicious activity
    if (detectSuspiciousActivity(offer)) {
        return false;
    }

    // Check if trade is balanced
    if (!isTradeBalanced(offer)) {
        // Warn but don't block
        std::cout << "[TradingSystem] Warning: Trade appears unbalanced" << std::endl;
    }

    return validateTradeItems();
}

void TradingSystem::update(float deltaTime) {
    // Check for expired trade
    if (isInTrade() && m_currentOffer->isExpired()) {
        if (m_debugMode) {
            std::cout << "[TradingSystem] Trade expired" << std::endl;
        }
        cancelTrade();
    }

    // Remove expired pending requests
    auto now = std::chrono::system_clock::now();
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
        if (elapsed > 60) {  // 60 second timeout
            it = m_pendingRequests.erase(it);
        } else {
            ++it;
        }
    }
}

void TradingSystem::handleTradeRequest(const NetworkPacket& packet) {
    if (!m_tradeRequestsEnabled) {
        return;
    }

    uint16_t fromPlayerId = packet.playerId;

    if (isBlacklisted(fromPlayerId)) {
        declineTradeRequest(fromPlayerId);
        return;
    }

    m_pendingRequests[fromPlayerId] = std::chrono::system_clock::now();

    if (m_onTradeRequest) {
        m_onTradeRequest(fromPlayerId, "Player" + std::to_string(fromPlayerId));
    }

    if (m_debugMode) {
        std::cout << "[TradingSystem] Received trade request from player " << fromPlayerId << std::endl;
    }
}

void TradingSystem::handleTradeAccept(const NetworkPacket& packet) {
    // Trade was accepted by other player
    if (m_debugMode) {
        std::cout << "[TradingSystem] Trade request accepted" << std::endl;
    }
}

void TradingSystem::handleTradeDecline(const NetworkPacket& packet) {
    uint16_t playerId = packet.playerId;
    m_pendingRequests.erase(playerId);

    if (m_debugMode) {
        std::cout << "[TradingSystem] Trade request declined by player " << playerId << std::endl;
    }
}

void TradingSystem::handleTradeAddItem(const NetworkPacket& packet) {
    // Other player added an item
    if (m_onTradeItemUpdate && m_currentOffer) {
        m_onTradeItemUpdate(*m_currentOffer);
    }
}

void TradingSystem::handleTradeRemoveItem(const NetworkPacket& packet) {
    // Other player removed an item
    if (m_onTradeItemUpdate && m_currentOffer) {
        m_onTradeItemUpdate(*m_currentOffer);
    }
}

void TradingSystem::handleTradeConfirm(const NetworkPacket& packet) {
    if (!isInTrade()) {
        return;
    }

    uint16_t playerId = packet.playerId;
    if (m_currentOffer->playerId1 == playerId) {
        m_currentOffer->player1Confirmed = true;
    } else {
        m_currentOffer->player2Confirmed = true;
    }

    if (m_currentOffer->isBothConfirmed()) {
        executeTrade();
    }
}

void TradingSystem::handleTradeCancel(const NetworkPacket& packet) {
    if (isInTrade()) {
        if (m_onTradeCancelled) {
            m_onTradeCancelled("Trade cancelled by other player");
        }

        recordTradeHistory(*m_currentOffer, false);
        delete m_currentOffer;
        m_currentOffer = nullptr;
        m_totalCancelledTrades++;
    }
}

void TradingSystem::createTrade(uint16_t player1, uint16_t player2) {
    m_currentOffer = new TradeOffer();
    m_currentOffer->playerId1 = player1;
    m_currentOffer->playerId2 = player2;
    m_currentOffer->player1Name = "Player" + std::to_string(player1);
    m_currentOffer->player2Name = "Player" + std::to_string(player2);
    m_currentOffer->createdTime = std::chrono::system_clock::now();
    m_currentOffer->expiryTime = m_currentOffer->createdTime +
                                 std::chrono::seconds(static_cast<int>(m_tradeTimeout));
}

void TradingSystem::completeTrade() {
    // Record successful trade
    recordTradeHistory(*m_currentOffer, true);

    // Update statistics
    m_totalGoldTraded += m_currentOffer->player1Gold + m_currentOffer->player2Gold;
    m_totalItemsTraded += m_currentOffer->player1Items.size() + m_currentOffer->player2Items.size();
    m_totalSuccessfulTrades++;

    if (m_onTradeComplete) {
        TradeHistory history;
        history.wasSuccessful = true;
        history.timestamp = std::chrono::system_clock::now();
        m_onTradeComplete(history);
    }

    delete m_currentOffer;
    m_currentOffer = nullptr;
}

void TradingSystem::recordTradeHistory(const TradeOffer& offer, bool successful) {
    TradeHistory history;
    history.wasSuccessful = successful;
    history.timestamp = std::chrono::system_clock::now();

    uint16_t localId = m_networkManager.getLocalPlayerId();
    if (offer.playerId1 == localId) {
        history.otherPlayerId = offer.playerId2;
        history.otherPlayerName = offer.player2Name;
        history.itemsGiven = offer.player1Items;
        history.itemsReceived = offer.player2Items;
        history.goldGiven = offer.player1Gold;
        history.goldReceived = offer.player2Gold;
    } else {
        history.otherPlayerId = offer.playerId1;
        history.otherPlayerName = offer.player1Name;
        history.itemsGiven = offer.player2Items;
        history.itemsReceived = offer.player1Items;
        history.goldGiven = offer.player2Gold;
        history.goldReceived = offer.player1Gold;
    }

    m_tradeHistory.push_back(history);
}

bool TradingSystem::validateTradeItems() const {
    // Validate that all items exist and are tradeable
    return true;
}

void TradingSystem::notifyTradeUpdate() {
    NetworkPacket packet;
    packet.type = PacketType::TRADE_ADD_ITEM;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    packet.playerId = m_networkManager.getLocalPlayerId();

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    if (m_onTradeItemUpdate && m_currentOffer) {
        m_onTradeItemUpdate(*m_currentOffer);
    }
}

uint64_t TradingSystem::calculateItemValue(uint32_t itemId) const {
    // Calculate item market value
    return 100; // Placeholder
}

bool TradingSystem::isTradeBalanced(const TradeOffer& offer) const {
    uint64_t value1 = offer.player1Gold;
    uint64_t value2 = offer.player2Gold;

    for (const auto& item : offer.player1Items) {
        value1 += item.value;
    }

    for (const auto& item : offer.player2Items) {
        value2 += item.value;
    }

    if (value1 == 0 || value2 == 0) {
        return true;  // One-sided trade is allowed
    }

    float ratio = static_cast<float>(std::max(value1, value2)) / static_cast<float>(std::min(value1, value2));
    return ratio <= (1.0f + TRADE_BALANCE_THRESHOLD);
}

bool TradingSystem::detectSuspiciousActivity(const TradeOffer& offer) const {
    // Check for known scam patterns
    // e.g., extremely valuable items for nothing
    return false;
}

void TradingSystem::logTradeInfo() {
    std::cout << "\n===== Trading System Statistics =====" << std::endl;
    std::cout << "Currently in trade: " << (isInTrade() ? "YES" : "NO") << std::endl;
    std::cout << "Total trades completed: " << m_totalSuccessfulTrades << std::endl;
    std::cout << "Total trades cancelled: " << m_totalCancelledTrades << std::endl;
    std::cout << "Total gold traded: " << m_totalGoldTraded << std::endl;
    std::cout << "Total items traded: " << m_totalItemsTraded << std::endl;
    std::cout << "Blacklisted players: " << m_blacklistedPlayers.size() << std::endl;
    std::cout << "====================================\n" << std::endl;
}

} // namespace Network
