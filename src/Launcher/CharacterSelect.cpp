#include "CharacterSelect.h"
#include <cmath>
#include <algorithm>

namespace Launcher {

CharacterSelect::CharacterSelect()
    : screenWidth_(1280),
      screenHeight_(720),
      renderer_(nullptr),
      selectedIndex_(0),
      hoveredIndex_(-1),
      customizationEnabled_(false),
      gamepadActive_(false),
      modelRotation_(0.0f),
      modelRotationSpeed_(30.0f),
      transitionTimer_(0.0f),
      cardAnimTimer_(0.0f),
      statsRevealTimer_(0.0f),
      gridColumns_(3),
      gridRows_(2),
      cardWidth_(280),
      cardHeight_(320),
      cardSpacing_(30),
      showParticles_(true),
      glowIntensity_(0.0f) {
}

CharacterSelect::~CharacterSelect() {
}

bool CharacterSelect::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Load character data
    LoadCharacterData();

    // Create character cards
    CreateCharacterCards();

    // Load assets
    LoadCharacterAssets();

    // Initialize stat bars
    UpdateStatBars();

    SDL_Log("CharacterSelect initialized with %zu characters", characters_.size());
    return true;
}

void CharacterSelect::Update(float deltaTime) {
    // Update animations
    UpdateAnimations(deltaTime);

    // Update model rotation
    UpdateModelRotation(deltaTime);

    // Update stat animations
    UpdateStatAnimations(deltaTime);

    // Update glow intensity
    float targetGlow = 1.0f;
    glowIntensity_ += (targetGlow - glowIntensity_) * deltaTime * 3.0f;
}

void CharacterSelect::Render(SDL_Renderer* renderer) {
    // Render background
    RenderBackground(renderer);

    // Render character grid
    RenderCharacterGrid(renderer);

    // Render character preview panel
    RenderCharacterPreview(renderer);

    // Render action buttons
    RenderActionButtons(renderer);

    // Render navigation hints
    if (gamepadActive_) {
        RenderNavigationHints(renderer);
    }
}

void CharacterSelect::HandleInput(const SDL_Event& event) {
    // Detect input type
    if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERAXISMOTION) {
        gamepadActive_ = true;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEMOTION) {
        gamepadActive_ = false;
    }

    HandleKeyboardInput(event);
    HandleGamepadInput(event);
    HandleMouseInput(event);
}

std::string CharacterSelect::GetSelectedCharacter() const {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(characters_.size())) {
        return characters_[selectedIndex_].id;
    }
    return "";
}

void CharacterSelect::SetSelectedCharacter(const std::string& characterId) {
    int index = GetCharacterIndex(characterId);
    if (index >= 0) {
        selectedIndex_ = index;
        UpdateStatBars();
    }
}

// Private methods

