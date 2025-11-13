#include "QuestSystem.h"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace QuestSystem {

void QuestSystem::initialize() {
    if (m_initialized) {
        return;
    }

    m_notifications.clear();
    m_questLog.clear();
    m_initialized = true;
}

void QuestSystem::shutdown() {
    m_quests.clear();
    m_questLog.clear();
    m_notifications.clear();
    m_initialized = false;
}

bool QuestSystem::loadQuestFromJson(const std::string& jsonPath) {
    // TODO: Implement JSON parsing
    // For now, this is a placeholder
    return false;
}

bool QuestSystem::loadQuestsFromDirectory(const std::string& directory) {
    // TODO: Implement directory scanning and loading
    return false;
}

void QuestSystem::registerQuest(std::shared_ptr<Quest> quest) {
    if (!quest) {
        return;
    }

    m_quests[quest->getId()] = quest;

    // Check if quest is available for player
    if (checkQuestPrerequisites(quest) && !quest->isHidden()) {
        quest->setStatus(QuestStatus::AVAILABLE);
        m_playerData.availableQuestIds.push_back(quest->getId());
    }
}

std::shared_ptr<Quest> QuestSystem::getQuest(const std::string& questId) {
    auto it = m_quests.find(questId);
    return (it != m_quests.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getAllQuests() const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& pair : m_quests) {
        quests.push_back(pair.second);
    }
    return quests;
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getQuestsByType(QuestType type) const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& pair : m_quests) {
        if (pair.second->getType() == type) {
            quests.push_back(pair.second);
        }
    }
    return quests;
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getQuestsByFaction(Faction faction) const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& pair : m_quests) {
        if (pair.second->getRequiredFaction() == faction) {
            quests.push_back(pair.second);
        }
    }
    return quests;
}

void QuestSystem::updateAvailableQuests() {
    m_playerData.availableQuestIds.clear();

    for (const auto& pair : m_quests) {
        auto quest = pair.second;

        // Skip if already completed and not repeatable
        if (isQuestCompleted(quest->getId()) && !quest->isRepeatable()) {
            continue;
        }

        // Skip if already active
        if (quest->getStatus() == QuestStatus::ACTIVE) {
            continue;
        }

        // Skip hidden quests not yet discovered
        if (quest->isHidden() &&
            m_discoveredHiddenQuests.find(quest->getId()) == m_discoveredHiddenQuests.end()) {
            continue;
        }

        // Check prerequisites
        if (checkQuestPrerequisites(quest)) {
            quest->setStatus(QuestStatus::AVAILABLE);
            m_playerData.availableQuestIds.push_back(quest->getId());
        }
    }
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getAvailableQuests() const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& questId : m_playerData.availableQuestIds) {
        auto quest = const_cast<QuestSystem*>(this)->getQuest(questId);
        if (quest && quest->getStatus() == QuestStatus::AVAILABLE) {
            quests.push_back(quest);
        }
    }
    return quests;
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getActiveQuests() const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& questId : m_playerData.activeQuestIds) {
        auto quest = const_cast<QuestSystem*>(this)->getQuest(questId);
        if (quest) {
            quests.push_back(quest);
        }
    }
    return quests;
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getCompletedQuests() const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& questId : m_playerData.completedQuestIds) {
        auto quest = const_cast<QuestSystem*>(this)->getQuest(questId);
        if (quest) {
            quests.push_back(quest);
        }
    }
    return quests;
}

bool QuestSystem::isQuestAvailable(const std::string& questId) const {
    return std::find(m_playerData.availableQuestIds.begin(),
                     m_playerData.availableQuestIds.end(),
                     questId) != m_playerData.availableQuestIds.end();
}

bool QuestSystem::isQuestCompleted(const std::string& questId) const {
    return std::find(m_playerData.completedQuestIds.begin(),
                     m_playerData.completedQuestIds.end(),
                     questId) != m_playerData.completedQuestIds.end();
}

