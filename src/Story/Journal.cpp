#include "Journal.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>

namespace Story {

// Journal::Implementation
struct Journal::Implementation {
    std::unordered_map<std::string, Quest> quests;
    std::unordered_map<std::string, JournalEntry> journalEntries;
    std::unordered_map<std::string, LoreEntry> loreEntries;
    std::unordered_map<std::string, BestiaryEntry> bestiaryEntries;

    std::string trackedQuestId;

    QuestCallback questCallback;
    ObjectiveCallback objectiveCallback;

    bool debugMode = false;
};

Journal::Journal() : impl(std::make_unique<Implementation>()) {}

Journal::~Journal() = default;

bool Journal::initialize() {
    impl->quests.clear();
    impl->journalEntries.clear();
    impl->loreEntries.clear();
    impl->trackedQuestId.clear();
    return true;
}

void Journal::shutdown() {
    impl->quests.clear();
    impl->journalEntries.clear();
    impl->loreEntries.clear();
}

bool Journal::loadQuests(const std::string& filePath) {
    // Would load from JSON
    return true;
}

void Journal::registerQuest(const Quest& quest) {
    impl->quests[quest.id] = quest;
}

void Journal::unlockQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    if (it != impl->quests.end() && it->second.status == QuestStatus::LOCKED) {
        it->second.status = QuestStatus::AVAILABLE;
    }
}

void Journal::startQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    if (it == impl->quests.end()) return;

    Quest& quest = it->second;
    if (quest.status != QuestStatus::AVAILABLE && quest.status != QuestStatus::LOCKED) {
        return;
    }

    QuestStatus oldStatus = quest.status;
    quest.status = QuestStatus::ACTIVE;
    quest.currentStage = 0;
    quest.startTime = static_cast<float>(std::time(nullptr));

    if (quest.hasTotalTimeLimit) {
        quest.timeRemaining = quest.timeLimit;
    }

    if (impl->questCallback) {
        impl->questCallback(questId, oldStatus, QuestStatus::ACTIVE);
    }

    // Add journal entry
    std::string entry = "Started quest: " + quest.title + "\n" + quest.description;
    addJournalEntry(quest.title, entry, "Quests");
}

void Journal::completeQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    if (it == impl->quests.end()) return;

    Quest& quest = it->second;
    if (quest.status != QuestStatus::ACTIVE) return;

    QuestStatus oldStatus = quest.status;
    quest.status = QuestStatus::COMPLETED;
    quest.completionTime = static_cast<float>(std::time(nullptr));

    // Give rewards
    giveQuestRewards(quest);

    if (impl->questCallback) {
        impl->questCallback(questId, oldStatus, QuestStatus::COMPLETED);
    }

    // Add journal entry
    std::string entry = "Completed quest: " + quest.title;
    addJournalEntry(quest.title + " (Completed)", entry, "Quests");

    // Unlock new quests
    for (const auto& unlockedQuestId : quest.rewards.unlockedQuests) {
        unlockQuest(unlockedQuestId);
    }

    checkQuestUnlocks();
}

void Journal::failQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    if (it == impl->quests.end()) return;

    Quest& quest = it->second;
    QuestStatus oldStatus = quest.status;
    quest.status = QuestStatus::FAILED;

    if (impl->questCallback) {
        impl->questCallback(questId, oldStatus, QuestStatus::FAILED);
    }

    std::string entry = "Failed quest: " + quest.title;
    addJournalEntry(quest.title + " (Failed)", entry, "Quests");
}

void Journal::abandonQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    if (it == impl->quests.end()) return;

    Quest& quest = it->second;
    if (quest.status != QuestStatus::ACTIVE) return;

    QuestStatus oldStatus = quest.status;
    quest.status = QuestStatus::ABANDONED;

    if (impl->questCallback) {
        impl->questCallback(questId, oldStatus, QuestStatus::ABANDONED);
    }
}

Quest* Journal::getQuest(const std::string& questId) {
    auto it = impl->quests.find(questId);
    return it != impl->quests.end() ? &it->second : nullptr;
}

const Quest* Journal::getQuest(const std::string& questId) const {
    auto it = impl->quests.find(questId);
    return it != impl->quests.end() ? &it->second : nullptr;
}

