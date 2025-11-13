#pragma once

#include "UIElement.h"
#include <string>
#include <vector>
#include <functional>

namespace UI {

// Dialogue choice
struct DialogueChoice {
    std::string text;
    std::string nextDialogueId;
    bool isEnabled;
    std::function<bool()> condition; // Condition to show this choice
    std::function<void()> onSelect; // Callback when selected

    DialogueChoice() : isEnabled(true), condition(nullptr), onSelect(nullptr) {}
};

// Dialogue node
struct DialogueNode {
    std::string id;
    std::string speakerName;
    std::string portraitPath;
    std::string text;
    std::vector<DialogueChoice> choices;
    bool autoAdvance;
    float autoAdvanceDelay;

    DialogueNode() : autoAdvance(false), autoAdvanceDelay(0.0f) {}
};

// Dialogue UI
class DialogueUI : public UIElement {
public:
    DialogueUI();
    ~DialogueUI() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Dialogue control
    void StartDialogue(const DialogueNode& node);
    void ShowDialogue(const DialogueNode& node);
    void EndDialogue();
    bool IsDialogueActive() const { return isActive_; }

    // Text reveal
    void SetTextRevealSpeed(float charsPerSecond) { textRevealSpeed_ = charsPerSecond; }
    void SetInstantText(bool instant) { instantText_ = instant; }
    bool IsTextRevealed() const { return revealedChars_ >= currentText_.length(); }
    void RevealAllText();

    // Callbacks
    void OnDialogueEnd(std::function<void()> callback) { onDialogueEnd_ = callback; }
    void OnChoiceSelected(std::function<void(int, const std::string&)> callback) {
        onChoiceSelected_ = callback;
    }

    // Configuration
    void SetSkipEnabled(bool enabled) { skipEnabled_ = enabled; }
    void SetVoiceEnabled(bool enabled) { voiceEnabled_ = enabled; }

private:
    void CreateLayout();
    void CreateSpeakerPanel();
    void CreateTextPanel();
    void CreateChoiceButtons();

    void RenderSpeakerPanel(SDL_Renderer* renderer);
    void RenderTextPanel(SDL_Renderer* renderer);
    void RenderChoices(SDL_Renderer* renderer);
    void RenderContinueIndicator(SDL_Renderer* renderer);

    void UpdateTextReveal(float deltaTime);
    void SelectChoice(int index);

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    bool isActive_;
    DialogueNode currentNode_;

    // Speaker info
    std::string currentSpeaker_;
    std::string currentPortrait_;
    std::shared_ptr<UIElement> speakerPanel_;
    std::shared_ptr<UIElement> portraitElement_;

    // Text display
    std::string currentText_;
    std::string revealedText_;
    size_t revealedChars_;
    float textRevealSpeed_;
    float revealTimer_;
    bool instantText_;
    std::shared_ptr<UIElement> textPanel_;

    // Choices
    std::vector<std::shared_ptr<UIElement>> choiceButtons_;
    std::shared_ptr<UIElement> choicePanel_;
    int selectedChoice_;
    int hoveredChoice_;

    // Auto-advance
    float autoAdvanceTimer_;

    // Continue indicator
    float continueBlinkTimer_;
    bool continueVisible_;

    // Settings
    bool skipEnabled_;
    bool voiceEnabled_;

    // Callbacks
    std::function<void()> onDialogueEnd_;
    std::function<void(int, const std::string&)> onChoiceSelected_;
};

} // namespace UI