bool QuestSystem::acceptQuest(const std::string& questId) {
    auto quest = getQuest(questId);
    if (!quest || !quest->canStart()) {
        return false;
    }

    quest->start();

    // Update player data
    m_playerData.activeQuestIds.push_back(questId);
    auto it = std::find(m_playerData.availableQuestIds.begin(),
                       m_playerData.availableQuestIds.end(),
                       questId);
    if (it != m_playerData.availableQuestIds.end()) {
        m_playerData.availableQuestIds.erase(it);
    }

    // Create quest log entry
    QuestLogEntry entry;
    entry.quest = quest;
    entry.acceptedTime = std::chrono::system_clock::now();
    entry.attempts = 1;
    m_questLog[questId] = entry;

    // Send notification
    addNotification(QuestNotificationType::QUEST_STARTED,
                   questId,
                   "Quest accepted: " + quest->getName());

    // Callback
    if (m_onQuestAccepted) {
        m_onQuestAccepted(quest);
    }

    return true;
}

bool QuestSystem::completeQuest(const std::string& questId) {
    auto quest = getQuest(questId);
    if (!quest || !quest->canComplete()) {
        return false;
    }

    quest->complete();

    // Grant rewards
    grantReward(quest->getReward());

    // Update player data
    m_playerData.completedQuestIds.push_back(questId);
    m_playerData.questCompletionCount[questId]++;

    auto it = std::find(m_playerData.activeQuestIds.begin(),
                       m_playerData.activeQuestIds.end(),
                       questId);
    if (it != m_playerData.activeQuestIds.end()) {
        m_playerData.activeQuestIds.erase(it);
    }

    // Unlock next quests in chain
    unlockNextQuests(quest->getNextQuests());

    // Send notification
    addNotification(QuestNotificationType::QUEST_COMPLETED,
                   questId,
                   "Quest completed: " + quest->getName());

    // Callback
    if (m_onQuestCompleted) {
        m_onQuestCompleted(quest);
    }

    return true;
}

bool QuestSystem::failQuest(const std::string& questId) {
    auto quest = getQuest(questId);
    if (!quest) {
        return false;
    }

    quest->fail();

    // Update player data
    auto it = std::find(m_playerData.activeQuestIds.begin(),
                       m_playerData.activeQuestIds.end(),
                       questId);
    if (it != m_playerData.activeQuestIds.end()) {
        m_playerData.activeQuestIds.erase(it);
    }

    // Send notification
    addNotification(QuestNotificationType::QUEST_FAILED,
                   questId,
                   "Quest failed: " + quest->getName());

    return true;
}

bool QuestSystem::abandonQuest(const std::string& questId) {
    auto quest = getQuest(questId);
    if (!quest) {
        return false;
    }

    quest->abandon();

    // Update player data
    auto it = std::find(m_playerData.activeQuestIds.begin(),
                       m_playerData.activeQuestIds.end(),
                       questId);
    if (it != m_playerData.activeQuestIds.end()) {
        m_playerData.activeQuestIds.erase(it);
    }

    // Make quest available again if repeatable
    if (quest->isRepeatable()) {
        m_playerData.availableQuestIds.push_back(questId);
        quest->setStatus(QuestStatus::AVAILABLE);
    }

    return true;
}

void QuestSystem::updateObjective(const std::string& questId,
                                  const std::string& objectiveId,
                                  int progress) {
    auto quest = getQuest(questId);
    if (!quest || quest->getStatus() != QuestStatus::ACTIVE) {
        return;
    }

    quest->updateObjective(objectiveId, progress);

    // Send notification
    addNotification(QuestNotificationType::QUEST_UPDATED,
                   questId,
                   "Quest updated: " + quest->getName());

    // Callback
    if (m_onObjectiveUpdated) {
        m_onObjectiveUpdated(quest, objectiveId);
    }
}

void QuestSystem::notifyEnemyKilled(const std::string& enemyId) {
    updateQuestObjectivesForEvent(enemyId, ObjectiveType::KILL, 1);
    updateQuestObjectivesForEvent(enemyId, ObjectiveType::BOSS_KILL, 1);
}

void QuestSystem::notifyItemCollected(const std::string& itemId, int count) {
    updateQuestObjectivesForEvent(itemId, ObjectiveType::COLLECT, count);
}

