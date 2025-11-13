#ifndef OBJECTIVE_TRACKER_H
#define OBJECTIVE_TRACKER_H

#include "Quest.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace QuestSystem {

// Objective progress update structure
struct ObjectiveUpdate {
    std::string questId;
    std::string objectiveId;
    int oldValue;
    int newValue;
    int requiredValue;
    bool justCompleted;
    std::chrono::system_clock::time_point timestamp;

    ObjectiveUpdate()
        : oldValue(0)
        , newValue(0)
        , requiredValue(0)
        , justCompleted(false)
        , timestamp(std::chrono::system_clock::now()) {}
};

// Tracked objective data
struct TrackedObjective {
    std::shared_ptr<Quest> quest;
    QuestObjective objective;
    float lastProgress;
    bool isVisible;
    bool isPinned;
    int updateCount;

    TrackedObjective()
        : lastProgress(0.0f)
        , isVisible(true)
        , isPinned(false)
        , updateCount(0) {}
};

// Objective tracker UI element
struct ObjectiveUIElement {
    std::string questName;
    std::string objectiveDescription;
    int currentCount;
    int requiredCount;
    float progress;
    bool isOptional;
    bool isCompleted;
    bool isPinned;
    std::string distanceToTarget;
    std::string iconType;

    ObjectiveUIElement()
        : currentCount(0)
        , requiredCount(0)
        , progress(0.0f)
        , isOptional(false)
        , isCompleted(false)
        , isPinned(false) {}
};

// Objective tracker class
class ObjectiveTracker {
public:
    static ObjectiveTracker& getInstance() {
        static ObjectiveTracker instance;
        return instance;
    }

    // Tracking management
    void trackQuest(std::shared_ptr<Quest> quest);
    void untrackQuest(const std::string& questId);
    void clearAllTracking();
    bool isTracking(const std::string& questId) const;

    // Objective updates
    void updateObjective(const std::string& questId,
                        const std::string& objectiveId,
                        int newValue);
    void incrementObjective(const std::string& questId,
                           const std::string& objectiveId,
                           int amount = 1);
    void completeObjective(const std::string& questId,
                          const std::string& objectiveId);

    // Bulk updates (for common events)
    void notifyKill(const std::string& enemyId, int count = 1);
    void notifyItemCollected(const std::string& itemId, int count = 1);
    void notifyLocationVisited(const std::string& locationId);
    void notifyNpcInteracted(const std::string& npcId);
    void notifyItemCrafted(const std::string& itemId, int count = 1);

    // Get tracked data
    std::vector<TrackedObjective> getTrackedObjectives() const;
    std::vector<TrackedObjective> getActiveObjectives() const;
    std::vector<TrackedObjective> getCompletedObjectives() const;
    std::vector<ObjectiveUpdate> getRecentUpdates(int count = 10) const;

    // UI generation
    std::vector<ObjectiveUIElement> generateUIElements() const;
    ObjectiveUIElement generateUIElementForObjective(const TrackedObjective& tracked) const;
    std::string formatObjectiveProgress(const QuestObjective& objective) const;

    // Pinning system
    void pinObjective(const std::string& questId, const std::string& objectiveId);
    void unpinObjective(const std::string& questId, const std::string& objectiveId);
    void unpinAllObjectives();
    std::vector<TrackedObjective> getPinnedObjectives() const;

    // Visibility
    void setObjectiveVisible(const std::string& questId,
                             const std::string& objectiveId,
                             bool visible);
    void hideCompletedObjectives(bool hide) { m_hideCompleted = hide; }
    void hideOptionalObjectives(bool hide) { m_hideOptional = hide; }

    // Distance tracking (for location-based objectives)
    void updatePlayerPosition(float x, float y, float z);
    void setObjectiveLocation(const std::string& objectiveId, float x, float y, float z);
    float getDistanceToObjective(const std::string& objectiveId) const;
    std::string formatDistance(float distance) const;

    // Statistics
    int getTotalTrackedObjectives() const;
    int getCompletedObjectivesCount() const;
    float getOverallCompletionRate() const;
    std::map<ObjectiveType, int> getObjectiveTypeStats() const;

    // Sorting and filtering
    std::vector<TrackedObjective> sortByProgress(bool ascending = true) const;
    std::vector<TrackedObjective> sortByDistance() const;
    std::vector<TrackedObjective> filterByType(ObjectiveType type) const;
    std::vector<TrackedObjective> filterByQuest(const std::string& questId) const;

    // Callbacks
    void setOnObjectiveUpdated(std::function<void(const ObjectiveUpdate&)> callback) {
        m_onObjectiveUpdated = callback;
    }
    void setOnObjectiveCompleted(std::function<void(const ObjectiveUpdate&)> callback) {
        m_onObjectiveCompleted = callback;
    }
    void setOnQuestCompleted(std::function<void(const std::string&)> callback) {
        m_onQuestCompleted = callback;
    }

    // Hints and guidance
    std::string getObjectiveHint(const std::string& objectiveId) const;
    std::vector<std::string> getSuggestedActions(const std::string& objectiveId) const;
    bool isObjectiveInActiveArea(const std::string& objectiveId) const;

