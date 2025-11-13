#include "DialogueSystem.h"
#include "DialogueTree.h"
#include "NarrativeEngine.h"
#include "RelationshipSystem.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <fstream>
#include <ctime>

// JSON parsing would normally use a library like nlohmann/json
// For now, we'll stub out the JSON functions

namespace Story {

// TextStyle implementation
std::string TextStyle::toMarkup() const {
    std::stringstream ss;
    if (bold) ss << "<b>";
    if (italic) ss << "<i>";
    if (underline) ss << "<u>";
    if (!color.empty()) ss << "<color=" << color << ">";
    return ss.str();
}

// DialogueChoice implementation
bool DialogueChoice::checkRequirements() const {
    // This would check actual game state
    // Placeholder implementation
    return isAvailable;
}

// DialogueSystem::Implementation
struct DialogueSystem::Implementation {
    // Data storage
    std::unordered_map<std::string, Cutscene> cutscenes;
    std::unordered_map<std::string, std::vector<DialogueNode>> dialogues;
    std::unordered_map<std::string, Speaker> speakers;

    // State
    DialogueSession currentSession;
    std::vector<std::string> history;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> globalVisitedNodes;

    // Variables
    std::unordered_map<std::string, int> intVariables;
    std::unordered_map<std::string, std::string> stringVariables;

    // Integration
    NarrativeEngine* narrativeEngine = nullptr;
    RelationshipSystem* relationshipSystem = nullptr;

    // Callbacks
    DialogueCallbacks callbacks;
    std::function<void(const std::string&, const Speaker&)> ttsCallback;

    // Settings
    bool voiceActingEnabled = true;
    std::string currentLanguage = "en";
    bool debugMode = false;

