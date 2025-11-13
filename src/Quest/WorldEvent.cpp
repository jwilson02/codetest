#include "WorldEvent.h"
#include <algorithm>
#include <sstream>

namespace QuestSystem {

// WorldEvent implementation
WorldEvent::WorldEvent(const std::string& id, const std::string& name, WorldEventType type)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_state(EventState::SCHEDULED)
    , m_minParticipants(1)
    , m_maxParticipants(100)
    , m_minLevel(1)
    , m_difficultyScale(1.0f)
    , m_repeatable(false)
{
}

void WorldEvent::schedule(const std::chrono::system_clock::time_point& startTime,
                          const std::chrono::system_clock::time_point& endTime) {
    m_startTime = startTime;
    m_endTime = endTime;
    m_state = EventState::SCHEDULED;
}

void WorldEvent::start() {
    if (m_state != EventState::SCHEDULED) {
        return;
    }

    m_state = EventState::ACTIVE;
    m_actualStartTime = std::chrono::system_clock::now();

    if (m_onEventStart) {
        m_onEventStart();
    }
}

void WorldEvent::update(float deltaTime) {
    if (m_state != EventState::ACTIVE) {
        return;
    }

    // Check if event time has expired
    auto now = std::chrono::system_clock::now();
    if (now >= m_endTime) {
        end();
    }

    // Scale difficulty based on participant count
    scaleDifficultyByParticipants();
}

void WorldEvent::end() {
    if (m_state != EventState::ACTIVE) {
        return;
    }

    // Check if objectives were completed
    if (areAllObjectivesComplete()) {
        m_state = EventState::COMPLETED;
        distributeRewards();
    } else {
        m_state = EventState::FAILED;
    }

    if (m_onEventEnd) {
        m_onEventEnd();
    }
}

void WorldEvent::cancel() {
    m_state = EventState::CANCELLED;

    if (m_onEventEnd) {
        m_onEventEnd();
    }
}

int WorldEvent::getRemainingTime() const {
    if (m_state != EventState::ACTIVE) {
        return 0;
    }

    auto now = std::chrono::system_clock::now();
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(m_endTime - now);
    return std::max(0, static_cast<int>(remaining.count()));
}

int WorldEvent::getDuration() const {
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(m_endTime - m_startTime);
    return static_cast<int>(duration.count());
}

float WorldEvent::getTimeProgress() const {
    if (m_state != EventState::ACTIVE) {
        return 0.0f;
    }

    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_actualStartTime);
    auto total = std::chrono::duration_cast<std::chrono::seconds>(m_endTime - m_actualStartTime);

    if (total.count() == 0) {
        return 0.0f;
    }

    return std::min(1.0f, static_cast<float>(elapsed.count()) / total.count());
}

void WorldEvent::addObjective(const WorldEventObjective& objective) {
    m_objectives.push_back(objective);
}

void WorldEvent::updateObjective(const std::string& objectiveId, int progress,
                                 const std::string& playerId) {
    for (auto& obj : m_objectives) {
        if (obj.id == objectiveId) {
            obj.currentProgress = std::min(obj.currentProgress + progress, obj.requiredProgress);

            // Track player contribution
            if (!playerId.empty() && obj.isGlobalProgress) {
                auto it = m_participants.find(playerId);
                if (it != m_participants.end()) {
                    it->second.objectiveContributions[objectiveId] += progress;
                    it->second.contributionScore += progress;
                    it->second.lastContributionTime = std::chrono::system_clock::now();
                    it->second.level = calculateParticipationLevel(it->second.contributionScore);
                }
            }

            if (obj.currentProgress >= obj.requiredProgress) {
                obj.isCompleted = true;
                if (m_onObjectiveComplete) {
                    m_onObjectiveComplete(objectiveId);
                }
            }
            break;
        }
    }
}

void WorldEvent::completeObjective(const std::string& objectiveId) {
    for (auto& obj : m_objectives) {
        if (obj.id == objectiveId) {
            obj.currentProgress = obj.requiredProgress;
            obj.isCompleted = true;

            if (m_onObjectiveComplete) {
                m_onObjectiveComplete(objectiveId);
            }
            break;
        }
    }
}

