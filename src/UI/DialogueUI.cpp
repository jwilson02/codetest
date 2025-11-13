#include "DialogueUI.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>

namespace UI {

DialogueUI::DialogueUI()
    : UIElement("dialogue"),
      isActive_(false), revealedChars_(0),
      textRevealSpeed_(30.0f), revealTimer_(0.0f), instantText_(false),
      selectedChoice_(-1), hoveredChoice_(-1),
      autoAdvanceTimer_(0.0f), continueBlinkTimer_(0.0f),
      continueVisible_(true), skipEnabled_(true), voiceEnabled_(true) {

    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    // Position at bottom of screen
    float width = uiSystem.GetScreenWidth() * 0.8f;
    float height = 250;
    SetSize(width, height);
    SetPosition(uiSystem.GetScreenWidth() * 0.1f,
                uiSystem.GetScreenHeight() - height - 50);

    SetBackgroundColor(Color(15, 15, 20, 240));
    SetBorderColor(theme.accentColor);
    SetBorderWidth(3.0f);
    SetVisible(false);

    CreateLayout();
}

DialogueUI::~DialogueUI() {
}

void DialogueUI::Update(float deltaTime) {
    if (!isActive_ || !visible_) return;

    UIElement::Update(deltaTime);

    // Update text reveal
    if (!instantText_ && revealedChars_ < currentText_.length()) {
        UpdateTextReveal(deltaTime);
    }

    // Update auto-advance
    if (currentNode_.autoAdvance && IsTextRevealed()) {
        autoAdvanceTimer_ += deltaTime;
        if (autoAdvanceTimer_ >= currentNode_.autoAdvanceDelay) {
            if (!currentNode_.choices.empty()) {
                SelectChoice(0); // Auto-select first choice
            } else {
                EndDialogue();
            }
        }
    }

    // Update continue indicator blink
    continueBlinkTimer_ += deltaTime;
    if (continueBlinkTimer_ >= 0.5f) {
        continueVisible_ = !continueVisible_;
        continueBlinkTimer_ = 0.0f;
    }

    // Update panels
    if (speakerPanel_) speakerPanel_->Update(deltaTime);
    if (textPanel_) textPanel_->Update(deltaTime);
    if (choicePanel_) choicePanel_->Update(deltaTime);
}

void DialogueUI::Render(SDL_Renderer* renderer) {
    if (!isActive_ || !visible_) return;

    UIElement::Render(renderer);

    RenderSpeakerPanel(renderer);
    RenderTextPanel(renderer);

    if (IsTextRevealed() && !currentNode_.choices.empty()) {
        RenderChoices(renderer);
    }

    if (IsTextRevealed() && currentNode_.choices.empty()) {
        RenderContinueIndicator(renderer);
    }
}

void DialogueUI::HandleInput(const SDL_Event& event) {
    if (!isActive_ || !visible_) return;

    // Space or Enter to advance
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) {
            if (!IsTextRevealed()) {
                RevealAllText();
            } else if (currentNode_.choices.empty()) {
                EndDialogue();
            }
        }

        // Number keys for choices
        if (IsTextRevealed() && !currentNode_.choices.empty()) {
            if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) {
                int choice = event.key.keysym.sym - SDLK_1;
                if (choice < static_cast<int>(currentNode_.choices.size())) {
                    SelectChoice(choice);
                }
            }
        }

        // ESC to skip dialogue (if enabled)
        if (event.key.keysym.sym == SDLK_ESCAPE && skipEnabled_) {
            EndDialogue();
        }
    }

    // Mouse click to advance
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (!IsTextRevealed()) {
            RevealAllText();
        } else if (currentNode_.choices.empty()) {
            EndDialogue();
        }
    }

    UIElement::HandleInput(event);
}

void DialogueUI::StartDialogue(const DialogueNode& node) {
    ShowDialogue(node);
}

