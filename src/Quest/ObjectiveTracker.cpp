#include "ObjectiveTracker.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace QuestSystem {

ObjectiveTracker::ObjectiveTracker()
    : m_hideCompleted(false)
    , m_hideOptional(false)
    , m_autoTracking(true)
    , m_maxTrackedQuests(10)
    , m_showProgressNotifications(true)
    , m_notificationThreshold(0.25f)  // Notify every 25% progress
    , m_maxRecentUpdates(50)
{
}

void ObjectiveTracker::trackQuest(std::shared_ptr<Quest> quest) {
    if (!quest) {
        return;
    }

    const std::string& questId = quest->getId();

    // Check if we've reached the max tracked quests
    if (m_trackedByQuest.size() >= static_cast<size_t>(m_maxTrackedQuests) &&
        m_trackedByQuest.find(questId) == m_trackedByQuest.end()) {
        return;
    }

    // Create tracked objectives for this quest
    std::vector<TrackedObjective> trackedObjectives;
    for (const auto& objective : quest->getObjectives()) {
        TrackedObjective tracked;
        tracked.quest = quest;
        tracked.objective = objective;
        tracked.lastProgress = objective.getProgress();
        tracked.isVisible = !objective.hidden;
        tracked.isPinned = false;
        tracked.updateCount = 0;

        trackedObjectives.push_back(tracked);
    }

    m_trackedByQuest[questId] = trackedObjectives;
}

void ObjectiveTracker::untrackQuest(const std::string& questId) {
    m_trackedByQuest.erase(questId);
}

void ObjectiveTracker::clearAllTracking() {
    m_trackedByQuest.clear();
}

bool ObjectiveTracker::isTracking(const std::string& questId) const {
    return m_trackedByQuest.find(questId) != m_trackedByQuest.end();
}

void ObjectiveTracker::updateObjective(const std::string& questId,
                                       const std::string& objectiveId,
                                       int newValue) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (!tracked) {
        return;
    }

    ObjectiveUpdate update;
    update.questId = questId;
    update.objectiveId = objectiveId;
    update.oldValue = tracked->objective.currentCount;
    update.newValue = newValue;
    update.requiredValue = tracked->objective.requiredCount;
    update.justCompleted = (newValue >= tracked->objective.requiredCount &&
                           update.oldValue < tracked->objective.requiredCount);

    tracked->objective.currentCount = newValue;
    tracked->updateCount++;

    addUpdate(update);
    notifyCallbacks(update);

    if (update.justCompleted) {
        checkQuestCompletion(questId);
    }
}

void ObjectiveTracker::incrementObjective(const std::string& questId,
                                          const std::string& objectiveId,
                                          int amount) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (!tracked) {
        return;
    }

    int newValue = std::min(tracked->objective.currentCount + amount,
                           tracked->objective.requiredCount);
    updateObjective(questId, objectiveId, newValue);
}

void ObjectiveTracker::completeObjective(const std::string& questId,
                                         const std::string& objectiveId) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (!tracked) {
        return;
    }

    updateObjective(questId, objectiveId, tracked->objective.requiredCount);
}

void ObjectiveTracker::notifyKill(const std::string& enemyId, int count) {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            if ((tracked.objective.type == ObjectiveType::KILL ||
                 tracked.objective.type == ObjectiveType::BOSS_KILL) &&
                tracked.objective.targetId == enemyId) {
                incrementObjective(pair.first, tracked.objective.id, count);
            }
        }
    }
}

void ObjectiveTracker::notifyItemCollected(const std::string& itemId, int count) {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            if (tracked.objective.type == ObjectiveType::COLLECT &&
                tracked.objective.targetId == itemId) {
                incrementObjective(pair.first, tracked.objective.id, count);
            }
        }
    }
}

