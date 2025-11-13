#include "QuestTracker.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>

namespace UI {

QuestTracker::QuestTracker()
    : UIElement("questTracker"),
      displayMode_(DisplayMode::Compact),
      maxTrackedQuests_(5),
      scrollOffset_(0.0f), maxScroll_(0.0f),
      isExpanding_(false), expandProgress_(0.0f) {

    auto& uiSystem = UISystem::Instance();
    const auto& theme = uiSystem.GetTheme();

    // Position in top-right corner
    float width = 350;
    float height = 400;
    SetSize(width, height);
    SetPosition(uiSystem.GetScreenWidth() - width - 20, 100);

    SetBackgroundColor(Color(20, 20, 25, 220));
    SetBorderColor(theme.accentColor);
    SetBorderWidth(2.0f);

    CreateLayout();
}

QuestTracker::~QuestTracker() {
}

void QuestTracker::Update(float deltaTime) {
    if (!visible_) return;

    UIElement::Update(deltaTime);

    // Update expand animation
    if (isExpanding_) {
        expandProgress_ += deltaTime * 3.0f;
        if (expandProgress_ >= 1.0f) {
            expandProgress_ = 1.0f;
            isExpanding_ = false;
        }
    }

    if (questListPanel_) {
        questListPanel_->Update(deltaTime);
    }
}

void QuestTracker::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    UIElement::Render(renderer);

    switch (displayMode_) {
        case DisplayMode::Compact:
            RenderCompactMode(renderer);
            break;
        case DisplayMode::Expanded:
            RenderExpandedMode(renderer);
            break;
        case DisplayMode::Full:
            RenderFullMode(renderer);
            break;
    }
}

void QuestTracker::HandleInput(const SDL_Event& event) {
    if (!visible_) return;

    // Toggle with 'L' key (Quest Log)
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_l) {
        if (displayMode_ == DisplayMode::Full) {
            SetDisplayMode(DisplayMode::Compact);
        } else {
            SetDisplayMode(DisplayMode::Full);
        }
    }

    // Mouse wheel for scrolling
    if (event.type == SDL_MOUSEWHEEL) {
        if (ContainsPoint(static_cast<float>(event.wheel.mouseX),
                         static_cast<float>(event.wheel.mouseY))) {
            scrollOffset_ -= event.wheel.y * 20.0f;
            scrollOffset_ = std::clamp(scrollOffset_, 0.0f, maxScroll_);
        }
    }

    UIElement::HandleInput(event);
}

void QuestTracker::AddQuest(const Quest& quest) {
    quests_[quest.id] = quest;
    UpdateQuestDisplay();
}

void QuestTracker::RemoveQuest(const std::string& questId) {
    quests_.erase(questId);

    // Remove from tracked list
    auto it = std::find(trackedQuestIds_.begin(), trackedQuestIds_.end(), questId);
    if (it != trackedQuestIds_.end()) {
        trackedQuestIds_.erase(it);
    }

    UpdateQuestDisplay();
}

void QuestTracker::UpdateQuest(const Quest& quest) {
    auto it = quests_.find(quest.id);
    if (it != quests_.end()) {
        // Check if quest was just completed
        bool wasCompleted = it->second.isCompleted;
        it->second = quest;

        if (!wasCompleted && quest.isCompleted) {
            if (onQuestCompleted_) {
                onQuestCompleted_(quest);
            }
        }

        UpdateQuestDisplay();
    }
}

Quest* QuestTracker::GetQuest(const std::string& questId) {
    auto it = quests_.find(questId);
    return (it != quests_.end()) ? &it->second : nullptr;
}

void QuestTracker::UpdateObjective(const std::string& questId, const std::string& objectiveId,
                                   int progress) {
    Quest* quest = GetQuest(questId);
    if (!quest) return;

    for (auto& obj : quest->objectives) {
        if (obj.id == objectiveId) {
            obj.current = progress;
            if (obj.current >= obj.required) {
                CompleteObjective(questId, objectiveId);
            }
            UpdateQuestDisplay();
            break;
        }
    }
}

