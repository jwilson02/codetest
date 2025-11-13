# Multiplayer & Co-op Network System

Comprehensive multiplayer and co-op functionality for shared adventures in the SCUMM-VM ARPG game.

## Overview

This network system provides robust client-server architecture with UDP for fast gameplay updates and TCP for reliable important data. It includes lag compensation, client-side prediction, and all necessary features for a stable multiplayer experience.

## Architecture

### Core Components

1. **NetworkManager** - Central network management
2. **ServerConnection** - Server connectivity and authentication
3. **PlayerSync** - Player state synchronization
4. **PartySystem** - Co-op party management
5. **ChatSystem** - Multi-channel chat
6. **TradingSystem** - Player-to-player trading
7. **Leaderboard** - Rankings and achievements

## Features

### 1. Network Manager (`NetworkManager.h/cpp`)

**Core Features:**
- Client-server architecture
- Dual protocol support (UDP for gameplay, TCP for critical data)
- Automatic heartbeat system
- Connection timeout detection
- Bandwidth management and throttling
- Packet reliability system for UDP
- Lag compensation support
- Client-side prediction
- Debug mode for development

**Key Methods:**
```cpp
bool initialize(bool isServer, uint16_t tcpPort, uint16_t udpPort);
bool connectToServer(const std::string& serverIP, uint16_t tcpPort, uint16_t udpPort);
void sendPacket(const NetworkPacket& packet, ProtocolType protocol);
void broadcastPacket(const NetworkPacket& packet, ProtocolType protocol);
void registerPacketHandler(PacketType type, PacketCallback callback);
```

**Network Statistics:**
- Packets sent/received
- Bytes transferred
- Packet loss rate
- Average ping
- Bandwidth usage

### 2. Server Connection (`ServerConnection.h/cpp`)

**Features:**
- Connection state management
- Authentication system
- Auto-reconnection
- Server browser
- Server list management
- Connection quality monitoring
- Token-based authentication
- Version checking

**Connection States:**
- DISCONNECTED
- CONNECTING
- AUTHENTICATING
- CONNECTED
- RECONNECTING
- ERROR

**Server Browser:**
- Filter by region
- Filter by game mode
- Sort by ping/players
- Favorites system
- Recent servers list

### 3. Player Synchronization (`PlayerSync.h/cpp`)

**Features:**
- Real-time position synchronization (20Hz default)
- Player action broadcasting
- Stats synchronization (5Hz default)
- Interpolation for smooth movement
- Client-side prediction
- Dead reckoning
- Lag compensation with snapshot history
- Delta compression for bandwidth optimization
- Priority-based updates (nearby players prioritized)

**Synchronized Data:**
- Position, rotation, velocity
- Animation state
- Health, mana, stamina
- Combat status
- Equipment

**Advanced Features:**
- Configurable update rates
- Interpolation delay adjustment
- Snapshot history for lag compensation (3 seconds @ 20Hz)
- Bandwidth throttling
- Distance-based priority system

### 4. Party System (`PartySystem.h/cpp`)

**Features:**
- Party creation and management (up to 4 players)
- Party invitations with timeout (60 seconds)
- Member roles (leader, member)
- Ready check system (30 second timeout)
- Loot distribution modes:
  - Free for all
  - Round robin
  - Master looter
  - Need before greed
- Experience sharing modes:
  - Equal split
  - Level-weighted
  - Damage-based
- Quest sharing
- Party chat
- Waypoint markers
- Target marking

**Party Settings:**
- Configurable max members
- Level restrictions
- Public/private parties
- Join request settings

**Loot System:**
- Need/Greed/Pass rolling
- 30-second roll timeout
- Automatic distribution
- Roll conflict resolution

### 5. Chat System (`ChatSystem.h/cpp`)

**Channels:**
- Global (all players)
- Local (nearby players)
- Party (party members)
- Guild (guild members)
- Whisper (private messages)
- System (announcements)
- Trade (trading channel)
- Combat (combat log)