void ObjectiveTracker::notifyLocationVisited(const std::string& locationId) {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            if ((tracked.objective.type == ObjectiveType::REACH_LOCATION ||
                 tracked.objective.type == ObjectiveType::DISCOVER) &&
                tracked.objective.targetId == locationId) {
                completeObjective(pair.first, tracked.objective.id);
            }
        }
    }
}

void ObjectiveTracker::notifyNpcInteracted(const std::string& npcId) {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            if ((tracked.objective.type == ObjectiveType::INTERACT ||
                 tracked.objective.type == ObjectiveType::DELIVER) &&
                tracked.objective.targetId == npcId) {
                completeObjective(pair.first, tracked.objective.id);
            }
        }
    }
}

void ObjectiveTracker::notifyItemCrafted(const std::string& itemId, int count) {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            if (tracked.objective.type == ObjectiveType::CRAFT &&
                tracked.objective.targetId == itemId) {
                incrementObjective(pair.first, tracked.objective.id, count);
            }
        }
    }
}

std::vector<TrackedObjective> ObjectiveTracker::getTrackedObjectives() const {
    std::vector<TrackedObjective> allTracked;
    for (const auto& pair : m_trackedByQuest) {
        for (const auto& tracked : pair.second) {
            if (!m_hideCompleted || !tracked.objective.isComplete()) {
                if (!m_hideOptional || !tracked.objective.optional) {
                    allTracked.push_back(tracked);
                }
            }
        }
    }
    return allTracked;
}

std::vector<TrackedObjective> ObjectiveTracker::getActiveObjectives() const {
    std::vector<TrackedObjective> active;
    for (const auto& pair : m_trackedByQuest) {
        for (const auto& tracked : pair.second) {
            if (!tracked.objective.isComplete()) {
                active.push_back(tracked);
            }
        }
    }
    return active;
}

std::vector<TrackedObjective> ObjectiveTracker::getCompletedObjectives() const {
    std::vector<TrackedObjective> completed;
    for (const auto& pair : m_trackedByQuest) {
        for (const auto& tracked : pair.second) {
            if (tracked.objective.isComplete()) {
                completed.push_back(tracked);
            }
        }
    }
    return completed;
}

std::vector<ObjectiveUpdate> ObjectiveTracker::getRecentUpdates(int count) const {
    int start = std::max(0, static_cast<int>(m_recentUpdates.size()) - count);
    return std::vector<ObjectiveUpdate>(m_recentUpdates.begin() + start,
                                        m_recentUpdates.end());
}

std::vector<ObjectiveUIElement> ObjectiveTracker::generateUIElements() const {
    std::vector<ObjectiveUIElement> elements;

    for (const auto& tracked : getTrackedObjectives()) {
        elements.push_back(generateUIElementForObjective(tracked));
    }

    return elements;
}

ObjectiveUIElement ObjectiveTracker::generateUIElementForObjective(
    const TrackedObjective& tracked) const {

    ObjectiveUIElement element;
    element.questName = tracked.quest->getName();
    element.objectiveDescription = tracked.objective.description;
    element.currentCount = tracked.objective.currentCount;
    element.requiredCount = tracked.objective.requiredCount;
    element.progress = tracked.objective.getProgress();
    element.isOptional = tracked.objective.optional;
    element.isCompleted = tracked.objective.isComplete();
    element.isPinned = tracked.isPinned;

    // Calculate distance if applicable
    float distance = getDistanceToObjective(tracked.objective.id);
    if (distance >= 0) {
        element.distanceToTarget = formatDistance(distance);
    }

    // Determine icon type based on objective type
    switch (tracked.objective.type) {
        case ObjectiveType::KILL:
        case ObjectiveType::BOSS_KILL:
            element.iconType = "combat";
            break;
        case ObjectiveType::COLLECT:
            element.iconType = "item";
            break;
        case ObjectiveType::REACH_LOCATION:
        case ObjectiveType::DISCOVER:
            element.iconType = "location";
            break;
        case ObjectiveType::INTERACT:
        case ObjectiveType::DELIVER:
            element.iconType = "npc";
            break;
        default:
            element.iconType = "default";
            break;
    }

    return element;
}

