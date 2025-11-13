#pragma once

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace Network {

/**
 * @brief Trade item structure
 */
struct TradeItem {
    uint32_t itemId;
    std::string itemName;
    int quantity;
    int rarity;
    uint64_t value;

    TradeItem() : itemId(0), quantity(0), rarity(0), value(0) {}
};

/**
 * @brief Trade offer structure
 */
struct TradeOffer {
    uint16_t offerId;
    uint16_t playerId1;
    uint16_t playerId2;
    std::string player1Name;
    std::string player2Name;

    std::vector<TradeItem> player1Items;
    std::vector<TradeItem> player2Items;
    uint64_t player1Gold;
    uint64_t player2Gold;

    bool player1Ready;
    bool player2Ready;
    bool player1Confirmed;
    bool player2Confirmed;

    std::chrono::system_clock::time_point createdTime;
    std::chrono::system_clock::time_point expiryTime;

    TradeOffer()
        : offerId(0), playerId1(0), playerId2(0)
        , player1Gold(0), player2Gold(0)
        , player1Ready(false), player2Ready(false)
        , player1Confirmed(false), player2Confirmed(false) {}

    bool isExpired() const {
        return std::chrono::system_clock::now() > expiryTime;
    }

    bool isBothReady() const {
        return player1Ready && player2Ready;
    }

    bool isBothConfirmed() const {
        return player1Confirmed && player2Confirmed;
    }
};

/**
 * @brief Trading history entry
 */
struct TradeHistory {
    uint16_t otherPlayerId;
    std::string otherPlayerName;
    std::vector<TradeItem> itemsGiven;
    std::vector<TradeItem> itemsReceived;
    uint64_t goldGiven;
    uint64_t goldReceived;
    std::chrono::system_clock::time_point timestamp;
    bool wasSuccessful;
};

/**
 * @brief Player-to-player trading system
 *
 * Handles secure trading between players with item and gold exchange.
 * Includes trade requests, confirmations, and trade history tracking.
 */
class TradingSystem {
public:
    static TradingSystem& getInstance() {
        static TradingSystem instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Trade requests
    void requestTrade(uint16_t playerId);
    void acceptTradeRequest(uint16_t fromPlayerId);
    void declineTradeRequest(uint16_t fromPlayerId);
    void cancelTradeRequest();

    // Trade management
    bool isInTrade() const { return m_currentOffer != nullptr; }
    const TradeOffer* getCurrentTrade() const { return m_currentOffer; }
    void cancelTrade();

    // Adding items to trade
    void addItem(uint32_t itemId, int quantity = 1);
    void removeItem(uint32_t itemId);
    void clearItems();
    void setGoldOffer(uint64_t amount);
    std::vector<TradeItem> getMyOfferedItems() const;
    uint64_t getMyOfferedGold() const;

    // Trade confirmation
    void setReady(bool ready);
    void confirmTrade();
    void executeTrade();
    bool isReady() const;
    bool isOtherPlayerReady() const;

    // Trade history
    std::vector<TradeHistory> getTradeHistory() const { return m_tradeHistory; }
    void clearTradeHistory();
    int getTradeCount() const { return static_cast<int>(m_tradeHistory.size()); }
    int getSuccessfulTradeCount() const;

    // Trade validation
    bool canTrade() const;
    bool canAddItem(uint32_t itemId) const;
    bool hasItem(uint32_t itemId, int quantity) const;
    bool hasGold(uint64_t amount) const;
    bool isItemTradeable(uint32_t itemId) const;

    // Trade settings
    void enableTradeRequests(bool enable) { m_tradeRequestsEnabled = enable; }
    bool areTradeRequestsEnabled() const { return m_tradeRequestsEnabled; }
    void setTradeTimeout(float seconds) { m_tradeTimeout = seconds; }
    float getTradeTimeout() const { return m_tradeTimeout; }

    // Blacklist
    void addToBlacklist(uint16_t playerId);
    void removeFromBlacklist(uint16_t playerId);
    bool isBlacklisted(uint16_t playerId) const;
    std::vector<uint16_t> getBlacklist() const { return m_blacklistedPlayers; }

    // Trade verification (prevent scams)
    void enableTradeVerification(bool enable) { m_verificationEnabled = enable; }
    bool isTradeVerificationEnabled() const { return m_verificationEnabled; }
    bool verifyTrade(const TradeOffer& offer) const;

    // Update
    void update(float deltaTime);

    // Callbacks
    using TradeRequestCallback = std::function<void(uint16_t playerId, const std::string& playerName)>;
    using TradeAcceptedCallback = std::function<void(uint16_t playerId)>;
    using TradeItemUpdateCallback = std::function<void(const TradeOffer& offer)>;
    using TradeCompleteCallback = std::function<void(const TradeHistory& trade)>;
    using TradeCancelledCallback = std::function<void(const std::string& reason)>;

    void setOnTradeRequest(TradeRequestCallback callback) { m_onTradeRequest = callback; }
    void setOnTradeAccepted(TradeAcceptedCallback callback) { m_onTradeAccepted = callback; }
    void setOnTradeItemUpdate(TradeItemUpdateCallback callback) { m_onTradeItemUpdate = callback; }
    void setOnTradeComplete(TradeCompleteCallback callback) { m_onTradeComplete = callback; }
    void setOnTradeCancelled(TradeCancelledCallback callback) { m_onTradeCancelled = callback; }

    // Statistics
    uint64_t getTotalGoldTraded() const { return m_totalGoldTraded; }
    int getTotalItemsTraded() const { return m_totalItemsTraded; }

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    void logTradeInfo();

private:
    TradingSystem();
    ~TradingSystem();
    TradingSystem(const TradingSystem&) = delete;
    TradingSystem& operator=(const TradingSystem&) = delete;

    // Packet handlers
    void handleTradeRequest(const NetworkPacket& packet);
    void handleTradeAccept(const NetworkPacket& packet);
    void handleTradeDecline(const NetworkPacket& packet);
    void handleTradeAddItem(const NetworkPacket& packet);
    void handleTradeRemoveItem(const NetworkPacket& packet);
    void handleTradeConfirm(const NetworkPacket& packet);
    void handleTradeCancel(const NetworkPacket& packet);

    // Internal helpers
    void createTrade(uint16_t player1, uint16_t player2);
    void completeTrade();
    void recordTradeHistory(const TradeOffer& offer, bool successful);
    bool validateTradeItems() const;
    void notifyTradeUpdate();

    // Anti-scam helpers
    uint64_t calculateItemValue(uint32_t itemId) const;
    bool isTradeBalanced(const TradeOffer& offer) const;
    bool detectSuspiciousActivity(const TradeOffer& offer) const;

    // Data members
    TradeOffer* m_currentOffer;
    std::vector<TradeHistory> m_tradeHistory;
    std::vector<uint16_t> m_blacklistedPlayers;
    std::unordered_map<uint16_t, std::chrono::system_clock::time_point> m_pendingRequests;

    // Settings
    bool m_tradeRequestsEnabled;
    bool m_verificationEnabled;
    float m_tradeTimeout;

    // Statistics
    uint64_t m_totalGoldTraded;
    int m_totalItemsTraded;
    int m_totalSuccessfulTrades;
    int m_totalCancelledTrades;

    // Callbacks
    TradeRequestCallback m_onTradeRequest;
    TradeAcceptedCallback m_onTradeAccepted;
    TradeItemUpdateCallback m_onTradeItemUpdate;
    TradeCompleteCallback m_onTradeComplete;
    TradeCancelledCallback m_onTradeCancelled;

    // Network reference
    NetworkManager& m_networkManager;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr float DEFAULT_TRADE_TIMEOUT = 300.0f;  // 5 minutes
    static constexpr int MAX_TRADE_ITEMS = 20;
    static constexpr float TRADE_BALANCE_THRESHOLD = 0.3f;  // 30% value difference warning
};

} // namespace Network