**Features:**
- Multi-channel support
- Player blocking
- Profanity filter (customizable)
- Spam protection (5 messages/second limit)
- Chat history (100 messages/channel)
- Away messages
- Chat commands (/help, /whisper, /party, /guild, /clear, /away)
- Emote system
- Sound notifications
- Mention notifications

**Settings Per Channel:**
- Enable/disable
- Mute
- Show timestamps
- Show player names
- History size limit

### 6. Trading System (`TradingSystem.h/cpp`)

**Features:**
- Secure player-to-player trading
- Item and gold exchange
- Trade request system
- Two-stage confirmation (ready + confirm)
- Trade timeout (5 minutes default)
- Trade history tracking
- Anti-scam verification
- Trade blacklist
- Maximum 20 items per trade

**Trade Flow:**
1. Request trade
2. Accept/decline
3. Add items and gold
4. Mark ready (both players)
5. Final confirmation (both players)
6. Execute trade

**Security Features:**
- Trade verification
- Balance checking (warns if >30% value difference)
- Suspicious activity detection
- Item validation
- Gold validation

**Trade History:**
- Track all trades
- Success/failure status
- Items exchanged
- Gold exchanged
- Timestamps

### 7. Leaderboard System (`Leaderboard.h/cpp`)

**Leaderboard Categories:**
- Overall ranking
- Level
- PvP kills
- PvP rating
- Arena wins
- Dungeon clears
- Boss kills
- Gold earned
- Quests completed
- Achievements
- Playtime
- Speed runs
- Crafting
- Trading volume
- Guild ratings

**Features:**
- Global rankings
- Regional filtering
- Class filtering
- Friend comparisons
- Guild comparisons
- Seasonal rankings
- Achievement system
- Arena tiers (Bronze, Silver, Gold, Platinum, Diamond, Master)
- Guild rankings
- Auto-refresh (configurable)
- Cache system (5-minute default)

**Arena System:**
- Rating-based matchmaking
- Win/loss tracking
- Win streak tracking
- Best rating tracking
- Tier progression

**Achievements:**
- Unlock tracking
- Points system
- Completion percentage
- Secret achievements
- Timestamps

## Network Optimization

### Lag Compensation

The system implements lag compensation by:
1. Storing snapshot history (60 snapshots @ 20Hz = 3 seconds)
2. Rewinding game state based on player latency
3. Validating actions against historical states
4. Smoothly interpolating between states

### Client-Side Prediction

Features:
- Predict local player movement
- Immediate response to input
- Server reconciliation
- Smooth correction of mispredictions

### Bandwidth Optimization

- Delta compression (only send changed values)
- Update rate throttling
- Priority-based updates
- Distance-based culling
- Configurable bandwidth limits (1MB/s default)

### Interpolation

- Linear interpolation between states
- Configurable delay (100ms default)
- Smooth movement rendering
- Animation blending

## Usage Examples

### Initialize Network System

```cpp
#include "Network/NetworkManager.h"
#include "Network/ServerConnection.h"
#include "Network/PlayerSync.h"
#include "Network/PartySystem.h"
#include "Network/ChatSystem.h"
#include "Network/TradingSystem.h"
#include "Network/Leaderboard.h"

// Initialize network
auto& netManager = Network::NetworkManager::getInstance();
netManager.initialize(false, 7777, 7778); // Client mode

// Initialize systems
auto& connection = Network::ServerConnection();
auto& playerSync = Network::PlayerSync::getInstance();
auto& partySystem = Network::PartySystem::getInstance();
auto& chatSystem = Network::ChatSystem::getInstance();
auto& tradingSystem = Network::TradingSystem::getInstance();
auto& leaderboard = Network::Leaderboard::getInstance();

playerSync.initialize();
partySystem.initialize();
chatSystem.initialize();
tradingSystem.initialize();
leaderboard.initialize();
```

