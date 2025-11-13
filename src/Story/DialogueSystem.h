#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <queue>

namespace Story {

// Forward declarations
class DialogueTree;
class NarrativeEngine;
class RelationshipSystem;

// Dialogue node types
enum class DialogueNodeType {
    TEXT,           // Regular dialogue text
    CHOICE,         // Player choice
    CONDITION,      // Conditional branch
    ACTION,         // Trigger action/event
    BRANCH,         // Narrative branch point
    END             // End of dialogue
};

// Speaker information
struct Speaker {
    std::string id;
    std::string name;
    std::string portraitPath;
    std::string voiceId;        // For TTS
    float pitchModifier;
    float speedModifier;

    Speaker() : pitchModifier(1.0f), speedModifier(1.0f) {}
};

// Rich text formatting
struct TextStyle {
    bool bold = false;
    bool italic = false;
    bool underline = false;
    std::string color;          // Hex color code
    int fontSize = 16;
    std::string fontFamily;

    std::string toMarkup() const;
};

// Dialogue choice
struct DialogueChoice {
    std::string id;
    std::string text;
    std::string nextNodeId;

    // Requirements
    std::vector<std::string> requiredItems;
    std::vector<std::pair<std::string, int>> requiredStats; // stat name, minimum value
    std::vector<std::pair<std::string, int>> requiredRelationships;
    std::string requiredQuest;
    int requiredQuestStage = -1;

    // Effects
    std::vector<std::string> giveItems;
    std::vector<std::pair<std::string, int>> modifyRelationships;
    std::vector<std::string> startQuests;
    std::vector<std::string> completeQuests;
    int experienceReward = 0;
    int goldReward = 0;

    // Display
    bool isAvailable = true;
    bool showRequirements = true;
    TextStyle style;
    std::string icon;

    // Consequence tracking
    std::string consequenceTag;  // For tracking choice impacts
    int moralityShift = 0;       // -100 (evil) to 100 (good)

    bool checkRequirements() const;
};

// Dialogue node
struct DialogueNode {
    std::string id;
    DialogueNodeType type;

    // Content
    std::string speakerId;
    std::string text;
    std::vector<std::string> textVariants; // Random variations
    TextStyle style;

    // Audio
    std::string voiceFile;
    bool autoPlayVoice = true;
    float voiceDelay = 0.0f;

    // Visual
    std::string animation;       // Speaker animation
    std::string cameraAngle;
    std::string background;
    std::vector<std::string> participantIds; // All NPCs in scene

    // Branching
    std::vector<DialogueChoice> choices;
    std::string nextNodeId;      // For linear dialogue
    std::string conditionScript; // For conditional branches
    std::string actionScript;    // Script to execute

    // Timing
    float autoAdvanceDelay = -1.0f; // -1 = wait for input
    bool skippable = true;

    // Metadata
    std::vector<std::string> tags;
    bool markAsRead = true;

    DialogueNode() : type(DialogueNodeType::TEXT) {}
};

// Dialogue session state
struct DialogueSession {
    std::string dialogueId;
    std::string currentNodeId;
    std::string previousNodeId;
    std::unordered_map<std::string, bool> visitedNodes;
    std::unordered_map<std::string, int> choicesMade;
    std::vector<std::string> consequenceTags;
    float sessionStartTime;
    bool isActive = false;

    void reset() {
        currentNodeId.clear();
        previousNodeId.clear();
        visitedNodes.clear();
        choicesMade.clear();
        consequenceTags.clear();
        isActive = false;
    }
};

// Dialogue event callbacks
struct DialogueCallbacks {
    std::function<void(const DialogueNode&)> onNodeEnter;
    std::function<void(const DialogueNode&)> onNodeExit;
    std::function<void(const DialogueChoice&)> onChoiceMade;
    std::function<void(const std::string&)> onDialogueStart;
    std::function<void(const std::string&)> onDialogueEnd;
    std::function<void(const std::string&, const std::string&)> onActionTriggered;
    std::function<void(const std::string&)> onConsequenceAdded;
};

// Main dialogue system
class DialogueSystem {
public:
    DialogueSystem();
    ~DialogueSystem();

    // Initialization
    bool initialize();
    void shutdown();

    // Dialogue management
    bool loadDialogue(const std::string& filePath);
    bool loadDialogueFromJson(const std::string& jsonData);
    void unloadDialogue(const std::string& dialogueId);
    void unloadAllDialogues();

    // Dialogue playback
    bool startDialogue(const std::string& dialogueId, const std::string& startNodeId = "");
    void endDialogue();
    bool isDialogueActive() const;

    // Navigation
    bool advanceDialogue();
    bool selectChoice(int choiceIndex);
    bool selectChoice(const std::string& choiceId);
    bool goToNode(const std::string& nodeId);
    bool canGoBack() const;
    bool goBack();

    // Current state
    const DialogueNode* getCurrentNode() const;
    const DialogueSession& getCurrentSession() const;
    std::vector<DialogueChoice> getAvailableChoices() const;
    const Speaker* getSpeaker(const std::string& speakerId) const;

    // Speaker management
    void registerSpeaker(const Speaker& speaker);
    void updateSpeaker(const std::string& speakerId, const Speaker& speaker);

    // History
    std::vector<std::string> getDialogueHistory() const;
    void clearHistory();
    bool hasVisitedNode(const std::string& dialogueId, const std::string& nodeId) const;

    // Callbacks
    void setCallbacks(const DialogueCallbacks& callbacks);

    // Integration points
    void setNarrativeEngine(NarrativeEngine* engine);
    void setRelationshipSystem(RelationshipSystem* system);

    // Variable system for dynamic dialogue
    void setVariable(const std::string& name, int value);
    void setVariable(const std::string& name, const std::string& value);
    int getIntVariable(const std::string& name, int defaultValue = 0) const;
    std::string getStringVariable(const std::string& name, const std::string& defaultValue = "") const;

    // Text processing
    std::string processText(const std::string& text) const; // Process {variables}, [player_name], etc.

    // Voice/TTS support
    void enableVoiceActing(bool enable);
    bool isVoiceActingEnabled() const;
    void setTTSCallback(std::function<void(const std::string&, const Speaker&)> callback);

    // Localization
    void setLanguage(const std::string& languageCode);
    std::string getCurrentLanguage() const;

    // Debug
    void setDebugMode(bool enable);
    void exportDialogueGraph(const std::string& dialogueId, const std::string& outputPath);

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal methods
    void processNodeActions(const DialogueNode& node);
    void applyChoiceEffects(const DialogueChoice& choice);
    bool evaluateCondition(const std::string& condition) const;
    void triggerCallbacks(const std::string& event, const DialogueNode* node = nullptr);
};

// Dialogue builder helper (fluent API)
class DialogueBuilder {
public:
    DialogueBuilder(const std::string& dialogueId);

    DialogueBuilder& addNode(const std::string& nodeId, const std::string& text);
    DialogueBuilder& setSpeaker(const std::string& speakerId);
    DialogueBuilder& addChoice(const std::string& text, const std::string& nextNode);
    DialogueBuilder& setNextNode(const std::string& nodeId);
    DialogueBuilder& addCondition(const std::string& condition);
    DialogueBuilder& addAction(const std::string& action);
    DialogueBuilder& setVoiceFile(const std::string& filePath);

    std::string toJson() const;

private:
    std::string dialogueId_;
    std::vector<DialogueNode> nodes_;
    DialogueNode* currentNode_;
};

} // namespace Story