std::string ObjectiveTracker::formatObjectiveProgress(const QuestObjective& objective) const {
    std::stringstream ss;
    ss << objective.currentCount << "/" << objective.requiredCount;

    if (objective.isComplete()) {
        ss << " [COMPLETE]";
    } else {
        int percentage = static_cast<int>(objective.getProgress() * 100);
        ss << " (" << percentage << "%)";
    }

    return ss.str();
}

void ObjectiveTracker::pinObjective(const std::string& questId,
                                    const std::string& objectiveId) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (tracked) {
        tracked->isPinned = true;
    }
}

void ObjectiveTracker::unpinObjective(const std::string& questId,
                                      const std::string& objectiveId) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (tracked) {
        tracked->isPinned = false;
    }
}

void ObjectiveTracker::unpinAllObjectives() {
    for (auto& pair : m_trackedByQuest) {
        for (auto& tracked : pair.second) {
            tracked.isPinned = false;
        }
    }
}

std::vector<TrackedObjective> ObjectiveTracker::getPinnedObjectives() const {
    std::vector<TrackedObjective> pinned;
    for (const auto& pair : m_trackedByQuest) {
        for (const auto& tracked : pair.second) {
            if (tracked.isPinned) {
                pinned.push_back(tracked);
            }
        }
    }
    return pinned;
}

void ObjectiveTracker::setObjectiveVisible(const std::string& questId,
                                           const std::string& objectiveId,
                                           bool visible) {
    auto* tracked = findTrackedObjective(questId, objectiveId);
    if (tracked) {
        tracked->isVisible = visible;
    }
}

void ObjectiveTracker::updatePlayerPosition(float x, float y, float z) {
    m_playerPosition.x = x;
    m_playerPosition.y = y;
    m_playerPosition.z = z;
}

void ObjectiveTracker::setObjectiveLocation(const std::string& objectiveId,
                                            float x, float y, float z) {
    m_objectiveLocations[objectiveId] = Position(x, y, z);
}

float ObjectiveTracker::getDistanceToObjective(const std::string& objectiveId) const {
    auto it = m_objectiveLocations.find(objectiveId);
    if (it == m_objectiveLocations.end()) {
        return -1.0f;
    }

    return calculateDistance(m_playerPosition, it->second);
}

std::string ObjectiveTracker::formatDistance(float distance) const {
    if (distance < 0) {
        return "Unknown";
    }

    if (distance < 100) {
        return std::to_string(static_cast<int>(distance)) + "m";
    } else if (distance < 1000) {
        return std::to_string(static_cast<int>(distance / 10) * 10) + "m";
    } else {
        return std::to_string(static_cast<int>(distance / 1000)) + "km";
    }
}

int ObjectiveTracker::getTotalTrackedObjectives() const {
    int count = 0;
    for (const auto& pair : m_trackedByQuest) {
        count += static_cast<int>(pair.second.size());
    }
    return count;
}

int ObjectiveTracker::getCompletedObjectivesCount() const {
    return static_cast<int>(getCompletedObjectives().size());
}

float ObjectiveTracker::getOverallCompletionRate() const {
    int total = getTotalTrackedObjectives();
    if (total == 0) {
        return 0.0f;
    }
    return static_cast<float>(getCompletedObjectivesCount()) / total;
}

std::map<ObjectiveType, int> ObjectiveTracker::getObjectiveTypeStats() const {
    std::map<ObjectiveType, int> stats;

    for (const auto& pair : m_trackedByQuest) {
        for (const auto& tracked : pair.second) {
            stats[tracked.objective.type]++;
        }
    }

    return stats;
}

