#ifndef QUEST_SYSTEM_H
#define QUEST_SYSTEM_H

#include "Quest.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace QuestSystem {

// Quest notification types
enum class QuestNotificationType {
    QUEST_AVAILABLE,
    QUEST_STARTED,
    QUEST_UPDATED,
    QUEST_COMPLETED,
    QUEST_FAILED,
    OBJECTIVE_COMPLETED,
    CHAIN_UNLOCKED,
    REWARD_GAINED
};

// Quest notification structure
struct QuestNotification {
    QuestNotificationType type;
    std::string questId;
    std::string message;
    std::chrono::system_clock::time_point timestamp;

    QuestNotification(QuestNotificationType t, const std::string& id, const std::string& msg)
        : type(t), questId(id), message(msg)
        , timestamp(std::chrono::system_clock::now()) {}
};

// Player quest data
struct PlayerQuestData {
    int level;
    std::map<Faction, int> reputations;
    std::vector<std::string> completedQuestIds;
    std::vector<std::string> activeQuestIds;
    std::vector<std::string> availableQuestIds;
    std::map<std::string, int> questCompletionCount;  // For repeatable quests
    std::chrono::system_clock::time_point lastDailyReset;

    PlayerQuestData() : level(1) {}
};

// Quest log entry
struct QuestLogEntry {
    std::shared_ptr<Quest> quest;
    std::chrono::system_clock::time_point acceptedTime;
    std::map<std::string, int> objectiveProgress;
    int attempts;

    QuestLogEntry() : attempts(0) {}
};

// Quest system manager
class QuestSystem {
public:
    static QuestSystem& getInstance() {
        static QuestSystem instance;
        return instance;
    }

    // Initialization
    void initialize();
    void shutdown();

    // Quest loading
    bool loadQuestFromJson(const std::string& jsonPath);
    bool loadQuestsFromDirectory(const std::string& directory);
    void registerQuest(std::shared_ptr<Quest> quest);

    // Quest access
    std::shared_ptr<Quest> getQuest(const std::string& questId);
    std::vector<std::shared_ptr<Quest>> getAllQuests() const;
    std::vector<std::shared_ptr<Quest>> getQuestsByType(QuestType type) const;
    std::vector<std::shared_ptr<Quest>> getQuestsByFaction(Faction faction) const;

    // Player quest management
    void setPlayerData(const PlayerQuestData& data) { m_playerData = data; }
    PlayerQuestData& getPlayerData() { return m_playerData; }

    // Quest availability
    void updateAvailableQuests();
    std::vector<std::shared_ptr<Quest>> getAvailableQuests() const;
    std::vector<std::shared_ptr<Quest>> getActiveQuests() const;
    std::vector<std::shared_ptr<Quest>> getCompletedQuests() const;
    bool isQuestAvailable(const std::string& questId) const;
    bool isQuestCompleted(const std::string& questId) const;

    // Quest operations
    bool acceptQuest(const std::string& questId);
    bool completeQuest(const std::string& questId);
    bool failQuest(const std::string& questId);
    bool abandonQuest(const std::string& questId);

    // Objective updates
    void updateObjective(const std::string& questId, const std::string& objectiveId, int progress);
    void notifyEnemyKilled(const std::string& enemyId);
    void notifyItemCollected(const std::string& itemId, int count = 1);
    void notifyLocationReached(const std::string& locationId);
    void notifyNpcInteraction(const std::string& npcId);

    // Quest chains
    void unlockQuestChain(const std::string& questId);
    std::vector<std::string> getQuestChain(const std::string& startQuestId) const;

    // Timed quests
    void updateTimedQuests(float deltaTime);
    void checkExpiredQuests();

    // Daily quests
    void resetDailyQuests();
    bool isDailyResetNeeded() const;

    // Reputation system
    void updateReputation(Faction faction, int amount);
    int getReputation(Faction faction) const;
    std::string getReputationTitle(Faction faction) const;

