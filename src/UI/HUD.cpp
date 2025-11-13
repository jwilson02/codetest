#include "HUD.h"
#include "UISystem.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace UI {

// ResourceBar implementation
ResourceBar::ResourceBar(ResourceType type, const std::string& id)
    : UIElement(id.empty() ? "resourceBar" : id),
      type_(type), currentValue_(100), maxValue_(100),
      displayValue_(100) {

    auto& theme = UISystem::Instance().GetTheme();

    // Set colors based on type
    switch (type_) {
        case ResourceType::Health:
            fillColor_ = Color::FromHex("#E74C3C");
            bgColor_ = Color::FromHex("#4A0000");
            label_ = "HP";
            break;
        case ResourceType::Mana:
            fillColor_ = Color::FromHex("#3498DB");
            bgColor_ = Color::FromHex("#00004A");
            label_ = "MP";
            break;
        case ResourceType::Stamina:
            fillColor_ = Color::FromHex("#F39C12");
            bgColor_ = Color::FromHex("#4A3000");
            label_ = "SP";
            break;
        case ResourceType::Experience:
            fillColor_ = Color::FromHex("#9B59B6");
            bgColor_ = Color::FromHex("#2C003E");
            label_ = "XP";
            break;
    }

    SetBackgroundColor(bgColor_);
    SetBorderColor(Color(0, 0, 0, 255));
    SetBorderWidth(2.0f);
}

void ResourceBar::Update(float deltaTime) {
    UIElement::Update(deltaTime);

    // Smooth value transition
    float diff = currentValue_ - displayValue_;
    if (std::abs(diff) > 0.1f) {
        displayValue_ += diff * deltaTime * 5.0f;
    } else {
        displayValue_ = currentValue_;
    }
}

void ResourceBar::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    RenderBar(renderer);
    RenderLabel(renderer);
}

void ResourceBar::SetValue(float current, float max) {
    currentValue_ = std::max(0.0f, std::min(current, max));
    maxValue_ = std::max(1.0f, max);
}

void ResourceBar::SetColors(const Color& fillColor, const Color& bgColor) {
    fillColor_ = fillColor;
    bgColor_ = bgColor;
    SetBackgroundColor(bgColor);
}

void ResourceBar::RenderBar(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();

    // Render background
    SDL_SetRenderDrawColor(renderer, bgColor_.r, bgColor_.g, bgColor_.b, bgColor_.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Render fill
    float percentage = GetPercentage();
    SDL_Rect fillRect = bounds;
    fillRect.w = static_cast<int>(bounds.w * percentage);

    SDL_SetRenderDrawColor(renderer, fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a);
    SDL_RenderFillRect(renderer, &fillRect);

    // Render smooth display value (lighter overlay)
    float displayPercentage = displayValue_ / maxValue_;
    if (displayPercentage > percentage) {
        SDL_Rect displayRect = bounds;
        displayRect.x = fillRect.x + fillRect.w;
        displayRect.w = static_cast<int>(bounds.w * (displayPercentage - percentage));

        Color lightColor = fillColor_;
        lightColor.a = 128;
        SDL_SetRenderDrawColor(renderer, lightColor.r, lightColor.g, lightColor.b, lightColor.a);
        SDL_RenderFillRect(renderer, &displayRect);
    }

    // Render border
    SDL_SetRenderDrawColor(renderer, borderColor_.r, borderColor_.g,
                          borderColor_.b, borderColor_.a);
    SDL_RenderDrawRect(renderer, &bounds);
}

void ResourceBar::RenderLabel(SDL_Renderer* renderer) {
    // Placeholder - would use TTF fonts in production
    // Text would show: "HP: 450/500" etc.
}

// SkillBar implementation
SkillBar::SkillBar(int numSlots)
    : UIElement("skillBar"), numSlots_(numSlots), hoveredSlot_(-1) {

    skills_.resize(numSlots);
    slotElements_.resize(numSlots);

    auto& theme = UISystem::Instance().GetTheme();

    // Create skill slot elements
    float slotSize = 50.0f;
    float spacing = 5.0f;

    for (int i = 0; i < numSlots_; ++i) {
        auto slot = std::make_shared<UIElement>("skillSlot_" + std::to_string(i));
        slot->SetSize(slotSize, slotSize);
        slot->SetPosition(i * (slotSize + spacing), 0);
        slot->SetBackgroundColor(Color(40, 40, 40, 200));
        slot->SetBorderColor(Color(100, 100, 100, 255));
        slot->SetBorderWidth(2.0f);

        // Set up hotkey display (1-0)
        skills_[i].hotkey = std::to_string((i + 1) % 10);

        slotElements_[i] = slot;
        AddChild(slot);
    }

    SetSize(numSlots * (slotSize + spacing) - spacing, slotSize);
}

void SkillBar::Update(float deltaTime) {
    UIElement::Update(deltaTime);

    // Update cooldowns
    for (auto& skill : skills_) {
        if (skill.cooldown > 0) {
            skill.cooldown -= deltaTime;
            if (skill.cooldown < 0) {
                skill.cooldown = 0;
            }
        }
    }
}

void SkillBar::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    UIElement::Render(renderer);

    for (int i = 0; i < numSlots_; ++i) {
        RenderSkillSlot(renderer, i);
        if (skills_[i].cooldown > 0) {
            RenderCooldown(renderer, i);
        }
        RenderHotkey(renderer, i);
    }
}