std::vector<const Quest*> Journal::getActiveQuests() const {
    std::vector<const Quest*> result;
    for (const auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::ACTIVE) {
            result.push_back(&quest);
        }
    }
    return result;
}

std::vector<const Quest*> Journal::getCompletedQuests() const {
    std::vector<const Quest*> result;
    for (const auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::COMPLETED) {
            result.push_back(&quest);
        }
    }
    return result;
}

std::vector<const Quest*> Journal::getAvailableQuests() const {
    std::vector<const Quest*> result;
    for (const auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::AVAILABLE) {
            result.push_back(&quest);
        }
    }
    return result;
}

std::vector<const Quest*> Journal::getQuestsByType(QuestType type) const {
    std::vector<const Quest*> result;
    for (const auto& [id, quest] : impl->quests) {
        if (quest.type == type) {
            result.push_back(&quest);
        }
    }
    return result;
}

std::vector<const Quest*> Journal::getTrackedQuests() const {
    std::vector<const Quest*> result;
    for (const auto& [id, quest] : impl->quests) {
        if (quest.tracked) {
            result.push_back(&quest);
        }
    }
    return result;
}

void Journal::setTrackedQuest(const std::string& questId) {
    // Untrack current
    if (!impl->trackedQuestId.empty()) {
        auto* oldQuest = getQuest(impl->trackedQuestId);
        if (oldQuest) oldQuest->tracked = false;
    }

    // Track new
    auto* quest = getQuest(questId);
    if (quest) {
        quest->tracked = true;
        impl->trackedQuestId = questId;
    }
}

void Journal::toggleQuestTracking(const std::string& questId) {
    auto* quest = getQuest(questId);
    if (quest) {
        quest->tracked = !quest->tracked;
        if (quest->tracked) {
            impl->trackedQuestId = questId;
        } else if (impl->trackedQuestId == questId) {
            impl->trackedQuestId.clear();
        }
    }
}

const Quest* Journal::getTrackedQuest() const {
    return getQuest(impl->trackedQuestId);
}

void Journal::advanceQuestStage(const std::string& questId) {
    auto* quest = getQuest(questId);
    if (!quest || quest->status != QuestStatus::ACTIVE) return;

    const QuestStage* currentStage = getCurrentQuestStage(questId);
    if (currentStage && currentStage->isCompleted()) {
        quest->currentStage++;

        if (quest->currentStage >= static_cast<int>(quest->stages.size())) {
            completeQuest(questId);
        } else {
            const QuestStage& newStage = quest->stages[quest->currentStage];
            if (!newStage.journalEntry.empty()) {
                addJournalEntry(quest->title, newStage.journalEntry, "Quests");
            }
        }
    }
}

void Journal::setQuestStage(const std::string& questId, int stage) {
    auto* quest = getQuest(questId);
    if (quest && stage >= 0 && stage < static_cast<int>(quest->stages.size())) {
        quest->currentStage = stage;
    }
}

int Journal::getCurrentStage(const std::string& questId) const {
    const Quest* quest = getQuest(questId);
    return quest ? quest->currentStage : 0;
}

const QuestStage* Journal::getCurrentQuestStage(const std::string& questId) const {
    const Quest* quest = getQuest(questId);
    if (!quest || quest->currentStage >= static_cast<int>(quest->stages.size())) {
        return nullptr;
    }
    return &quest->stages[quest->currentStage];
}

void Journal::updateObjective(const std::string& questId, const std::string& objectiveId,
                             int progress) {
    auto* quest = getQuest(questId);
    if (!quest || quest->status != QuestStatus::ACTIVE) return;

    if (quest->currentStage >= static_cast<int>(quest->stages.size())) return;

    QuestStage& stage = quest->stages[quest->currentStage];
    for (auto& objective : stage.objectives) {
        if (objective.id == objectiveId) {
            objective.currentProgress = std::min(progress, objective.requiredProgress);

            if (objective.currentProgress >= objective.requiredProgress && !objective.completed) {
                completeObjective(questId, objectiveId);
            }
            break;
        }
    }

    // Check if stage is complete
    if (stage.isCompleted()) {
        advanceQuestStage(questId);
    }
}