    // Quest tracking
    void setTrackedQuest(const std::string& questId);
    std::shared_ptr<Quest> getTrackedQuest() const;
    void clearTrackedQuest();

    // Notifications
    std::vector<QuestNotification> getNotifications() const { return m_notifications; }
    void clearNotifications() { m_notifications.clear(); }
    void addNotification(QuestNotificationType type, const std::string& questId, const std::string& message);

    // Quest discovery (hidden quests)
    void discoverHiddenQuest(const std::string& questId);
    std::vector<std::shared_ptr<Quest>> getDiscoveredHiddenQuests() const;

    // Statistics
    int getTotalQuestsCompleted() const;
    int getQuestsCompletedByType(QuestType type) const;
    float getQuestCompletionRate() const;
    std::map<std::string, int> getQuestStatistics() const;

    // Save/Load
    std::string serializePlayerProgress() const;
    void loadPlayerProgress(const std::string& json);

    // Callbacks
    void setOnQuestAccepted(std::function<void(std::shared_ptr<Quest>)> callback) {
        m_onQuestAccepted = callback;
    }
    void setOnQuestCompleted(std::function<void(std::shared_ptr<Quest>)> callback) {
        m_onQuestCompleted = callback;
    }
    void setOnObjectiveUpdated(std::function<void(std::shared_ptr<Quest>, const std::string&)> callback) {
        m_onObjectiveUpdated = callback;
    }

private:
    QuestSystem() = default;
    ~QuestSystem() = default;
    QuestSystem(const QuestSystem&) = delete;
    QuestSystem& operator=(const QuestSystem&) = delete;

    // Internal helper methods
    void grantReward(const QuestReward& reward);
    void unlockNextQuests(const std::vector<std::string>& questIds);
    void updateQuestObjectivesForEvent(const std::string& targetId, ObjectiveType type, int count = 1);
    bool checkQuestPrerequisites(std::shared_ptr<Quest> quest) const;

    // Data members
    std::unordered_map<std::string, std::shared_ptr<Quest>> m_quests;
    std::unordered_map<std::string, QuestLogEntry> m_questLog;
    PlayerQuestData m_playerData;

    std::string m_trackedQuestId;
    std::vector<QuestNotification> m_notifications;
    std::unordered_set<std::string> m_discoveredHiddenQuests;

    // Callbacks
    std::function<void(std::shared_ptr<Quest>)> m_onQuestAccepted;
    std::function<void(std::shared_ptr<Quest>)> m_onQuestCompleted;
    std::function<void(std::shared_ptr<Quest>, const std::string&)> m_onObjectiveUpdated;

    bool m_initialized;
};

// Helper class for quest chains
class QuestChainBuilder {
public:
    QuestChainBuilder& addQuest(std::shared_ptr<Quest> quest);
    QuestChainBuilder& addPrerequisite(const std::string& questId, const std::string& prerequisiteId);
    QuestChainBuilder& addBranch(const std::string& fromQuestId, const std::string& toQuestId);
    void build();

private:
    std::vector<std::shared_ptr<Quest>> m_quests;
    std::map<std::string, std::vector<std::string>> m_prerequisites;
    std::map<std::string, std::vector<std::string>> m_branches;
};

// Quest filter helper
class QuestFilter {
public:
    QuestFilter& byType(QuestType type);
    QuestFilter& byDifficulty(QuestDifficulty difficulty);
    QuestFilter& byMinLevel(int level);
    QuestFilter& byMaxLevel(int level);
    QuestFilter& byFaction(Faction faction);
    QuestFilter& byStatus(QuestStatus status);
    QuestFilter& onlyRepeatable();
    QuestFilter& onlyHidden();

    std::vector<std::shared_ptr<Quest>> apply(const std::vector<std::shared_ptr<Quest>>& quests) const;

private:
    std::vector<std::function<bool(const std::shared_ptr<Quest>&)>> m_filters;
};

} // namespace QuestSystem

#endif // QUEST_SYSTEM_H
