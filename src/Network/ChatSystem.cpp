#include "ChatSystem.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cctype>

namespace Network {

ChatSystem::ChatSystem()
    : m_spamProtectionEnabled(true)
    , m_maxMessagesPerSecond(DEFAULT_MAX_MESSAGES_PER_SECOND)
    , m_spamCounterTimer(0.0f)
    , m_profanityFilterEnabled(true)
    , m_soundNotificationsEnabled(true)
    , m_whisperNotificationsEnabled(true)
    , m_mentionNotificationsEnabled(true)
    , m_networkManager(NetworkManager::getInstance())
    , m_totalMessagesSent(0)
    , m_totalMessagesReceived(0)
    , m_totalWhispersSent(0)
    , m_totalWhispersReceived(0)
    , m_debugMode(false)
{
    // Initialize default channel settings
    for (int i = 0; i <= static_cast<int>(ChatMessage::Channel::COMBAT); i++) {
        auto channel = static_cast<ChatMessage::Channel>(i);
        m_channelSettings[channel] = ChannelSettings();
        m_unreadCounts[channel] = 0;
    }

    // Add some default profanity words
    m_profanityWords = {"badword1", "badword2", "badword3"};
}

ChatSystem::~ChatSystem() {
    shutdown();
}

void ChatSystem::initialize() {
    // Register packet handlers
    m_networkManager.registerPacketHandler(PacketType::CHAT_MESSAGE,
        [this](const NetworkPacket& packet) { handleChatMessage(packet); });

    m_networkManager.registerPacketHandler(PacketType::CHAT_WHISPER,
        [this](const NetworkPacket& packet) { handleWhisper(packet); });

    m_networkManager.registerPacketHandler(PacketType::CHAT_PARTY,
        [this](const NetworkPacket& packet) { handleChatMessage(packet); });

    m_networkManager.registerPacketHandler(PacketType::CHAT_GUILD,
        [this](const NetworkPacket& packet) { handleChatMessage(packet); });

    // Register default commands
    registerCommand("/help", [](const std::vector<std::string>& args) {
        std::cout << "Available commands: /help, /whisper, /party, /guild, /clear, /away" << std::endl;
    });

    registerCommand("/clear", [this](const std::vector<std::string>& args) {
        if (args.size() > 1) {
            // Clear specific channel
        } else {
            clearAllHistory();
        }
    });

    registerCommand("/away", [this](const std::vector<std::string>& args) {
        if (args.size() > 1) {
            std::string message;
            for (size_t i = 1; i < args.size(); i++) {
                if (i > 1) message += " ";
                message += args[i];
            }
            setAwayMessage(message);
        } else {
            clearAwayMessage();
        }
    });

    std::cout << "[ChatSystem] Initialized" << std::endl;
}

void ChatSystem::shutdown() {
    clearAllHistory();
    m_commands.clear();
}

void ChatSystem::sendMessage(const std::string& message, ChatMessage::Channel channel) {
    if (message.empty() || message.length() > MAX_MESSAGE_LENGTH) {
        return;
    }

    if (!canSendMessage()) {
        if (m_debugMode) {
            std::cout << "[ChatSystem] Message blocked by spam protection" << std::endl;
        }
        return;
    }

    // Check if it's a command
    if (isCommand(message)) {
        executeCommand(message);
        return;
    }

    ChatMessage chatMsg;
    chatMsg.senderId = m_networkManager.getLocalPlayerId();
    chatMsg.senderName = "Player" + std::to_string(chatMsg.senderId);
    chatMsg.content = message;
    chatMsg.channel = channel;
    chatMsg.timestamp = std::chrono::system_clock::now();
    chatMsg.isFromServer = false;

    // Apply profanity filter
    if (m_profanityFilterEnabled) {
        chatMsg.content = filterProfanity(chatMsg.content);
    }

    // Create and send packet
    NetworkPacket packet;
    packet.type = PacketType::CHAT_MESSAGE;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        chatMsg.timestamp.time_since_epoch()).count();
    packet.playerId = chatMsg.senderId;

    // Pack message data
    std::string data = std::to_string(static_cast<int>(channel)) + ":" + chatMsg.content;
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::UDP);

    // Add to local history
    addMessage(chatMsg);

    recordMessageSent();
    m_totalMessagesSent++;

    if (m_debugMode) {
        std::cout << "[ChatSystem] Sent message to " << getChannelName(channel) << std::endl;
    }
}