void QuestTracker::CompleteObjective(const std::string& questId, const std::string& objectiveId) {
    Quest* quest = GetQuest(questId);
    if (!quest) return;

    for (auto& obj : quest->objectives) {
        if (obj.id == objectiveId && !obj.completed) {
            obj.completed = true;
            obj.current = obj.required;

            if (onObjectiveCompleted_) {
                onObjectiveCompleted_(questId, objectiveId);
            }

            // Check if all objectives are completed
            bool allCompleted = true;
            for (const auto& o : quest->objectives) {
                if (!o.completed) {
                    allCompleted = false;
                    break;
                }
            }

            if (allCompleted && !quest->isCompleted) {
                quest->isCompleted = true;
                if (onQuestCompleted_) {
                    onQuestCompleted_(*quest);
                }
            }

            UpdateQuestDisplay();
            break;
        }
    }
}

void QuestTracker::TrackQuest(const std::string& questId) {
    auto it = std::find(trackedQuestIds_.begin(), trackedQuestIds_.end(), questId);
    if (it == trackedQuestIds_.end()) {
        if (static_cast<int>(trackedQuestIds_.size()) < maxTrackedQuests_) {
            trackedQuestIds_.push_back(questId);
            UpdateQuestDisplay();
        }
    }
}

void QuestTracker::UntrackQuest(const std::string& questId) {
    auto it = std::find(trackedQuestIds_.begin(), trackedQuestIds_.end(), questId);
    if (it != trackedQuestIds_.end()) {
        trackedQuestIds_.erase(it);
        UpdateQuestDisplay();
    }
}

void QuestTracker::ToggleQuestTracking(const std::string& questId) {
    if (IsQuestTracked(questId)) {
        UntrackQuest(questId);
    } else {
        TrackQuest(questId);
    }
}

bool QuestTracker::IsQuestTracked(const std::string& questId) const {
    return std::find(trackedQuestIds_.begin(), trackedQuestIds_.end(), questId) !=
           trackedQuestIds_.end();
}

std::vector<Quest*> QuestTracker::GetTrackedQuests() {
    std::vector<Quest*> tracked;
    for (const auto& id : trackedQuestIds_) {
        Quest* quest = GetQuest(id);
        if (quest && !quest->isCompleted) {
            tracked.push_back(quest);
        }
    }
    return tracked;
}

std::vector<Quest*> QuestTracker::GetActiveQuests() {
    std::vector<Quest*> active;
    for (auto& pair : quests_) {
        if (!pair.second.isCompleted) {
            active.push_back(&pair.second);
        }
    }
    return active;
}

std::vector<Quest*> QuestTracker::GetCompletedQuests() {
    std::vector<Quest*> completed;
    for (auto& pair : quests_) {
        if (pair.second.isCompleted) {
            completed.push_back(&pair.second);
        }
    }
    return completed;
}

void QuestTracker::SetDisplayMode(DisplayMode mode) {
    displayMode_ = mode;

    // Adjust size based on mode
    if (mode == DisplayMode::Full) {
        SetSize(600, 700);
        SetPosition(UISystem::Instance().GetScreenWidth() / 2 - 300,
                   UISystem::Instance().GetScreenHeight() / 2 - 350);
    } else {
        SetSize(350, 400);
        SetPosition(UISystem::Instance().GetScreenWidth() - 370, 100);
    }

    UpdateQuestDisplay();
}

void QuestTracker::ToggleExpanded() {
    if (displayMode_ == DisplayMode::Compact) {
        SetDisplayMode(DisplayMode::Expanded);
    } else if (displayMode_ == DisplayMode::Expanded) {
        SetDisplayMode(DisplayMode::Compact);
    }
}

void QuestTracker::CreateLayout() {
    auto& theme = UISystem::Instance().GetTheme();

    headerPanel_ = std::make_shared<UIElement>("questHeader");
    headerPanel_->SetSize(width_ - 20, 30);
    headerPanel_->SetPosition(10, 10);
    headerPanel_->SetBackgroundColor(Color(30, 30, 35, 255));
    headerPanel_->SetBorderColor(theme.accentColor);
    headerPanel_->SetBorderWidth(1.0f);

    AddChild(headerPanel_);

    CreateQuestList();
}

void QuestTracker::CreateQuestList() {
    questListPanel_ = std::make_shared<UIElement>("questList");
    questListPanel_->SetSize(width_ - 20, height_ - 60);
    questListPanel_->SetPosition(10, 50);
    questListPanel_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    AddChild(questListPanel_);
}

void QuestTracker::UpdateQuestDisplay() {
    // Clear existing quest elements
    questElements_.clear();

    // This would recreate the quest display based on current data
    // For now, placeholder
}