void CharacterSelect::LoadCharacterData() {
    characters_.clear();

    // Warrior
    CharacterClass warrior;
    warrior.id = "warrior";
    warrior.name = "Warrior";
    warrior.description = "A mighty fighter skilled in close combat";
    warrior.loreText = "Born in the harsh northern mountains, warriors are trained from youth in the art of combat. "
                       "Their strength and endurance are unmatched, making them formidable opponents on the battlefield.";
    warrior.iconPath = "assets/characters/warrior_icon.png";
    warrior.portraitPath = "assets/characters/warrior_portrait.png";
    warrior.modelPath = "assets/characters/warrior_model.obj";
    warrior.baseHealth = 150;
    warrior.baseMana = 50;
    warrior.baseStamina = 100;
    warrior.baseStrength = 20;
    warrior.baseDexterity = 12;
    warrior.baseIntelligence = 8;
    warrior.baseVitality = 18;
    warrior.difficulty = "Easy";
    warrior.playstyle = "Melee";
    warrior.accentColor = {196, 30, 58, 255}; // Red
    warrior.unlocked = true;
    characters_.push_back(warrior);

    // Mage
    CharacterClass mage;
    mage.id = "mage";
    mage.name = "Mage";
    mage.description = "Master of arcane arts and devastating spells";
    mage.loreText = "Scholars of the mystical arts, mages spend years studying ancient tomes and practicing "
                    "their craft. Their command over elemental forces makes them powerful but fragile.";
    mage.iconPath = "assets/characters/mage_icon.png";
    mage.portraitPath = "assets/characters/mage_portrait.png";
    mage.modelPath = "assets/characters/mage_model.obj";
    mage.baseHealth = 80;
    mage.baseMana = 200;
    mage.baseStamina = 60;
    mage.baseStrength = 8;
    mage.baseDexterity = 10;
    mage.baseIntelligence = 25;
    mage.baseVitality = 10;
    mage.difficulty = "Hard";
    mage.playstyle = "Magic";
    mage.accentColor = {63, 199, 235, 255}; // Cyan
    mage.unlocked = true;
    characters_.push_back(mage);

    // Rogue
    CharacterClass rogue;
    rogue.id = "rogue";
    rogue.name = "Rogue";
    rogue.description = "Cunning assassin who strikes from shadows";
    rogue.loreText = "Masters of stealth and precision, rogues operate in the shadows. "
                     "Their agility and cunning make them deadly when catching enemies off-guard.";
    rogue.iconPath = "assets/characters/rogue_icon.png";
    rogue.portraitPath = "assets/characters/rogue_portrait.png";
    rogue.modelPath = "assets/characters/rogue_model.obj";
    rogue.baseHealth = 100;
    rogue.baseMana = 80;
    rogue.baseStamina = 120;
    rogue.baseStrength = 12;
    rogue.baseDexterity = 22;
    rogue.baseIntelligence = 12;
    rogue.baseVitality = 12;
    rogue.difficulty = "Medium";
    rogue.playstyle = "Melee";
    rogue.accentColor = {255, 244, 104, 255}; // Yellow
    rogue.unlocked = true;
    characters_.push_back(rogue);

    // Ranger
    CharacterClass ranger;
    ranger.id = "ranger";
    ranger.name = "Ranger";
    ranger.description = "Skilled marksman and nature's guardian";
    ranger.loreText = "Rangers are one with nature, using bow and blade to protect the wilderness. "
                      "Their tracking skills and ranged expertise make them versatile combatants.";
    ranger.iconPath = "assets/characters/ranger_icon.png";
    ranger.portraitPath = "assets/characters/ranger_portrait.png";
    ranger.modelPath = "assets/characters/ranger_model.obj";
    ranger.baseHealth = 110;
    ranger.baseMana = 100;
    ranger.baseStamina = 110;
    ranger.baseStrength = 14;
    ranger.baseDexterity = 18;
    ranger.baseIntelligence = 14;
    ranger.baseVitality = 14;
    ranger.difficulty = "Medium";
    ranger.playstyle = "Ranged";
    ranger.accentColor = {170, 211, 114, 255}; // Green
    ranger.unlocked = true;
    characters_.push_back(ranger);

    // Paladin
    CharacterClass paladin;
    paladin.id = "paladin";
    paladin.name = "Paladin";
    paladin.description = "Holy warrior blessed with divine power";
    paladin.loreText = "Champions of light and justice, paladins blend martial prowess with divine magic. "
                       "They are the bulwark against darkness, protecting the innocent.";
    paladin.iconPath = "assets/characters/paladin_icon.png";
    paladin.portraitPath = "assets/characters/paladin_portrait.png";
    paladin.modelPath = "assets/characters/paladin_model.obj";
    paladin.baseHealth = 130;
    paladin.baseMana = 120;
    paladin.baseStamina = 90;
    paladin.baseStrength = 16;
    paladin.baseDexterity = 10;
    paladin.baseIntelligence = 16;
    paladin.baseVitality = 16;
    paladin.difficulty = "Easy";
    paladin.playstyle = "Hybrid";
    paladin.accentColor = {244, 140, 186, 255}; // Pink
    paladin.unlocked = true;
    characters_.push_back(paladin);

    // Necromancer
    CharacterClass necromancer;
    necromancer.id = "necromancer";
    necromancer.name = "Necromancer";
    necromancer.description = "Dark sorcerer who commands the undead";
    necromancer.loreText = "Practitioners of forbidden arts, necromancers wield the power of death itself. "
                           "They summon and control the undead to do their bidding.";
    necromancer.iconPath = "assets/characters/necromancer_icon.png";
    necromancer.portraitPath = "assets/characters/necromancer_portrait.png";
    necromancer.modelPath = "assets/characters/necromancer_model.obj";
    necromancer.baseHealth = 90;
    necromancer.baseMana = 180;
    necromancer.baseStamina = 70;
    necromancer.baseStrength = 10;
    necromancer.baseDexterity = 12;
    necromancer.baseIntelligence = 22;
    necromancer.baseVitality = 11;
    necromancer.difficulty = "Hard";
    necromancer.playstyle = "Magic";
    necromancer.accentColor = {147, 130, 201, 255}; // Purple
    necromancer.unlocked = true;
    characters_.push_back(necromancer);
}

