#pragma once

#include "UIElement.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace UI {

// Quest objective
struct QuestObjective {
    std::string id;
    std::string description;
    int current;
    int required;
    bool completed;

    QuestObjective()
        : current(0), required(1), completed(false) {}

    float GetProgress() const {
        return required > 0 ? static_cast<float>(current) / required : 0.0f;
    }
};

// Quest data
struct Quest {
    std::string id;
    std::string title;
    std::string description;
    std::string category; // Main, Side, Daily, Event
    int level;
    std::vector<QuestObjective> objectives;
    bool isTracked;
    bool isCompleted;
    std::string rewardText;

    Quest()
        : level(1), isTracked(false), isCompleted(false) {}

    int GetCompletedObjectives() const {
        int completed = 0;
        for (const auto& obj : objectives) {
            if (obj.completed) completed++;
        }
        return completed;
    }

    float GetProgress() const {
        if (objectives.empty()) return 0.0f;
        return static_cast<float>(GetCompletedObjectives()) / objectives.size();
    }
};

// Quest tracker UI
class QuestTracker : public UIElement {
public:
    QuestTracker();
    ~QuestTracker() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Quest management
    void AddQuest(const Quest& quest);
    void RemoveQuest(const std::string& questId);
    void UpdateQuest(const Quest& quest);
    Quest* GetQuest(const std::string& questId);

    // Objective updates
    void UpdateObjective(const std::string& questId, const std::string& objectiveId,
                        int progress);
    void CompleteObjective(const std::string& questId, const std::string& objectiveId);

    // Tracking
    void TrackQuest(const std::string& questId);
    void UntrackQuest(const std::string& questId);
    void ToggleQuestTracking(const std::string& questId);
    bool IsQuestTracked(const std::string& questId) const;

    std::vector<Quest*> GetTrackedQuests();
    std::vector<Quest*> GetActiveQuests();
    std::vector<Quest*> GetCompletedQuests();

    // Display modes
    enum class DisplayMode {
        Compact,   // Show only tracked quests
        Expanded,  // Show all active quests
        Full       // Quest log view
    };

    void SetDisplayMode(DisplayMode mode);
    DisplayMode GetDisplayMode() const { return displayMode_; }

    // UI control
    void ToggleExpanded();
    void SetMaxTrackedQuests(int max) { maxTrackedQuests_ = max; }

    // Callbacks
    void OnQuestCompleted(std::function<void(const Quest&)> callback) {
        onQuestCompleted_ = callback;
    }
    void OnObjectiveCompleted(std::function<void(const std::string&, const std::string&)> callback) {
        onObjectiveCompleted_ = callback;
    }

private:
    void CreateLayout();
    void CreateQuestList();
    void UpdateQuestDisplay();

    void RenderCompactMode(SDL_Renderer* renderer);
    void RenderExpandedMode(SDL_Renderer* renderer);
    void RenderFullMode(SDL_Renderer* renderer);

    void RenderQuestEntry(SDL_Renderer* renderer, const Quest& quest,
                         float x, float y, float width);
    void RenderObjective(SDL_Renderer* renderer, const QuestObjective& obj,
                        float x, float y, float width);
    void RenderProgressBar(SDL_Renderer* renderer, float progress,
                          float x, float y, float width, float height);

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    std::unordered_map<std::string, Quest> quests_;
    std::vector<std::string> trackedQuestIds_;
    DisplayMode displayMode_;
    int maxTrackedQuests_;

    // UI elements
    std::shared_ptr<UIElement> questListPanel_;
    std::shared_ptr<UIElement> headerPanel_;
    std::vector<std::shared_ptr<UIElement>> questElements_;

    // Scroll state
    float scrollOffset_;
    float maxScroll_;

    // Animation
    bool isExpanding_;
    float expandProgress_;

    // Callbacks
    std::function<void(const Quest&)> onQuestCompleted_;
    std::function<void(const std::string&, const std::string&)> onObjectiveCompleted_;
};

} // namespace UI
