#include "UIElement.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace UI {

// Color implementation
Color Color::FromHex(const std::string& hex) {
    std::string clean = hex;
    if (clean[0] == '#') clean = clean.substr(1);

    unsigned int value;
    std::stringstream ss;
    ss << std::hex << clean;
    ss >> value;

    if (clean.length() == 6) {
        return Color((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF, 255);
    } else if (clean.length() == 8) {
        return Color((value >> 24) & 0xFF, (value >> 16) & 0xFF,
                     (value >> 8) & 0xFF, value & 0xFF);
    }

    return Color();
}

std::string Color::ToHex() const {
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0')
       << std::setw(2) << (int)r
       << std::setw(2) << (int)g
       << std::setw(2) << (int)b
       << std::setw(2) << (int)a;
    return ss.str();
}

// Animation implementation
float Animation::GetEasedProgress() const {
    float t = GetProgress();
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;

    switch (type) {
        case AnimationType::FadeIn:
        case AnimationType::FadeOut:
        case AnimationType::SlideIn:
        case AnimationType::SlideOut:
            return UIUtils::EaseInOut(t);
        case AnimationType::Scale:
            return UIUtils::EaseOut(t);
        case AnimationType::Bounce:
            return UIUtils::Bounce(t);
        case AnimationType::Shake:
            return std::sin(t * 10.0f * M_PI) * (1.0f - t);
        default:
            return t;
    }
}

// UIElement implementation
UIElement::UIElement(const std::string& id)
    : id_(id), x_(0), y_(0), width_(100), height_(100),
      anchor_(Anchor::TopLeft), pivotX_(0.0f), pivotY_(0.0f),
      parent_(nullptr), state_(UIState::Normal),
      visible_(true), interactive_(true), enabled_(true), layer_(0),
      backgroundColor_(50, 50, 50, 200), borderColor_(100, 100, 100, 255),
      borderWidth_(1.0f), opacity_(1.0f), padding_(5.0f), margin_(0.0f),
      isHovered_(false), isPressed_(false), isDragging_(false),
      dragStartX_(0), dragStartY_(0) {
}

UIElement::~UIElement() {
    children_.clear();
}

void UIElement::Update(float deltaTime) {
    if (!visible_) return;

    // Update animation
    UpdateAnimation(deltaTime);

    // Update children
    for (auto& child : children_) {
        child->Update(deltaTime);
    }
}

void UIElement::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    // Apply opacity
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Render background
    RenderBackground(renderer);

    // Render border
    if (borderWidth_ > 0) {
        RenderBorder(renderer);
    }

    // Render children (sorted by layer)
    std::vector<std::shared_ptr<UIElement>> sortedChildren = children_;
    std::sort(sortedChildren.begin(), sortedChildren.end(),
              [](const auto& a, const auto& b) { return a->GetLayer() < b->GetLayer(); });

    for (auto& child : sortedChildren) {
        child->Render(renderer);
    }
}