void Journal::completeObjective(const std::string& questId, const std::string& objectiveId) {
    auto* quest = getQuest(questId);
    if (!quest || quest->status != QuestStatus::ACTIVE) return;

    if (quest->currentStage >= static_cast<int>(quest->stages.size())) return;

    QuestStage& stage = quest->stages[quest->currentStage];
    for (auto& objective : stage.objectives) {
        if (objective.id == objectiveId && !objective.completed) {
            objective.completed = true;
            objective.currentProgress = objective.requiredProgress;

            if (impl->objectiveCallback) {
                impl->objectiveCallback(questId, objectiveId);
            }

            std::string entry = "Completed objective: " + objective.description;
            addJournalEntry(quest->title, entry, "Quests");
            break;
        }
    }

    if (stage.isCompleted()) {
        advanceQuestStage(questId);
    }
}

void Journal::incrementObjective(const std::string& questId, const std::string& objectiveId,
                                int amount) {
    auto* quest = getQuest(questId);
    if (!quest || quest->status != QuestStatus::ACTIVE) return;

    if (quest->currentStage >= static_cast<int>(quest->stages.size())) return;

    QuestStage& stage = quest->stages[quest->currentStage];
    for (auto& objective : stage.objectives) {
        if (objective.id == objectiveId) {
            updateObjective(questId, objectiveId, objective.currentProgress + amount);
            break;
        }
    }
}

bool Journal::isObjectiveComplete(const std::string& questId,
                                 const std::string& objectiveId) const {
    const Quest* quest = getQuest(questId);
    if (!quest || quest->currentStage >= static_cast<int>(quest->stages.size())) {
        return false;
    }

    const QuestStage& stage = quest->stages[quest->currentStage];
    for (const auto& objective : stage.objectives) {
        if (objective.id == objectiveId) {
            return objective.completed;
        }
    }

    return false;
}

void Journal::addJournalEntry(const JournalEntry& entry) {
    JournalEntry entryCopy = entry;
    entryCopy.timestamp = static_cast<float>(std::time(nullptr));
    entryCopy.isNew = true;
    impl->journalEntries[entry.id] = entryCopy;
}

void Journal::addJournalEntry(const std::string& title, const std::string& content,
                             const std::string& category) {
    JournalEntry entry;
    entry.id = "entry_" + std::to_string(impl->journalEntries.size());
    entry.title = title;
    entry.content = content;
    entry.category = category;
    addJournalEntry(entry);
}

const JournalEntry* Journal::getJournalEntry(const std::string& entryId) const {
    auto it = impl->journalEntries.find(entryId);
    return it != impl->journalEntries.end() ? &it->second : nullptr;
}

std::vector<const JournalEntry*> Journal::getAllJournalEntries() const {
    std::vector<const JournalEntry*> result;
    for (const auto& [id, entry] : impl->journalEntries) {
        result.push_back(&entry);
    }

    // Sort by timestamp (newest first)
    std::sort(result.begin(), result.end(),
             [](const JournalEntry* a, const JournalEntry* b) {
                 return a->timestamp > b->timestamp;
             });

    return result;
}

std::vector<const JournalEntry*> Journal::getJournalEntriesByCategory(
    const std::string& category) const {
    std::vector<const JournalEntry*> result;
    for (const auto& [id, entry] : impl->journalEntries) {
        if (entry.category == category) {
            result.push_back(&entry);
        }
    }
    return result;
}

std::vector<const JournalEntry*> Journal::getNewJournalEntries() const {
    std::vector<const JournalEntry*> result;
    for (const auto& [id, entry] : impl->journalEntries) {
        if (entry.isNew) {
            result.push_back(&entry);
        }
    }
    return result;
}

void Journal::markEntryAsRead(const std::string& entryId) {
    auto it = impl->journalEntries.find(entryId);
    if (it != impl->journalEntries.end()) {
        it->second.isNew = false;
    }
}

void Journal::markAllEntriesAsRead() {
    for (auto& [id, entry] : impl->journalEntries) {
        entry.isNew = false;
    }
}

void Journal::registerLoreEntry(const LoreEntry& entry) {
    impl->loreEntries[entry.id] = entry;
}

