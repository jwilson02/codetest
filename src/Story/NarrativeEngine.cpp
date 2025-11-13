#include "NarrativeEngine.h"
#include "DialogueSystem.h"
#include "RelationshipSystem.h"
#include "Journal.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>

namespace Story {

// Ending implementation
bool Ending::isAvailable(const NarrativeEngine& engine) const {
    // Check morality range
    int morality = engine.getMorality();
    if (morality < moralityMin || morality > moralityMax) {
        return false;
    }

    // Check flags
    for (const auto& flag : requiredFlags) {
        if (!engine.getFlag(flag)) {
            return false;
        }
    }

    return true;
}

// NarrativeEngine::Implementation
struct NarrativeEngine::Implementation {
    // State
    StoryState currentState = StoryState::NOT_STARTED;
    std::unordered_map<std::string, Chapter> chapters;
    std::unordered_map<std::string, StoryFlag> flags;
    std::unordered_map<std::string, StoryDecision> decisions;
    std::vector<WorldStateChange> worldHistory;
    std::vector<Ending> endings;

    // Morality and reputation
    int morality = 0; // -100 to 100
    std::unordered_map<std::string, int> factionReputations;

    // Variables
    std::unordered_map<std::string, int> intVars;
    std::unordered_map<std::string, std::string> stringVars;
    std::unordered_map<std::string, bool> boolVars;

    // Player data
    std::string playerName = "Adventurer";
    std::string playerClass = "Warrior";
    int playerLevel = 1;
    std::unordered_map<std::string, bool> achievements;

    // Integration
    DialogueSystem* dialogueSystem = nullptr;
    RelationshipSystem* relationshipSystem = nullptr;
    Journal* journal = nullptr;

    // Event handlers
    std::unordered_map<std::string, std::vector<EventCallback>> eventHandlers;

    // Current chapter
    std::string currentChapterId;