void DialogueUI::ShowDialogue(const DialogueNode& node) {
    currentNode_ = node;
    currentSpeaker_ = node.speakerName;
    currentPortrait_ = node.portraitPath;
    currentText_ = node.text;
    revealedText_.clear();
    revealedChars_ = 0;
    revealTimer_ = 0.0f;
    autoAdvanceTimer_ = 0.0f;
    selectedChoice_ = -1;
    hoveredChoice_ = -1;

    isActive_ = true;
    SetVisible(true);

    // Animate in
    PlayAnimation(AnimationType::SlideIn, 0.3f);

    // Create choice buttons
    CreateChoiceButtons();
}

void DialogueUI::EndDialogue() {
    isActive_ = false;

    // Animate out
    PlayAnimation(AnimationType::FadeOut, 0.2f);

    if (onDialogueEnd_) {
        onDialogueEnd_();
    }

    // Hide after animation
    auto animComplete = [this]() {
        SetVisible(false);
    };
    animation_.onComplete = animComplete;
}

void DialogueUI::RevealAllText() {
    revealedChars_ = currentText_.length();
    revealedText_ = currentText_;
}

void DialogueUI::CreateLayout() {
    CreateSpeakerPanel();
    CreateTextPanel();
}

void DialogueUI::CreateSpeakerPanel() {
    auto& theme = UISystem::Instance().GetTheme();

    speakerPanel_ = std::make_shared<UIElement>("speakerPanel");
    speakerPanel_->SetSize(200, 50);
    speakerPanel_->SetPosition(20, -40); // Above main panel
    speakerPanel_->SetBackgroundColor(Color(20, 20, 25, 240));
    speakerPanel_->SetBorderColor(theme.accentColor);
    speakerPanel_->SetBorderWidth(2.0f);

    AddChild(speakerPanel_);

    // Portrait
    portraitElement_ = std::make_shared<UIElement>("portrait");
    portraitElement_->SetSize(80, 80);
    portraitElement_->SetPosition(-100, -20);
    portraitElement_->SetBackgroundColor(Color(30, 30, 35, 255));
    portraitElement_->SetBorderColor(theme.accentColor);
    portraitElement_->SetBorderWidth(2.0f);

    AddChild(portraitElement_);
}

void DialogueUI::CreateTextPanel() {
    auto& theme = UISystem::Instance().GetTheme();

    textPanel_ = std::make_shared<UIElement>("textPanel");
    textPanel_->SetSize(width_ - 40, 120);
    textPanel_->SetPosition(20, 40);
    textPanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent
    textPanel_->SetPadding(15);

    AddChild(textPanel_);
}

void DialogueUI::CreateChoiceButtons() {
    // Clear existing choices
    choiceButtons_.clear();

    if (choicePanel_) {
        RemoveChild(choicePanel_->GetId());
    }

    if (currentNode_.choices.empty()) return;

    auto& theme = UISystem::Instance().GetTheme();

    choicePanel_ = std::make_shared<UIElement>("choicePanel");
    choicePanel_->SetSize(width_ - 40, 80);
    choicePanel_->SetPosition(20, 165);
    choicePanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    AddChild(choicePanel_);

    float buttonHeight = 30;
    float spacing = 5;
    float y = 0;

    for (size_t i = 0; i < currentNode_.choices.size(); ++i) {
        const auto& choice = currentNode_.choices[i];

        // Check condition
        if (choice.condition && !choice.condition()) {
            continue; // Skip this choice
        }

        auto button = std::make_shared<UIElement>("choice_" + std::to_string(i));
        button->SetSize(choicePanel_->GetWidth() - 20, buttonHeight);
        button->SetPosition(10, y);
        button->SetBackgroundColor(theme.secondaryColor);
        button->SetBorderColor(theme.accentColor);
        button->SetBorderWidth(2.0f);
        button->SetEnabled(choice.isEnabled);

        if (choice.isEnabled) {
            int index = static_cast<int>(i);
            button->OnClick([this, index]() {
                SelectChoice(index);
            });

            button->OnHover([button, &theme]() {
                button->SetBackgroundColor(theme.primaryColor);
                button->SetBorderWidth(3.0f);
            });
        } else {
            button->SetBackgroundColor(theme.disabledColor);
        }

        choicePanel_->AddChild(button);
        choiceButtons_.push_back(button);

        y += buttonHeight + spacing;
    }
}