void Journal::discoverLore(const std::string& loreId, const std::string& source) {
    auto it = impl->loreEntries.find(loreId);
    if (it != impl->loreEntries.end() && !it->second.discovered) {
        it->second.discovered = true;
        it->second.discoveredFrom = source;
        it->second.discoveryTime = static_cast<float>(std::time(nullptr));

        std::string entry = "Discovered lore: " + it->second.title + "\n" + it->second.content;
        addJournalEntry(it->second.title, entry, "Lore");
    }
}

bool Journal::isLoreDiscovered(const std::string& loreId) const {
    auto it = impl->loreEntries.find(loreId);
    return it != impl->loreEntries.end() && it->second.discovered;
}

const LoreEntry* Journal::getLoreEntry(const std::string& loreId) const {
    auto it = impl->loreEntries.find(loreId);
    return it != impl->loreEntries.end() ? &it->second : nullptr;
}

std::vector<const LoreEntry*> Journal::getDiscoveredLore() const {
    std::vector<const LoreEntry*> result;
    for (const auto& [id, entry] : impl->loreEntries) {
        if (entry.discovered) {
            result.push_back(&entry);
        }
    }
    return result;
}

std::vector<const LoreEntry*> Journal::getLoreByCategory(const std::string& category) const {
    std::vector<const LoreEntry*> result;
    for (const auto& [id, entry] : impl->loreEntries) {
        if (entry.category == category && entry.discovered) {
            result.push_back(&entry);
        }
    }
    return result;
}

bool Journal::isQuestAvailable(const std::string& questId) const {
    const Quest* quest = getQuest(questId);
    if (!quest) return false;

    // Would check requirements
    return quest->status == QuestStatus::AVAILABLE;
}

std::vector<std::string> Journal::getQuestRequirements(const std::string& questId) const {
    std::vector<std::string> requirements;
    const Quest* quest = getQuest(questId);
    if (!quest) return requirements;

    if (quest->minimumLevel > 0) {
        requirements.push_back("Level " + std::to_string(quest->minimumLevel));
    }

    for (const auto& reqQuest : quest->requiredQuests) {
        requirements.push_back("Complete: " + reqQuest);
    }

    return requirements;
}

void Journal::onQuestStatusChanged(QuestCallback callback) {
    impl->questCallback = callback;
}

void Journal::onObjectiveCompleted(ObjectiveCallback callback) {
    impl->objectiveCallback = callback;
}

Journal::JournalStats Journal::getStatistics() const {
    JournalStats stats;
    stats.totalQuests = static_cast<int>(impl->quests.size());
    stats.activeQuests = static_cast<int>(getActiveQuests().size());
    stats.completedQuests = static_cast<int>(getCompletedQuests().size());
    stats.failedQuests = 0;
    stats.mainQuestsCompleted = 0;
    stats.sideQuestsCompleted = 0;
    stats.loreDiscovered = static_cast<int>(getDiscoveredLore().size());
    stats.journalEntries = static_cast<int>(impl->journalEntries.size());
    stats.totalQuestTime = 0.0f;

    for (const auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::FAILED) {
            stats.failedQuests++;
        }
        if (quest.status == QuestStatus::COMPLETED) {
            if (quest.type == QuestType::MAIN_STORY) {
                stats.mainQuestsCompleted++;
            } else if (quest.type == QuestType::SIDE_QUEST) {
                stats.sideQuestsCompleted++;
            }

            stats.totalQuestTime += (quest.completionTime - quest.startTime);
        }
    }

    return stats;
}

std::vector<std::string> Journal::getQuestChain(const std::string& questId) const {
    std::vector<std::string> chain;
    // Would trace quest dependencies
    chain.push_back(questId);
    return chain;
}

std::string Journal::getNextQuestInChain(const std::string& questId) const {
    // Would find next quest
    return "";
}

void Journal::registerBestiaryEntry(const std::string& enemyId, const std::string& name,
                                   const std::string& description) {
    BestiaryEntry entry;
    entry.id = enemyId;
    entry.name = name;
    entry.description = description;
    entry.discovered = false;
    entry.killCount = 0;
    impl->bestiaryEntries[enemyId] = entry;
}