void QuestTracker::RenderCompactMode(SDL_Renderer* renderer) {
    if (!questListPanel_) return;

    // Render header
    if (headerPanel_) {
        headerPanel_->Render(renderer);
        // Would render "Quests" title
    }

    // Render tracked quests
    auto tracked = GetTrackedQuests();
    float y = questListPanel_->GetY();

    for (size_t i = 0; i < tracked.size() && i < static_cast<size_t>(maxTrackedQuests_); ++i) {
        RenderQuestEntry(renderer, *tracked[i], questListPanel_->GetX(), y,
                        questListPanel_->GetWidth());
        y += 80; // Height per quest entry
    }
}

void QuestTracker::RenderExpandedMode(SDL_Renderer* renderer) {
    RenderCompactMode(renderer); // Similar to compact but shows more info
}

void QuestTracker::RenderFullMode(SDL_Renderer* renderer) {
    // Render full quest log
    if (headerPanel_) {
        headerPanel_->Render(renderer);
    }

    float y = questListPanel_->GetY() - scrollOffset_;
    auto active = GetActiveQuests();

    for (auto* quest : active) {
        RenderQuestEntry(renderer, *quest, questListPanel_->GetX(), y,
                        questListPanel_->GetWidth());
        y += 100;
    }

    // Update max scroll
    maxScroll_ = std::max(0.0f, y - questListPanel_->GetY() - questListPanel_->GetHeight());
}

void QuestTracker::RenderQuestEntry(SDL_Renderer* renderer, const Quest& quest,
                                   float x, float y, float width) {
    auto& theme = UISystem::Instance().GetTheme();

    // Quest background
    SDL_Rect questRect;
    questRect.x = static_cast<int>(x);
    questRect.y = static_cast<int>(y);
    questRect.w = static_cast<int>(width);
    questRect.h = 70;

    SDL_SetRenderDrawColor(renderer, 30, 30, 35, 200);
    SDL_RenderFillRect(renderer, &questRect);

    // Quest border
    Color borderColor = quest.category == "Main" ? theme.accentColor :
                       quest.category == "Side" ? Color::FromHex("#4CAF50") :
                       Color::FromHex("#9E9E9E");

    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, 255);
    SDL_RenderDrawRect(renderer, &questRect);

    // Would render quest title using TTF fonts
    // quest.title

    // Render objectives
    float objY = y + 25;
    for (const auto& obj : quest.objectives) {
        RenderObjective(renderer, obj, x + 10, objY, width - 20);
        objY += 15;
    }
}

void QuestTracker::RenderObjective(SDL_Renderer* renderer, const QuestObjective& obj,
                                  float x, float y, float width) {
    auto& theme = UISystem::Instance().GetTheme();

    // Checkbox
    SDL_Rect checkbox;
    checkbox.x = static_cast<int>(x);
    checkbox.y = static_cast<int>(y);
    checkbox.w = 12;
    checkbox.h = 12;

    Color checkColor = obj.completed ? theme.successColor : Color(100, 100, 100, 255);
    SDL_SetRenderDrawColor(renderer, checkColor.r, checkColor.g, checkColor.b, 255);

    if (obj.completed) {
        SDL_RenderFillRect(renderer, &checkbox);
    } else {
        SDL_RenderDrawRect(renderer, &checkbox);
    }

    // Objective text (would use TTF fonts)
    // obj.description + " (" + obj.current + "/" + obj.required + ")"

    // Progress bar if not completed
    if (!obj.completed && obj.required > 1) {
        float progress = obj.GetProgress();
        RenderProgressBar(renderer, progress, x + 20, y + 2, width - 30, 8);
    }
}

void QuestTracker::RenderProgressBar(SDL_Renderer* renderer, float progress,
                                    float x, float y, float width, float height) {
    auto& theme = UISystem::Instance().GetTheme();

    // Background
    SDL_Rect bgRect;
    bgRect.x = static_cast<int>(x);
    bgRect.y = static_cast<int>(y);
    bgRect.w = static_cast<int>(width);
    bgRect.h = static_cast<int>(height);

    SDL_SetRenderDrawColor(renderer, 40, 40, 45, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // Fill
    SDL_Rect fillRect = bgRect;
    fillRect.w = static_cast<int>(width * progress);

    SDL_SetRenderDrawColor(renderer, theme.accentColor.r, theme.accentColor.g,
                          theme.accentColor.b, 255);
    SDL_RenderFillRect(renderer, &fillRect);

    // Border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
}

void QuestTracker::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void QuestTracker::OnSizeChanged() {
    UIElement::OnSizeChanged();
}

} // namespace UI