void CharacterSelect::CreateCharacterCards() {
    // Cards are created dynamically based on character data
    SDL_Log("Created %zu character cards", characters_.size());
}

void CharacterSelect::LoadCharacterAssets() {
    // Load character textures, models, etc.
    SDL_Log("Loading character assets...");
}

void CharacterSelect::RenderBackground(SDL_Renderer* renderer) {
    // Gradient background
    for (int y = 0; y < screenHeight_; y++) {
        float gradient = static_cast<float>(y) / screenHeight_;
        Uint8 r = static_cast<Uint8>(15 + gradient * 15);
        Uint8 g = static_cast<Uint8>(15 + gradient * 15);
        Uint8 b = static_cast<Uint8>(25 + gradient * 35);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, screenWidth_, y);
    }

    // Decorative elements
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(characters_.size())) {
        const auto& character = characters_[selectedIndex_];
        SDL_SetRenderDrawColor(renderer,
            character.accentColor.r,
            character.accentColor.g,
            character.accentColor.b,
            20);

        for (int i = 0; i < 3; i++) {
            SDL_Rect decorRect = {
                screenWidth_ - 300 + i * 100,
                i * 200,
                200,
                200
            };
            SDL_RenderFillRect(renderer, &decorRect);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderCharacterGrid(SDL_Renderer* renderer) {
    int startX = 50;
    int startY = 100;

    for (size_t i = 0; i < characters_.size(); ++i) {
        int row = i / gridColumns_;
        int col = i % gridColumns_;

        SDL_Rect cardRect = {
            startX + col * (cardWidth_ + cardSpacing_),
            startY + row * (cardHeight_ + cardSpacing_),
            cardWidth_,
            cardHeight_
        };

        bool selected = (static_cast<int>(i) == selectedIndex_);
        bool hovered = (static_cast<int>(i) == hoveredIndex_);

        RenderCharacterCard(renderer, characters_[i], cardRect, selected, hovered);
    }
}

void CharacterSelect::RenderCharacterCard(SDL_Renderer* renderer,
                                         const CharacterClass& character,
                                         SDL_Rect rect,
                                         bool selected,
                                         bool hovered) {
    // Apply selection scale
    if (selected || hovered) {
        float scale = selected ? 1.05f : 1.02f;
        int widthDiff = static_cast<int>(rect.w * (scale - 1.0f));
        int heightDiff = static_cast<int>(rect.h * (scale - 1.0f));
        rect.x -= widthDiff / 2;
        rect.y -= heightDiff / 2;
        rect.w += widthDiff;
        rect.h += heightDiff;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Render glow if selected
    if (selected) {
        for (int i = 0; i < 5; i++) {
            SDL_Rect glowRect = {
                rect.x - i * 3,
                rect.y - i * 3,
                rect.w + i * 6,
                rect.h + i * 6
            };
            Uint8 alpha = static_cast<Uint8>(glowIntensity * 100 / (i + 1));
            SDL_SetRenderDrawColor(renderer,
                character.accentColor.r,
                character.accentColor.g,
                character.accentColor.b,
                alpha);
            SDL_RenderDrawRect(renderer, &glowRect);
        }
    }

    // Card background
    if (character.unlocked) {
        SDL_SetRenderDrawColor(renderer, 40, 40, 55, 230);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 25, 200);
    }
    SDL_RenderFillRect(renderer, &rect);

    // Card border
    SDL_SetRenderDrawColor(renderer,
        character.accentColor.r,
        character.accentColor.g,
        character.accentColor.b,
        selected ? 255 : 180);

    for (int i = 0; i < (selected ? 3 : 2); i++) {
        SDL_RenderDrawRect(renderer, &rect);
        rect.x++; rect.y++;
        rect.w -= 2; rect.h -= 2;
    }

    // Character portrait area
    SDL_Rect portraitRect = {
        rect.x + 10,
        rect.y + 10,
        rect.w - 20,
        rect.h - 100
    };
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    SDL_RenderFillRect(renderer, &portraitRect);

    SDL_SetRenderDrawColor(renderer,
        character.accentColor.r,
        character.accentColor.g,
        character.accentColor.b,
        255);
    SDL_RenderDrawRect(renderer, &portraitRect);

    // Character name area
    SDL_Rect nameRect = {
        rect.x + 10,
        rect.y + rect.h - 80,
        rect.w - 20,
        30
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &nameRect);

    // Difficulty indicator
    SDL_Rect diffRect = {
        rect.x + 10,
        rect.y + rect.h - 40,
        rect.w - 20,
        25
    };

    SDL_Color diffColor;
    if (character.difficulty == "Easy") {
        diffColor = {76, 175, 80, 255}; // Green
    } else if (character.difficulty == "Medium") {
        diffColor = {255, 152, 0, 255}; // Orange
    } else {
        diffColor = {244, 67, 54, 255}; // Red
    }

    SDL_SetRenderDrawColor(renderer, diffColor.r, diffColor.g, diffColor.b, 200);
    SDL_RenderFillRect(renderer, &diffRect);

    SDL_SetRenderDrawColor(renderer, diffColor.r, diffColor.g, diffColor.b, 255);
    SDL_RenderDrawRect(renderer, &diffRect);

    // Locked overlay
    if (!character.unlocked) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &rect);

        SDL_Rect lockRect = {
            rect.x + rect.w / 2 - 30,
            rect.y + rect.h / 2 - 30,
            60,
            60
        };
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &lockRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderCharacterPreview(SDL_Renderer* renderer) {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(characters_.size())) return;

    const auto& character = characters_[selectedIndex_];

    // Preview panel
    SDL_Rect previewPanel = {
        screenWidth_ - 480,
        80,
        460,
        screenHeight_ - 160
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 240);
    SDL_RenderFillRect(renderer, &previewPanel);

    SDL_SetRenderDrawColor(renderer,
        character.accentColor.r,
        character.accentColor.g,
        character.accentColor.b,
        255);

    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(renderer, &previewPanel);
        previewPanel.x++; previewPanel.y++;
        previewPanel.w -= 2; previewPanel.h -= 2;
    }

    // Render sections
    RenderCharacterModel(renderer);
    RenderCharacterStats(renderer);
    RenderCharacterLore(renderer);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderCharacterModel(SDL_Renderer* renderer) {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(characters_.size())) return;

    const auto& character = characters_[selectedIndex_];

    // Model viewport
    SDL_Rect modelRect = {
        screenWidth_ - 460,
        100,
        420,
        280
    };

    SDL_SetRenderDrawColor(renderer, 40, 40, 55, 255);
    SDL_RenderFillRect(renderer, &modelRect);

    SDL_SetRenderDrawColor(renderer,
        character.accentColor.r,
        character.accentColor.g,
        character.accentColor.b,
        150);
    SDL_RenderDrawRect(renderer, &modelRect);

    // Placeholder for 3D model - draw rotating squares
    int centerX = modelRect.x + modelRect.w / 2;
    int centerY = modelRect.y + modelRect.h / 2;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 3; i++) {
        float angle = modelRotation_ + i * 120.0f;
        float rad = angle * M_PI / 180.0f;
        int size = 80 - i * 20;

        int x = centerX + static_cast<int>(std::cos(rad) * 50);
        int y = centerY + static_cast<int>(std::sin(rad) * 50);

        SDL_Rect square = {x - size/2, y - size/2, size, size};

        Uint8 alpha = 150 - i * 40;
        SDL_SetRenderDrawColor(renderer,
            character.accentColor.r,
            character.accentColor.g,
            character.accentColor.b,
            alpha);
        SDL_RenderFillRect(renderer, &square);
        SDL_RenderDrawRect(renderer, &square);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderCharacterStats(SDL_Renderer* renderer) {
    SDL_Rect statsRect = {
        screenWidth_ - 460,
        390,
        420,
        200
    };

    // Stats title
    SDL_Rect titleRect = {
        statsRect.x + 10,
        statsRect.y + 10,
        statsRect.w - 20,
        25
    };
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &titleRect);

    // Render stat bars
    int barY = statsRect.y + 45;
    int barHeight = 18;
    int barSpacing = 22;

    for (const auto& stat : currentStats_) {
        SDL_Rect barRect = {
            statsRect.x + 10,
            barY,
            statsRect.w - 20,
            barHeight
        };
        RenderStatBar(renderer, stat, barRect);
        barY += barHeight + barSpacing;
    }
}

