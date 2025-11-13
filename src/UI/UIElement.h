#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

// Forward declarations
struct SDL_Rect;
struct SDL_Renderer;

namespace UI {

// Input types
enum class InputType {
    Mouse,
    Keyboard,
    Gamepad
};

// UI Element states
enum class UIState {
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused
};

// Anchor points for responsive positioning
enum class Anchor {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    Center,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

// Animation types
enum class AnimationType {
    None,
    FadeIn,
    FadeOut,
    SlideIn,
    SlideOut,
    Scale,
    Bounce,
    Shake
};

// Structure for UI colors
struct Color {
    uint8_t r, g, b, a;

    Color(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    static Color FromHex(const std::string& hex);
    std::string ToHex() const;
};

// Structure for animations
struct Animation {
    AnimationType type;
    float duration;      // in seconds
    float elapsed;
    bool loop;
    float startValue;
    float endValue;
    std::function<void()> onComplete;

    Animation() : type(AnimationType::None), duration(0.0f),
                  elapsed(0.0f), loop(false),
                  startValue(0.0f), endValue(1.0f), onComplete(nullptr) {}

    bool IsComplete() const { return elapsed >= duration; }
    float GetProgress() const { return elapsed / duration; }
    float GetEasedProgress() const;
};

// Base class for all UI elements
class UIElement {
public:
    UIElement(const std::string& id = "");
    virtual ~UIElement();

    // Core methods
    virtual void Update(float deltaTime);
    virtual void Render(SDL_Renderer* renderer);
    virtual void HandleInput(const SDL_Event& event);

    // Position and size
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetAnchor(Anchor anchor);
    void SetPivot(float x, float y); // 0-1 range

    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetWidth() const { return width_; }
    float GetHeight() const { return height_; }

    // Get actual screen position based on anchor and parent
    void GetScreenPosition(float& outX, float& outY) const;
    SDL_Rect GetBounds() const;

    // Hierarchy
    void AddChild(std::shared_ptr<UIElement> child);
    void RemoveChild(const std::string& id);
    UIElement* GetChild(const std::string& id);
    UIElement* GetParent() const { return parent_; }
    void SetParent(UIElement* parent) { parent_ = parent; }

    // Visibility and interaction
    void SetVisible(bool visible) { visible_ = visible; }
    bool IsVisible() const { return visible_; }
    void SetInteractive(bool interactive) { interactive_ = interactive; }
    bool IsInteractive() const { return interactive_; }
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return enabled_; }

    // State management
    UIState GetState() const { return state_; }
    void SetState(UIState state);

    // Styling
    void SetBackgroundColor(const Color& color) { backgroundColor_ = color; }
    void SetBorderColor(const Color& color) { borderColor_ = color; }
    void SetBorderWidth(float width) { borderWidth_ = width; }
    void SetOpacity(float opacity) { opacity_ = opacity; }
    void SetPadding(float padding) { padding_ = padding; }
    void SetMargin(float margin) { margin_ = margin; }

    Color GetBackgroundColor() const { return backgroundColor_; }
    float GetOpacity() const { return opacity_; }

    // Animation
    void PlayAnimation(AnimationType type, float duration, bool loop = false);
    void StopAnimation();
    bool IsAnimating() const { return animation_.type != AnimationType::None; }

    // Event callbacks
    void OnClick(std::function<void()> callback) { onClick_ = callback; }
    void OnHover(std::function<void()> callback) { onHover_ = callback; }
    void OnFocus(std::function<void()> callback) { onFocus_ = callback; }
    void OnDragStart(std::function<void()> callback) { onDragStart_ = callback; }
    void OnDragEnd(std::function<void()> callback) { onDragEnd_ = callback; }

    // Utility
    bool ContainsPoint(float x, float y) const;
    std::string GetId() const { return id_; }
    void SetId(const std::string& id) { id_ = id; }

    // Layer/depth for rendering order
    void SetLayer(int layer) { layer_ = layer; }
    int GetLayer() const { return layer_; }

protected:
    // Protected methods for derived classes
    virtual void OnStateChanged(UIState oldState, UIState newState);
    virtual void OnPositionChanged();
    virtual void OnSizeChanged();

    // Helper for rendering
    void RenderBackground(SDL_Renderer* renderer);
    void RenderBorder(SDL_Renderer* renderer);

    // Animation update
    void UpdateAnimation(float deltaTime);
    float GetAnimationValue() const;

protected:
    // Core properties
    std::string id_;
    float x_, y_;
    float width_, height_;
    Anchor anchor_;
    float pivotX_, pivotY_;

    // Hierarchy
    UIElement* parent_;
    std::vector<std::shared_ptr<UIElement>> children_;

    // State
    UIState state_;
    bool visible_;
    bool interactive_;
    bool enabled_;
    int layer_;

    // Styling
    Color backgroundColor_;
    Color borderColor_;
    float borderWidth_;
    float opacity_;
    float padding_;
    float margin_;

    // Animation
    Animation animation_;

    // Event callbacks
    std::function<void()> onClick_;
    std::function<void()> onHover_;
    std::function<void()> onFocus_;
    std::function<void()> onDragStart_;
    std::function<void()> onDragEnd_;

    // Input tracking
    bool isHovered_;
    bool isPressed_;
    bool isDragging_;
    float dragStartX_, dragStartY_;
};

// Utility functions for UI
namespace UIUtils {
    // Easing functions
    float EaseInOut(float t);
    float EaseIn(float t);
    float EaseOut(float t);
    float Bounce(float t);

    // Layout helpers
    void AlignHorizontal(std::vector<UIElement*> elements, float spacing);
    void AlignVertical(std::vector<UIElement*> elements, float spacing);
    void DistributeHorizontal(std::vector<UIElement*> elements, float totalWidth);
    void DistributeVertical(std::vector<UIElement*> elements, float totalHeight);
}

} // namespace UI