### Connect to Server

```cpp
Network::ServerConnection::ServerInfo server;
server.name = "Main Server";
server.address = "127.0.0.1";
server.tcpPort = 7777;
server.udpPort = 7778;

Network::ServerConnection::AuthenticationData auth;
auth.username = "Player1";
auth.password = "password";
auth.clientVersion = "1.0.0";

connection.connect(server, auth);
```

### Send Chat Message

```cpp
auto& chat = Network::ChatSystem::getInstance();

// Send to global chat
chat.sendMessage("Hello, world!", Network::ChatMessage::Channel::GLOBAL);

// Send whisper
chat.sendWhisper(targetPlayerId, "Private message");

// Send party message
chat.sendPartyMessage("Let's go!");
```

### Create and Manage Party

```cpp
auto& party = Network::PartySystem::getInstance();

// Create party
party.createParty("My Party");

// Invite player
party.invitePlayer(playerId);

// Accept invite
party.acceptInvite(fromPlayerId);

// Share quest
party.shareQuest("quest_001");

// Set loot mode
party.setLootMode(Network::PartySettings::LootMode::NEED_BEFORE_GREED);

// Initiate ready check
party.initiateReadyCheck();
```

### Trade with Player

```cpp
auto& trading = Network::TradingSystem::getInstance();

// Request trade
trading.requestTrade(targetPlayerId);

// Accept trade request
trading.acceptTradeRequest(fromPlayerId);

// Add item to trade
trading.addItem(itemId, quantity);

// Offer gold
trading.setGoldOffer(1000);

// Mark ready
trading.setReady(true);

// Confirm trade
trading.confirmTrade();
```

### Check Leaderboard

```cpp
auto& leaderboard = Network::Leaderboard::getInstance();

// Request leaderboard
leaderboard.requestLeaderboard(Network::LeaderboardCategory::PVP_KILLS);

// Get my rank
int myRank = leaderboard.getMyRank(Network::LeaderboardCategory::OVERALL);

// Submit score
leaderboard.submitScore(Network::LeaderboardCategory::PVP_KILLS, 100);

// Request achievements
leaderboard.requestAchievements();

// Get achievement completion
float completion = leaderboard.getAchievementCompletion();
```

### Update in Game Loop

```cpp
void GameUpdate(float deltaTime) {
    auto& netManager = Network::NetworkManager::getInstance();
    auto& playerSync = Network::PlayerSync::getInstance();
    auto& partySystem = Network::PartySystem::getInstance();
    auto& chatSystem = Network::ChatSystem::getInstance();
    auto& tradingSystem = Network::TradingSystem::getInstance();
    auto& leaderboard = Network::Leaderboard::getInstance();

    netManager.update(deltaTime);
    playerSync.update(deltaTime);
    partySystem.update(deltaTime);
    chatSystem.update(deltaTime);
    tradingSystem.update(deltaTime);
    leaderboard.update(deltaTime);
}
```

## Configuration

### Network Settings

```cpp
// Adjust update rates
playerSync.setPositionUpdateRate(30.0f); // 30 Hz
playerSync.setStatsUpdateRate(10.0f);    // 10 Hz

// Configure interpolation
playerSync.setInterpolationDelay(0.15f); // 150ms
playerSync.enableInterpolation(true);

// Enable prediction
playerSync.enablePrediction(true);
playerSync.setPredictionTime(0.05f); // 50ms

// Bandwidth limits
netManager.setMaxBandwidth(2 * 1024 * 1024); // 2 MB/s
```

### Party Settings

```cpp
Network::PartySettings settings;
settings.partyName = "Elite Squad";
settings.lootMode = Network::PartySettings::LootMode::NEED_BEFORE_GREED;
settings.expShare = Network::PartySettings::ExperienceShare::LEVEL_WEIGHTED;
settings.maxMembers = 4;
settings.allowJoinRequests = true;
settings.isPublic = false;

partySystem.updateSettings(settings);
```