void CharacterSelect::RenderStatBar(SDL_Renderer* renderer, const StatBar& stat, SDL_Rect rect) {
    // Background
    SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
    SDL_RenderFillRect(renderer, &rect);

    // Value bar
    int fillWidth = static_cast<int>((rect.w * stat.value / stat.maxValue) * stat.animProgress);
    SDL_Rect fillRect = {rect.x, rect.y, fillWidth, rect.h};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, stat.color.r, stat.color.g, stat.color.b, 200);
    SDL_RenderFillRect(renderer, &fillRect);

    // Border
    SDL_SetRenderDrawColor(renderer, stat.color.r, stat.color.g, stat.color.b, 255);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderCharacterLore(SDL_Renderer* renderer) {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(characters_.size())) return;

    SDL_Rect loreRect = {
        screenWidth_ - 460,
        600,
        420,
        110
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 220);
    SDL_RenderFillRect(renderer, &loreRect);

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &loreRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderActionButtons(SDL_Renderer* renderer) {
    // Confirm button
    SDL_Rect confirmRect = {
        screenWidth_ - 460,
        screenHeight_ - 120,
        200,
        50
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 76, 175, 80, 220);
    SDL_RenderFillRect(renderer, &confirmRect);

    SDL_SetRenderDrawColor(renderer, 76, 175, 80, 255);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawRect(renderer, &confirmRect);
        confirmRect.x++; confirmRect.y++;
        confirmRect.w -= 2; confirmRect.h -= 2;
    }

    // Back button
    SDL_Rect backRect = {
        screenWidth_ - 250,
        screenHeight_ - 120,
        190,
        50
    };

    SDL_SetRenderDrawColor(renderer, 244, 67, 54, 220);
    SDL_RenderFillRect(renderer, &backRect);

    SDL_SetRenderDrawColor(renderer, 244, 67, 54, 255);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawRect(renderer, &backRect);
        backRect.x++; backRect.y++;
        backRect.w -= 2; backRect.h -= 2;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::RenderNavigationHints(SDL_Renderer* renderer) {
    SDL_Rect hintRect = {
        20,
        screenHeight_ - 80,
        400,
        60
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 200);
    SDL_RenderFillRect(renderer, &hintRect);

    SDL_SetRenderDrawColor(renderer, 75, 144, 226, 200);
    SDL_RenderDrawRect(renderer, &hintRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void CharacterSelect::UpdateAnimations(float deltaTime) {
    transitionTimer_ += deltaTime;
    cardAnimTimer_ += deltaTime * 2.0f;
}

void CharacterSelect::UpdateModelRotation(float deltaTime) {
    modelRotation_ += modelRotationSpeed_ * deltaTime;
    if (modelRotation_ >= 360.0f) {
        modelRotation_ -= 360.0f;
    }
}

void CharacterSelect::UpdateStatAnimations(float deltaTime) {
    statsRevealTimer_ += deltaTime * 2.0f;

    for (auto& stat : currentStats_) {
        float target = 1.0f;
        stat.animProgress += (target - stat.animProgress) * deltaTime * 5.0f;
    }
}

void CharacterSelect::HandleKeyboardInput(const SDL_Event& event) {
    if (event.type != SDL_KEYDOWN) return;

    switch (event.key.keysym.sym) {
        case SDLK_LEFT:
        case SDLK_a:
            SelectPreviousCharacter();
            break;

        case SDLK_RIGHT:
        case SDLK_d:
            SelectNextCharacter();
            break;

        case SDLK_UP:
        case SDLK_w:
            SelectCharacterUp();
            break;

        case SDLK_DOWN:
        case SDLK_s:
            SelectCharacterDown();
            break;

        case SDLK_RETURN:
        case SDLK_SPACE:
            ConfirmSelection();
            break;

        case SDLK_ESCAPE:
            GoBack();
            break;
    }
}

void CharacterSelect::HandleGamepadInput(const SDL_Event& event) {
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                SelectPreviousCharacter();
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                SelectNextCharacter();
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                SelectCharacterUp();
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                SelectCharacterDown();
                break;

            case SDL_CONTROLLER_BUTTON_A:
                ConfirmSelection();
                break;

            case SDL_CONTROLLER_BUTTON_B:
                GoBack();
                break;
        }
    }
}