float WorldEvent::getOverallProgress() const {
    if (m_objectives.empty()) {
        return 0.0f;
    }

    float totalProgress = 0.0f;
    for (const auto& obj : m_objectives) {
        totalProgress += obj.getProgress();
    }

    return totalProgress / m_objectives.size();
}

bool WorldEvent::areAllObjectivesComplete() const {
    for (const auto& obj : m_objectives) {
        if (!obj.isCompleted) {
            return false;
        }
    }
    return !m_objectives.empty();
}

void WorldEvent::addParticipant(const std::string& playerId) {
    if (m_participants.find(playerId) != m_participants.end()) {
        return;
    }

    PlayerParticipation participation;
    participation.playerId = playerId;
    participation.joinTime = std::chrono::system_clock::now();
    participation.level = ParticipationLevel::OBSERVER;

    m_participants[playerId] = participation;
}

void WorldEvent::removeParticipant(const std::string& playerId) {
    m_participants.erase(playerId);
}

void WorldEvent::updateParticipantContribution(const std::string& playerId, int contribution) {
    auto it = m_participants.find(playerId);
    if (it != m_participants.end()) {
        it->second.contributionScore += contribution;
        it->second.lastContributionTime = std::chrono::system_clock::now();
        it->second.level = calculateParticipationLevel(it->second.contributionScore);
    }
}

PlayerParticipation* WorldEvent::getParticipation(const std::string& playerId) {
    auto it = m_participants.find(playerId);
    return (it != m_participants.end()) ? &it->second : nullptr;
}

std::vector<PlayerParticipation> WorldEvent::getAllParticipants() const {
    std::vector<PlayerParticipation> participants;
    for (const auto& pair : m_participants) {
        participants.push_back(pair.second);
    }
    return participants;
}

int WorldEvent::getParticipantCount() const {
    return static_cast<int>(m_participants.size());
}

ParticipationLevel WorldEvent::calculateParticipationLevel(int contributionScore) const {
    if (contributionScore >= 1000) return ParticipationLevel::LEGEND;
    if (contributionScore >= 500) return ParticipationLevel::HERO;
    if (contributionScore >= 200) return ParticipationLevel::CONTRIBUTOR;
    if (contributionScore >= 50) return ParticipationLevel::PARTICIPANT;
    if (contributionScore > 0) return ParticipationLevel::OBSERVER;
    return ParticipationLevel::NONE;
}

WorldEventReward WorldEvent::calculatePlayerReward(const std::string& playerId) const {
    auto participation = const_cast<WorldEvent*>(this)->getParticipation(playerId);
    if (!participation) {
        return WorldEventReward();
    }

    WorldEventReward playerReward = m_reward;

    // Apply tier multipliers
    ParticipationLevel level = participation->level;

    auto expIt = m_reward.tieredExperience.find(level);
    if (expIt != m_reward.tieredExperience.end()) {
        playerReward.experienceBase = expIt->second;
    }

    auto goldIt = m_reward.tieredGold.find(level);
    if (goldIt != m_reward.tieredGold.end()) {
        playerReward.goldBase = goldIt->second;
    }

    auto itemsIt = m_reward.tieredItems.find(level);
    if (itemsIt != m_reward.tieredItems.end()) {
        playerReward.items = itemsIt->second;
    }

    return playerReward;
}

void WorldEvent::distributeRewards() {
    // This would integrate with the game's reward system
    // For now, just calculate rewards for each participant
    for (const auto& pair : m_participants) {
        WorldEventReward playerReward = calculatePlayerReward(pair.first);
        // TODO: Actually grant the rewards to the player
    }
}

void WorldEvent::scaleDifficultyByParticipants() {
    int participantCount = getParticipantCount();

    // Scale difficulty based on participant count
    if (participantCount >= m_maxParticipants) {
        m_difficultyScale = 2.0f;
    } else if (participantCount >= m_maxParticipants / 2) {
        m_difficultyScale = 1.5f;
    } else if (participantCount >= m_minParticipants) {
        m_difficultyScale = 1.0f;
    } else {
        m_difficultyScale = 0.75f;
    }
}

std::string WorldEvent::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\":\"" << m_id << "\",";
    ss << "\"name\":\"" << m_name << "\",";
    ss << "\"type\":" << static_cast<int>(m_type) << ",";
    ss << "\"state\":" << static_cast<int>(m_state) << ",";
    ss << "\"progress\":" << getOverallProgress() << ",";
    ss << "\"participants\":" << getParticipantCount();
    ss << "}";
    return ss.str();
}