    // Advanced features
    void enableAutoTracking(bool enable) { m_autoTracking = enable; }
    void setMaxTrackedQuests(int max) { m_maxTrackedQuests = max; }
    void enableProgressNotifications(bool enable) { m_showProgressNotifications = enable; }
    void setNotificationThreshold(float threshold) { m_notificationThreshold = threshold; }

private:
    ObjectiveTracker();
    ~ObjectiveTracker() = default;
    ObjectiveTracker(const ObjectiveTracker&) = delete;
    ObjectiveTracker& operator=(const ObjectiveTracker&) = delete;

    // Internal helpers
    TrackedObjective* findTrackedObjective(const std::string& questId,
                                           const std::string& objectiveId);
    void checkQuestCompletion(const std::string& questId);
    void addUpdate(const ObjectiveUpdate& update);
    void notifyCallbacks(const ObjectiveUpdate& update);

    // Position utilities
    struct Position {
        float x, y, z;
        Position() : x(0), y(0), z(0) {}
        Position(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    };

    float calculateDistance(const Position& a, const Position& b) const;

    // Data members
    std::map<std::string, std::vector<TrackedObjective>> m_trackedByQuest;
    std::vector<ObjectiveUpdate> m_recentUpdates;
    std::map<std::string, Position> m_objectiveLocations;
    Position m_playerPosition;

    // Settings
    bool m_hideCompleted;
    bool m_hideOptional;
    bool m_autoTracking;
    int m_maxTrackedQuests;
    bool m_showProgressNotifications;
    float m_notificationThreshold;
    int m_maxRecentUpdates;

    // Callbacks
    std::function<void(const ObjectiveUpdate&)> m_onObjectiveUpdated;
    std::function<void(const ObjectiveUpdate&)> m_onObjectiveCompleted;
    std::function<void(const std::string&)> m_onQuestCompleted;
};

// Objective hint system
class ObjectiveHintSystem {
public:
    ObjectiveHintSystem();

    // Add hints for objectives
    void registerHint(const std::string& objectiveId, const std::string& hint);
    void registerHints(const std::string& objectiveId, const std::vector<std::string>& hints);

    // Get hints
    std::string getHint(const std::string& objectiveId) const;
    std::vector<std::string> getAllHints(const std::string& objectiveId) const;
    std::string getRandomHint(const std::string& objectiveId) const;

    // Contextual hints
    std::string getContextualHint(const std::string& objectiveId,
                                  const std::string& playerLocation) const;

    // Load hints from data
    void loadHintsFromJson(const std::string& jsonPath);

private:
    std::map<std::string, std::vector<std::string>> m_hints;
    std::mt19937 m_randomEngine;
};

// Objective waypoint system
class ObjectiveWaypointSystem {
public:
    struct Waypoint {
        std::string id;
        std::string name;
        float x, y, z;
        std::string iconType;
        bool isVisible;
        std::string questId;
        std::string objectiveId;

        Waypoint()
            : x(0), y(0), z(0)
            , isVisible(true) {}
    };

    ObjectiveWaypointSystem();

    // Waypoint management
    void addWaypoint(const Waypoint& waypoint);
    void removeWaypoint(const std::string& waypointId);
    void clearWaypoints();
    void clearWaypointsForQuest(const std::string& questId);

    // Waypoint queries
    std::vector<Waypoint> getAllWaypoints() const;
    std::vector<Waypoint> getVisibleWaypoints() const;
    std::vector<Waypoint> getWaypointsForQuest(const std::string& questId) const;
    Waypoint* getWaypoint(const std::string& waypointId);
    Waypoint* getNearestWaypoint(float x, float y, float z);

    // Waypoint updates
    void setWaypointVisible(const std::string& waypointId, bool visible);
    void updateWaypointPosition(const std::string& waypointId, float x, float y, float z);

    // Auto-generation
    void generateWaypointsForObjective(std::shared_ptr<Quest> quest,
                                       const QuestObjective& objective);
    void generateWaypointsForQuest(std::shared_ptr<Quest> quest);

private:
    std::map<std::string, Waypoint> m_waypoints;
    int m_waypointCounter;
};

// Objective completion tracker (for achievements)
class ObjectiveCompletionHistory {
public:
    struct CompletionRecord {
        std::string questId;
        std::string objectiveId;
        std::chrono::system_clock::time_point completionTime;
        int attemptCount;
        float completionTimeSeconds;
        bool wasOptional;

        CompletionRecord()
            : attemptCount(0)
            , completionTimeSeconds(0.0f)
            , wasOptional(false) {}
    };

    ObjectiveCompletionHistory();

    // Record completions
    void recordCompletion(const std::string& questId,
                         const std::string& objectiveId,
                         const QuestObjective& objective,
                         float completionTime);

    // Query history
    std::vector<CompletionRecord> getCompletionHistory() const;
    std::vector<CompletionRecord> getCompletionsForQuest(const std::string& questId) const;
    int getTotalCompletions() const;
    int getCompletionsByType(ObjectiveType type) const;

    // Statistics
    float getAverageCompletionTime() const;
    float getFastestCompletion() const;
    std::map<ObjectiveType, int> getCompletionsByType() const;

    // Save/Load
    std::string serialize() const;
    void deserialize(const std::string& data);

private:
    std::vector<CompletionRecord> m_history;
    std::map<std::string, int> m_attemptCounts;
};

} // namespace QuestSystem

#endif // OBJECTIVE_TRACKER_H