void CharacterSelect::HandleMouseInput(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;

        hoveredIndex_ = -1;
        for (size_t i = 0; i < characters_.size(); ++i) {
            SDL_Rect cardRect = GetCharacterCardRect(static_cast<int>(i));
            if (mouseX >= cardRect.x && mouseX < cardRect.x + cardRect.w &&
                mouseY >= cardRect.y && mouseY < cardRect.y + cardRect.h) {
                hoveredIndex_ = static_cast<int>(i);
                break;
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (hoveredIndex_ >= 0 && characters_[hoveredIndex_].unlocked) {
            selectedIndex_ = hoveredIndex_;
            UpdateStatBars();
        }

        // Check button clicks
        int mouseX = event.button.x;
        int mouseY = event.button.y;

        SDL_Rect confirmRect = {screenWidth_ - 460, screenHeight_ - 120, 200, 50};
        SDL_Rect backRect = {screenWidth_ - 250, screenHeight_ - 120, 190, 50};

        if (mouseX >= confirmRect.x && mouseX < confirmRect.x + confirmRect.w &&
            mouseY >= confirmRect.y && mouseY < confirmRect.y + confirmRect.h) {
            ConfirmSelection();
        } else if (mouseX >= backRect.x && mouseX < backRect.x + backRect.w &&
                  mouseY >= backRect.y && mouseY < backRect.y + backRect.h) {
            GoBack();
        }
    }
}

void CharacterSelect::SelectNextCharacter() {
    int start = selectedIndex_;
    do {
        selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(characters_.size());
        if (characters_[selectedIndex_].unlocked) {
            UpdateStatBars();
            break;
        }
    } while (selectedIndex_ != start);
}

void CharacterSelect::SelectPreviousCharacter() {
    int start = selectedIndex_;
    do {
        selectedIndex_--;
        if (selectedIndex_ < 0) {
            selectedIndex_ = static_cast<int>(characters_.size()) - 1;
        }
        if (characters_[selectedIndex_].unlocked) {
            UpdateStatBars();
            break;
        }
    } while (selectedIndex_ != start);
}

void CharacterSelect::SelectCharacterUp() {
    int newIndex = selectedIndex_ - gridColumns_;
    if (newIndex >= 0 && characters_[newIndex].unlocked) {
        selectedIndex_ = newIndex;
        UpdateStatBars();
    }
}

void CharacterSelect::SelectCharacterDown() {
    int newIndex = selectedIndex_ + gridColumns_;
    if (newIndex < static_cast<int>(characters_.size()) && characters_[newIndex].unlocked) {
        selectedIndex_ = newIndex;
        UpdateStatBars();
    }
}

void CharacterSelect::ConfirmSelection() {
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(characters_.size())) {
        const auto& character = characters_[selectedIndex_];
        if (character.unlocked && onCharacterConfirmed_) {
            SDL_Log("Character confirmed: %s", character.name.c_str());
            onCharacterConfirmed_(character.id);
        }
    }
}

