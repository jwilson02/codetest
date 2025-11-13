#ifndef WORLD_EVENT_H
#define WORLD_EVENT_H

#include "Quest.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace QuestSystem {

// World event types
enum class WorldEventType {
    INVASION,           // Enemy invasion of an area
    BOSS_SPAWN,         // World boss appears
    SEASONAL,           // Seasonal event (holidays, etc.)
    WEATHER,            // Weather-related event
    COMMUNITY,          // Community goal event
    PVP_TOURNAMENT,     // PvP tournament event
    DUNGEON_RESET,      // Special dungeon available
    RAID_EVENT,         // Raid event
    FACTION_WAR,        // Faction vs faction war
    RESOURCE_BOOM,      // Increased resource availability
    XP_BOOST,           // Experience boost period
    TRADE_FAIR,         // Special trading event
    METEOR_SHOWER,      // Rare resource event
    PLAGUE_OUTBREAK,    // Negative event
    CELEBRATION         // Celebration event
};

// Event state
enum class EventState {
    SCHEDULED,          // Event is scheduled but not started
    PREPARING,          // Event preparation phase
    ACTIVE,             // Event is currently active
    ENDING,             // Event is in ending phase
    COMPLETED,          // Event completed successfully
    FAILED,             // Event failed
    CANCELLED           // Event was cancelled
};

// Event participation level
enum class ParticipationLevel {
    NONE,
    OBSERVER,
    PARTICIPANT,
    CONTRIBUTOR,
    HERO,
    LEGEND
};

// World event objective (similar to quest objectives)
struct WorldEventObjective {
    std::string id;
    std::string description;
    ObjectiveType type;
    std::string targetId;
    int currentProgress;
    int requiredProgress;
    bool isGlobalProgress;      // If true, all players contribute
    bool isCompleted;

    WorldEventObjective()
        : currentProgress(0)
        , requiredProgress(100)
        , isGlobalProgress(true)
        , isCompleted(false) {}

    float getProgress() const {
        return requiredProgress > 0 ?
               static_cast<float>(currentProgress) / requiredProgress : 0.0f;
    }
};

// Event reward structure
struct WorldEventReward {
    int experienceBase;
    int goldBase;
    std::vector<std::string> items;
    std::vector<std::string> titles;
    std::vector<std::string> cosmetics;
    std::map<Faction, int> reputationGains;

    // Tier-based rewards (based on participation)
    std::map<ParticipationLevel, std::vector<std::string>> tieredItems;
    std::map<ParticipationLevel, int> tieredExperience;
    std::map<ParticipationLevel, int> tieredGold;

    WorldEventReward()
        : experienceBase(0)
        , goldBase(0) {}
};

// Player participation data
struct PlayerParticipation {
    std::string playerId;
    int contributionScore;
    ParticipationLevel level;
    std::chrono::system_clock::time_point joinTime;
    std::chrono::system_clock::time_point lastContributionTime;
    std::map<std::string, int> objectiveContributions;

    PlayerParticipation()
        : contributionScore(0)
        , level(ParticipationLevel::NONE) {}
};

// World event class
class WorldEvent {
public:
    WorldEvent(const std::string& id, const std::string& name, WorldEventType type);
    virtual ~WorldEvent() = default;

