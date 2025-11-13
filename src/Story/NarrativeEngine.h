#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Story {

// Forward declarations
class DialogueSystem;
class RelationshipSystem;
class Journal;

// Story state
enum class StoryState {
    NOT_STARTED,
    PROLOGUE,
    ACT_ONE,
    ACT_TWO,
    ACT_THREE,
    CLIMAX,
    EPILOGUE,
    COMPLETED
};

// Chapter information
struct Chapter {
    std::string id;
    std::string title;
    std::string description;
    int chapterNumber;

    // Requirements
    std::vector<std::string> requiredQuests;
    std::vector<std::string> requiredFlags;
    int minimumLevel = 1;

    // Content
    std::string introDialogueId;
    std::string outroDialogueId;
    std::vector<std::string> questIds;
    std::vector<std::string> availableDialogues;

    // State
    bool unlocked = false;
    bool completed = false;
    float completionPercentage = 0.0f;

    // Metadata
    std::string location;
    float estimatedPlayTime = 0.0f; // In hours
};

// Story flag (for tracking player choices and world state)
struct StoryFlag {
    std::string id;
    std::string description;
    bool value;
    float timestamp;
    std::string source; // Which quest/dialogue set this

    StoryFlag() : value(false), timestamp(0.0f) {}
};

// Major story decision
struct StoryDecision {
    std::string id;
    std::string description;
    std::string choiceText;
    std::vector<std::string> consequenceFlags;
    std::vector<std::string> affectedQuests;
    std::vector<std::pair<std::string, int>> relationshipChanges;
    float timestamp;

    // Impact tracking
    enum class Impact {
        MINOR,
        MODERATE,
        MAJOR,
        ENDING_DETERMINANT
    };
    Impact impact;

    StoryDecision() : timestamp(0.0f), impact(Impact::MINOR) {}
};

// Ending type
struct Ending {
    std::string id;
    std::string title;
    std::string description;
    std::vector<std::string> requiredFlags;
    std::vector<std::pair<std::string, int>> requiredRelationships; // NPC, min relationship
    std::vector<std::string> requiredDecisions;
    int moralityMin = -100;
    int moralityMax = 100;

    bool isAvailable(const class NarrativeEngine& engine) const;

    // Content
    std::string cutsceneId;
    std::string epilogueDialogueId;
    std::string achievementId;
};

// World state changes
struct WorldStateChange {
    std::string id;
    std::string description;
    float timestamp;

    // Changes
    std::vector<std::string> flagsSet;
    std::vector<std::string> flagsCleared;
    std::vector<std::string> npcsSpawned;
    std::vector<std::string> npcsRemoved;
    std::vector<std::string> locationsUnlocked;
    std::unordered_map<std::string, std::string> objectStates;
};

// Main narrative engine
class NarrativeEngine {
public:
    NarrativeEngine();
    ~NarrativeEngine();

    // Initialization
    bool initialize();
    void shutdown();

    // Chapter management
    bool loadChapters(const std::string& filePath);
    void unlockChapter(const std::string& chapterId);
    void completeChapter(const std::string& chapterId);
    const Chapter* getCurrentChapter() const;
    const Chapter* getChapter(const std::string& chapterId) const;
    std::vector<const Chapter*> getUnlockedChapters() const;
    float getStoryProgress() const; // 0.0 to 1.0

    // Story state
    void setStoryState(StoryState state);
    StoryState getStoryState() const;
    void advanceAct();

    // Flags
    void setFlag(const std::string& flagId, bool value, const std::string& source = "");
    bool getFlag(const std::string& flagId) const;
    bool hasFlag(const std::string& flagId) const;
    void clearFlag(const std::string& flagId);
    std::vector<StoryFlag> getAllFlags() const;

    // Decisions
    void recordDecision(const StoryDecision& decision);
    const StoryDecision* getDecision(const std::string& decisionId) const;
    std::vector<const StoryDecision*> getAllDecisions() const;
    std::vector<const StoryDecision*> getMajorDecisions() const;

    // World state
    void applyWorldStateChange(const WorldStateChange& change);
    WorldStateChange createWorldStateChange(const std::string& id, const std::string& description);
    std::vector<WorldStateChange> getWorldStateHistory() const;

    // Morality/alignment system
    void adjustMorality(int amount);
    int getMorality() const; // -100 (evil) to 100 (good)
    std::string getMoralityAlignment() const; // "Evil", "Neutral", "Good"

    // Reputation system integration
    void adjustReputation(const std::string& factionId, int amount);
    int getReputation(const std::string& factionId) const;
    std::string getReputationLevel(const std::string& factionId) const;

    // Endings
    void registerEnding(const Ending& ending);
    std::vector<const Ending*> getAvailableEndings() const;
    const Ending* determineEnding() const;
    void triggerEnding(const std::string& endingId);

    // Integration
    void setDialogueSystem(DialogueSystem* system);
    void setRelationshipSystem(RelationshipSystem* system);
    void setJournal(Journal* journal);

    // Event system
    using EventCallback = std::function<void(const std::string&, const std::string&)>;
    void registerEventHandler(const std::string& eventType, EventCallback callback);
    void triggerEvent(const std::string& eventType, const std::string& data);

    // Conditionals for dialogue
    bool evaluateCondition(const std::string& condition) const;

    // Variable system (shared with dialogue)
    void setVariable(const std::string& name, int value);
    void setVariable(const std::string& name, const std::string& value);
    void setVariable(const std::string& name, bool value);
    int getIntVariable(const std::string& name, int defaultValue = 0) const;
    std::string getStringVariable(const std::string& name, const std::string& defaultValue = "") const;
    bool getBoolVariable(const std::string& name, bool defaultValue = false) const;

    // Player data
    void setPlayerName(const std::string& name);
    std::string getPlayerName() const;
    void setPlayerClass(const std::string& className);
    std::string getPlayerClass() const;
    void setPlayerLevel(int level);
    int getPlayerLevel() const;

    // Achievements/milestones
    void unlockAchievement(const std::string& achievementId);
    bool hasAchievement(const std::string& achievementId) const;

    // Save/load
    std::string serializeState() const;
    bool deserializeState(const std::string& data);
    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);

    // Statistics
    struct NarrativeStats {
        float totalPlayTime;
        int dialoguesCompleted;
        int choicesMade;
        int majorDecisions;
        int chaptersCompleted;
        float storyProgress;
        std::string currentAct;
        int morality;
        std::unordered_map<std::string, int> factionReputations;
    };
    NarrativeStats getStatistics() const;

    // Debug
    void setDebugMode(bool enable);
    void unlockAllChapters();
    void dumpState(const std::string& filePath) const;

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal
    void checkChapterUnlocks();
    void updateStoryProgress();
    void processConsequences(const StoryDecision& decision);
};

} // namespace Story