void CharacterSelect::GoBack() {
    if (onBack_) {
        onBack_();
    }
}

int CharacterSelect::GetCharacterIndex(const std::string& characterId) const {
    for (size_t i = 0; i < characters_.size(); ++i) {
        if (characters_[i].id == characterId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

SDL_Rect CharacterSelect::GetCharacterCardRect(int index) const {
    int row = index / gridColumns_;
    int col = index % gridColumns_;

    return {
        50 + col * (cardWidth_ + cardSpacing_),
        100 + row * (cardHeight_ + cardSpacing_),
        cardWidth_,
        cardHeight_
    };
}

void CharacterSelect::UpdateStatBars() {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(characters_.size())) return;

    const auto& character = characters_[selectedIndex_];
    currentStats_.clear();

    // Health
    StatBar health = {"Health", character.baseHealth, 200, {231, 76, 60, 255}, 0.0f};
    currentStats_.push_back(health);

    // Mana
    StatBar mana = {"Mana", character.baseMana, 200, {52, 152, 219, 255}, 0.0f};
    currentStats_.push_back(mana);

    // Stamina
    StatBar stamina = {"Stamina", character.baseStamina, 150, {243, 156, 18, 255}, 0.0f};
    currentStats_.push_back(stamina);

    // Strength
    StatBar strength = {"Strength", character.baseStrength, 30, {192, 57, 43, 255}, 0.0f};
    currentStats_.push_back(strength);

    // Dexterity
    StatBar dexterity = {"Dexterity", character.baseDexterity, 30, {39, 174, 96, 255}, 0.0f};
    currentStats_.push_back(dexterity);

    // Intelligence
    StatBar intelligence = {"Intelligence", character.baseIntelligence, 30, {155, 89, 182, 255}, 0.0f};
    currentStats_.push_back(intelligence);

    statsRevealTimer_ = 0.0f;
}

} // namespace Launcher