void UIElement::HandleInput(const SDL_Event& event) {
    if (!visible_ || !interactive_ || !enabled_) return;

    // Handle mouse events
    if (event.type == SDL_MOUSEMOTION) {
        float mouseX = static_cast<float>(event.motion.x);
        float mouseY = static_cast<float>(event.motion.y);

        bool wasHovered = isHovered_;
        isHovered_ = ContainsPoint(mouseX, mouseY);

        if (isHovered_ && !wasHovered) {
            SetState(UIState::Hovered);
            if (onHover_) onHover_();
        } else if (!isHovered_ && wasHovered) {
            SetState(UIState::Normal);
        }

        if (isDragging_) {
            x_ = mouseX - dragStartX_;
            y_ = mouseY - dragStartY_;
            OnPositionChanged();
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
        float mouseX = static_cast<float>(event.button.x);
        float mouseY = static_cast<float>(event.button.y);

        if (ContainsPoint(mouseX, mouseY)) {
            isPressed_ = true;
            SetState(UIState::Pressed);

            if (event.button.clicks == 1 && onClick_) {
                onClick_();
            }

            // Check if draggable
            if (onDragStart_) {
                isDragging_ = true;
                dragStartX_ = mouseX - x_;
                dragStartY_ = mouseY - y_;
                onDragStart_();
            }
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
        if (isPressed_) {
            isPressed_ = false;
            SetState(isHovered_ ? UIState::Hovered : UIState::Normal);

            if (isDragging_ && onDragEnd_) {
                onDragEnd_();
            }
            isDragging_ = false;
        }
    }

    // Pass input to children
    for (auto& child : children_) {
        child->HandleInput(event);
    }
}

void UIElement::SetPosition(float x, float y) {
    x_ = x;
    y_ = y;
    OnPositionChanged();
}

void UIElement::SetSize(float width, float height) {
    width_ = width;
    height_ = height;
    OnSizeChanged();
}

void UIElement::SetAnchor(Anchor anchor) {
    anchor_ = anchor;
    OnPositionChanged();
}

void UIElement::SetPivot(float x, float y) {
    pivotX_ = std::clamp(x, 0.0f, 1.0f);
    pivotY_ = std::clamp(y, 0.0f, 1.0f);
}

void UIElement::GetScreenPosition(float& outX, float& outY) const {
    outX = x_;
    outY = y_;

    // Apply anchor offset if there's a parent
    if (parent_) {
        float parentX, parentY;
        parent_->GetScreenPosition(parentX, parentY);

        switch (anchor_) {
            case Anchor::TopLeft:
                outX += parentX;
                outY += parentY;
                break;
            case Anchor::TopCenter:
                outX += parentX + parent_->width_ / 2 - width_ / 2;
                outY += parentY;
                break;
            case Anchor::TopRight:
                outX += parentX + parent_->width_ - width_;
                outY += parentY;
                break;
            case Anchor::MiddleLeft:
                outX += parentX;
                outY += parentY + parent_->height_ / 2 - height_ / 2;
                break;
            case Anchor::Center:
                outX += parentX + parent_->width_ / 2 - width_ / 2;
                outY += parentY + parent_->height_ / 2 - height_ / 2;
                break;
            case Anchor::MiddleRight:
                outX += parentX + parent_->width_ - width_;
                outY += parentY + parent_->height_ / 2 - height_ / 2;
                break;
            case Anchor::BottomLeft:
                outX += parentX;
                outY += parentY + parent_->height_ - height_;
                break;
            case Anchor::BottomCenter:
                outX += parentX + parent_->width_ / 2 - width_ / 2;
                outY += parentY + parent_->height_ - height_;
                break;
            case Anchor::BottomRight:
                outX += parentX + parent_->width_ - width_;
                outY += parentY + parent_->height_ - height_;
                break;
        }
    }

    // Apply pivot
    outX -= width_ * pivotX_;
    outY -= height_ * pivotY_;
}

SDL_Rect UIElement::GetBounds() const {
    float screenX, screenY;
    GetScreenPosition(screenX, screenY);

    SDL_Rect rect;
    rect.x = static_cast<int>(screenX);
    rect.y = static_cast<int>(screenY);
    rect.w = static_cast<int>(width_);
    rect.h = static_cast<int>(height_);

    return rect;
}

void UIElement::AddChild(std::shared_ptr<UIElement> child) {
    if (child) {
        child->SetParent(this);
        children_.push_back(child);
    }
}

void UIElement::RemoveChild(const std::string& id) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
                      [&id](const auto& child) { return child->GetId() == id; }),
        children_.end()
    );
}

UIElement* UIElement::GetChild(const std::string& id) {
    for (auto& child : children_) {
        if (child->GetId() == id) {
            return child.get();
        }
    }
    return nullptr;
}

void UIElement::SetEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled) {
        SetState(UIState::Disabled);
    } else {
        SetState(UIState::Normal);
    }
}

void UIElement::SetState(UIState state) {
    if (state_ != state) {
        UIState oldState = state_;
        state_ = state;
        OnStateChanged(oldState, state);
    }
}

void UIElement::PlayAnimation(AnimationType type, float duration, bool loop) {
    animation_.type = type;
    animation_.duration = duration;
    animation_.elapsed = 0.0f;
    animation_.loop = loop;

    // Set start and end values based on animation type
    switch (type) {
        case AnimationType::FadeIn:
            animation_.startValue = 0.0f;
            animation_.endValue = opacity_;
            opacity_ = 0.0f;
            break;
        case AnimationType::FadeOut:
            animation_.startValue = opacity_;
            animation_.endValue = 0.0f;
            break;
        case AnimationType::Scale:
            animation_.startValue = 0.0f;
            animation_.endValue = 1.0f;
            break;
        default:
            animation_.startValue = 0.0f;
            animation_.endValue = 1.0f;
            break;
    }
}

