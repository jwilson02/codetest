#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Story {

// Quest types
enum class QuestType {
    MAIN_STORY,
    SIDE_QUEST,
    FACTION,
    ROMANCE,
    EXPLORATION,
    COLLECTION,
    BOUNTY,
    REPEATABLE,
    HIDDEN
};

// Quest status
enum class QuestStatus {
    LOCKED,         // Not yet available
    AVAILABLE,      // Can be started
    ACTIVE,         // Currently active
    COMPLETED,      // Successfully completed
    FAILED,         // Failed
    ABANDONED       // Player abandoned
};

// Objective types
enum class ObjectiveType {
    KILL,           // Kill enemies
    COLLECT,        // Collect items
    TALK,           // Talk to NPC
    REACH,          // Reach location
    ESCORT,         // Escort NPC
    DEFEND,         // Defend location/NPC
    CRAFT,          // Craft items
    DISCOVER,       // Discover location
    CUSTOM          // Custom objective
};

// Quest objective
struct QuestObjective {
    std::string id;
    std::string description;
    ObjectiveType type;

    // Progress tracking
    int currentProgress;
    int requiredProgress;
    bool completed;
    bool optional;
    bool hidden;            // Hidden until revealed

    // Specific data based on type
    std::string targetId;   // Enemy ID, item ID, NPC ID, location ID, etc.
    std::string targetName;

    // Conditions
    std::vector<std::string> requiredFlags;

    // Callbacks
    std::string onCompleteScript;

    QuestObjective() : currentProgress(0), requiredProgress(1),
                      completed(false), optional(false), hidden(false) {}

    float getProgressPercentage() const {
        return requiredProgress > 0 ? (float)currentProgress / requiredProgress : 0.0f;
    }
};

// Quest stage
struct QuestStage {
    int stage;
    std::string description;
    std::vector<QuestObjective> objectives;
    std::string dialogueId;
    std::string journalEntry;

    bool isCompleted() const {
        for (const auto& obj : objectives) {
            if (!obj.optional && !obj.completed) {
                return false;
            }
        }
        return true;
    }
};

// Quest reward
struct QuestReward {
    int experience;
    int gold;
    std::vector<std::string> items;
    std::vector<std::pair<std::string, int>> relationshipChanges; // NPC ID, amount
    std::vector<std::string> unlockedQuests;
    std::vector<std::string> unlockedLocations;
    std::vector<std::string> unlockedAbilities;
    std::string titleUnlocked;

    QuestReward() : experience(0), gold(0) {}
};

// Main quest structure
struct Quest {
    std::string id;
    std::string title;
    std::string description;
    QuestType type;
    QuestStatus status;

    // Quest giver
    std::string questGiverId;
    std::string questGiverName;

    // Stages
    std::vector<QuestStage> stages;
    int currentStage;

    // Requirements
    int minimumLevel;
    std::vector<std::string> requiredQuests;     // Must complete these first
    std::vector<std::string> requiredFlags;
    std::vector<std::pair<std::string, int>> requiredRelationships;

    // Conflicts (cannot be active simultaneously)
    std::vector<std::string> conflictingQuests;

    // Rewards
    QuestReward rewards;
    QuestReward stageRewards;   // Rewards given at each stage

    // Time limit
    bool hasTotalTimeLimit;
    float timeLimit;            // In seconds
    float timeRemaining;

    // Location
    std::string startLocation;
    std::string currentLocation;

    // Tracking
    float startTime;
    float completionTime;
    bool tracked;               // Is this quest tracked in UI?

    // Metadata
    std::string category;
    int recommendedLevel;
    float estimatedDuration;    // In minutes
    std::vector<std::string> tags;

    Quest() : status(QuestStatus::LOCKED), currentStage(0), minimumLevel(1),
             hasTotalTimeLimit(false), timeLimit(0), timeRemaining(0),
             startTime(0), completionTime(0), tracked(false), recommendedLevel(1),
             estimatedDuration(0) {}
};

// Journal entry
struct JournalEntry {
    std::string id;
    std::string title;
    std::string content;
    std::string category;       // "Quests", "Lore", "Characters", "Locations", etc.
    float timestamp;
    bool isNew;                 // Not yet read by player

    // Related entries
    std::vector<std::string> relatedQuestIds;
    std::vector<std::string> relatedNPCIds;
    std::vector<std::string> relatedLocationIds;

    JournalEntry() : timestamp(0.0f), isNew(true) {}
};

// Lore entry (for world building)
struct LoreEntry {
    std::string id;
    std::string title;
    std::string content;
    std::string category;       // "History", "Myths", "Factions", "Bestiary", etc.
    bool discovered;

    // Source
    std::string discoveredFrom;  // Book, NPC dialogue, etc.
    float discoveryTime;

    LoreEntry() : discovered(false), discoveryTime(0.0f) {}
};

// Main journal system
class Journal {
public:
    Journal();
    ~Journal();