void Journal::discoverEnemy(const std::string& enemyId) {
    auto it = impl->bestiaryEntries.find(enemyId);
    if (it != impl->bestiaryEntries.end() && !it->second.discovered) {
        it->second.discovered = true;

        std::string entry = "Discovered enemy: " + it->second.name + "\n" + it->second.description;
        addJournalEntry(it->second.name, entry, "Bestiary");
    }
}

void Journal::updateEnemyKillCount(const std::string& enemyId, int count) {
    auto it = impl->bestiaryEntries.find(enemyId);
    if (it != impl->bestiaryEntries.end()) {
        it->second.killCount += count;
        if (!it->second.discovered) {
            discoverEnemy(enemyId);
        }
    }
}

const Journal::BestiaryEntry* Journal::getBestiaryEntry(const std::string& enemyId) const {
    auto it = impl->bestiaryEntries.find(enemyId);
    return it != impl->bestiaryEntries.end() ? &it->second : nullptr;
}

std::vector<const Journal::BestiaryEntry*> Journal::getDiscoveredEnemies() const {
    std::vector<const BestiaryEntry*> result;
    for (const auto& [id, entry] : impl->bestiaryEntries) {
        if (entry.discovered) {
            result.push_back(&entry);
        }
    }
    return result;
}

std::vector<const Quest*> Journal::searchQuests(const std::string& query) const {
    std::vector<const Quest*> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& [id, quest] : impl->quests) {
        std::string lowerTitle = quest.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);

        if (lowerTitle.find(lowerQuery) != std::string::npos) {
            result.push_back(&quest);
        }
    }

    return result;
}

std::vector<const JournalEntry*> Journal::searchJournal(const std::string& query) const {
    std::vector<const JournalEntry*> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& [id, entry] : impl->journalEntries) {
        std::string lowerContent = entry.content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

        if (lowerContent.find(lowerQuery) != std::string::npos) {
            result.push_back(&entry);
        }
    }

    return result;
}

std::vector<const LoreEntry*> Journal::searchLore(const std::string& query) const {
    std::vector<const LoreEntry*> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& [id, entry] : impl->loreEntries) {
        if (!entry.discovered) continue;

        std::string lowerContent = entry.content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

        if (lowerContent.find(lowerQuery) != std::string::npos) {
            result.push_back(&entry);
        }
    }

    return result;
}

std::string Journal::serializeState() const {
    // Would serialize to JSON
    return "{}";
}

bool Journal::deserializeState(const std::string& data) {
    // Would deserialize from JSON
    return true;
}

void Journal::unlockAllQuests() {
    for (auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::LOCKED) {
            quest.status = QuestStatus::AVAILABLE;
        }
    }
}

void Journal::completeAllActiveQuests() {
    for (auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::ACTIVE) {
            completeQuest(id);
        }
    }
}

void Journal::dumpQuestState(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return;

    file << "=== Quest Journal State ===\n\n";
    file << "Active Quests: " << getActiveQuests().size() << "\n";
    file << "Completed Quests: " << getCompletedQuests().size() << "\n";
    file << "Available Quests: " << getAvailableQuests().size() << "\n\n";

    for (const auto& [id, quest] : impl->quests) {
        file << quest.title << " [" << static_cast<int>(quest.status) << "]\n";
    }
}

void Journal::giveQuestRewards(const Quest& quest) {
    // Would give actual rewards
    // This is a placeholder
}

void Journal::checkQuestUnlocks() {
    // Check if any locked quests can be unlocked
    for (auto& [id, quest] : impl->quests) {
        if (quest.status != QuestStatus::LOCKED) continue;

        bool canUnlock = true;

        // Check required quests
        for (const auto& reqQuestId : quest.requiredQuests) {
            const Quest* reqQuest = getQuest(reqQuestId);
            if (!reqQuest || reqQuest->status != QuestStatus::COMPLETED) {
                canUnlock = false;
                break;
            }
        }

        if (canUnlock) {
            unlockQuest(id);
        }
    }
}

void Journal::updateQuestTimers(float deltaTime) {
    for (auto& [id, quest] : impl->quests) {
        if (quest.status == QuestStatus::ACTIVE && quest.hasTotalTimeLimit) {
            quest.timeRemaining -= deltaTime;
            if (quest.timeRemaining <= 0) {
                failQuest(id);
            }
        }
    }
}

} // namespace Story