    // Getters
    std::string getId() const { return m_id; }
    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_description; }
    WorldEventType getType() const { return m_type; }
    EventState getState() const { return m_state; }

    // Event lifecycle
    void schedule(const std::chrono::system_clock::time_point& startTime,
                  const std::chrono::system_clock::time_point& endTime);
    void start();
    void update(float deltaTime);
    void end();
    void cancel();

    bool isActive() const { return m_state == EventState::ACTIVE; }
    bool isScheduled() const { return m_state == EventState::SCHEDULED; }
    bool isCompleted() const { return m_state == EventState::COMPLETED; }

    // Timing
    std::chrono::system_clock::time_point getStartTime() const { return m_startTime; }
    std::chrono::system_clock::time_point getEndTime() const { return m_endTime; }
    int getRemainingTime() const;
    int getDuration() const;
    float getTimeProgress() const;

    // Objectives
    void addObjective(const WorldEventObjective& objective);
    void updateObjective(const std::string& objectiveId, int progress, const std::string& playerId = "");
    void completeObjective(const std::string& objectiveId);
    std::vector<WorldEventObjective>& getObjectives() { return m_objectives; }
    const std::vector<WorldEventObjective>& getObjectives() const { return m_objectives; }
    float getOverallProgress() const;
    bool areAllObjectivesComplete() const;

    // Participation
    void addParticipant(const std::string& playerId);
    void removeParticipant(const std::string& playerId);
    void updateParticipantContribution(const std::string& playerId, int contribution);
    PlayerParticipation* getParticipation(const std::string& playerId);
    std::vector<PlayerParticipation> getAllParticipants() const;
    int getParticipantCount() const;
    ParticipationLevel calculateParticipationLevel(int contributionScore) const;

    // Rewards
    void setReward(const WorldEventReward& reward) { m_reward = reward; }
    const WorldEventReward& getReward() const { return m_reward; }
    WorldEventReward calculatePlayerReward(const std::string& playerId) const;
    void distributeRewards();

    // Location
    void setEventLocation(const std::string& location) { m_eventLocation = location; }
    std::string getEventLocation() const { return m_eventLocation; }
    void setEventRegion(const std::string& region) { m_eventRegion = region; }
    std::string getEventRegion() const { return m_eventRegion; }

    // Difficulty scaling
    void setDifficultyScale(float scale) { m_difficultyScale = scale; }
    float getDifficultyScale() const { return m_difficultyScale; }
    void scaleDifficultyByParticipants();

    // Metadata
    void setDescription(const std::string& desc) { m_description = desc; }
    void setMinParticipants(int count) { m_minParticipants = count; }
    void setMaxParticipants(int count) { m_maxParticipants = count; }
    void setMinLevel(int level) { m_minLevel = level; }
    void setRepeatable(bool repeatable) { m_repeatable = repeatable; }

    int getMinParticipants() const { return m_minParticipants; }
    int getMaxParticipants() const { return m_maxParticipants; }
    int getMinLevel() const { return m_minLevel; }
    bool isRepeatable() const { return m_repeatable; }

    // Callbacks
    void setOnEventStart(std::function<void()> callback) { m_onEventStart = callback; }
    void setOnEventEnd(std::function<void()> callback) { m_onEventEnd = callback; }
    void setOnObjectiveComplete(std::function<void(const std::string&)> callback) {
        m_onObjectiveComplete = callback;
    }

    // Serialization
    std::string toJson() const;
    static std::shared_ptr<WorldEvent> fromJson(const std::string& json);

protected:
    // Core properties
    std::string m_id;
    std::string m_name;
    std::string m_description;
    WorldEventType m_type;
    EventState m_state;

    // Timing
    std::chrono::system_clock::time_point m_startTime;
    std::chrono::system_clock::time_point m_endTime;
    std::chrono::system_clock::time_point m_actualStartTime;

    // Objectives
    std::vector<WorldEventObjective> m_objectives;

    // Participation
    std::map<std::string, PlayerParticipation> m_participants;
    int m_minParticipants;
    int m_maxParticipants;
    int m_minLevel;

    // Location
    std::string m_eventLocation;
    std::string m_eventRegion;

    // Rewards
    WorldEventReward m_reward;

    // Properties
    float m_difficultyScale;
    bool m_repeatable;

    // Callbacks
    std::function<void()> m_onEventStart;
    std::function<void()> m_onEventEnd;
    std::function<void(const std::string&)> m_onObjectiveComplete;
};

// Specialized world event types
class InvasionEvent : public WorldEvent {
public:
    InvasionEvent(const std::string& id, const std::string& name, const std::string& invasionLocation);

    void setInvadingFaction(Faction faction) { m_invadingFaction = faction; }
    Faction getInvadingFaction() const { return m_invadingFaction; }

    void setWaveCount(int waves) { m_totalWaves = waves; }
    int getCurrentWave() const { return m_currentWave; }
    int getTotalWaves() const { return m_totalWaves; }
    void advanceWave();

private:
    Faction m_invadingFaction;
    int m_currentWave;
    int m_totalWaves;
};

class BossSpawnEvent : public WorldEvent {
public:
    BossSpawnEvent(const std::string& id, const std::string& name, const std::string& bossId);

    void setBossId(const std::string& bossId) { m_bossId = bossId; }
    std::string getBossId() const { return m_bossId; }

    void setBossHealth(int current, int max) {
        m_currentBossHealth = current;
        m_maxBossHealth = max;
    }
    float getBossHealthPercent() const {
        return m_maxBossHealth > 0 ?
               static_cast<float>(m_currentBossHealth) / m_maxBossHealth : 0.0f;
    }

private:
    std::string m_bossId;
    int m_currentBossHealth;
    int m_maxBossHealth;
};

class SeasonalEvent : public WorldEvent {
public:
    SeasonalEvent(const std::string& id, const std::string& name, const std::string& season);

    void setSeason(const std::string& season) { m_season = season; }
    std::string getSeason() const { return m_season; }

    void setYear(int year) { m_year = year; }
    int getYear() const { return m_year; }

private:
    std::string m_season;
    int m_year;
};

// World event manager
class WorldEventManager {
public:
    static WorldEventManager& getInstance() {
        static WorldEventManager instance;
        return instance;
    }

    // Event lifecycle
    void registerEvent(std::shared_ptr<WorldEvent> event);
    void unregisterEvent(const std::string& eventId);
    void startEvent(const std::string& eventId);
    void endEvent(const std::string& eventId);
    void cancelEvent(const std::string& eventId);