    // Helpers
    DialogueNode* findNode(const std::string& dialogueId, const std::string& nodeId);
    const DialogueNode* findNode(const std::string& dialogueId, const std::string& nodeId) const;
};

DialogueNode* DialogueSystem::Implementation::findNode(const std::string& dialogueId, const std::string& nodeId) {
    auto it = dialogues.find(dialogueId);
    if (it == dialogues.end()) return nullptr;

    for (auto& node : it->second) {
        if (node.id == nodeId) return &node;
    }
    return nullptr;
}

const DialogueNode* DialogueSystem::Implementation::findNode(const std::string& dialogueId, const std::string& nodeId) const {
    auto it = dialogues.find(dialogueId);
    if (it == dialogues.end()) return nullptr;

    for (const auto& node : it->second) {
        if (node.id == nodeId) return &node;
    }
    return nullptr;
}

// DialogueSystem implementation
DialogueSystem::DialogueSystem() : impl(std::make_unique<Implementation>()) {}

DialogueSystem::~DialogueSystem() = default;

bool DialogueSystem::initialize() {
    impl->currentSession.reset();
    impl->history.clear();
    return true;
}

void DialogueSystem::shutdown() {
    if (impl->currentSession.isActive) {
        endDialogue();
    }
    impl->dialogues.clear();
    impl->speakers.clear();
}

bool DialogueSystem::loadDialogue(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadDialogueFromJson(buffer.str());
}

bool DialogueSystem::loadDialogueFromJson(const std::string& jsonData) {
    // In a real implementation, this would parse JSON
    // For now, we'll just return success
    return true;
}

void DialogueSystem::unloadDialogue(const std::string& dialogueId) {
    impl->dialogues.erase(dialogueId);
}

void DialogueSystem::unloadAllDialogues() {
    impl->dialogues.clear();
}

bool DialogueSystem::startDialogue(const std::string& dialogueId, const std::string& startNodeId) {
    if (impl->currentSession.isActive) {
        endDialogue();
    }

    auto it = impl->dialogues.find(dialogueId);
    if (it == impl->dialogues.end() || it->second.empty()) {
        return false;
    }

    impl->currentSession.dialogueId = dialogueId;
    impl->currentSession.currentNodeId = startNodeId.empty() ? it->second[0].id : startNodeId;
    impl->currentSession.isActive = true;
    impl->currentSession.sessionStartTime = static_cast<float>(std::time(nullptr));

    if (impl->callbacks.onDialogueStart) {
        impl->callbacks.onDialogueStart(dialogueId);
    }

    const DialogueNode* node = getCurrentNode();
    if (node) {
        processNodeActions(*node);
        if (impl->callbacks.onNodeEnter) {
            impl->callbacks.onNodeEnter(*node);
        }
    }

    return true;
}

void DialogueSystem::endDialogue() {
    if (!impl->currentSession.isActive) return;

    const DialogueNode* node = getCurrentNode();
    if (node && impl->callbacks.onNodeExit) {
        impl->callbacks.onNodeExit(*node);
    }

    if (impl->callbacks.onDialogueEnd) {
        impl->callbacks.onDialogueEnd(impl->currentSession.dialogueId);
    }

    impl->currentSession.reset();
}

bool DialogueSystem::isDialogueActive() const {
    return impl->currentSession.isActive;
}

bool DialogueSystem::advanceDialogue() {
    if (!impl->currentSession.isActive) return false;

    const DialogueNode* current = getCurrentNode();
    if (!current || current->nextNodeId.empty()) {
        endDialogue();
        return false;
    }

    return goToNode(current->nextNodeId);
}

bool DialogueSystem::selectChoice(int choiceIndex) {
    const DialogueNode* current = getCurrentNode();
    if (!current || choiceIndex < 0 || choiceIndex >= static_cast<int>(current->choices.size())) {
        return false;
    }

    return selectChoice(current->choices[choiceIndex].id);
}

bool DialogueSystem::selectChoice(const std::string& choiceId) {
    const DialogueNode* current = getCurrentNode();
    if (!current) return false;

    for (const auto& choice : current->choices) {
        if (choice.id == choiceId) {
            if (!choice.checkRequirements()) return false;

            applyChoiceEffects(choice);

            if (impl->callbacks.onChoiceMade) {
                impl->callbacks.onChoiceMade(choice);
            }

            impl->currentSession.choicesMade[choiceId]++;

            if (!choice.consequenceTag.empty()) {
                impl->currentSession.consequenceTags.push_back(choice.consequenceTag);
                if (impl->callbacks.onConsequenceAdded) {
                    impl->callbacks.onConsequenceAdded(choice.consequenceTag);
                }
            }

            return goToNode(choice.nextNodeId);
        }
    }

    return false;
}

bool DialogueSystem::goToNode(const std::string& nodeId) {
    if (!impl->currentSession.isActive) return false;

    const DialogueNode* newNode = impl->findNode(impl->currentSession.dialogueId, nodeId);
    if (!newNode) return false;

    const DialogueNode* oldNode = getCurrentNode();
    if (oldNode && impl->callbacks.onNodeExit) {
        impl->callbacks.onNodeExit(*oldNode);
    }

    impl->currentSession.previousNodeId = impl->currentSession.currentNodeId;
    impl->currentSession.currentNodeId = nodeId;
    impl->currentSession.visitedNodes[nodeId] = true;
    impl->globalVisitedNodes[impl->currentSession.dialogueId][nodeId] = true;

    impl->history.push_back(nodeId);

    processNodeActions(*newNode);

    if (impl->callbacks.onNodeEnter) {
        impl->callbacks.onNodeEnter(*newNode);
    }

    // Auto-advance if set
    if (newNode->type == DialogueNodeType::ACTION && !newNode->nextNodeId.empty()) {
        return goToNode(newNode->nextNodeId);
    }

    return true;
}

bool DialogueSystem::canGoBack() const {
    return !impl->currentSession.previousNodeId.empty();
}

bool DialogueSystem::goBack() {
    if (!canGoBack()) return false;
    return goToNode(impl->currentSession.previousNodeId);
}

const DialogueNode* DialogueSystem::getCurrentNode() const {
    if (!impl->currentSession.isActive) return nullptr;
    return impl->findNode(impl->currentSession.dialogueId, impl->currentSession.currentNodeId);
}

const DialogueSession& DialogueSystem::getCurrentSession() const {
    return impl->currentSession;
}

std::vector<DialogueChoice> DialogueSystem::getAvailableChoices() const {
    const DialogueNode* node = getCurrentNode();
    if (!node) return {};

    std::vector<DialogueChoice> available;
    for (const auto& choice : node->choices) {
        if (choice.checkRequirements()) {
            available.push_back(choice);
        }
    }

    return available;
}

const Speaker* DialogueSystem::getSpeaker(const std::string& speakerId) const {
    auto it = impl->speakers.find(speakerId);
    return it != impl->speakers.end() ? &it->second : nullptr;
}

void DialogueSystem::registerSpeaker(const Speaker& speaker) {
    impl->speakers[speaker.id] = speaker;
}

void DialogueSystem::updateSpeaker(const std::string& speakerId, const Speaker& speaker) {
    impl->speakers[speakerId] = speaker;
}

std::vector<std::string> DialogueSystem::getDialogueHistory() const {
    return impl->history;
}

void DialogueSystem::clearHistory() {
    impl->history.clear();
}

bool DialogueSystem::hasVisitedNode(const std::string& dialogueId, const std::string& nodeId) const {
    auto it = impl->globalVisitedNodes.find(dialogueId);
    if (it == impl->globalVisitedNodes.end()) return false;

    auto nodeIt = it->second.find(nodeId);
    return nodeIt != it->second.end() && nodeIt->second;
}

void DialogueSystem::setCallbacks(const DialogueCallbacks& callbacks) {
    impl->callbacks = callbacks;
}

void DialogueSystem::setNarrativeEngine(NarrativeEngine* engine) {
    impl->narrativeEngine = engine;
}

void DialogueSystem::setRelationshipSystem(RelationshipSystem* system) {
    impl->relationshipSystem = system;
}

void DialogueSystem::setVariable(const std::string& name, int value) {
    impl->intVariables[name] = value;
}

void DialogueSystem::setVariable(const std::string& name, const std::string& value) {
    impl->stringVariables[name] = value;
}

int DialogueSystem::getIntVariable(const std::string& name, int defaultValue) const {
    auto it = impl->intVariables.find(name);
    return it != impl->intVariables.end() ? it->second : defaultValue;
}

std::string DialogueSystem::getStringVariable(const std::string& name, const std::string& defaultValue) const {
    auto it = impl->stringVariables.find(name);
    return it != impl->stringVariables.end() ? it->second : defaultValue;
}

std::string DialogueSystem::processText(const std::string& text) const {
    std::string result = text;

    // Replace {variables}
    std::regex varRegex("\\{([^}]+)\\}");
    std::smatch match;
    while (std::regex_search(result, match, varRegex)) {
        std::string varName = match[1].str();
        std::string replacement = getStringVariable(varName, "");
        result = std::regex_replace(result, std::regex("\\{" + varName + "\\}"), replacement);
    }

    // Replace [player_name]
    if (impl->narrativeEngine) {
        result = std::regex_replace(result, std::regex("\\[player_name\\]"),
                                   impl->narrativeEngine->getPlayerName());
    }

    return result;
}

void DialogueSystem::enableVoiceActing(bool enable) {
    impl->voiceActingEnabled = enable;
}

bool DialogueSystem::isVoiceActingEnabled() const {
    return impl->voiceActingEnabled;
}

void DialogueSystem::setTTSCallback(std::function<void(const std::string&, const Speaker&)> callback) {
    impl->ttsCallback = callback;
}

void DialogueSystem::setLanguage(const std::string& languageCode) {
    impl->currentLanguage = languageCode;
}

std::string DialogueSystem::getCurrentLanguage() const {
    return impl->currentLanguage;
}

void DialogueSystem::setDebugMode(bool enable) {
    impl->debugMode = enable;
}

void DialogueSystem::exportDialogueGraph(const std::string& dialogueId, const std::string& outputPath) {
    // Would generate Graphviz DOT format
}

void DialogueSystem::processNodeActions(const DialogueNode& node) {
    if (!node.actionScript.empty() && impl->callbacks.onActionTriggered) {
        impl->callbacks.onActionTriggered(node.id, node.actionScript);
    }
}

void DialogueSystem::applyChoiceEffects(const DialogueChoice& choice) {
    // Apply relationship changes
    if (impl->relationshipSystem) {
        for (const auto& [npcId, amount] : choice.modifyRelationships) {
            impl->relationshipSystem->modifyRelationship(npcId, amount, "dialogue_choice");
        }
    }

    // Morality shift
    if (impl->narrativeEngine && choice.moralityShift != 0) {
        impl->narrativeEngine->adjustMorality(choice.moralityShift);
    }
}

bool DialogueSystem::evaluateCondition(const std::string& condition) const {
    // Simple condition evaluation
    // In real implementation, would use a scripting language
    return true;
}

void DialogueSystem::triggerCallbacks(const std::string& event, const DialogueNode* node) {
    // Trigger appropriate callbacks
}

// DialogueBuilder implementation
DialogueBuilder::DialogueBuilder(const std::string& dialogueId)
    : dialogueId_(dialogueId), currentNode_(nullptr) {}

DialogueBuilder& DialogueBuilder::addNode(const std::string& nodeId, const std::string& text) {
    DialogueNode node;
    node.id = nodeId;
    node.text = text;
    nodes_.push_back(node);
    currentNode_ = &nodes_.back();
    return *this;
}

DialogueBuilder& DialogueBuilder::setSpeaker(const std::string& speakerId) {
    if (currentNode_) {
        currentNode_->speakerId = speakerId;
    }
    return *this;
}

DialogueBuilder& DialogueBuilder::addChoice(const std::string& text, const std::string& nextNode) {
    if (currentNode_) {
        DialogueChoice choice;
        choice.id = "choice_" + std::to_string(currentNode_->choices.size());
        choice.text = text;
        choice.nextNodeId = nextNode;
        currentNode_->choices.push_back(choice);
        currentNode_->type = DialogueNodeType::CHOICE;
    }
    return *this;
}

DialogueBuilder& DialogueBuilder::setNextNode(const std::string& nodeId) {
    if (currentNode_) {
        currentNode_->nextNodeId = nodeId;
    }
    return *this;
}

DialogueBuilder& DialogueBuilder::addCondition(const std::string& condition) {
    if (currentNode_) {
        currentNode_->conditionScript = condition;
        currentNode_->type = DialogueNodeType::CONDITION;
    }
    return *this;
}

DialogueBuilder& DialogueBuilder::addAction(const std::string& action) {
    if (currentNode_) {
        currentNode_->actionScript = action;
        currentNode_->type = DialogueNodeType::ACTION;
    }
    return *this;
}

DialogueBuilder& DialogueBuilder::setVoiceFile(const std::string& filePath) {
    if (currentNode_) {
        currentNode_->voiceFile = filePath;
    }
    return *this;
}

std::string DialogueBuilder::toJson() const {
    // Would generate JSON output
    return "{}";
}

} // namespace Story