void SkillBar::HandleInput(const SDL_Event& event) {
    UIElement::HandleInput(event);

    // Keyboard hotkeys
    if (event.type == SDL_KEYDOWN) {
        int slot = -1;
        if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) {
            slot = event.key.keysym.sym - SDLK_1;
        } else if (event.key.keysym.sym == SDLK_0) {
            slot = 9;
        }

        if (slot >= 0 && slot < numSlots_) {
            TriggerSkill(slot);
        }
    }
}

void SkillBar::SetSkill(int slot, const SkillSlot& skill) {
    if (slot >= 0 && slot < numSlots_) {
        skills_[slot] = skill;
    }
}

void SkillBar::ClearSkill(int slot) {
    if (slot >= 0 && slot < numSlots_) {
        skills_[slot] = SkillSlot();
    }
}

const SkillSlot* SkillBar::GetSkill(int slot) const {
    if (slot >= 0 && slot < numSlots_) {
        return &skills_[slot];
    }
    return nullptr;
}

void SkillBar::UpdateCooldown(int slot, float cooldown) {
    if (slot >= 0 && slot < numSlots_) {
        skills_[slot].cooldown = cooldown;
    }
}

void SkillBar::TriggerSkill(int slot) {
    if (slot >= 0 && slot < numSlots_) {
        if (!skills_[slot].skillId.empty() && skills_[slot].cooldown <= 0) {
            if (onSkillActivated_) {
                onSkillActivated_(slot, skills_[slot].skillId);
            }
            skills_[slot].cooldown = skills_[slot].maxCooldown;
        }
    }
}

void SkillBar::RenderSkillSlot(SDL_Renderer* renderer, int slot) {
    if (!slotElements_[slot]) return;

    // Icon would be rendered here
    // For now, just highlight if skill is available

    if (!skills_[slot].skillId.empty()) {
        SDL_Rect bounds = slotElements_[slot]->GetBounds();
        bounds.x += 5;
        bounds.y += 5;
        bounds.w -= 10;
        bounds.h -= 10;

        Color iconColor = skills_[slot].cooldown > 0 ?
                         Color(100, 100, 100, 255) : Color(200, 200, 50, 255);

        SDL_SetRenderDrawColor(renderer, iconColor.r, iconColor.g,
                              iconColor.b, iconColor.a);
        SDL_RenderFillRect(renderer, &bounds);
    }
}

void SkillBar::RenderCooldown(SDL_Renderer* renderer, int slot) {
    if (slot < 0 || slot >= numSlots_) return;

    float percentage = skills_[slot].cooldown / skills_[slot].maxCooldown;
    if (percentage <= 0) return;

    SDL_Rect bounds = slotElements_[slot]->GetBounds();

    // Render cooldown overlay
    SDL_Rect cooldownRect = bounds;
    cooldownRect.h = static_cast<int>(bounds.h * percentage);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &cooldownRect);
}

void SkillBar::RenderHotkey(SDL_Renderer* renderer, int slot) {
    // Placeholder - would render hotkey text in bottom-right corner
    // Using TTF fonts: "1", "2", "3", etc.
}