void ChatSystem::sendWhisper(uint16_t recipientId, const std::string& message) {
    if (message.empty()) {
        return;
    }

    if (!canSendMessage()) {
        return;
    }

    ChatMessage chatMsg;
    chatMsg.senderId = m_networkManager.getLocalPlayerId();
    chatMsg.senderName = "Player" + std::to_string(chatMsg.senderId);
    chatMsg.content = message;
    chatMsg.channel = ChatMessage::Channel::WHISPER;
    chatMsg.recipientId = recipientId;
    chatMsg.timestamp = std::chrono::system_clock::now();

    NetworkPacket packet;
    packet.type = PacketType::CHAT_WHISPER;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        chatMsg.timestamp.time_since_epoch()).count();
    packet.playerId = chatMsg.senderId;

    std::string data = std::to_string(recipientId) + ":" + message;
    packet.data.assign(data.begin(), data.end());
    packet.dataSize = static_cast<uint16_t>(packet.data.size());

    m_networkManager.sendPacket(packet, ProtocolType::TCP);

    addMessage(chatMsg);
    recordMessageSent();
    m_totalWhispersSent++;

    if (m_debugMode) {
        std::cout << "[ChatSystem] Sent whisper to player " << recipientId << std::endl;
    }
}

void ChatSystem::sendWhisper(const std::string& recipientName, const std::string& message) {
    // Would look up player ID by name
    // For now, just use a dummy ID
    sendWhisper(0, message);
}

void ChatSystem::sendPartyMessage(const std::string& message) {
    sendMessage(message, ChatMessage::Channel::PARTY);
}

void ChatSystem::sendGuildMessage(const std::string& message) {
    sendMessage(message, ChatMessage::Channel::GUILD);
}

void ChatSystem::sendLocalMessage(const std::string& message) {
    sendMessage(message, ChatMessage::Channel::LOCAL);
}

std::vector<ChatMessage> ChatSystem::getMessages(ChatMessage::Channel channel) const {
    auto it = m_channelMessages.find(channel);
    if (it != m_channelMessages.end()) {
        return it->second;
    }
    return std::vector<ChatMessage>();
}

std::vector<ChatMessage> ChatSystem::getAllMessages() const {
    std::vector<ChatMessage> all;

    for (const auto& [channel, messages] : m_channelMessages) {
        all.insert(all.end(), messages.begin(), messages.end());
    }

    // Sort by timestamp
    std::sort(all.begin(), all.end(),
        [](const ChatMessage& a, const ChatMessage& b) {
            return a.timestamp < b.timestamp;
        });

    return all;
}

ChatMessage ChatSystem::getLastMessage(ChatMessage::Channel channel) const {
    auto it = m_channelMessages.find(channel);
    if (it != m_channelMessages.end() && !it->second.empty()) {
        return it->second.back();
    }
    return ChatMessage();
}

int ChatSystem::getUnreadCount(ChatMessage::Channel channel) const {
    auto it = m_unreadCounts.find(channel);
    if (it != m_unreadCounts.end()) {
        return it->second;
    }
    return 0;
}

void ChatSystem::markChannelAsRead(ChatMessage::Channel channel) {
    m_unreadCounts[channel] = 0;
}

void ChatSystem::joinChannel(ChatMessage::Channel channel) {
    m_channelSettings[channel].enabled = true;
}

void ChatSystem::leaveChannel(ChatMessage::Channel channel) {
    m_channelSettings[channel].enabled = false;
}

bool ChatSystem::isInChannel(ChatMessage::Channel channel) const {
    auto it = m_channelSettings.find(channel);
    return it != m_channelSettings.end() && it->second.enabled;
}

void ChatSystem::setChannelSettings(ChatMessage::Channel channel, const ChannelSettings& settings) {
    m_channelSettings[channel] = settings;
}

ChannelSettings ChatSystem::getChannelSettings(ChatMessage::Channel channel) const {
    auto it = m_channelSettings.find(channel);
    if (it != m_channelSettings.end()) {
        return it->second;
    }
    return ChannelSettings();
}

void ChatSystem::blockPlayer(uint16_t playerId) {
    if (!isPlayerBlocked(playerId)) {
        m_blockedPlayers.push_back(playerId);

        if (m_debugMode) {
            std::cout << "[ChatSystem] Blocked player " << playerId << std::endl;
        }
    }
}

void ChatSystem::unblockPlayer(uint16_t playerId) {
    m_blockedPlayers.erase(
        std::remove(m_blockedPlayers.begin(), m_blockedPlayers.end(), playerId),
        m_blockedPlayers.end()
    );
}

bool ChatSystem::isPlayerBlocked(uint16_t playerId) const {
    return std::find(m_blockedPlayers.begin(), m_blockedPlayers.end(), playerId)
           != m_blockedPlayers.end();
}

std::vector<uint16_t> ChatSystem::getBlockedPlayers() const {
    return m_blockedPlayers;
}

