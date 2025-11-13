#pragma once

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace Network {

/**
 * @brief Chat message structure
 */
struct ChatMessage {
    enum class Channel {
        GLOBAL,     // All players
        LOCAL,      // Nearby players
        PARTY,      // Party members only
        GUILD,      // Guild members only
        WHISPER,    // Private message
        SYSTEM,     // System announcements
        TRADE,      // Trade channel
        COMBAT      // Combat log
    };

    uint16_t senderId;
    std::string senderName;
    std::string content;
    Channel channel;
    uint16_t recipientId;  // For whispers
    std::chrono::system_clock::time_point timestamp;
    bool isFromServer;

    ChatMessage()
        : senderId(0), channel(Channel::GLOBAL)
        , recipientId(0), isFromServer(false) {}
};

/**
 * @brief Chat channel settings
 */
struct ChannelSettings {
    bool enabled;
    bool muted;
    bool showTimestamp;
    bool showPlayerNames;
    int maxHistorySize;
    std::vector<uint16_t> blockedPlayers;

    ChannelSettings()
        : enabled(true), muted(false)
        , showTimestamp(true), showPlayerNames(true)
        , maxHistorySize(100) {}
};

/**
 * @brief In-game chat system with multiple channels
 *
 * Supports global, local, party, guild, whisper, and system channels.
 * Includes profanity filtering, spam protection, and player blocking.
 */
class ChatSystem {
public:
    static ChatSystem& getInstance() {
        static ChatSystem instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Sending messages
    void sendMessage(const std::string& message, ChatMessage::Channel channel = ChatMessage::Channel::GLOBAL);
    void sendWhisper(uint16_t recipientId, const std::string& message);
    void sendWhisper(const std::string& recipientName, const std::string& message);
    void sendPartyMessage(const std::string& message);
    void sendGuildMessage(const std::string& message);
    void sendLocalMessage(const std::string& message);

    // Receiving messages
    std::vector<ChatMessage> getMessages(ChatMessage::Channel channel) const;
    std::vector<ChatMessage> getAllMessages() const;
    ChatMessage getLastMessage(ChatMessage::Channel channel) const;
    int getUnreadCount(ChatMessage::Channel channel) const;
    void markChannelAsRead(ChatMessage::Channel channel);

    // Channel management
    void joinChannel(ChatMessage::Channel channel);
    void leaveChannel(ChatMessage::Channel channel);
    bool isInChannel(ChatMessage::Channel channel) const;
    void setChannelSettings(ChatMessage::Channel channel, const ChannelSettings& settings);
    ChannelSettings getChannelSettings(ChatMessage::Channel channel) const;

    // Player blocking
    void blockPlayer(uint16_t playerId);
    void unblockPlayer(uint16_t playerId);
    bool isPlayerBlocked(uint16_t playerId) const;
    std::vector<uint16_t> getBlockedPlayers() const;

    // Profanity filter
    void enableProfanityFilter(bool enable) { m_profanityFilterEnabled = enable; }
    bool isProfanityFilterEnabled() const { return m_profanityFilterEnabled; }
    void addProfanityWord(const std::string& word);
    void removeProfanityWord(const std::string& word);
    std::string filterProfanity(const std::string& message) const;

    // Spam protection
    void enableSpamProtection(bool enable) { m_spamProtectionEnabled = enable; }
    bool isSpamProtectionEnabled() const { return m_spamProtectionEnabled; }
    void setMaxMessagesPerSecond(int max) { m_maxMessagesPerSecond = max; }
    bool canSendMessage() const;

    // Chat history
    void clearHistory(ChatMessage::Channel channel);
    void clearAllHistory();
    void saveHistory(const std::string& filename) const;
    void loadHistory(const std::string& filename);

    // Auto-responses
    void setAwayMessage(const std::string& message);
    void clearAwayMessage();
    bool isAway() const { return !m_awayMessage.empty(); }
    std::string getAwayMessage() const { return m_awayMessage; }

    // Chat commands
    void registerCommand(const std::string& command, std::function<void(const std::vector<std::string>&)> handler);
    void executeCommand(const std::string& input);
    bool isCommand(const std::string& input) const;

    // Emotes
    void sendEmote(const std::string& emote);
    std::vector<std::string> getAvailableEmotes() const;

    // Chat notifications
    void enableSoundNotifications(bool enable) { m_soundNotificationsEnabled = enable; }
    bool areSoundNotificationsEnabled() const { return m_soundNotificationsEnabled; }
    void enableWhisperNotifications(bool enable) { m_whisperNotificationsEnabled = enable; }
    void enableMentionNotifications(bool enable) { m_mentionNotificationsEnabled = enable; }

    // Update
    void update(float deltaTime);

    // Callbacks
    using MessageCallback = std::function<void(const ChatMessage&)>;
    using WhisperCallback = std::function<void(const ChatMessage&)>;
    using SystemMessageCallback = std::function<void(const std::string&)>;

    void setOnMessageReceived(MessageCallback callback) { m_onMessageReceived = callback; }
    void setOnWhisperReceived(WhisperCallback callback) { m_onWhisperReceived = callback; }
    void setOnSystemMessage(SystemMessageCallback callback) { m_onSystemMessage = callback; }

    // Utility
    static std::string getChannelName(ChatMessage::Channel channel);
    static std::string getChannelColor(ChatMessage::Channel channel);

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    void logChatStats();

private:
    ChatSystem();
    ~ChatSystem();
    ChatSystem(const ChatSystem&) = delete;
    ChatSystem& operator=(const ChatSystem&) = delete;

    // Packet handlers
    void handleChatMessage(const NetworkPacket& packet);
    void handleWhisper(const NetworkPacket& packet);
    void handleSystemMessage(const NetworkPacket& packet);

    // Internal message handling
    void addMessage(const ChatMessage& message);
    void processMessage(ChatMessage& message);
    bool shouldReceiveMessage(const ChatMessage& message) const;

    // Spam detection
    void recordMessageSent();
    void updateSpamCounter(float deltaTime);

    // Profanity filtering
    bool containsProfanity(const std::string& message) const;

    // Command parsing
    std::vector<std::string> parseCommand(const std::string& input) const;

    // Data members
    std::unordered_map<ChatMessage::Channel, std::vector<ChatMessage>> m_channelMessages;
    std::unordered_map<ChatMessage::Channel, ChannelSettings> m_channelSettings;
    std::unordered_map<ChatMessage::Channel, int> m_unreadCounts;
    std::vector<uint16_t> m_blockedPlayers;
    std::vector<std::string> m_profanityWords;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> m_commands;

    // Spam protection
    std::queue<std::chrono::steady_clock::time_point> m_recentMessages;
    bool m_spamProtectionEnabled;
    int m_maxMessagesPerSecond;
    float m_spamCounterTimer;

    // Settings
    bool m_profanityFilterEnabled;
    bool m_soundNotificationsEnabled;
    bool m_whisperNotificationsEnabled;
    bool m_mentionNotificationsEnabled;

    // Away system
    std::string m_awayMessage;

    // Callbacks
    MessageCallback m_onMessageReceived;
    WhisperCallback m_onWhisperReceived;
    SystemMessageCallback m_onSystemMessage;

    // Network reference
    NetworkManager& m_networkManager;

    // Statistics
    int m_totalMessagesSent;
    int m_totalMessagesReceived;
    int m_totalWhispersSent;
    int m_totalWhispersReceived;

    // Debug
    bool m_debugMode;

    // Constants
    static constexpr int DEFAULT_MAX_MESSAGES_PER_SECOND = 5;
    static constexpr int DEFAULT_MAX_HISTORY_SIZE = 100;
    static constexpr int MAX_MESSAGE_LENGTH = 500;
};

} // namespace Network
