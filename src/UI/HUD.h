#pragma once

#include "UIElement.h"
#include <array>
#include <vector>

namespace UI {

// Resource bar types
enum class ResourceType {
    Health,
    Mana,
    Stamina,
    Experience
};

// Skill slot data
struct SkillSlot {
    std::string skillId;
    std::string iconPath;
    std::string hotkey;
    float cooldown;
    float maxCooldown;
    int charges;
    int maxCharges;
    bool isActive;

    SkillSlot()
        : cooldown(0.0f), maxCooldown(0.0f),
          charges(1), maxCharges(1), isActive(false) {}
};

// Minimap data
struct MinimapData {
    std::vector<std::pair<float, float>> revealedTiles; // x, y positions
    std::vector<std::pair<float, float>> enemyPositions;
    std::vector<std::pair<float, float>> npcPositions;
    std::vector<std::pair<float, float>> objectivePositions;
    float playerX, playerY;
    float playerRotation;
    float zoomLevel;

    MinimapData()
        : playerX(0), playerY(0), playerRotation(0), zoomLevel(1.0f) {}
};

// Resource bar UI component
class ResourceBar : public UIElement {
public:
    ResourceBar(ResourceType type, const std::string& id = "");

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;

    void SetValue(float current, float max);
    void SetColors(const Color& fillColor, const Color& bgColor);

    float GetCurrentValue() const { return currentValue_; }
    float GetMaxValue() const { return maxValue_; }
    float GetPercentage() const { return maxValue_ > 0 ? currentValue_ / maxValue_ : 0; }

private:
    void RenderBar(SDL_Renderer* renderer);
    void RenderLabel(SDL_Renderer* renderer);

private:
    ResourceType type_;
    float currentValue_;
    float maxValue_;
    float displayValue_; // For smooth animation
    Color fillColor_;
    Color bgColor_;
    std::string label_;
};

// Skill bar UI component
class SkillBar : public UIElement {
public:
    SkillBar(int numSlots = 10);

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    void SetSkill(int slot, const SkillSlot& skill);
    void ClearSkill(int slot);
    const SkillSlot* GetSkill(int slot) const;

    void UpdateCooldown(int slot, float cooldown);
    void TriggerSkill(int slot);

    // Callbacks
    void OnSkillActivated(std::function<void(int, const std::string&)> callback) {
        onSkillActivated_ = callback;
    }

private:
    void RenderSkillSlot(SDL_Renderer* renderer, int slot);
    void RenderCooldown(SDL_Renderer* renderer, int slot);
    void RenderHotkey(SDL_Renderer* renderer, int slot);

private:
    std::vector<SkillSlot> skills_;
    std::vector<std::shared_ptr<UIElement>> slotElements_;
    int numSlots_;
    int hoveredSlot_;
    std::function<void(int, const std::string&)> onSkillActivated_;
};

// Minimap UI component
class Minimap : public UIElement {
public:
    Minimap();

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    void SetMinimapData(const MinimapData& data);
    void SetSize(float size) { mapSize_ = size; SetSize(size, size); }
    void SetZoom(float zoom);

    void ToggleFogOfWar(bool enabled) { fogOfWarEnabled_ = enabled; }
    bool IsFogOfWarEnabled() const { return fogOfWarEnabled_; }

    void SetInteractive(bool interactive) { interactive_ = interactive; }

private:
    void RenderMap(SDL_Renderer* renderer);
    void RenderFogOfWar(SDL_Renderer* renderer);
    void RenderEntities(SDL_Renderer* renderer);
    void RenderPlayer(SDL_Renderer* renderer);
    void RenderBorder(SDL_Renderer* renderer);

    // Convert world coordinates to minimap coordinates
    void WorldToMinimap(float worldX, float worldY, float& mapX, float& mapY);

private:
    MinimapData data_;
    float mapSize_;
    bool fogOfWarEnabled_;
    bool isPinned_;
    Color fogColor_;
    Color playerColor_;
    Color enemyColor_;
    Color npcColor_;
    Color objectiveColor_;
};

// Main HUD class
class HUD : public UIElement {
public:
    HUD();
    ~HUD() override;

    void Update(float deltaTime) override;
    void Render(SDL_Renderer* renderer) override;
    void HandleInput(const SDL_Event& event) override;

    // Resource management
    void SetHealth(float current, float max);
    void SetMana(float current, float max);
    void SetStamina(float current, float max);
    void SetExperience(float current, float max);

    // Player info
    void SetPlayerLevel(int level) { playerLevel_ = level; }
    void SetPlayerName(const std::string& name) { playerName_ = name; }

    // Target info
    void SetTarget(const std::string& name, float health, float maxHealth);
    void ClearTarget();
    bool HasTarget() const { return hasTarget_; }

    // Skill bar
    SkillBar* GetSkillBar() const { return skillBar_.get(); }

    // Minimap
    Minimap* GetMinimap() const { return minimap_.get(); }
    void UpdateMinimap(const MinimapData& data);

    // Buffs/Debuffs
    void AddBuff(const std::string& buffId, const std::string& iconPath,
                 float duration, bool isBuff = true);
    void RemoveBuff(const std::string& buffId);
    void ClearBuffs();

    // Combat text (damage numbers, etc.)
    void ShowCombatText(const std::string& text, float x, float y,
                       const Color& color, bool isCritical = false);

    // Notifications
    void ShowNotification(const std::string& text, float duration = 3.0f);

    // HUD visibility
    void SetHUDVisible(bool visible);
    void ToggleHUD();

private:
    void CreateResourceBars();
    void CreateSkillBar();
    void CreateMinimap();
    void CreateTargetFrame();
    void CreateBuffBar();

    void RenderPlayerFrame(SDL_Renderer* renderer);
    void RenderTargetFrame(SDL_Renderer* renderer);
    void RenderBuffBar(SDL_Renderer* renderer);
    void RenderCombatText(SDL_Renderer* renderer);

    void OnPositionChanged() override;
    void OnSizeChanged() override;

private:
    // Resource bars
    std::shared_ptr<ResourceBar> healthBar_;
    std::shared_ptr<ResourceBar> manaBar_;
    std::shared_ptr<ResourceBar> staminaBar_;
    std::shared_ptr<ResourceBar> experienceBar_;

    // Skill bar
    std::shared_ptr<SkillBar> skillBar_;

    // Minimap
    std::shared_ptr<Minimap> minimap_;

    // Player info
    std::string playerName_;
    int playerLevel_;

    // Target info
    std::string targetName_;
    float targetHealth_;
    float targetMaxHealth_;
    bool hasTarget_;
    std::shared_ptr<UIElement> targetFrame_;

    // Buffs/Debuffs
    struct BuffInfo {
        std::string id;
        std::string iconPath;
        float duration;
        float elapsed;
        bool isBuff;
    };
    std::vector<BuffInfo> buffs_;
    std::shared_ptr<UIElement> buffBar_;

    // Combat text
    struct CombatText {
        std::string text;
        float x, y;
        Color color;
        float duration;
        float elapsed;
        bool isCritical;
        float velocityY;
    };
    std::vector<CombatText> combatTexts_;

    // Layout
    bool hudVisible_;
};

} // namespace UI