// InvasionEvent implementation
InvasionEvent::InvasionEvent(const std::string& id, const std::string& name,
                             const std::string& invasionLocation)
    : WorldEvent(id, name, WorldEventType::INVASION)
    , m_invadingFaction(Faction::NONE)
    , m_currentWave(0)
    , m_totalWaves(5)
{
    setEventLocation(invasionLocation);
}

void InvasionEvent::advanceWave() {
    if (m_currentWave < m_totalWaves) {
        m_currentWave++;
    }
}

// BossSpawnEvent implementation
BossSpawnEvent::BossSpawnEvent(const std::string& id, const std::string& name,
                               const std::string& bossId)
    : WorldEvent(id, name, WorldEventType::BOSS_SPAWN)
    , m_bossId(bossId)
    , m_currentBossHealth(10000)
    , m_maxBossHealth(10000)
{
}

// SeasonalEvent implementation
SeasonalEvent::SeasonalEvent(const std::string& id, const std::string& name,
                             const std::string& season)
    : WorldEvent(id, name, WorldEventType::SEASONAL)
    , m_season(season)
    , m_year(2025)
{
}

// WorldEventManager implementation
void WorldEventManager::registerEvent(std::shared_ptr<WorldEvent> event) {
    if (!event) {
        return;
    }
    m_events[event->getId()] = event;
}

void WorldEventManager::unregisterEvent(const std::string& eventId) {
    m_events.erase(eventId);
}

void WorldEventManager::startEvent(const std::string& eventId) {
    auto event = getEvent(eventId);
    if (event) {
        event->start();

        if (m_onEventStarted) {
            m_onEventStarted(event);
        }
    }
}

void WorldEventManager::endEvent(const std::string& eventId) {
    auto event = getEvent(eventId);
    if (event) {
        event->end();

        if (event->isCompleted()) {
            m_totalEventsCompleted++;
        }

        if (m_onEventEnded) {
            m_onEventEnded(event);
        }
    }
}

void WorldEventManager::cancelEvent(const std::string& eventId) {
    auto event = getEvent(eventId);
    if (event) {
        event->cancel();

        if (m_onEventEnded) {
            m_onEventEnded(event);
        }
    }
}

void WorldEventManager::update(float deltaTime) {
    checkScheduledEvents();

    // Update all active events
    for (auto& pair : m_events) {
        if (pair.second->isActive()) {
            pair.second->update(deltaTime);
        }
    }
}

void WorldEventManager::checkScheduledEvents() {
    auto now = std::chrono::system_clock::now();

    for (auto& pair : m_events) {
        auto event = pair.second;
        if (event->isScheduled() && now >= event->getStartTime()) {
            startEvent(event->getId());
        }
    }
}

