#pragma once

#include "UIElement.h"
#include <string>
#include <vector>

namespace UI {

// Tooltip style
enum class TooltipStyle {
    Simple,     // Just text
    Rich,       // Title + description + stats
    Item,       // Item tooltip with rarity, stats, etc.
    Skill       // Skill tooltip with cooldown, cost, etc.
};

// Tooltip positioning
enum class TooltipPosition {
    Cursor,         // Follow cursor
    Fixed,          // Fixed position
    Smart,          // Adjust to stay on screen
    ElementBelow,   // Below element
    ElementAbove,   // Above element
    ElementRight,   // To the right of element
    ElementLeft     // To the left of element
};

// Rich tooltip content
struct TooltipContent {
    std::string title;
    std::string subtitle;
    std::string description;
    std::vector<std::pair<std::string, std::string>> stats; // Label, Value pairs
    Color titleColor;
    Color subtitleColor;
    Color borderColor;

    TooltipContent()
        : titleColor(255, 255, 255, 255),
          subtitleColor(200, 200, 200, 255),
          borderColor(100, 100, 100, 255) {}
};

// Tooltip system
class Tooltip : public UIElement {
public:
    Tooltip();
    ~Tooltip() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;

    // Simple tooltip
    void Show(const std::string& text, float x, float y);
    void Show(const std::string& text); // Show at cursor

    // Rich tooltip
    void ShowRich(const TooltipContent& content, float x, float y);
    void ShowRich(const TooltipContent& content); // Show at cursor

    void Hide();

    // Configuration
    void SetStyle(TooltipStyle style) { style_ = style; }
    void SetPosition(TooltipPosition position) { positionMode_ = position; }
    void SetDelay(float delay) { showDelay_ = delay; }
    void SetMaxWidth(float width) { maxWidth_ = width; }

    // Word wrapping
    void SetWordWrap(bool enabled) { wordWrap_ = enabled; }

    bool IsShowing() const { return isShowing_; }

private:
    void UpdatePosition(float deltaTime);
    void CalculateSize();

    void RenderSimple(SDL_Renderer* renderer);
    void RenderRich(SDL_Renderer* renderer);
    void RenderItem(SDL_Renderer* renderer);
    void RenderSkill(SDL_Renderer* renderer);

    void RenderText(SDL_Renderer* renderer, const std::string& text,
                   float x, float y, const Color& color);
    std::vector<std::string> WrapText(const std::string& text, float maxWidth);

    void AdjustPositionToScreen();

    void OnPositionChanged() override;

private:
    bool isShowing_;
    TooltipStyle style_;
    TooltipPosition positionMode_;

    // Simple tooltip
    std::string simpleText_;

    // Rich tooltip
    TooltipContent richContent_;

    // Positioning
    float targetX_, targetY_;
    float offsetX_, offsetY_;
    float showDelay_;
    float delayTimer_;

    // Configuration
    float maxWidth_;
    bool wordWrap_;
    float padding_;

    // Animation
    float fadeIn_;
    float fadeSpeed_;
};

} // namespace UI