### Chat Settings

```cpp
// Enable profanity filter
chatSystem.enableProfanityFilter(true);

// Set spam protection
chatSystem.enableSpamProtection(true);
chatSystem.setMaxMessagesPerSecond(5);

// Configure channel
Network::ChannelSettings channelSettings;
channelSettings.enabled = true;
channelSettings.showTimestamp = true;
channelSettings.maxHistorySize = 200;

chatSystem.setChannelSettings(Network::ChatMessage::Channel::GLOBAL, channelSettings);
```

## Debug Mode

All systems support debug mode for development:

```cpp
netManager.setDebugMode(true);
playerSync.setDebugMode(true);
partySystem.setDebugMode(true);
chatSystem.setDebugMode(true);
tradingSystem.setDebugMode(true);
leaderboard.setDebugMode(true);

// Log statistics
playerSync.logSyncStats();
partySystem.logPartyInfo();
chatSystem.logChatStats();
tradingSystem.logTradeInfo();
leaderboard.logLeaderboardInfo();
```

## Platform Support

The network system is cross-platform compatible:
- Windows (Winsock2)
- Linux (POSIX sockets)
- macOS (POSIX sockets)

Socket initialization is handled automatically based on the platform.

## Security Considerations

1. **Authentication**: Token-based authentication with expiry
2. **Encryption**: Use TLS/SSL for TCP connections (implementation required)
3. **Validation**: Server-side validation of all actions
4. **Anti-cheat**: Lag compensation prevents time manipulation
5. **Rate limiting**: Spam protection on all systems
6. **Input sanitization**: Profanity filter and message validation

## Performance Metrics

**Recommended Settings:**
- Position updates: 20-30 Hz
- Stats updates: 5-10 Hz
- Interpolation delay: 100-150ms
- Max bandwidth: 1-2 MB/s per client
- Snapshot history: 60 snapshots (3 seconds @ 20Hz)

**Expected Performance:**
- Latency: <100ms good, <50ms excellent
- Packet loss: <1% acceptable, <0.1% excellent
- Bandwidth per player: 10-50 KB/s average
- CPU usage: <5% for network systems

## Future Enhancements

1. Voice chat integration
2. Replay system
3. Spectator mode
4. Cross-server play
5. Persistent world events
6. Clan/guild system expansion
7. Matchmaking system
8. Tournament system
9. Anti-cheat improvements
10. Mobile platform support

## Files Created

### Header Files (.h)
- `NetworkManager.h` - Core network management (11KB, 327 lines)
- `ServerConnection.h` - Server connectivity (8.4KB, 238 lines)
- `PlayerSync.h` - Player synchronization (11KB, 355 lines)
- `PartySystem.h` - Party/co-op system (9.3KB, 281 lines)
- `ChatSystem.h` - Chat system (7.9KB, 233 lines)
- `TradingSystem.h` - Trading system (7.5KB, 228 lines)
- `Leaderboard.h` - Rankings/achievements (9.4KB, 284 lines)

### Implementation Files (.cpp)
- `NetworkManager.cpp` - Network implementation (20KB, 640 lines)
- `ServerConnection.cpp` - Connection implementation (18KB, 485 lines)
- `PlayerSync.cpp` - Sync implementation (25KB, 703 lines)
- `PartySystem.cpp` - Party implementation (20KB, 565 lines)
- `ChatSystem.cpp` - Chat implementation (18KB, 562 lines)
- `TradingSystem.cpp` - Trading implementation (20KB, 607 lines)
- `Leaderboard.cpp` - Leaderboard implementation (18KB, 544 lines)

**Total: 14 files, 6,602 lines of code**

## License

This network system is part of the SCUMM-VM ARPG game project.

## Credits

Created as part of the multiplayer and co-op functionality expansion for shared adventures.