void DialogueUI::RenderSpeakerPanel(SDL_Renderer* renderer) {
    if (!speakerPanel_) return;

    speakerPanel_->Render(renderer);

    // Render speaker name (would use TTF fonts)
    // currentSpeaker_

    if (portraitElement_) {
        portraitElement_->Render(renderer);
        // Would render portrait texture from currentPortrait_
    }
}

void DialogueUI::RenderTextPanel(SDL_Renderer* renderer) {
    if (!textPanel_) return;

    textPanel_->Render(renderer);

    // Render dialogue text (would use TTF fonts with word wrapping)
    // revealedText_

    // For now, render a placeholder
    SDL_Rect textBounds = textPanel_->GetBounds();
    textBounds.x += 10;
    textBounds.y += 10;
    textBounds.w -= 20;
    textBounds.h -= 20;

    auto& theme = UISystem::Instance().GetTheme();
    SDL_SetRenderDrawColor(renderer, theme.textColor.r, theme.textColor.g,
                          theme.textColor.b, 255);
    SDL_RenderDrawRect(renderer, &textBounds);
}

void DialogueUI::RenderChoices(SDL_Renderer* renderer) {
    if (!choicePanel_) return;

    choicePanel_->Render(renderer);

    // Choice buttons are rendered as children
}

void DialogueUI::RenderContinueIndicator(SDL_Renderer* renderer) {
    if (!continueVisible_) return;

    // Render blinking continue indicator (arrow or text)
    SDL_Rect indicatorRect;
    indicatorRect.x = static_cast<int>(x_ + width_ - 50);
    indicatorRect.y = static_cast<int>(y_ + height_ - 30);
    indicatorRect.w = 20;
    indicatorRect.h = 20;

    auto& theme = UISystem::Instance().GetTheme();
    SDL_SetRenderDrawColor(renderer, theme.accentColor.r, theme.accentColor.g,
                          theme.accentColor.b, 255);

    // Draw simple arrow (triangle pointing down)
    SDL_RenderDrawLine(renderer, indicatorRect.x, indicatorRect.y,
                      indicatorRect.x + 10, indicatorRect.y + 15);
    SDL_RenderDrawLine(renderer, indicatorRect.x + 10, indicatorRect.y + 15,
                      indicatorRect.x + 20, indicatorRect.y);
}

void DialogueUI::UpdateTextReveal(float deltaTime) {
    revealTimer_ += deltaTime;

    float charsToReveal = textRevealSpeed_ * revealTimer_;
    size_t targetChars = std::min(static_cast<size_t>(charsToReveal),
                                  currentText_.length());

    if (targetChars > revealedChars_) {
        revealedChars_ = targetChars;
        revealedText_ = currentText_.substr(0, revealedChars_);
    }
}

void DialogueUI::SelectChoice(int index) {
    if (index < 0 || index >= static_cast<int>(currentNode_.choices.size())) {
        return;
    }

    const auto& choice = currentNode_.choices[index];

    if (!choice.isEnabled) return;

    selectedChoice_ = index;

    // Execute choice callback
    if (choice.onSelect) {
        choice.onSelect();
    }

    // Notify callback
    if (onChoiceSelected_) {
        onChoiceSelected_(index, choice.nextDialogueId);
    }

    // End dialogue or continue to next node
    if (choice.nextDialogueId.empty()) {
        EndDialogue();
    }
    // Otherwise, the game should call ShowDialogue with the next node
}

void DialogueUI::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void DialogueUI::OnSizeChanged() {
    UIElement::OnSizeChanged();
}

} // namespace UI