    // Updates
    void update(float deltaTime);
    void checkScheduledEvents();

    // Event queries
    std::shared_ptr<WorldEvent> getEvent(const std::string& eventId);
    std::vector<std::shared_ptr<WorldEvent>> getAllEvents() const;
    std::vector<std::shared_ptr<WorldEvent>> getActiveEvents() const;
    std::vector<std::shared_ptr<WorldEvent>> getScheduledEvents() const;
    std::vector<std::shared_ptr<WorldEvent>> getEventsByType(WorldEventType type) const;
    std::vector<std::shared_ptr<WorldEvent>> getEventsByLocation(const std::string& location) const;

    // Player participation
    void joinEvent(const std::string& eventId, const std::string& playerId);
    void leaveEvent(const std::string& eventId, const std::string& playerId);
    void contributeToEvent(const std::string& eventId,
                          const std::string& playerId,
                          const std::string& objectiveId,
                          int contribution);

    // Event scheduling
    void scheduleEvent(std::shared_ptr<WorldEvent> event,
                      const std::chrono::system_clock::time_point& startTime,
                      const std::chrono::system_clock::time_point& endTime);
    void scheduleRecurringEvent(std::shared_ptr<WorldEvent> event,
                               int intervalHours,
                               int durationMinutes);

    // Event generation
    std::shared_ptr<WorldEvent> generateRandomEvent(int playerLevel);
    std::shared_ptr<InvasionEvent> generateInvasionEvent(const std::string& location);
    std::shared_ptr<BossSpawnEvent> generateBossEvent(int playerLevel);
    std::shared_ptr<SeasonalEvent> generateSeasonalEvent(const std::string& season);

    // Event loading
    void loadEventsFromJson(const std::string& jsonPath);
    void saveEventProgress(const std::string& jsonPath);

    // Event announcements
    void announceEvent(const std::string& eventId, const std::string& message);
    std::vector<std::string> getRecentAnnouncements(int count = 10) const;

    // Statistics
    int getTotalEventsCompleted() const { return m_totalEventsCompleted; }
    int getTotalEventsActive() const;
    std::map<WorldEventType, int> getEventTypeStatistics() const;

    // Callbacks
    void setOnEventStarted(std::function<void(std::shared_ptr<WorldEvent>)> callback) {
        m_onEventStarted = callback;
    }
    void setOnEventEnded(std::function<void(std::shared_ptr<WorldEvent>)> callback) {
        m_onEventEnded = callback;
    }

private:
    WorldEventManager() : m_totalEventsCompleted(0) {}
    ~WorldEventManager() = default;
    WorldEventManager(const WorldEventManager&) = delete;
    WorldEventManager& operator=(const WorldEventManager&) = delete;

    // Data members
    std::map<std::string, std::shared_ptr<WorldEvent>> m_events;
    std::vector<std::string> m_announcements;
    int m_totalEventsCompleted;

    // Callbacks
    std::function<void(std::shared_ptr<WorldEvent>)> m_onEventStarted;
    std::function<void(std::shared_ptr<WorldEvent>)> m_onEventEnded;
};

// Event calendar system
class EventCalendar {
public:
    struct ScheduledEvent {
        std::string eventId;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        bool isRecurring;
        int recurIntervalHours;

        ScheduledEvent()
            : isRecurring(false)
            , recurIntervalHours(0) {}
    };

    EventCalendar();

    // Scheduling
    void addSchedule(const ScheduledEvent& schedule);
    void removeSchedule(const std::string& eventId);
    std::vector<ScheduledEvent> getUpcomingEvents(int hours = 24) const;
    std::vector<ScheduledEvent> getAllSchedules() const;

    // Queries
    bool isEventScheduled(const std::string& eventId) const;
    std::chrono::system_clock::time_point getNextEventTime(const std::string& eventId) const;

private:
    std::map<std::string, ScheduledEvent> m_schedule;
};

// Event leaderboard
class EventLeaderboard {
public:
    struct LeaderboardEntry {
        std::string playerId;
        std::string playerName;
        int score;
        ParticipationLevel level;
        int rank;

        LeaderboardEntry()
            : score(0)
            , level(ParticipationLevel::NONE)
            , rank(0) {}
    };

    EventLeaderboard(const std::string& eventId);

    // Leaderboard management
    void updateScore(const std::string& playerId, const std::string& playerName, int score);
    void recalculateRanks();

    // Queries
    std::vector<LeaderboardEntry> getTopPlayers(int count = 10) const;
    LeaderboardEntry getPlayerEntry(const std::string& playerId) const;
    int getPlayerRank(const std::string& playerId) const;

    // Rewards
    void distributeRankRewards(const WorldEventReward& baseReward);

private:
    std::string m_eventId;
    std::map<std::string, LeaderboardEntry> m_entries;
    std::vector<std::string> m_rankedPlayerIds;  // Sorted by score
};

} // namespace QuestSystem

#endif // WORLD_EVENT_H