void ChatSystem::addProfanityWord(const std::string& word) {
    m_profanityWords.push_back(word);
}

void ChatSystem::removeProfanityWord(const std::string& word) {
    m_profanityWords.erase(
        std::remove(m_profanityWords.begin(), m_profanityWords.end(), word),
        m_profanityWords.end()
    );
}

std::string ChatSystem::filterProfanity(const std::string& message) const {
    std::string filtered = message;

    for (const auto& word : m_profanityWords) {
        size_t pos = 0;
        while ((pos = filtered.find(word, pos)) != std::string::npos) {
            filtered.replace(pos, word.length(), std::string(word.length(), '*'));
            pos += word.length();
        }
    }

    return filtered;
}

bool ChatSystem::canSendMessage() const {
    if (!m_spamProtectionEnabled) {
        return true;
    }

    return static_cast<int>(m_recentMessages.size()) < m_maxMessagesPerSecond;
}

void ChatSystem::clearHistory(ChatMessage::Channel channel) {
    m_channelMessages[channel].clear();
    m_unreadCounts[channel] = 0;
}

void ChatSystem::clearAllHistory() {
    m_channelMessages.clear();
    m_unreadCounts.clear();
}

void ChatSystem::setAwayMessage(const std::string& message) {
    m_awayMessage = message;

    if (m_debugMode) {
        std::cout << "[ChatSystem] Set away message: " << message << std::endl;
    }
}

void ChatSystem::clearAwayMessage() {
    m_awayMessage.clear();
}

void ChatSystem::registerCommand(const std::string& command, std::function<void(const std::vector<std::string>&)> handler) {
    m_commands[command] = handler;
}

void ChatSystem::executeCommand(const std::string& input) {
    auto args = parseCommand(input);

    if (args.empty()) {
        return;
    }

    auto it = m_commands.find(args[0]);
    if (it != m_commands.end()) {
        it->second(args);
    } else {
        if (m_debugMode) {
            std::cout << "[ChatSystem] Unknown command: " << args[0] << std::endl;
        }
    }
}

bool ChatSystem::isCommand(const std::string& input) const {
    return !input.empty() && input[0] == '/';
}

void ChatSystem::sendEmote(const std::string& emote) {
    std::string message = "*" + emote + "*";
    sendMessage(message, ChatMessage::Channel::LOCAL);
}

std::vector<std::string> ChatSystem::getAvailableEmotes() const {
    return {"wave", "dance", "laugh", "cry", "angry", "cheer", "sleep", "sit"};
}

void ChatSystem::update(float deltaTime) {
    updateSpamCounter(deltaTime);
}

void ChatSystem::handleChatMessage(const NetworkPacket& packet) {
    if (packet.data.empty()) {
        return;
    }

    ChatMessage message;
    message.senderId = packet.playerId;
    message.senderName = "Player" + std::to_string(packet.playerId);
    message.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(packet.timestamp));
    message.isFromServer = false;

    // Parse channel and content
    std::string data(packet.data.begin(), packet.data.end());
    size_t colonPos = data.find(':');

    if (colonPos != std::string::npos) {
        int channelInt = std::stoi(data.substr(0, colonPos));
        message.channel = static_cast<ChatMessage::Channel>(channelInt);
        message.content = data.substr(colonPos + 1);
    } else {
        message.content = data;
        message.channel = ChatMessage::Channel::GLOBAL;
    }

    if (shouldReceiveMessage(message)) {
        processMessage(message);
        addMessage(message);
        m_totalMessagesReceived++;

        if (m_onMessageReceived) {
            m_onMessageReceived(message);
        }
    }
}

void ChatSystem::handleWhisper(const NetworkPacket& packet) {
    if (packet.data.empty()) {
        return;
    }

    ChatMessage message;
    message.senderId = packet.playerId;
    message.senderName = "Player" + std::to_string(packet.playerId);
    message.channel = ChatMessage::Channel::WHISPER;
    message.timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(packet.timestamp));

    std::string data(packet.data.begin(), packet.data.end());
    size_t colonPos = data.find(':');
    if (colonPos != std::string::npos) {
        message.content = data.substr(colonPos + 1);
    }

    if (!isPlayerBlocked(message.senderId)) {
        addMessage(message);
        m_totalWhispersReceived++;

        if (m_onWhisperReceived) {
            m_onWhisperReceived(message);
        }

        // Send auto-reply if away
        if (isAway()) {
            sendWhisper(message.senderId, "Auto-reply: " + m_awayMessage);
        }
    }
}

