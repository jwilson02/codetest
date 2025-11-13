#include "Quest.h"
#include <algorithm>
#include <sstream>
#include <ctime>

namespace QuestSystem {

// Quest implementation
Quest::Quest(const std::string& id, const std::string& name, QuestType type)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_status(QuestStatus::LOCKED)
    , m_difficulty(QuestDifficulty::NORMAL)
    , m_recommendedLevel(1)
    , m_prerequisiteLevel(0)
    , m_requiredFaction(Faction::NONE)
    , m_requiredReputation(0)
    , m_timed(false)
    , m_timeLimitSeconds(0)
    , m_hidden(false)
    , m_repeatable(false)
{
}

bool Quest::canStart() const {
    return m_status == QuestStatus::AVAILABLE;
}

bool Quest::canComplete() const {
    if (m_status != QuestStatus::ACTIVE) {
        return false;
    }

    // Check if timed quest has expired
    if (m_timed && isExpired()) {
        return false;
    }

    // Check all required objectives
    for (const auto& obj : m_objectives) {
        if (!obj.optional && !obj.isComplete()) {
            return false;
        }
    }

    return true;
}

void Quest::start() {
    if (!canStart()) {
        return;
    }

    m_status = QuestStatus::ACTIVE;

    if (m_timed) {
        m_startTime = std::chrono::system_clock::now();
    }
}

void Quest::complete() {
    if (!canComplete()) {
        return;
    }

    m_status = QuestStatus::COMPLETED;

    if (m_onComplete) {
        m_onComplete();
    }
}

void Quest::fail() {
    if (m_status != QuestStatus::ACTIVE) {
        return;
    }

    m_status = QuestStatus::FAILED;

    if (m_onFail) {
        m_onFail();
    }
}

void Quest::abandon() {
    if (m_status != QuestStatus::ACTIVE) {
        return;
    }

    m_status = QuestStatus::ABANDONED;
}

void Quest::addObjective(const QuestObjective& objective) {
    m_objectives.push_back(objective);
}

void Quest::updateObjective(const std::string& objectiveId, int progress) {
    for (auto& obj : m_objectives) {
        if (obj.id == objectiveId) {
            obj.currentCount = std::min(obj.currentCount + progress, obj.requiredCount);

            if (m_onProgress) {
                m_onProgress(getOverallProgress());
            }
            break;
        }
    }
}

void Quest::completeObjective(const std::string& objectiveId) {
    for (auto& obj : m_objectives) {
        if (obj.id == objectiveId) {
            obj.currentCount = obj.requiredCount;

            if (m_onProgress) {
                m_onProgress(getOverallProgress());
            }
            break;
        }
    }
}

float Quest::getOverallProgress() const {
    if (m_objectives.empty()) {
        return 0.0f;
    }

    float totalProgress = 0.0f;
    int requiredObjectives = 0;

    for (const auto& obj : m_objectives) {
        if (!obj.optional) {
            totalProgress += obj.getProgress();
            requiredObjectives++;
        }
    }

    return requiredObjectives > 0 ? totalProgress / requiredObjectives : 0.0f;
}

void Quest::addPrerequisiteQuest(const std::string& questId) {
    m_prerequisiteQuests.push_back(questId);
}

void Quest::addPrerequisiteLevel(int level) {
    m_prerequisiteLevel = level;
}

void Quest::addPrerequisiteReputation(Faction faction, int reputation) {
    m_requiredFaction = faction;
    m_requiredReputation = reputation;
}

bool Quest::checkPrerequisites(int playerLevel,
                               const std::map<Faction, int>& reputations,
                               const std::vector<std::string>& completedQuests) const {
    // Check level requirement
    if (playerLevel < m_prerequisiteLevel) {
        return false;
    }

    // Check reputation requirement
    if (m_requiredFaction != Faction::NONE) {
        auto it = reputations.find(m_requiredFaction);
        if (it == reputations.end() || it->second < m_requiredReputation) {
            return false;
        }
    }

    // Check prerequisite quests
    for (const auto& prereqId : m_prerequisiteQuests) {
        if (std::find(completedQuests.begin(), completedQuests.end(), prereqId)
            == completedQuests.end()) {
            return false;
        }
    }

    return true;
}

void Quest::setTimeLimit(int seconds) {
    m_timed = true;
    m_timeLimitSeconds = seconds;
}

bool Quest::isExpired() const {
    if (!m_timed || m_status != QuestStatus::ACTIVE) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    return elapsed.count() >= m_timeLimitSeconds;
}

int Quest::getRemainingTime() const {
    if (!m_timed || m_status != QuestStatus::ACTIVE) {
        return 0;
    }

    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    return std::max(0, static_cast<int>(m_timeLimitSeconds - elapsed.count()));
}

void Quest::setRequiredFaction(Faction faction, int reputation) {
    m_requiredFaction = faction;
    m_requiredReputation = reputation;
}

std::string Quest::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\":\"" << m_id << "\",";
    ss << "\"name\":\"" << m_name << "\",";
    ss << "\"description\":\"" << m_description << "\",";
    ss << "\"type\":" << static_cast<int>(m_type) << ",";
    ss << "\"status\":" << static_cast<int>(m_status) << ",";
    ss << "\"difficulty\":" << static_cast<int>(m_difficulty) << ",";
    ss << "\"recommendedLevel\":" << m_recommendedLevel << ",";
    ss << "\"progress\":" << getOverallProgress();
    ss << "}";
    return ss.str();
}

// StoryQuest implementation
StoryQuest::StoryQuest(const std::string& id, const std::string& name, int chapter)
    : Quest(id, name, QuestType::MAIN_STORY)
    , m_chapter(chapter)
{
}

// BossQuest implementation
BossQuest::BossQuest(const std::string& id, const std::string& name, const std::string& bossId)
    : Quest(id, name, QuestType::BOSS_QUEST)
    , m_bossId(bossId)
    , m_minPlayers(1)
    , m_maxPlayers(5)
{
}

// DailyQuest implementation
DailyQuest::DailyQuest(const std::string& id, const std::string& name)
    : Quest(id, name, QuestType::DAILY_QUEST)
{
    m_repeatable = true;
}

void DailyQuest::resetDaily() {
    if (m_status == QuestStatus::COMPLETED) {
        m_status = QuestStatus::AVAILABLE;

        // Reset objectives
        for (auto& obj : m_objectives) {
            obj.currentCount = 0;
        }
    }
}

bool DailyQuest::canAcceptToday() const {
    if (m_status != QuestStatus::COMPLETED) {
        return m_status == QuestStatus::AVAILABLE;
    }

    // Check if 24 hours have passed since last completion
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - m_lastCompletionTime);
    return elapsed.count() >= 24;
}

} // namespace QuestSystem