std::vector<TrackedObjective> ObjectiveTracker::sortByProgress(bool ascending) const {
    auto objectives = getTrackedObjectives();

    std::sort(objectives.begin(), objectives.end(),
             [ascending](const TrackedObjective& a, const TrackedObjective& b) {
                 float progressA = a.objective.getProgress();
                 float progressB = b.objective.getProgress();
                 return ascending ? (progressA < progressB) : (progressA > progressB);
             });

    return objectives;
}

std::vector<TrackedObjective> ObjectiveTracker::sortByDistance() const {
    auto objectives = getTrackedObjectives();

    std::sort(objectives.begin(), objectives.end(),
             [this](const TrackedObjective& a, const TrackedObjective& b) {
                 float distA = getDistanceToObjective(a.objective.id);
                 float distB = getDistanceToObjective(b.objective.id);

                 if (distA < 0) return false;
                 if (distB < 0) return true;
                 return distA < distB;
             });

    return objectives;
}

std::vector<TrackedObjective> ObjectiveTracker::filterByType(ObjectiveType type) const {
    std::vector<TrackedObjective> filtered;

    for (const auto& tracked : getTrackedObjectives()) {
        if (tracked.objective.type == type) {
            filtered.push_back(tracked);
        }
    }

    return filtered;
}

std::vector<TrackedObjective> ObjectiveTracker::filterByQuest(const std::string& questId) const {
    auto it = m_trackedByQuest.find(questId);
    if (it == m_trackedByQuest.end()) {
        return std::vector<TrackedObjective>();
    }

    return it->second;
}

std::string ObjectiveTracker::getObjectiveHint(const std::string& objectiveId) const {
    // This would integrate with the ObjectiveHintSystem
    return "Check your map for the objective location.";
}

std::vector<std::string> ObjectiveTracker::getSuggestedActions(
    const std::string& objectiveId) const {
    // Return suggested actions based on objective type
    return {"Follow the quest marker", "Check your inventory", "Talk to NPCs"};
}

bool ObjectiveTracker::isObjectiveInActiveArea(const std::string& objectiveId) const {
    float distance = getDistanceToObjective(objectiveId);
    return distance >= 0 && distance < 1000.0f;  // Within 1km
}

TrackedObjective* ObjectiveTracker::findTrackedObjective(const std::string& questId,
                                                         const std::string& objectiveId) {
    auto it = m_trackedByQuest.find(questId);
    if (it == m_trackedByQuest.end()) {
        return nullptr;
    }

    for (auto& tracked : it->second) {
        if (tracked.objective.id == objectiveId) {
            return &tracked;
        }
    }

    return nullptr;
}

void ObjectiveTracker::checkQuestCompletion(const std::string& questId) {
    auto it = m_trackedByQuest.find(questId);
    if (it == m_trackedByQuest.end()) {
        return;
    }

    bool allComplete = true;
    for (const auto& tracked : it->second) {
        if (!tracked.objective.optional && !tracked.objective.isComplete()) {
            allComplete = false;
            break;
        }
    }

    if (allComplete && m_onQuestCompleted) {
        m_onQuestCompleted(questId);
    }
}

void ObjectiveTracker::addUpdate(const ObjectiveUpdate& update) {
    m_recentUpdates.push_back(update);

    // Keep only the most recent updates
    if (m_recentUpdates.size() > static_cast<size_t>(m_maxRecentUpdates)) {
        m_recentUpdates.erase(m_recentUpdates.begin());
    }
}

void ObjectiveTracker::notifyCallbacks(const ObjectiveUpdate& update) {
    if (update.justCompleted && m_onObjectiveCompleted) {
        m_onObjectiveCompleted(update);
    }

    if (m_onObjectiveUpdated) {
        m_onObjectiveUpdated(update);
    }
}