void QuestSystem::notifyLocationReached(const std::string& locationId) {
    updateQuestObjectivesForEvent(locationId, ObjectiveType::REACH_LOCATION, 1);
    updateQuestObjectivesForEvent(locationId, ObjectiveType::DISCOVER, 1);
}

void QuestSystem::notifyNpcInteraction(const std::string& npcId) {
    updateQuestObjectivesForEvent(npcId, ObjectiveType::INTERACT, 1);
    updateQuestObjectivesForEvent(npcId, ObjectiveType::DELIVER, 1);
}

void QuestSystem::updateQuestObjectivesForEvent(const std::string& targetId,
                                                ObjectiveType type,
                                                int count) {
    for (const auto& questId : m_playerData.activeQuestIds) {
        auto quest = getQuest(questId);
        if (!quest) continue;

        for (auto& objective : quest->getObjectives()) {
            if (objective.type == type && objective.targetId == targetId) {
                quest->updateObjective(objective.id, count);

                if (objective.isComplete()) {
                    addNotification(QuestNotificationType::OBJECTIVE_COMPLETED,
                                  questId,
                                  "Objective completed: " + objective.description);
                }
            }
        }
    }
}

void QuestSystem::unlockQuestChain(const std::string& questId) {
    auto chain = getQuestChain(questId);
    for (const auto& chainQuestId : chain) {
        auto quest = getQuest(chainQuestId);
        if (quest && quest->getStatus() == QuestStatus::LOCKED) {
            if (checkQuestPrerequisites(quest)) {
                quest->setStatus(QuestStatus::AVAILABLE);
                m_playerData.availableQuestIds.push_back(chainQuestId);
            }
        }
    }

    if (!chain.empty()) {
        addNotification(QuestNotificationType::CHAIN_UNLOCKED,
                       questId,
                       "New quest chain unlocked!");
    }
}

std::vector<std::string> QuestSystem::getQuestChain(const std::string& startQuestId) const {
    std::vector<std::string> chain;
    std::queue<std::string> toProcess;
    std::unordered_set<std::string> visited;

    toProcess.push(startQuestId);

    while (!toProcess.empty()) {
        std::string currentId = toProcess.front();
        toProcess.pop();

        if (visited.find(currentId) != visited.end()) {
            continue;
        }

        visited.insert(currentId);
        chain.push_back(currentId);

        auto quest = const_cast<QuestSystem*>(this)->getQuest(currentId);
        if (quest) {
            for (const auto& nextId : quest->getNextQuests()) {
                if (visited.find(nextId) == visited.end()) {
                    toProcess.push(nextId);
                }
            }
        }
    }

    return chain;
}

void QuestSystem::updateTimedQuests(float deltaTime) {
    checkExpiredQuests();
}

void QuestSystem::checkExpiredQuests() {
    std::vector<std::string> expiredQuests;

    for (const auto& questId : m_playerData.activeQuestIds) {
        auto quest = getQuest(questId);
        if (quest && quest->isExpired()) {
            expiredQuests.push_back(questId);
        }
    }

    for (const auto& questId : expiredQuests) {
        failQuest(questId);
    }
}

void QuestSystem::resetDailyQuests() {
    for (const auto& pair : m_quests) {
        if (pair.second->getType() == QuestType::DAILY_QUEST) {
            auto dailyQuest = std::dynamic_pointer_cast<DailyQuest>(pair.second);
            if (dailyQuest) {
                dailyQuest->resetDaily();
            }
        }
    }

    m_playerData.lastDailyReset = std::chrono::system_clock::now();
    updateAvailableQuests();
}

bool QuestSystem::isDailyResetNeeded() const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
        now - m_playerData.lastDailyReset);
    return elapsed.count() >= 24;
}

void QuestSystem::updateReputation(Faction faction, int amount) {
    m_playerData.reputations[faction] += amount;
    updateAvailableQuests();
}

int QuestSystem::getReputation(Faction faction) const {
    auto it = m_playerData.reputations.find(faction);
    return (it != m_playerData.reputations.end()) ? it->second : 0;
}