// Minimap implementation
Minimap::Minimap()
    : UIElement("minimap"), mapSize_(200), fogOfWarEnabled_(true),
      isPinned_(true) {

    SetSize(mapSize_, mapSize_);
    SetBackgroundColor(Color(20, 20, 20, 230));
    SetBorderColor(Color(100, 100, 100, 255));
    SetBorderWidth(3.0f);

    fogColor_ = Color(0, 0, 0, 200);
    playerColor_ = Color::FromHex("#00FF00");
    enemyColor_ = Color::FromHex("#FF0000");
    npcColor_ = Color::FromHex("#FFFF00");
    objectiveColor_ = Color::FromHex("#00FFFF");

    data_.zoomLevel = 1.0f;
}

void Minimap::Update(float deltaTime) {
    UIElement::Update(deltaTime);
}

void Minimap::Render(SDL_Renderer* renderer) {
    if (!visible_) return;

    RenderMap(renderer);

    if (fogOfWarEnabled_) {
        RenderFogOfWar(renderer);
    }

    RenderEntities(renderer);
    RenderPlayer(renderer);
    RenderBorder(renderer);
}

void Minimap::HandleInput(const SDL_Event& event) {
    UIElement::HandleInput(event);

    // Mouse wheel for zoom
    if (event.type == SDL_MOUSEWHEEL) {
        if (ContainsPoint(static_cast<float>(event.wheel.mouseX),
                         static_cast<float>(event.wheel.mouseY))) {
            data_.zoomLevel += event.wheel.y * 0.1f;
            data_.zoomLevel = std::clamp(data_.zoomLevel, 0.5f, 3.0f);
        }
    }
}

void Minimap::SetMinimapData(const MinimapData& data) {
    data_ = data;
}

void Minimap::SetZoom(float zoom) {
    data_.zoomLevel = std::clamp(zoom, 0.5f, 3.0f);
}

void Minimap::RenderMap(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();
    SDL_SetRenderDrawColor(renderer, backgroundColor_.r, backgroundColor_.g,
                          backgroundColor_.b, backgroundColor_.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Render revealed tiles
    for (const auto& tile : data_.revealedTiles) {
        float mapX, mapY;
        WorldToMinimap(tile.first, tile.second, mapX, mapY);

        SDL_Rect tileRect;
        tileRect.x = static_cast<int>(mapX - 1);
        tileRect.y = static_cast<int>(mapY - 1);
        tileRect.w = 2;
        tileRect.h = 2;

        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderFillRect(renderer, &tileRect);
    }
}

void Minimap::RenderFogOfWar(SDL_Renderer* renderer) {
    // Render fog over unexplored areas
    // This is simplified - in production would use a proper fog texture

    SDL_Rect bounds = GetBounds();

    // Create a grid of fog tiles
    int gridSize = 10;
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            // Check if this area is revealed
            bool isRevealed = false;
            // Simplified check - would be more sophisticated in production

            if (!isRevealed) {
                SDL_Rect fogTile;
                fogTile.x = bounds.x + (x * bounds.w / gridSize);
                fogTile.y = bounds.y + (y * bounds.h / gridSize);
                fogTile.w = bounds.w / gridSize;
                fogTile.h = bounds.h / gridSize;

                SDL_SetRenderDrawColor(renderer, fogColor_.r, fogColor_.g,
                                      fogColor_.b, fogColor_.a);
                SDL_RenderFillRect(renderer, &fogTile);
            }
        }
    }
}

void Minimap::RenderEntities(SDL_Renderer* renderer) {
    SDL_Rect bounds = GetBounds();

    // Render enemies
    for (const auto& enemy : data_.enemyPositions) {
        float mapX, mapY;
        WorldToMinimap(enemy.first, enemy.second, mapX, mapY);

        SDL_Rect enemyRect;
        enemyRect.x = static_cast<int>(mapX - 3);
        enemyRect.y = static_cast<int>(mapY - 3);
        enemyRect.w = 6;
        enemyRect.h = 6;

        SDL_SetRenderDrawColor(renderer, enemyColor_.r, enemyColor_.g,
                              enemyColor_.b, enemyColor_.a);
        SDL_RenderFillRect(renderer, &enemyRect);
    }

    // Render NPCs
    for (const auto& npc : data_.npcPositions) {
        float mapX, mapY;
        WorldToMinimap(npc.first, npc.second, mapX, mapY);

        SDL_Rect npcRect;
        npcRect.x = static_cast<int>(mapX - 2);
        npcRect.y = static_cast<int>(mapY - 2);
        npcRect.w = 4;
        npcRect.h = 4;

        SDL_SetRenderDrawColor(renderer, npcColor_.r, npcColor_.g,
                              npcColor_.b, npcColor_.a);
        SDL_RenderFillRect(renderer, &npcRect);
    }

    // Render objectives
    for (const auto& obj : data_.objectivePositions) {
        float mapX, mapY;
        WorldToMinimap(obj.first, obj.second, mapX, mapY);

        // Draw star/marker
        SDL_Rect objRect;
        objRect.x = static_cast<int>(mapX - 4);
        objRect.y = static_cast<int>(mapY - 4);
        objRect.w = 8;
        objRect.h = 8;

        SDL_SetRenderDrawColor(renderer, objectiveColor_.r, objectiveColor_.g,
                              objectiveColor_.b, objectiveColor_.a);
        SDL_RenderFillRect(renderer, &objRect);
    }
}