std::shared_ptr<WorldEvent> WorldEventManager::getEvent(const std::string& eventId) {
    auto it = m_events.find(eventId);
    return (it != m_events.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<WorldEvent>> WorldEventManager::getAllEvents() const {
    std::vector<std::shared_ptr<WorldEvent>> events;
    for (const auto& pair : m_events) {
        events.push_back(pair.second);
    }
    return events;
}

std::vector<std::shared_ptr<WorldEvent>> WorldEventManager::getActiveEvents() const {
    std::vector<std::shared_ptr<WorldEvent>> active;
    for (const auto& pair : m_events) {
        if (pair.second->isActive()) {
            active.push_back(pair.second);
        }
    }
    return active;
}

std::vector<std::shared_ptr<WorldEvent>> WorldEventManager::getScheduledEvents() const {
    std::vector<std::shared_ptr<WorldEvent>> scheduled;
    for (const auto& pair : m_events) {
        if (pair.second->isScheduled()) {
            scheduled.push_back(pair.second);
        }
    }
    return scheduled;
}

std::vector<std::shared_ptr<WorldEvent>> WorldEventManager::getEventsByType(
    WorldEventType type) const {
    std::vector<std::shared_ptr<WorldEvent>> events;
    for (const auto& pair : m_events) {
        if (pair.second->getType() == type) {
            events.push_back(pair.second);
        }
    }
    return events;
}

std::vector<std::shared_ptr<WorldEvent>> WorldEventManager::getEventsByLocation(
    const std::string& location) const {
    std::vector<std::shared_ptr<WorldEvent>> events;
    for (const auto& pair : m_events) {
        if (pair.second->getEventLocation() == location) {
            events.push_back(pair.second);
        }
    }
    return events;
}

void WorldEventManager::joinEvent(const std::string& eventId, const std::string& playerId) {
    auto event = getEvent(eventId);
    if (event && event->isActive()) {
        event->addParticipant(playerId);
    }
}

void WorldEventManager::leaveEvent(const std::string& eventId, const std::string& playerId) {
    auto event = getEvent(eventId);
    if (event) {
        event->removeParticipant(playerId);
    }
}

void WorldEventManager::contributeToEvent(const std::string& eventId,
                                          const std::string& playerId,
                                          const std::string& objectiveId,
                                          int contribution) {
    auto event = getEvent(eventId);
    if (event && event->isActive()) {
        event->updateObjective(objectiveId, contribution, playerId);
    }
}

void WorldEventManager::scheduleEvent(std::shared_ptr<WorldEvent> event,
                                      const std::chrono::system_clock::time_point& startTime,
                                      const std::chrono::system_clock::time_point& endTime) {
    event->schedule(startTime, endTime);
    registerEvent(event);
}

void WorldEventManager::scheduleRecurringEvent(std::shared_ptr<WorldEvent> event,
                                               int intervalHours,
                                               int durationMinutes) {
    auto now = std::chrono::system_clock::now();
    auto startTime = now + std::chrono::hours(intervalHours);
    auto endTime = startTime + std::chrono::minutes(durationMinutes);

    scheduleEvent(event, startTime, endTime);
}

std::shared_ptr<WorldEvent> WorldEventManager::generateRandomEvent(int playerLevel) {
    // Simple random event generation
    WorldEventType types[] = {
        WorldEventType::INVASION,
        WorldEventType::BOSS_SPAWN,
        WorldEventType::RESOURCE_BOOM,
        WorldEventType::XP_BOOST
    };

    int typeIndex = rand() % 4;
    WorldEventType type = types[typeIndex];

    std::string eventId = "random_event_" + std::to_string(time(nullptr));
    std::string eventName = "Random Event";

    auto event = std::make_shared<WorldEvent>(eventId, eventName, type);
    event->setMinLevel(std::max(1, playerLevel - 5));

    return event;
}

std::shared_ptr<InvasionEvent> WorldEventManager::generateInvasionEvent(
    const std::string& location) {
    std::string eventId = "invasion_" + location + "_" + std::to_string(time(nullptr));
    std::string eventName = "Invasion of " + location;

    auto event = std::make_shared<InvasionEvent>(eventId, eventName, location);
    event->setWaveCount(5);
    event->setInvadingFaction(Faction::DARK_BROTHERHOOD);

    // Add objectives
    WorldEventObjective obj;
    obj.id = "defeat_invaders";
    obj.description = "Defeat the invading forces";
    obj.type = ObjectiveType::KILL;
    obj.targetId = "invader";
    obj.requiredProgress = 100;
    obj.isGlobalProgress = true;
    event->addObjective(obj);

    return event;
}

std::shared_ptr<BossSpawnEvent> WorldEventManager::generateBossEvent(int playerLevel) {
    std::string bossId = "world_boss_" + std::to_string(playerLevel);
    std::string eventId = "boss_spawn_" + std::to_string(time(nullptr));
    std::string eventName = "World Boss: Ancient Dragon";

    auto event = std::make_shared<BossSpawnEvent>(eventId, eventName, bossId);
    event->setMinLevel(playerLevel);
    event->setMinParticipants(5);
    event->setMaxParticipants(50);
    event->setBossHealth(100000, 100000);

    return event;
}

std::shared_ptr<SeasonalEvent> WorldEventManager::generateSeasonalEvent(
    const std::string& season) {
    std::string eventId = "seasonal_" + season + "_" + std::to_string(time(nullptr));
    std::string eventName = season + " Festival";

    auto event = std::make_shared<SeasonalEvent>(eventId, eventName, season);
    event->setSeason(season);
    event->setYear(2025);
    event->setRepeatable(true);

    return event;
}

void WorldEventManager::announceEvent(const std::string& eventId, const std::string& message) {
    m_announcements.push_back("[Event: " + eventId + "] " + message);

    // Keep only recent announcements
    if (m_announcements.size() > 100) {
        m_announcements.erase(m_announcements.begin());
    }
}

std::vector<std::string> WorldEventManager::getRecentAnnouncements(int count) const {
    int start = std::max(0, static_cast<int>(m_announcements.size()) - count);
    return std::vector<std::string>(m_announcements.begin() + start, m_announcements.end());
}

int WorldEventManager::getTotalEventsActive() const {
    return static_cast<int>(getActiveEvents().size());
}

std::map<WorldEventType, int> WorldEventManager::getEventTypeStatistics() const {
    std::map<WorldEventType, int> stats;

    for (const auto& pair : m_events) {
        stats[pair.second->getType()]++;
    }

    return stats;
}

// EventCalendar implementation
EventCalendar::EventCalendar() {
}

void EventCalendar::addSchedule(const ScheduledEvent& schedule) {
    m_schedule[schedule.eventId] = schedule;
}

void EventCalendar::removeSchedule(const std::string& eventId) {
    m_schedule.erase(eventId);
}

std::vector<EventCalendar::ScheduledEvent> EventCalendar::getUpcomingEvents(int hours) const {
    std::vector<ScheduledEvent> upcoming;
    auto now = std::chrono::system_clock::now();
    auto cutoff = now + std::chrono::hours(hours);

    for (const auto& pair : m_schedule) {
        if (pair.second.startTime >= now && pair.second.startTime <= cutoff) {
            upcoming.push_back(pair.second);
        }
    }

    // Sort by start time
    std::sort(upcoming.begin(), upcoming.end(),
             [](const ScheduledEvent& a, const ScheduledEvent& b) {
                 return a.startTime < b.startTime;
             });

    return upcoming;
}

std::vector<EventCalendar::ScheduledEvent> EventCalendar::getAllSchedules() const {
    std::vector<ScheduledEvent> schedules;
    for (const auto& pair : m_schedule) {
        schedules.push_back(pair.second);
    }
    return schedules;
}

bool EventCalendar::isEventScheduled(const std::string& eventId) const {
    return m_schedule.find(eventId) != m_schedule.end();
}

// EventLeaderboard implementation
EventLeaderboard::EventLeaderboard(const std::string& eventId)
    : m_eventId(eventId)
{
}

void EventLeaderboard::updateScore(const std::string& playerId,
                                   const std::string& playerName,
                                   int score) {
    auto& entry = m_entries[playerId];
    entry.playerId = playerId;
    entry.playerName = playerName;
    entry.score = score;

    recalculateRanks();
}

void EventLeaderboard::recalculateRanks() {
    // Create sorted list
    m_rankedPlayerIds.clear();
    for (const auto& pair : m_entries) {
        m_rankedPlayerIds.push_back(pair.first);
    }

    std::sort(m_rankedPlayerIds.begin(), m_rankedPlayerIds.end(),
             [this](const std::string& a, const std::string& b) {
                 return m_entries[a].score > m_entries[b].score;
             });

    // Assign ranks
    for (size_t i = 0; i < m_rankedPlayerIds.size(); ++i) {
        m_entries[m_rankedPlayerIds[i]].rank = static_cast<int>(i + 1);
    }
}

std::vector<EventLeaderboard::LeaderboardEntry> EventLeaderboard::getTopPlayers(int count) const {
    std::vector<LeaderboardEntry> top;

    int limit = std::min(count, static_cast<int>(m_rankedPlayerIds.size()));
    for (int i = 0; i < limit; ++i) {
        auto it = m_entries.find(m_rankedPlayerIds[i]);
        if (it != m_entries.end()) {
            top.push_back(it->second);
        }
    }

    return top;
}

EventLeaderboard::LeaderboardEntry EventLeaderboard::getPlayerEntry(
    const std::string& playerId) const {
    auto it = m_entries.find(playerId);
    if (it != m_entries.end()) {
        return it->second;
    }
    return LeaderboardEntry();
}

int EventLeaderboard::getPlayerRank(const std::string& playerId) const {
    auto it = m_entries.find(playerId);
    if (it != m_entries.end()) {
        return it->second.rank;
    }
    return 0;
}

} // namespace QuestSystem