    // Debug
    bool debugMode = false;
};

NarrativeEngine::NarrativeEngine() : impl(std::make_unique<Implementation>()) {}

NarrativeEngine::~NarrativeEngine() = default;

bool NarrativeEngine::initialize() {
    impl->currentState = StoryState::NOT_STARTED;
    impl->flags.clear();
    impl->decisions.clear();
    impl->worldHistory.clear();
    impl->morality = 0;
    impl->factionReputations.clear();
    return true;
}

void NarrativeEngine::shutdown() {
    impl->chapters.clear();
    impl->flags.clear();
    impl->decisions.clear();
    impl->worldHistory.clear();
}

bool NarrativeEngine::loadChapters(const std::string& filePath) {
    // Would load from JSON file
    return true;
}

void NarrativeEngine::unlockChapter(const std::string& chapterId) {
    auto it = impl->chapters.find(chapterId);
    if (it != impl->chapters.end()) {
        it->second.unlocked = true;
        triggerEvent("chapter_unlocked", chapterId);
    }
}

void NarrativeEngine::completeChapter(const std::string& chapterId) {
    auto it = impl->chapters.find(chapterId);
    if (it != impl->chapters.end()) {
        it->second.completed = true;
        it->second.completionPercentage = 100.0f;
        updateStoryProgress();
        triggerEvent("chapter_completed", chapterId);
        checkChapterUnlocks();
    }
}

const Chapter* NarrativeEngine::getCurrentChapter() const {
    if (impl->currentChapterId.empty()) return nullptr;
    auto it = impl->chapters.find(impl->currentChapterId);
    return it != impl->chapters.end() ? &it->second : nullptr;
}

const Chapter* NarrativeEngine::getChapter(const std::string& chapterId) const {
    auto it = impl->chapters.find(chapterId);
    return it != impl->chapters.end() ? &it->second : nullptr;
}

std::vector<const Chapter*> NarrativeEngine::getUnlockedChapters() const {
    std::vector<const Chapter*> unlocked;
    for (const auto& [id, chapter] : impl->chapters) {
        if (chapter.unlocked) {
            unlocked.push_back(&chapter);
        }
    }
    return unlocked;
}

float NarrativeEngine::getStoryProgress() const {
    if (impl->chapters.empty()) return 0.0f;

    int completedChapters = 0;
    for (const auto& [id, chapter] : impl->chapters) {
        if (chapter.completed) completedChapters++;
    }

    return static_cast<float>(completedChapters) / impl->chapters.size();
}

void NarrativeEngine::setStoryState(StoryState state) {
    StoryState oldState = impl->currentState;
    impl->currentState = state;

    if (oldState != state) {
        triggerEvent("story_state_changed", std::to_string(static_cast<int>(state)));
    }
}

StoryState NarrativeEngine::getStoryState() const {
    return impl->currentState;
}

void NarrativeEngine::advanceAct() {
    switch (impl->currentState) {
        case StoryState::NOT_STARTED:
            setStoryState(StoryState::PROLOGUE);
            break;
        case StoryState::PROLOGUE:
            setStoryState(StoryState::ACT_ONE);
            break;
        case StoryState::ACT_ONE:
            setStoryState(StoryState::ACT_TWO);
            break;
        case StoryState::ACT_TWO:
            setStoryState(StoryState::ACT_THREE);
            break;
        case StoryState::ACT_THREE:
            setStoryState(StoryState::CLIMAX);
            break;
        case StoryState::CLIMAX:
            setStoryState(StoryState::EPILOGUE);
            break;
        case StoryState::EPILOGUE:
            setStoryState(StoryState::COMPLETED);
            break;
        default:
            break;
    }
}

void NarrativeEngine::setFlag(const std::string& flagId, bool value, const std::string& source) {
    StoryFlag& flag = impl->flags[flagId];
    flag.id = flagId;
    flag.value = value;
    flag.timestamp = static_cast<float>(std::time(nullptr));
    flag.source = source;

    triggerEvent("flag_changed", flagId);
}

bool NarrativeEngine::getFlag(const std::string& flagId) const {
    auto it = impl->flags.find(flagId);
    return it != impl->flags.end() && it->second.value;
}

bool NarrativeEngine::hasFlag(const std::string& flagId) const {
    return impl->flags.find(flagId) != impl->flags.end();
}

void NarrativeEngine::clearFlag(const std::string& flagId) {
    setFlag(flagId, false);
}

std::vector<StoryFlag> NarrativeEngine::getAllFlags() const {
    std::vector<StoryFlag> result;
    for (const auto& [id, flag] : impl->flags) {
        result.push_back(flag);
    }
    return result;
}

void NarrativeEngine::recordDecision(const StoryDecision& decision) {
    StoryDecision decisionCopy = decision;
    decisionCopy.timestamp = static_cast<float>(std::time(nullptr));
    impl->decisions[decision.id] = decisionCopy;

    processConsequences(decisionCopy);
    triggerEvent("decision_made", decision.id);
}

const StoryDecision* NarrativeEngine::getDecision(const std::string& decisionId) const {
    auto it = impl->decisions.find(decisionId);
    return it != impl->decisions.end() ? &it->second : nullptr;
}

std::vector<const StoryDecision*> NarrativeEngine::getAllDecisions() const {
    std::vector<const StoryDecision*> result;
    for (const auto& [id, decision] : impl->decisions) {
        result.push_back(&decision);
    }
    return result;
}

std::vector<const StoryDecision*> NarrativeEngine::getMajorDecisions() const {
    std::vector<const StoryDecision*> result;
    for (const auto& [id, decision] : impl->decisions) {
        if (decision.impact >= StoryDecision::Impact::MAJOR) {
            result.push_back(&decision);
        }
    }
    return result;
}

void NarrativeEngine::applyWorldStateChange(const WorldStateChange& change) {
    WorldStateChange changeCopy = change;
    changeCopy.timestamp = static_cast<float>(std::time(nullptr));

    // Apply flag changes
    for (const auto& flag : change.flagsSet) {
        setFlag(flag, true, "world_state_change");
    }
    for (const auto& flag : change.flagsCleared) {
        setFlag(flag, false, "world_state_change");
    }

    impl->worldHistory.push_back(changeCopy);
    triggerEvent("world_state_changed", change.id);
}

WorldStateChange NarrativeEngine::createWorldStateChange(const std::string& id,
                                                         const std::string& description) {
    WorldStateChange change;
    change.id = id;
    change.description = description;
    return change;
}

std::vector<WorldStateChange> NarrativeEngine::getWorldStateHistory() const {
    return impl->worldHistory;
}

void NarrativeEngine::adjustMorality(int amount) {
    int oldMorality = impl->morality;
    impl->morality = std::clamp(impl->morality + amount, -100, 100);

    if (oldMorality != impl->morality) {
        triggerEvent("morality_changed", std::to_string(impl->morality));
    }
}

int NarrativeEngine::getMorality() const {
    return impl->morality;
}

std::string NarrativeEngine::getMoralityAlignment() const {
    if (impl->morality < -30) return "Evil";
    if (impl->morality > 30) return "Good";
    return "Neutral";
}

void NarrativeEngine::adjustReputation(const std::string& factionId, int amount) {
    impl->factionReputations[factionId] += amount;
    triggerEvent("reputation_changed", factionId);
}

int NarrativeEngine::getReputation(const std::string& factionId) const {
    auto it = impl->factionReputations.find(factionId);
    return it != impl->factionReputations.end() ? it->second : 0;
}

std::string NarrativeEngine::getReputationLevel(const std::string& factionId) const {
    int rep = getReputation(factionId);
    if (rep < -80) return "Hated";
    if (rep < -40) return "Hostile";
    if (rep < 0) return "Unfriendly";
    if (rep < 40) return "Neutral";
    if (rep < 80) return "Friendly";
    return "Exalted";
}

void NarrativeEngine::registerEnding(const Ending& ending) {
    impl->endings.push_back(ending);
}

std::vector<const Ending*> NarrativeEngine::getAvailableEndings() const {
    std::vector<const Ending*> available;
    for (const auto& ending : impl->endings) {
        if (ending.isAvailable(*this)) {
            available.push_back(&ending);
        }
    }
    return available;
}

const Ending* NarrativeEngine::determineEnding() const {
    auto available = getAvailableEndings();
    if (available.empty()) return nullptr;

    // Return first available ending (could implement priority system)
    return available[0];
}

void NarrativeEngine::triggerEnding(const std::string& endingId) {
    for (const auto& ending : impl->endings) {
        if (ending.id == endingId) {
            setStoryState(StoryState::EPILOGUE);
            triggerEvent("ending_triggered", endingId);
            if (!ending.achievementId.empty()) {
                unlockAchievement(ending.achievementId);
            }
            break;
        }
    }
}

void NarrativeEngine::setDialogueSystem(DialogueSystem* system) {
    impl->dialogueSystem = system;
}

void NarrativeEngine::setRelationshipSystem(RelationshipSystem* system) {
    impl->relationshipSystem = system;
}

void NarrativeEngine::setJournal(Journal* journal) {
    impl->journal = journal;
}

void NarrativeEngine::registerEventHandler(const std::string& eventType, EventCallback callback) {
    impl->eventHandlers[eventType].push_back(callback);
}

void NarrativeEngine::triggerEvent(const std::string& eventType, const std::string& data) {
    auto it = impl->eventHandlers.find(eventType);
    if (it != impl->eventHandlers.end()) {
        for (const auto& callback : it->second) {
            callback(eventType, data);
        }
    }
}

bool NarrativeEngine::evaluateCondition(const std::string& condition) const {
    // Simple condition parser
    // In real implementation would use scripting language

    // Handle flag checks: "flag:flag_name"
    if (condition.substr(0, 5) == "flag:") {
        return getFlag(condition.substr(5));
    }

    // Handle morality checks: "morality>50"
    if (condition.find("morality") == 0) {
        // Parse morality condition
        return true; // Simplified
    }

    return true;
}

void NarrativeEngine::setVariable(const std::string& name, int value) {
    impl->intVars[name] = value;
}

void NarrativeEngine::setVariable(const std::string& name, const std::string& value) {
    impl->stringVars[name] = value;
}

void NarrativeEngine::setVariable(const std::string& name, bool value) {
    impl->boolVars[name] = value;
}

int NarrativeEngine::getIntVariable(const std::string& name, int defaultValue) const {
    auto it = impl->intVars.find(name);
    return it != impl->intVars.end() ? it->second : defaultValue;
}

std::string NarrativeEngine::getStringVariable(const std::string& name,
                                               const std::string& defaultValue) const {
    auto it = impl->stringVars.find(name);
    return it != impl->stringVars.end() ? it->second : defaultValue;
}

bool NarrativeEngine::getBoolVariable(const std::string& name, bool defaultValue) const {
    auto it = impl->boolVars.find(name);
    return it != impl->boolVars.end() ? it->second : defaultValue;
}

void NarrativeEngine::setPlayerName(const std::string& name) {
    impl->playerName = name;
}

std::string NarrativeEngine::getPlayerName() const {
    return impl->playerName;
}

void NarrativeEngine::setPlayerClass(const std::string& className) {
    impl->playerClass = className;
}

std::string NarrativeEngine::getPlayerClass() const {
    return impl->playerClass;
}

void NarrativeEngine::setPlayerLevel(int level) {
    impl->playerLevel = level;
}

int NarrativeEngine::getPlayerLevel() const {
    return impl->playerLevel;
}

void NarrativeEngine::unlockAchievement(const std::string& achievementId) {
    impl->achievements[achievementId] = true;
    triggerEvent("achievement_unlocked", achievementId);
}

bool NarrativeEngine::hasAchievement(const std::string& achievementId) const {
    auto it = impl->achievements.find(achievementId);
    return it != impl->achievements.end() && it->second;
}

std::string NarrativeEngine::serializeState() const {
    // Would serialize to JSON
    return "{}";
}

bool NarrativeEngine::deserializeState(const std::string& data) {
    // Would deserialize from JSON
    return true;
}

bool NarrativeEngine::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    file << serializeState();
    return true;
}