void Minimap::RenderPlayer(SDL_Renderer* renderer) {
    float mapX, mapY;
    WorldToMinimap(data_.playerX, data_.playerY, mapX, mapY);

    // Render player as triangle pointing in direction
    SDL_Rect playerRect;
    playerRect.x = static_cast<int>(mapX - 4);
    playerRect.y = static_cast<int>(mapY - 4);
    playerRect.w = 8;
    playerRect.h = 8;

    SDL_SetRenderDrawColor(renderer, playerColor_.r, playerColor_.g,
                          playerColor_.b, playerColor_.a);
    SDL_RenderFillRect(renderer, &playerRect);

    // Render direction indicator
    float dirX = std::cos(data_.playerRotation) * 10;
    float dirY = std::sin(data_.playerRotation) * 10;

    SDL_SetRenderDrawColor(renderer, playerColor_.r, playerColor_.g,
                          playerColor_.b, 255);
    SDL_RenderDrawLine(renderer,
                      static_cast<int>(mapX),
                      static_cast<int>(mapY),
                      static_cast<int>(mapX + dirX),
                      static_cast<int>(mapY + dirY));
}

void Minimap::WorldToMinimap(float worldX, float worldY, float& mapX, float& mapY) {
    SDL_Rect bounds = GetBounds();

    // Center on player with zoom
    float relX = (worldX - data_.playerX) * data_.zoomLevel;
    float relY = (worldY - data_.playerY) * data_.zoomLevel;

    mapX = bounds.x + bounds.w / 2 + relX;
    mapY = bounds.y + bounds.h / 2 + relY;
}

// HUD implementation
HUD::HUD()
    : UIElement("hud"), playerLevel_(1), targetHealth_(0), targetMaxHealth_(0),
      hasTarget_(false), hudVisible_(true) {

    auto& uiSystem = UISystem::Instance();
    SetSize(static_cast<float>(uiSystem.GetScreenWidth()),
            static_cast<float>(uiSystem.GetScreenHeight()));
    SetPosition(0, 0);
    SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent

    CreateResourceBars();
    CreateSkillBar();
    CreateMinimap();
    CreateTargetFrame();
    CreateBuffBar();
}

HUD::~HUD() {
}

void HUD::Update(float deltaTime) {
    if (!hudVisible_) return;

    UIElement::Update(deltaTime);

    if (healthBar_) healthBar_->Update(deltaTime);
    if (manaBar_) manaBar_->Update(deltaTime);
    if (staminaBar_) staminaBar_->Update(deltaTime);
    if (experienceBar_) experienceBar_->Update(deltaTime);
    if (skillBar_) skillBar_->Update(deltaTime);
    if (minimap_) minimap_->Update(deltaTime);

    // Update buffs
    for (auto it = buffs_.begin(); it != buffs_.end();) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration) {
            it = buffs_.erase(it);
        } else {
            ++it;
        }
    }

    // Update combat text
    for (auto it = combatTexts_.begin(); it != combatTexts_.end();) {
        it->elapsed += deltaTime;
        it->y += it->velocityY * deltaTime;
        it->velocityY -= 50.0f * deltaTime; // Gravity

        if (it->elapsed >= it->duration) {
            it = combatTexts_.erase(it);
        } else {
            ++it;
        }
    }
}