float ObjectiveTracker::calculateDistance(const Position& a, const Position& b) const {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// ObjectiveHintSystem implementation
ObjectiveHintSystem::ObjectiveHintSystem()
    : m_randomEngine(static_cast<unsigned int>(std::time(nullptr)))
{
}

void ObjectiveHintSystem::registerHint(const std::string& objectiveId,
                                       const std::string& hint) {
    m_hints[objectiveId].push_back(hint);
}

void ObjectiveHintSystem::registerHints(const std::string& objectiveId,
                                        const std::vector<std::string>& hints) {
    m_hints[objectiveId] = hints;
}

std::string ObjectiveHintSystem::getHint(const std::string& objectiveId) const {
    auto it = m_hints.find(objectiveId);
    if (it == m_hints.end() || it->second.empty()) {
        return "No hints available.";
    }
    return it->second[0];
}

std::vector<std::string> ObjectiveHintSystem::getAllHints(const std::string& objectiveId) const {
    auto it = m_hints.find(objectiveId);
    if (it == m_hints.end()) {
        return std::vector<std::string>();
    }
    return it->second;
}

std::string ObjectiveHintSystem::getRandomHint(const std::string& objectiveId) const {
    auto it = m_hints.find(objectiveId);
    if (it == m_hints.end() || it->second.empty()) {
        return "No hints available.";
    }

    std::uniform_int_distribution<int> dist(0, static_cast<int>(it->second.size()) - 1);
    int index = dist(const_cast<std::mt19937&>(m_randomEngine));
    return it->second[index];
}

// ObjectiveWaypointSystem implementation
ObjectiveWaypointSystem::ObjectiveWaypointSystem()
    : m_waypointCounter(0)
{
}

void ObjectiveWaypointSystem::addWaypoint(const Waypoint& waypoint) {
    m_waypoints[waypoint.id] = waypoint;
}

void ObjectiveWaypointSystem::removeWaypoint(const std::string& waypointId) {
    m_waypoints.erase(waypointId);
}

void ObjectiveWaypointSystem::clearWaypoints() {
    m_waypoints.clear();
}

void ObjectiveWaypointSystem::clearWaypointsForQuest(const std::string& questId) {
    auto it = m_waypoints.begin();
    while (it != m_waypoints.end()) {
        if (it->second.questId == questId) {
            it = m_waypoints.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<ObjectiveWaypointSystem::Waypoint> ObjectiveWaypointSystem::getAllWaypoints() const {
    std::vector<Waypoint> waypoints;
    for (const auto& pair : m_waypoints) {
        waypoints.push_back(pair.second);
    }
    return waypoints;
}

std::vector<ObjectiveWaypointSystem::Waypoint> ObjectiveWaypointSystem::getVisibleWaypoints() const {
    std::vector<Waypoint> visible;
    for (const auto& pair : m_waypoints) {
        if (pair.second.isVisible) {
            visible.push_back(pair.second);
        }
    }
    return visible;
}

// ObjectiveCompletionHistory implementation
ObjectiveCompletionHistory::ObjectiveCompletionHistory() {
}

void ObjectiveCompletionHistory::recordCompletion(const std::string& questId,
                                                  const std::string& objectiveId,
                                                  const QuestObjective& objective,
                                                  float completionTime) {
    CompletionRecord record;
    record.questId = questId;
    record.objectiveId = objectiveId;
    record.completionTime = std::chrono::system_clock::now();
    record.attemptCount = ++m_attemptCounts[objectiveId];
    record.completionTimeSeconds = completionTime;
    record.wasOptional = objective.optional;

    m_history.push_back(record);
}

std::vector<ObjectiveCompletionHistory::CompletionRecord>
ObjectiveCompletionHistory::getCompletionHistory() const {
    return m_history;
}

int ObjectiveCompletionHistory::getTotalCompletions() const {
    return static_cast<int>(m_history.size());
}

float ObjectiveCompletionHistory::getAverageCompletionTime() const {
    if (m_history.empty()) {
        return 0.0f;
    }

    float total = 0.0f;
    for (const auto& record : m_history) {
        total += record.completionTimeSeconds;
    }

    return total / m_history.size();
}

} // namespace QuestSystem