bool NarrativeEngine::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return deserializeState(buffer.str());
}

NarrativeEngine::NarrativeStats NarrativeEngine::getStatistics() const {
    NarrativeStats stats;
    stats.totalPlayTime = 0.0f; // Would track actual playtime
    stats.dialoguesCompleted = 0;
    stats.choicesMade = static_cast<int>(impl->decisions.size());
    stats.majorDecisions = static_cast<int>(getMajorDecisions().size());
    stats.chaptersCompleted = 0;
    stats.storyProgress = getStoryProgress();
    stats.morality = impl->morality;
    stats.factionReputations = impl->factionReputations;

    // Count completed chapters
    for (const auto& [id, chapter] : impl->chapters) {
        if (chapter.completed) stats.chaptersCompleted++;
    }

    // Get current act
    switch (impl->currentState) {
        case StoryState::PROLOGUE: stats.currentAct = "Prologue"; break;
        case StoryState::ACT_ONE: stats.currentAct = "Act I"; break;
        case StoryState::ACT_TWO: stats.currentAct = "Act II"; break;
        case StoryState::ACT_THREE: stats.currentAct = "Act III"; break;
        case StoryState::CLIMAX: stats.currentAct = "Climax"; break;
        case StoryState::EPILOGUE: stats.currentAct = "Epilogue"; break;
        default: stats.currentAct = "Not Started"; break;
    }

    return stats;
}