void UIElement::StopAnimation() {
    animation_.type = AnimationType::None;
    animation_.elapsed = 0.0f;
}

bool UIElement::ContainsPoint(float x, float y) const {
    float screenX, screenY;
    GetScreenPosition(screenX, screenY);

    return x >= screenX && x <= screenX + width_ &&
           y >= screenY && y <= screenY + height_;
}

void UIElement::OnStateChanged(UIState oldState, UIState newState) {
    // Can be overridden by derived classes
}

void UIElement::OnPositionChanged() {
    // Can be overridden by derived classes
}

void UIElement::OnSizeChanged() {
    // Can be overridden by derived classes
}

void UIElement::RenderBackground(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();

    uint8_t alpha = static_cast<uint8_t>(backgroundColor_.a * opacity_);
    SDL_SetRenderDrawColor(renderer, backgroundColor_.r, backgroundColor_.g,
                          backgroundColor_.b, alpha);
    SDL_RenderFillRect(renderer, &bounds);
}

void UIElement::RenderBorder(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();

    uint8_t alpha = static_cast<uint8_t>(borderColor_.a * opacity_);
    SDL_SetRenderDrawColor(renderer, borderColor_.r, borderColor_.g,
                          borderColor_.b, alpha);

    for (int i = 0; i < static_cast<int>(borderWidth_); ++i) {
        SDL_Rect borderRect = {
            bounds.x - i,
            bounds.y - i,
            bounds.w + i * 2,
            bounds.h + i * 2
        };
        SDL_RenderDrawRect(renderer, &borderRect);
    }
}

void UIElement::UpdateAnimation(float deltaTime) {
    if (animation_.type == AnimationType::None) return;

    animation_.elapsed += deltaTime;

    if (animation_.IsComplete()) {
        if (animation_.loop) {
            animation_.elapsed = 0.0f;
        } else {
            if (animation_.onComplete) {
                animation_.onComplete();
            }
            StopAnimation();
            return;
        }
    }

    // Apply animation based on type
    float value = GetAnimationValue();

    switch (animation_.type) {
        case AnimationType::FadeIn:
        case AnimationType::FadeOut:
            opacity_ = value;
            break;
        case AnimationType::Scale: {
            float scale = value;
            // This is a simple scale, could be enhanced
            break;
        }
        default:
            break;
    }
}

float UIElement::GetAnimationValue() const {
    float t = animation_.GetEasedProgress();
    return animation_.startValue + (animation_.endValue - animation_.startValue) * t;
}

// UIUtils implementation
namespace UIUtils {

float EaseInOut(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float EaseIn(float t) {
    return t * t;
}

float EaseOut(float t) {
    return t * (2.0f - t);
}

float Bounce(float t) {
    if (t < 0.36363636f) {
        return 7.5625f * t * t;
    } else if (t < 0.72727273f) {
        t -= 0.54545455f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 0.90909091f) {
        t -= 0.81818182f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 0.95454545f;
        return 7.5625f * t * t + 0.984375f;
    }
}

void AlignHorizontal(std::vector<UIElement*> elements, float spacing) {
    float currentX = 0;
    for (auto* element : elements) {
        element->SetPosition(currentX, element->GetY());
        currentX += element->GetWidth() + spacing;
    }
}

void AlignVertical(std::vector<UIElement*> elements, float spacing) {
    float currentY = 0;
    for (auto* element : elements) {
        element->SetPosition(element->GetX(), currentY);
        currentY += element->GetHeight() + spacing;
    }
}

void DistributeHorizontal(std::vector<UIElement*> elements, float totalWidth) {
    if (elements.empty()) return;

    float totalElementWidth = 0;
    for (auto* element : elements) {
        totalElementWidth += element->GetWidth();
    }

    float spacing = (totalWidth - totalElementWidth) / (elements.size() - 1);
    AlignHorizontal(elements, spacing);
}

void DistributeVertical(std::vector<UIElement*> elements, float totalHeight) {
    if (elements.empty()) return;

    float totalElementHeight = 0;
    for (auto* element : elements) {
        totalElementHeight += element->GetHeight();
    }

    float spacing = (totalHeight - totalElementHeight) / (elements.size() - 1);
    AlignVertical(elements, spacing);
}

} // namespace UIUtils

} // namespace UI