    // Initialization
    bool initialize();
    void shutdown();

    // Quest loading
    bool loadQuests(const std::string& filePath);
    void registerQuest(const Quest& quest);

    // Quest management
    void unlockQuest(const std::string& questId);
    void startQuest(const std::string& questId);
    void completeQuest(const std::string& questId);
    void failQuest(const std::string& questId);
    void abandonQuest(const std::string& questId);

    // Quest queries
    Quest* getQuest(const std::string& questId);
    const Quest* getQuest(const std::string& questId) const;
    std::vector<const Quest*> getActiveQuests() const;
    std::vector<const Quest*> getCompletedQuests() const;
    std::vector<const Quest*> getAvailableQuests() const;
    std::vector<const Quest*> getQuestsByType(QuestType type) const;
    std::vector<const Quest*> getTrackedQuests() const;

    // Quest tracking
    void setTrackedQuest(const std::string& questId);
    void toggleQuestTracking(const std::string& questId);
    const Quest* getTrackedQuest() const;

    // Stage management
    void advanceQuestStage(const std::string& questId);
    void setQuestStage(const std::string& questId, int stage);
    int getCurrentStage(const std::string& questId) const;
    const QuestStage* getCurrentQuestStage(const std::string& questId) const;

    // Objective management
    void updateObjective(const std::string& questId, const std::string& objectiveId, int progress);
    void completeObjective(const std::string& questId, const std::string& objectiveId);
    void incrementObjective(const std::string& questId, const std::string& objectiveId, int amount = 1);
    bool isObjectiveComplete(const std::string& questId, const std::string& objectiveId) const;

    // Journal entries
    void addJournalEntry(const JournalEntry& entry);
    void addJournalEntry(const std::string& title, const std::string& content, const std::string& category = "General");
    const JournalEntry* getJournalEntry(const std::string& entryId) const;
    std::vector<const JournalEntry*> getAllJournalEntries() const;
    std::vector<const JournalEntry*> getJournalEntriesByCategory(const std::string& category) const;
    std::vector<const JournalEntry*> getNewJournalEntries() const;
    void markEntryAsRead(const std::string& entryId);
    void markAllEntriesAsRead();

    // Lore system
    void registerLoreEntry(const LoreEntry& entry);
    void discoverLore(const std::string& loreId, const std::string& source = "");
    bool isLoreDiscovered(const std::string& loreId) const;
    const LoreEntry* getLoreEntry(const std::string& loreId) const;
    std::vector<const LoreEntry*> getDiscoveredLore() const;
    std::vector<const LoreEntry*> getLoreByCategory(const std::string& category) const;

    // Quest availability
    bool isQuestAvailable(const std::string& questId) const;
    std::vector<std::string> getQuestRequirements(const std::string& questId) const;

    // Quest notifications
    using QuestCallback = std::function<void(const std::string& questId, QuestStatus oldStatus, QuestStatus newStatus)>;
    using ObjectiveCallback = std::function<void(const std::string& questId, const std::string& objectiveId)>;
    void onQuestStatusChanged(QuestCallback callback);
    void onObjectiveCompleted(ObjectiveCallback callback);

    // Statistics
    struct JournalStats {
        int totalQuests;
        int activeQuests;
        int completedQuests;
        int failedQuests;
        int mainQuestsCompleted;
        int sideQuestsCompleted;
        int loreDiscovered;
        int journalEntries;
        float totalQuestTime;
    };
    JournalStats getStatistics() const;

    // Quest chains
    std::vector<std::string> getQuestChain(const std::string& questId) const; // All quests in chain
    std::string getNextQuestInChain(const std::string& questId) const;

    // Bestiary (enemy lore)
    void registerBestiaryEntry(const std::string& enemyId, const std::string& name, const std::string& description);
    void discoverEnemy(const std::string& enemyId);
    void updateEnemyKillCount(const std::string& enemyId, int count = 1);

    struct BestiaryEntry {
        std::string id;
        std::string name;
        std::string description;
        bool discovered;
        int killCount;
        std::vector<std::string> weaknesses;
        std::vector<std::string> resistances;
    };
    const BestiaryEntry* getBestiaryEntry(const std::string& enemyId) const;
    std::vector<const BestiaryEntry*> getDiscoveredEnemies() const;

    // Search
    std::vector<const Quest*> searchQuests(const std::string& query) const;
    std::vector<const JournalEntry*> searchJournal(const std::string& query) const;
    std::vector<const LoreEntry*> searchLore(const std::string& query) const;

    // Save/load
    std::string serializeState() const;
    bool deserializeState(const std::string& data);

    // Debug
    void unlockAllQuests();
    void completeAllActiveQuests();
    void dumpQuestState(const std::string& filePath) const;

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal
    void giveQuestRewards(const Quest& quest);
    void checkQuestUnlocks();
    void updateQuestTimers(float deltaTime);
};

} // namespace Story