void NarrativeEngine::setDebugMode(bool enable) {
    impl->debugMode = enable;
}

void NarrativeEngine::unlockAllChapters() {
    for (auto& [id, chapter] : impl->chapters) {
        chapter.unlocked = true;
    }
}

void NarrativeEngine::dumpState(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return;

    file << "=== Narrative Engine State ===\n\n";
    file << "Story State: " << static_cast<int>(impl->currentState) << "\n";
    file << "Morality: " << impl->morality << "\n";
    file << "Chapters: " << impl->chapters.size() << "\n";
    file << "Flags: " << impl->flags.size() << "\n";
    file << "Decisions: " << impl->decisions.size() << "\n";
}

void NarrativeEngine::checkChapterUnlocks() {
    for (auto& [id, chapter] : impl->chapters) {
        if (chapter.unlocked) continue;

        bool canUnlock = true;

        // Check required quests
        if (impl->journal) {
            for (const auto& questId : chapter.requiredQuests) {
                const auto* quest = impl->journal->getQuest(questId);
                if (!quest || quest->status != QuestStatus::COMPLETED) {
                    canUnlock = false;
                    break;
                }
            }
        }

        // Check required flags
        for (const auto& flag : chapter.requiredFlags) {
            if (!getFlag(flag)) {
                canUnlock = false;
                break;
            }
        }

        // Check level
        if (impl->playerLevel < chapter.minimumLevel) {
            canUnlock = false;
        }

        if (canUnlock) {
            unlockChapter(id);
        }
    }
}

void NarrativeEngine::updateStoryProgress() {
    // Calculate overall progress
    float progress = getStoryProgress();

    // Update story state based on progress
    if (progress >= 0.9f && impl->currentState < StoryState::CLIMAX) {
        setStoryState(StoryState::CLIMAX);
    } else if (progress >= 0.6f && impl->currentState < StoryState::ACT_THREE) {
        setStoryState(StoryState::ACT_THREE);
    } else if (progress >= 0.3f && impl->currentState < StoryState::ACT_TWO) {
        setStoryState(StoryState::ACT_TWO);
    } else if (progress > 0.0f && impl->currentState == StoryState::NOT_STARTED) {
        setStoryState(StoryState::ACT_ONE);
    }
}

void NarrativeEngine::processConsequences(const StoryDecision& decision) {
    // Set consequence flags
    for (const auto& flag : decision.consequenceFlags) {
        setFlag(flag, true, "decision:" + decision.id);
    }

    // Apply relationship changes
    if (impl->relationshipSystem) {
        for (const auto& [npcId, amount] : decision.relationshipChanges) {
            impl->relationshipSystem->modifyRelationship(npcId, amount,
                                                        "decision:" + decision.id);
        }
    }

    // Affect quests
    if (impl->journal) {
        for (const auto& questId : decision.affectedQuests) {
            // Would update quest states
        }
    }
}

} // namespace Story