std::string QuestSystem::getReputationTitle(Faction faction) const {
    int rep = getReputation(faction);

    if (rep < -5000) return "Hated";
    if (rep < -2000) return "Hostile";
    if (rep < -500) return "Unfriendly";
    if (rep < 500) return "Neutral";
    if (rep < 2000) return "Friendly";
    if (rep < 5000) return "Honored";
    if (rep < 10000) return "Revered";
    return "Exalted";
}

void QuestSystem::setTrackedQuest(const std::string& questId) {
    m_trackedQuestId = questId;
}

std::shared_ptr<Quest> QuestSystem::getTrackedQuest() const {
    return m_trackedQuestId.empty() ? nullptr :
           const_cast<QuestSystem*>(this)->getQuest(m_trackedQuestId);
}

void QuestSystem::clearTrackedQuest() {
    m_trackedQuestId.clear();
}

void QuestSystem::addNotification(QuestNotificationType type,
                                  const std::string& questId,
                                  const std::string& message) {
    m_notifications.emplace_back(type, questId, message);
}

void QuestSystem::discoverHiddenQuest(const std::string& questId) {
    auto quest = getQuest(questId);
    if (!quest || !quest->isHidden()) {
        return;
    }

    m_discoveredHiddenQuests.insert(questId);

    if (checkQuestPrerequisites(quest)) {
        quest->setStatus(QuestStatus::AVAILABLE);
        m_playerData.availableQuestIds.push_back(questId);

        addNotification(QuestNotificationType::QUEST_AVAILABLE,
                       questId,
                       "Hidden quest discovered: " + quest->getName());
    }
}

std::vector<std::shared_ptr<Quest>> QuestSystem::getDiscoveredHiddenQuests() const {
    std::vector<std::shared_ptr<Quest>> quests;
    for (const auto& questId : m_discoveredHiddenQuests) {
        auto quest = const_cast<QuestSystem*>(this)->getQuest(questId);
        if (quest) {
            quests.push_back(quest);
        }
    }
    return quests;
}

int QuestSystem::getTotalQuestsCompleted() const {
    return static_cast<int>(m_playerData.completedQuestIds.size());
}

int QuestSystem::getQuestsCompletedByType(QuestType type) const {
    int count = 0;
    for (const auto& questId : m_playerData.completedQuestIds) {
        auto quest = const_cast<QuestSystem*>(this)->getQuest(questId);
        if (quest && quest->getType() == type) {
            count++;
        }
    }
    return count;
}

float QuestSystem::getQuestCompletionRate() const {
    if (m_quests.empty()) {
        return 0.0f;
    }
    return static_cast<float>(getTotalQuestsCompleted()) / m_quests.size();
}

std::map<std::string, int> QuestSystem::getQuestStatistics() const {
    std::map<std::string, int> stats;
    stats["total_quests"] = static_cast<int>(m_quests.size());
    stats["completed"] = getTotalQuestsCompleted();
    stats["active"] = static_cast<int>(m_playerData.activeQuestIds.size());
    stats["available"] = static_cast<int>(m_playerData.availableQuestIds.size());
    return stats;
}

void QuestSystem::grantReward(const QuestReward& reward) {
    // TODO: Implement reward granting (XP, gold, items, etc.)
    // This would integrate with the game's inventory and character systems

    // Update reputation
    for (const auto& pair : reward.reputationGains) {
        updateReputation(pair.first, pair.second);
    }

    addNotification(QuestNotificationType::REWARD_GAINED,
                   "",
                   "Rewards granted!");
}

void QuestSystem::unlockNextQuests(const std::vector<std::string>& questIds) {
    for (const auto& questId : questIds) {
        auto quest = getQuest(questId);
        if (quest && quest->getStatus() == QuestStatus::LOCKED) {
            if (checkQuestPrerequisites(quest)) {
                quest->setStatus(QuestStatus::AVAILABLE);
                m_playerData.availableQuestIds.push_back(questId);

                addNotification(QuestNotificationType::QUEST_AVAILABLE,
                              questId,
                              "New quest available: " + quest->getName());
            }
        }
    }
}