void ChatSystem::handleSystemMessage(const NetworkPacket& packet) {
    std::string message(packet.data.begin(), packet.data.end());

    if (m_onSystemMessage) {
        m_onSystemMessage(message);
    }

    ChatMessage chatMsg;
    chatMsg.senderId = 0;
    chatMsg.senderName = "SYSTEM";
    chatMsg.content = message;
    chatMsg.channel = ChatMessage::Channel::SYSTEM;
    chatMsg.timestamp = std::chrono::system_clock::now();
    chatMsg.isFromServer = true;

    addMessage(chatMsg);
}

void ChatSystem::addMessage(const ChatMessage& message) {
    auto& channelMsgs = m_channelMessages[message.channel];
    channelMsgs.push_back(message);

    // Limit history size
    const auto& settings = m_channelSettings[message.channel];
    if (channelMsgs.size() > settings.maxHistorySize) {
        channelMsgs.erase(channelMsgs.begin());
    }

    // Increment unread count
    m_unreadCounts[message.channel]++;
}

void ChatSystem::processMessage(ChatMessage& message) {
    // Apply profanity filter if enabled
    if (m_profanityFilterEnabled) {
        message.content = filterProfanity(message.content);
    }
}

bool ChatSystem::shouldReceiveMessage(const ChatMessage& message) const {
    // Check if player is blocked
    if (isPlayerBlocked(message.senderId)) {
        return false;
    }

    // Check if channel is enabled
    if (!isInChannel(message.channel)) {
        return false;
    }

    // Check if channel is muted
    const auto& settings = getChannelSettings(message.channel);
    if (settings.muted) {
        return false;
    }

    return true;
}

void ChatSystem::recordMessageSent() {
    m_recentMessages.push(std::chrono::steady_clock::now());
}

void ChatSystem::updateSpamCounter(float deltaTime) {
    auto now = std::chrono::steady_clock::now();

    // Remove old messages from spam counter
    while (!m_recentMessages.empty()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_recentMessages.front()).count();

        if (elapsed >= 1) {
            m_recentMessages.pop();
        } else {
            break;
        }
    }
}

bool ChatSystem::containsProfanity(const std::string& message) const {
    std::string lowerMsg = message;
    std::transform(lowerMsg.begin(), lowerMsg.end(), lowerMsg.begin(), ::tolower);

    for (const auto& word : m_profanityWords) {
        if (lowerMsg.find(word) != std::string::npos) {
            return true;
        }
    }

    return false;
}

std::vector<std::string> ChatSystem::parseCommand(const std::string& input) const {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string arg;

    while (iss >> arg) {
        args.push_back(arg);
    }

    return args;
}

std::string ChatSystem::getChannelName(ChatMessage::Channel channel) {
    switch (channel) {
        case ChatMessage::Channel::GLOBAL: return "Global";
        case ChatMessage::Channel::LOCAL: return "Local";
        case ChatMessage::Channel::PARTY: return "Party";
        case ChatMessage::Channel::GUILD: return "Guild";
        case ChatMessage::Channel::WHISPER: return "Whisper";
        case ChatMessage::Channel::SYSTEM: return "System";
        case ChatMessage::Channel::TRADE: return "Trade";
        case ChatMessage::Channel::COMBAT: return "Combat";
        default: return "Unknown";
    }
}

std::string ChatSystem::getChannelColor(ChatMessage::Channel channel) {
    switch (channel) {
        case ChatMessage::Channel::GLOBAL: return "#FFFFFF";
        case ChatMessage::Channel::LOCAL: return "#FFFF00";
        case ChatMessage::Channel::PARTY: return "#00FF00";
        case ChatMessage::Channel::GUILD: return "#00FFFF";
        case ChatMessage::Channel::WHISPER: return "#FF00FF";
        case ChatMessage::Channel::SYSTEM: return "#FF8800";
        case ChatMessage::Channel::TRADE: return "#FFD700";
        case ChatMessage::Channel::COMBAT: return "#FF0000";
        default: return "#CCCCCC";
    }
}

void ChatSystem::logChatStats() {
    std::cout << "\n===== Chat System Statistics =====" << std::endl;
    std::cout << "Messages Sent: " << m_totalMessagesSent << std::endl;
    std::cout << "Messages Received: " << m_totalMessagesReceived << std::endl;
    std::cout << "Whispers Sent: " << m_totalWhispersSent << std::endl;
    std::cout << "Whispers Received: " << m_totalWhispersReceived << std::endl;
    std::cout << "Blocked Players: " << m_blockedPlayers.size() << std::endl;

    std::cout << "\nChannel History:" << std::endl;
    for (const auto& [channel, messages] : m_channelMessages) {
        std::cout << "  " << getChannelName(channel) << ": " << messages.size() << " messages" << std::endl;
    }

    std::cout << "==================================\n" << std::endl;
}

} // namespace Network