void HUD::Render(SDL_Renderer* renderer) {
    if (!hudVisible_) return;

    RenderPlayerFrame(renderer);

    if (healthBar_) healthBar_->Render(renderer);
    if (manaBar_) manaBar_->Render(renderer);
    if (staminaBar_) staminaBar_->Render(renderer);
    if (experienceBar_) experienceBar_->Render(renderer);
    if (skillBar_) skillBar_->Render(renderer);
    if (minimap_) minimap_->Render(renderer);

    if (hasTarget_) {
        RenderTargetFrame(renderer);
    }

    RenderBuffBar(renderer);
    RenderCombatText(renderer);

    UIElement::Render(renderer);
}

void HUD::HandleInput(const SDL_Event& event) {
    if (!hudVisible_) return;

    UIElement::HandleInput(event);

    if (skillBar_) skillBar_->HandleInput(event);
    if (minimap_) minimap_->HandleInput(event);
}

void HUD::SetHealth(float current, float max) {
    if (healthBar_) {
        healthBar_->SetValue(current, max);
    }
}

void HUD::SetMana(float current, float max) {
    if (manaBar_) {
        manaBar_->SetValue(current, max);
    }
}

void HUD::SetStamina(float current, float max) {
    if (staminaBar_) {
        staminaBar_->SetValue(current, max);
    }
}

void HUD::SetExperience(float current, float max) {
    if (experienceBar_) {
        experienceBar_->SetValue(current, max);
    }
}

void HUD::SetTarget(const std::string& name, float health, float maxHealth) {
    targetName_ = name;
    targetHealth_ = health;
    targetMaxHealth_ = maxHealth;
    hasTarget_ = true;
}

void HUD::ClearTarget() {
    hasTarget_ = false;
    targetName_.clear();
}

void HUD::UpdateMinimap(const MinimapData& data) {
    if (minimap_) {
        minimap_->SetMinimapData(data);
    }
}

void HUD::AddBuff(const std::string& buffId, const std::string& iconPath,
                  float duration, bool isBuff) {
    BuffInfo buff;
    buff.id = buffId;
    buff.iconPath = iconPath;
    buff.duration = duration;
    buff.elapsed = 0.0f;
    buff.isBuff = isBuff;
    buffs_.push_back(buff);
}

void HUD::RemoveBuff(const std::string& buffId) {
    buffs_.erase(
        std::remove_if(buffs_.begin(), buffs_.end(),
                      [&buffId](const BuffInfo& buff) { return buff.id == buffId; }),
        buffs_.end()
    );
}

void HUD::ClearBuffs() {
    buffs_.clear();
}

void HUD::ShowCombatText(const std::string& text, float x, float y,
                        const Color& color, bool isCritical) {
    CombatText ct;
    ct.text = text;
    ct.x = x;
    ct.y = y;
    ct.color = color;
    ct.duration = 2.0f;
    ct.elapsed = 0.0f;
    ct.isCritical = isCritical;
    ct.velocityY = -100.0f; // Move upward
    combatTexts_.push_back(ct);
}

void HUD::ShowNotification(const std::string& text, float duration) {
    UISystem::Instance().ShowNotification(text, duration);
}

void HUD::SetHUDVisible(bool visible) {
    hudVisible_ = visible;
    SetVisible(visible);
}

void HUD::ToggleHUD() {
    SetHUDVisible(!hudVisible_);
}

void HUD::CreateResourceBars() {
    float barWidth = 300;
    float barHeight = 25;
    float spacing = 5;
    float xPos = 20;
    float yPos = 20;

    // Health bar
    healthBar_ = std::make_shared<ResourceBar>(ResourceType::Health, "healthBar");
    healthBar_->SetSize(barWidth, barHeight);
    healthBar_->SetPosition(xPos, yPos);
    AddChild(healthBar_);

    // Mana bar
    manaBar_ = std::make_shared<ResourceBar>(ResourceType::Mana, "manaBar");
    manaBar_->SetSize(barWidth, barHeight);
    manaBar_->SetPosition(xPos, yPos + barHeight + spacing);
    AddChild(manaBar_);

    // Stamina bar
    staminaBar_ = std::make_shared<ResourceBar>(ResourceType::Stamina, "staminaBar");
    staminaBar_->SetSize(barWidth, barHeight);
    staminaBar_->SetPosition(xPos, yPos + (barHeight + spacing) * 2);
    AddChild(staminaBar_);

    // Experience bar (at bottom of screen)
    experienceBar_ = std::make_shared<ResourceBar>(ResourceType::Experience, "experienceBar");
    experienceBar_->SetSize(width_, 10);
    experienceBar_->SetPosition(0, height_ - 10);
    AddChild(experienceBar_);
}