bool QuestSystem::checkQuestPrerequisites(std::shared_ptr<Quest> quest) const {
    if (!quest) {
        return false;
    }

    return quest->checkPrerequisites(m_playerData.level,
                                     m_playerData.reputations,
                                     m_playerData.completedQuestIds);
}

std::string QuestSystem::serializePlayerProgress() const {
    // TODO: Implement JSON serialization
    return "{}";
}

void QuestSystem::loadPlayerProgress(const std::string& json) {
    // TODO: Implement JSON deserialization
}

// QuestChainBuilder implementation
QuestChainBuilder& QuestChainBuilder::addQuest(std::shared_ptr<Quest> quest) {
    m_quests.push_back(quest);
    return *this;
}

QuestChainBuilder& QuestChainBuilder::addPrerequisite(const std::string& questId,
                                                       const std::string& prerequisiteId) {
    m_prerequisites[questId].push_back(prerequisiteId);
    return *this;
}

QuestChainBuilder& QuestChainBuilder::addBranch(const std::string& fromQuestId,
                                                 const std::string& toQuestId) {
    m_branches[fromQuestId].push_back(toQuestId);
    return *this;
}

void QuestChainBuilder::build() {
    // Apply prerequisites
    for (const auto& pair : m_prerequisites) {
        for (auto& quest : m_quests) {
            if (quest->getId() == pair.first) {
                for (const auto& prereqId : pair.second) {
                    quest->addPrerequisiteQuest(prereqId);
                }
            }
        }
    }

    // Apply branches
    for (const auto& pair : m_branches) {
        for (auto& quest : m_quests) {
            if (quest->getId() == pair.first) {
                for (const auto& nextId : pair.second) {
                    quest->addNextQuest(nextId);
                }
            }
        }
    }

    // Register quests
    for (auto& quest : m_quests) {
        QuestSystem::getInstance().registerQuest(quest);
    }
}

// QuestFilter implementation
QuestFilter& QuestFilter::byType(QuestType type) {
    m_filters.push_back([type](const std::shared_ptr<Quest>& q) {
        return q->getType() == type;
    });
    return *this;
}

QuestFilter& QuestFilter::byDifficulty(QuestDifficulty difficulty) {
    m_filters.push_back([difficulty](const std::shared_ptr<Quest>& q) {
        return q->getDifficulty() == difficulty;
    });
    return *this;
}

QuestFilter& QuestFilter::byMinLevel(int level) {
    m_filters.push_back([level](const std::shared_ptr<Quest>& q) {
        return q->getRecommendedLevel() >= level;
    });
    return *this;
}

QuestFilter& QuestFilter::byMaxLevel(int level) {
    m_filters.push_back([level](const std::shared_ptr<Quest>& q) {
        return q->getRecommendedLevel() <= level;
    });
    return *this;
}

QuestFilter& QuestFilter::byFaction(Faction faction) {
    m_filters.push_back([faction](const std::shared_ptr<Quest>& q) {
        return q->getRequiredFaction() == faction;
    });
    return *this;
}

QuestFilter& QuestFilter::byStatus(QuestStatus status) {
    m_filters.push_back([status](const std::shared_ptr<Quest>& q) {
        return q->getStatus() == status;
    });
    return *this;
}

QuestFilter& QuestFilter::onlyRepeatable() {
    m_filters.push_back([](const std::shared_ptr<Quest>& q) {
        return q->isRepeatable();
    });
    return *this;
}

QuestFilter& QuestFilter::onlyHidden() {
    m_filters.push_back([](const std::shared_ptr<Quest>& q) {
        return q->isHidden();
    });
    return *this;
}

std::vector<std::shared_ptr<Quest>> QuestFilter::apply(
    const std::vector<std::shared_ptr<Quest>>& quests) const {
    std::vector<std::shared_ptr<Quest>> result;

    for (const auto& quest : quests) {
        bool passesAllFilters = true;
        for (const auto& filter : m_filters) {
            if (!filter(quest)) {
                passesAllFilters = false;
                break;
            }
        }
        if (passesAllFilters) {
            result.push_back(quest);
        }
    }

    return result;
}

} // namespace QuestSystem
