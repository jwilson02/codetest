#ifndef QUEST_H
#define QUEST_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>

namespace QuestSystem {

// Quest types
enum class QuestType {
    MAIN_STORY,
    SIDE_QUEST,
    DAILY_QUEST,
    WEEKLY_QUEST,
    REPUTATION_QUEST,
    TIMED_EVENT,
    WORLD_EVENT,
    HIDDEN_QUEST,
    BOSS_QUEST,
    RAID_QUEST,
    ESCORT_QUEST,
    COLLECTION_QUEST
};

// Quest status
enum class QuestStatus {
    LOCKED,           // Prerequisites not met
    AVAILABLE,        // Can be accepted
    ACTIVE,           // Currently in progress
    COMPLETED,        // Finished successfully
    FAILED,           // Failed to complete
    ABANDONED,        // Player abandoned
    EXPIRED           // Timed quest expired
};

// Objective types
enum class ObjectiveType {
    KILL,             // Kill X enemies
    COLLECT,          // Collect X items
    ESCORT,           // Escort NPC
    REACH_LOCATION,   // Travel to location
    INTERACT,         // Interact with object/NPC
    DELIVER,          // Deliver item to NPC
    DEFEND,           // Defend location/NPC
    PUZZLE,           // Solve puzzle
    CRAFT,            // Craft items
    EXPLORE,          // Explore area
    BOSS_KILL,        // Defeat boss
    REPUTATION,       // Reach reputation level
    LEVEL_UP,         // Reach character level
    DISCOVER,         // Discover location/secret
    STEALTH,          // Complete without detection
    TIME_TRIAL        // Complete within time limit
};

// Quest difficulty
enum class QuestDifficulty {
    TRIVIAL,
    EASY,
    NORMAL,
    HARD,
    VERY_HARD,
    LEGENDARY,
    MYTHIC
};

// Reputation factions
enum class Faction {
    NONE,
    WARRIORS_GUILD,
    MAGES_COLLEGE,
    THIEVES_GUILD,
    MERCHANTS_LEAGUE,
    ROYAL_GUARD,
    DARK_BROTHERHOOD,
    NATURE_KEEPERS,
    ANCIENT_ORDER,
    REBELS,
    EMPIRE
};

// Quest objective structure
struct QuestObjective {
    std::string id;
    ObjectiveType type;
    std::string description;
    std::string targetId;        // Enemy/item/NPC/location ID
    int requiredCount;
    int currentCount;
    bool optional;
    bool hidden;                 // Hidden until discovered
    std::map<std::string, std::string> metadata;

    QuestObjective() : requiredCount(1), currentCount(0), optional(false), hidden(false) {}

    bool isComplete() const { return currentCount >= requiredCount; }
    float getProgress() const { return static_cast<float>(currentCount) / requiredCount; }
};

// Quest reward structure
struct QuestReward {
    int experience;
    int gold;
    std::vector<std::string> items;
    std::map<Faction, int> reputationGains;
    std::map<std::string, int> skillPoints;
    std::string unlockQuest;     // Quest unlocked on completion
    std::string unlockAbility;   // Ability unlocked
    std::string unlockLocation;  // Location unlocked
    std::vector<std::string> achievements;

    QuestReward() : experience(0), gold(0) {}
};

// Quest class
class Quest {
public:
    Quest(const std::string& id, const std::string& name, QuestType type);
    virtual ~Quest() = default;

    // Getters
    std::string getId() const { return m_id; }
    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_description; }
    QuestType getType() const { return m_type; }
    QuestStatus getStatus() const { return m_status; }
    QuestDifficulty getDifficulty() const { return m_difficulty; }
    int getRecommendedLevel() const { return m_recommendedLevel; }
    Faction getRequiredFaction() const { return m_requiredFaction; }
    int getRequiredReputation() const { return m_requiredReputation; }

    // Quest state management
    void setStatus(QuestStatus status) { m_status = status; }
    bool canStart() const;
    bool canComplete() const;
    void start();
    void complete();
    void fail();
    void abandon();

    // Objectives
    void addObjective(const QuestObjective& objective);
    void updateObjective(const std::string& objectiveId, int progress);
    void completeObjective(const std::string& objectiveId);
    std::vector<QuestObjective>& getObjectives() { return m_objectives; }
    const std::vector<QuestObjective>& getObjectives() const { return m_objectives; }
    float getOverallProgress() const;

