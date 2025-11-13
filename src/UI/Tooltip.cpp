#include "Tooltip.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <sstream>

namespace UI {

Tooltip::Tooltip()
    : UIElement("tooltip"),
      isShowing_(false), style_(TooltipStyle::Simple),
      positionMode_(TooltipPosition::Smart),
      targetX_(0), targetY_(0), offsetX_(10), offsetY_(10),
      showDelay_(0.5f), delayTimer_(0.0f),
      maxWidth_(300), wordWrap_(true), padding_(10.0f),
      fadeIn_(0.0f), fadeSpeed_(5.0f) {

    auto& theme = UISystem::Instance().GetTheme();

    SetSize(200, 50);
    SetBackgroundColor(Color(20, 20, 25, 240));
    SetBorderColor(Color(150, 150, 150, 255));
    SetBorderWidth(2.0f);
    SetVisible(false);
    SetLayer(10000); // Render on top of everything
    SetInteractive(false); // Don't block input
}

Tooltip::~Tooltip() {
}

void Tooltip::Update(float deltaTime) {
    if (!isShowing_) return;

    UIElement::Update(deltaTime);

    // Update delay timer
    if (delayTimer_ < showDelay_) {
        delayTimer_ += deltaTime;
        if (delayTimer_ >= showDelay_) {
            SetVisible(true);
            fadeIn_ = 0.0f;
        }
        return;
    }

    // Update fade in
    if (fadeIn_ < 1.0f) {
        fadeIn_ += deltaTime * fadeSpeed_;
        fadeIn_ = std::min(fadeIn_, 1.0f);
        SetOpacity(fadeIn_);
    }

    // Update position
    UpdatePosition(deltaTime);
}

void Tooltip::Render(SDL_Renderer* renderer) {
    if (!visible_ || !isShowing_) return;

    UIElement::Render(renderer);

    switch (style_) {
        case TooltipStyle::Simple:
            RenderSimple(renderer);
            break;
        case TooltipStyle::Rich:
            RenderRich(renderer);
            break;
        case TooltipStyle::Item:
            RenderItem(renderer);
            break;
        case TooltipStyle::Skill:
            RenderSkill(renderer);
            break;
    }
}

void Tooltip::Show(const std::string& text, float x, float y) {
    simpleText_ = text;
    targetX_ = x;
    targetY_ = y;
    style_ = TooltipStyle::Simple;
    isShowing_ = true;
    delayTimer_ = 0.0f;
    fadeIn_ = 0.0f;

    CalculateSize();
    UpdatePosition(0.0f);
}

void Tooltip::Show(const std::string& text) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Show(text, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void Tooltip::ShowRich(const TooltipContent& content, float x, float y) {
    richContent_ = content;
    targetX_ = x;
    targetY_ = y;
    style_ = TooltipStyle::Rich;
    isShowing_ = true;
    delayTimer_ = 0.0f;
    fadeIn_ = 0.0f;

    SetBorderColor(content.borderColor);

    CalculateSize();
    UpdatePosition(0.0f);
}

void Tooltip::ShowRich(const TooltipContent& content) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    ShowRich(content, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void Tooltip::Hide() {
    isShowing_ = false;
    SetVisible(false);
    delayTimer_ = 0.0f;
    fadeIn_ = 0.0f;
    simpleText_.clear();
}

void Tooltip::UpdatePosition(float deltaTime) {
    float newX = targetX_;
    float newY = targetY_;

    switch (positionMode_) {
        case TooltipPosition::Cursor:
            // Follow cursor with offset
            SDL_GetMouseState(reinterpret_cast<int*>(&newX),
                            reinterpret_cast<int*>(&newY));
            newX += offsetX_;
            newY += offsetY_;
            break;

        case TooltipPosition::Fixed:
            // Use target position
            break;

        case TooltipPosition::Smart:
            // Adjust to cursor but stay on screen
            SDL_GetMouseState(reinterpret_cast<int*>(&newX),
                            reinterpret_cast<int*>(&newY));
            newX += offsetX_;
            newY += offsetY_;
            break;

        default:
            break;
    }

    SetPosition(newX, newY);

    if (positionMode_ == TooltipPosition::Smart) {
        AdjustPositionToScreen();
    }
}

void Tooltip::CalculateSize() {
    float width = 200;
    float height = 50;

    switch (style_) {
        case TooltipStyle::Simple: {
            // Calculate size based on text length (simplified)
            // In production, would measure actual text with TTF
            float textWidth = simpleText_.length() * 8.0f; // Rough estimate
            width = std::min(textWidth + padding_ * 2, maxWidth_);
            height = 30 + padding_ * 2;
            break;
        }

        case TooltipStyle::Rich: {
            width = maxWidth_;
            height = padding_ * 2;

            // Title
            if (!richContent_.title.empty()) {
                height += 25;
            }

            // Subtitle
            if (!richContent_.subtitle.empty()) {
                height += 20;
            }

            // Description (with word wrap)
            if (!richContent_.description.empty()) {
                auto lines = WrapText(richContent_.description, maxWidth_ - padding_ * 2);
                height += lines.size() * 18 + 10;
            }

            // Stats
            if (!richContent_.stats.empty()) {
                height += richContent_.stats.size() * 18 + 10;
            }

            break;
        }

        default:
            break;
    }

    SetSize(width, height);
}

void Tooltip::RenderSimple(SDL_Renderer* renderer) {
    if (simpleText_.empty()) return;

    SDL_Rect bounds = GetBounds();

    auto& theme = UISystem::Instance().GetTheme();

    // Render text (would use TTF fonts in production)
    // Placeholder: just render text bounds
    SDL_Rect textRect = bounds;
    textRect.x += static_cast<int>(padding_);
    textRect.y += static_cast<int>(padding_);
    textRect.w -= static_cast<int>(padding_ * 2);
    textRect.h -= static_cast<int>(padding_ * 2);

    SDL_SetRenderDrawColor(renderer, theme.textColor.r, theme.textColor.g,
                          theme.textColor.b, static_cast<uint8_t>(255 * fadeIn_));
    SDL_RenderDrawRect(renderer, &textRect);
}

void Tooltip::RenderRich(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();
    float y = bounds.y + padding_;
    float x = bounds.x + padding_;

    auto& theme = UISystem::Instance().GetTheme();

    // Render title
    if (!richContent_.title.empty()) {
        RenderText(renderer, richContent_.title, x, y, richContent_.titleColor);
        y += 25;
    }

    // Render subtitle
    if (!richContent_.subtitle.empty()) {
        RenderText(renderer, richContent_.subtitle, x, y, richContent_.subtitleColor);
        y += 20;
    }

    // Render separator
    if (!richContent_.title.empty() || !richContent_.subtitle.empty()) {
        SDL_Rect separator;
        separator.x = bounds.x + static_cast<int>(padding_);
        separator.y = static_cast<int>(y);
        separator.w = bounds.w - static_cast<int>(padding_ * 2);
        separator.h = 1;

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, static_cast<uint8_t>(200 * fadeIn_));
        SDL_RenderFillRect(renderer, &separator);
        y += 10;
    }

    // Render description
    if (!richContent_.description.empty()) {
        auto lines = WrapText(richContent_.description, width_ - padding_ * 2);
        for (const auto& line : lines) {
            RenderText(renderer, line, x, y, theme.textColor);
            y += 18;
        }
        y += 10;
    }

    // Render stats
    for (const auto& stat : richContent_.stats) {
        std::string statLine = stat.first + ": " + stat.second;
        RenderText(renderer, statLine, x, y, Color::FromHex("#4CAF50"));
        y += 18;
    }
}

void Tooltip::RenderItem(SDL_Renderer* renderer) {
    // Similar to rich but with item-specific formatting
    RenderRich(renderer);
}

void Tooltip::RenderSkill(SDL_Renderer* renderer) {
    // Similar to rich but with skill-specific formatting
    RenderRich(renderer);
}

void Tooltip::RenderText(SDL_Renderer* renderer, const std::string& text,
                        float x, float y, const Color& color) {
    // Placeholder - would render actual text with TTF fonts
    SDL_Rect textRect;
    textRect.x = static_cast<int>(x);
    textRect.y = static_cast<int>(y);
    textRect.w = static_cast<int>(text.length() * 8);
    textRect.h = 16;

    uint8_t alpha = static_cast<uint8_t>(color.a * fadeIn_);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
    SDL_RenderDrawRect(renderer, &textRect);
}

std::vector<std::string> Tooltip::WrapText(const std::string& text, float maxWidth) {
    std::vector<std::string> lines;

    if (!wordWrap_) {
        lines.push_back(text);
        return lines;
    }

    // Simple word wrapping (simplified)
    // In production, would measure actual text width with TTF
    std::istringstream words(text);
    std::string word;
    std::string currentLine;
    int charsPerLine = static_cast<int>(maxWidth / 8); // Rough estimate

    while (words >> word) {
        if (currentLine.length() + word.length() + 1 > static_cast<size_t>(charsPerLine)) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
        }

        if (!currentLine.empty()) {
            currentLine += " ";
        }
        currentLine += word;
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

void Tooltip::AdjustPositionToScreen() {
    auto& uiSystem = UISystem::Instance();
    float screenWidth = static_cast<float>(uiSystem.GetScreenWidth());
    float screenHeight = static_cast<float>(uiSystem.GetScreenHeight());

    float newX = x_;
    float newY = y_;

    // Adjust horizontally
    if (x_ + width_ > screenWidth) {
        newX = screenWidth - width_ - 10;
    }
    if (newX < 0) {
        newX = 10;
    }

    // Adjust vertically
    if (y_ + height_ > screenHeight) {
        newY = screenHeight - height_ - 10;
    }
    if (newY < 0) {
        newY = 10;
    }

    if (newX != x_ || newY != y_) {
        x_ = newX;
        y_ = newY;
    }
}

void Tooltip::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

} // namespace UI