void HUD::CreateSkillBar() {
    skillBar_ = std::make_shared<SkillBar>(10);
    skillBar_->SetPosition(width_ / 2 - skillBar_->GetWidth() / 2, height_ - 80);
    AddChild(skillBar_);
}

void HUD::CreateMinimap() {
    minimap_ = std::make_shared<Minimap>();
    minimap_->SetSize(200);
    minimap_->SetPosition(width_ - 220, 20);
    AddChild(minimap_);
}

void HUD::CreateTargetFrame() {
    targetFrame_ = std::make_shared<UIElement>("targetFrame");
    targetFrame_->SetSize(300, 60);
    targetFrame_->SetPosition(width_ / 2 - 150, 20);
    targetFrame_->SetBackgroundColor(Color(20, 20, 20, 200));
    targetFrame_->SetBorderColor(Color(200, 0, 0, 255));
    targetFrame_->SetBorderWidth(2.0f);
    targetFrame_->SetVisible(false);
    AddChild(targetFrame_);
}

void HUD::CreateBuffBar() {
    buffBar_ = std::make_shared<UIElement>("buffBar");
    buffBar_->SetSize(400, 40);
    buffBar_->SetPosition(width_ / 2 - 200, 100);
    buffBar_->SetBackgroundColor(Color(0, 0, 0, 0)); // Transparent
    AddChild(buffBar_);
}

void HUD::RenderPlayerFrame(SDL_Renderer* renderer) {
    // Additional player frame rendering (portrait, etc.)
}

void HUD::RenderTargetFrame(SDL_Renderer* renderer) {
    if (!hasTarget_ || !targetFrame_) return;

    targetFrame_->SetVisible(true);
    targetFrame_->Render(renderer);

    // Render target health bar
    SDL_Rect bounds = targetFrame_->GetBounds();
    SDL_Rect healthRect = bounds;
    healthRect.x += 10;
    healthRect.y += 30;
    healthRect.w -= 20;
    healthRect.h = 20;

    // Background
    SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);

    // Fill
    float percentage = targetMaxHealth_ > 0 ? targetHealth_ / targetMaxHealth_ : 0;
    healthRect.w = static_cast<int>((bounds.w - 20) * percentage);
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);
}

void HUD::RenderBuffBar(SDL_Renderer* renderer) {
    if (buffs_.empty()) return;

    float iconSize = 35;
    float spacing = 5;
    float xPos = buffBar_->GetX();
    float yPos = buffBar_->GetY();

    for (size_t i = 0; i < buffs_.size(); ++i) {
        SDL_Rect iconRect;
        iconRect.x = static_cast<int>(xPos + i * (iconSize + spacing));
        iconRect.y = static_cast<int>(yPos);
        iconRect.w = static_cast<int>(iconSize);
        iconRect.h = static_cast<int>(iconSize);

        // Background
        Color bgColor = buffs_[i].isBuff ?
                       Color::FromHex("#00FF00") : Color::FromHex("#FF0000");
        bgColor.a = 150;

        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &iconRect);

        // Border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &iconRect);

        // Duration overlay
        float timeLeft = buffs_[i].duration - buffs_[i].elapsed;
        if (timeLeft < 5.0f) {
            // Blink when nearly expired
            if (static_cast<int>(timeLeft * 4) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
                SDL_RenderFillRect(renderer, &iconRect);
            }
        }
    }
}

void HUD::RenderCombatText(SDL_Renderer* renderer) {
    for (const auto& text : combatTexts_) {
        // Placeholder - would render text using TTF fonts
        // Size would be larger for critical hits
        // Color would fade out over time

        float alpha = 1.0f - (text.elapsed / text.duration);
        Color color = text.color;
        color.a = static_cast<uint8_t>(255 * alpha);

        // Simple rect as placeholder
        SDL_Rect textRect;
        textRect.x = static_cast<int>(text.x);
        textRect.y = static_cast<int>(text.y);
        textRect.w = text.isCritical ? 60 : 40;
        textRect.h = text.isCritical ? 30 : 20;

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(renderer, &textRect);
    }
}

void HUD::OnPositionChanged() {
    UIElement::OnPositionChanged();
}

void HUD::OnSizeChanged() {
    UIElement::OnSizeChanged();
}

} // namespace UI