    // Prerequisites
    void addPrerequisiteQuest(const std::string& questId);
    void addPrerequisiteLevel(int level);
    void addPrerequisiteReputation(Faction faction, int reputation);
    bool checkPrerequisites(int playerLevel, const std::map<Faction, int>& reputations,
                           const std::vector<std::string>& completedQuests) const;

    // Quest chain
    void addNextQuest(const std::string& questId) { m_nextQuests.push_back(questId); }
    const std::vector<std::string>& getNextQuests() const { return m_nextQuests; }

    // Rewards
    void setReward(const QuestReward& reward) { m_reward = reward; }
    const QuestReward& getReward() const { return m_reward; }

    // Time management (for timed quests)
    void setTimeLimit(int seconds);
    bool isExpired() const;
    int getRemainingTime() const;

    // Metadata
    void setDescription(const std::string& desc) { m_description = desc; }
    void setDifficulty(QuestDifficulty diff) { m_difficulty = diff; }
    void setRecommendedLevel(int level) { m_recommendedLevel = level; }
    void setRequiredFaction(Faction faction, int reputation);
    void setHidden(bool hidden) { m_hidden = hidden; }
    void setRepeatable(bool repeatable) { m_repeatable = repeatable; }
    bool isHidden() const { return m_hidden; }
    bool isRepeatable() const { return m_repeatable; }

    // Quest givers and locations
    void setQuestGiver(const std::string& npcId) { m_questGiverId = npcId; }
    void setTurnInNpc(const std::string& npcId) { m_turnInNpcId = npcId; }
    void setStartLocation(const std::string& location) { m_startLocation = location; }
    std::string getQuestGiver() const { return m_questGiverId; }
    std::string getTurnInNpc() const { return m_turnInNpcId; }
    std::string getStartLocation() const { return m_startLocation; }

    // Callbacks
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }
    void setOnFail(std::function<void()> callback) { m_onFail = callback; }
    void setOnProgress(std::function<void(float)> callback) { m_onProgress = callback; }

    // Serialization
    std::string toJson() const;
    static std::shared_ptr<Quest> fromJson(const std::string& json);

protected:
    // Core properties
    std::string m_id;
    std::string m_name;
    std::string m_description;
    QuestType m_type;
    QuestStatus m_status;
    QuestDifficulty m_difficulty;

    // Requirements
    int m_recommendedLevel;
    std::vector<std::string> m_prerequisiteQuests;
    int m_prerequisiteLevel;
    Faction m_requiredFaction;
    int m_requiredReputation;

    // Quest chain
    std::vector<std::string> m_nextQuests;

    // Objectives and rewards
    std::vector<QuestObjective> m_objectives;
    QuestReward m_reward;

    // NPCs and locations
    std::string m_questGiverId;
    std::string m_turnInNpcId;
    std::string m_startLocation;

    // Time management
    bool m_timed;
    std::chrono::system_clock::time_point m_startTime;
    int m_timeLimitSeconds;

    // Properties
    bool m_hidden;
    bool m_repeatable;

    // Callbacks
    std::function<void()> m_onComplete;
    std::function<void()> m_onFail;
    std::function<void(float)> m_onProgress;
};

// Specialized quest types
class StoryQuest : public Quest {
public:
    StoryQuest(const std::string& id, const std::string& name, int chapter);

    void setChapter(int chapter) { m_chapter = chapter; }
    int getChapter() const { return m_chapter; }

    void setStoryArc(const std::string& arc) { m_storyArc = arc; }
    std::string getStoryArc() const { return m_storyArc; }

private:
    int m_chapter;
    std::string m_storyArc;
};

class BossQuest : public Quest {
public:
    BossQuest(const std::string& id, const std::string& name, const std::string& bossId);

    void setBossId(const std::string& bossId) { m_bossId = bossId; }
    std::string getBossId() const { return m_bossId; }

    void setMinPlayers(int count) { m_minPlayers = count; }
    void setMaxPlayers(int count) { m_maxPlayers = count; }
    int getMinPlayers() const { return m_minPlayers; }
    int getMaxPlayers() const { return m_maxPlayers; }

private:
    std::string m_bossId;
    int m_minPlayers;
    int m_maxPlayers;
};

class DailyQuest : public Quest {
public:
    DailyQuest(const std::string& id, const std::string& name);

    void resetDaily();
    bool canAcceptToday() const;

private:
    std::chrono::system_clock::time_point m_lastCompletionTime;
};

} // namespace QuestSystem

#endif // QUEST_H
