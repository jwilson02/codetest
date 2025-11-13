#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>

namespace Launcher {

/**
 * @brief Character class information
 */
struct CharacterClass {
    std::string id;
    std::string name;
    std::string description;
    std::string loreText;
    std::string iconPath;
    std::string portraitPath;
    std::string modelPath;

    // Base stats
    int baseHealth;
    int baseMana;
    int baseStamina;
    int baseStrength;
    int baseDexterity;
    int baseIntelligence;
    int baseVitality;

    // Difficulty
    std::string difficulty; // "Easy", "Medium", "Hard"
    std::string playstyle;  // "Melee", "Ranged", "Magic", "Hybrid"

    // Visual
    SDL_Color accentColor;
    bool unlocked;
};

/**
 * @brief Stat bar configuration
 */
struct StatBar {
    std::string label;
    int value;
    int maxValue;
    SDL_Color color;
    float animProgress;
};

/**
 * @brief Character Selection Screen
 *
 * Professional character selection interface with:
 * - Grid-based character display
 * - Detailed character preview with 3D model rotation
 * - Animated stat displays
 * - Lore and backstory panels
 * - Customization options
 * - Smooth transitions and particle effects
 */
class CharacterSelect {
public:
    CharacterSelect();
    ~CharacterSelect();

    /**
     * @brief Initialize the character selection screen
     * @param renderer SDL renderer
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @return True if initialization successful
     */
    bool Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight);

    /**
     * @brief Update logic
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    /**
     * @brief Render the character selection screen
     * @param renderer SDL renderer
     */
    void Render(SDL_Renderer* renderer);

    /**
     * @brief Handle input events
     * @param event SDL event
     */
    void HandleInput(const SDL_Event& event);

    /**
     * @brief Set callback for character confirmation
     * @param callback Function to call with selected character
     */
    void SetOnCharacterConfirmed(std::function<void(const std::string&)> callback) {
        onCharacterConfirmed_ = callback;
    }

    /**
     * @brief Set callback for back button
     * @param callback Function to call
     */
    void SetOnBack(std::function<void()> callback) {
        onBack_ = callback;
    }

    /**
     * @brief Get currently selected character
     * @return Character class ID
     */
    std::string GetSelectedCharacter() const;

    /**
     * @brief Set selected character by ID
     * @param characterId Character ID
     */
    void SetSelectedCharacter(const std::string& characterId);

    /**
     * @brief Enable or disable character customization
     * @param enabled Customization state
     */
    void SetCustomizationEnabled(bool enabled) { customizationEnabled_ = enabled; }

private:
    // Initialization
    void LoadCharacterData();
    void CreateCharacterCards();
    void LoadCharacterAssets();

    // Rendering
    void RenderBackground(SDL_Renderer* renderer);
    void RenderCharacterGrid(SDL_Renderer* renderer);
    void RenderCharacterCard(SDL_Renderer* renderer, const CharacterClass& character,
                            SDL_Rect rect, bool selected, bool hovered);
    void RenderCharacterPreview(SDL_Renderer* renderer);
    void RenderCharacterModel(SDL_Renderer* renderer);
    void RenderCharacterStats(SDL_Renderer* renderer);
    void RenderStatBar(SDL_Renderer* renderer, const StatBar& stat, SDL_Rect rect);
    void RenderCharacterLore(SDL_Renderer* renderer);
    void RenderActionButtons(SDL_Renderer* renderer);
    void RenderNavigationHints(SDL_Renderer* renderer);

    // Updates
    void UpdateAnimations(float deltaTime);
    void UpdateModelRotation(float deltaTime);
    void UpdateStatAnimations(float deltaTime);

    // Input handling
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleGamepadInput(const SDL_Event& event);
    void HandleMouseInput(const SDL_Event& event);

    // Navigation
    void SelectNextCharacter();
    void SelectPreviousCharacter();
    void SelectCharacterUp();
    void SelectCharacterDown();
    void ConfirmSelection();
    void GoBack();

    // Utilities
    int GetCharacterIndex(const std::string& characterId) const;
    SDL_Rect GetCharacterCardRect(int index) const;
    void UpdateStatBars();

    // Screen properties
    int screenWidth_;
    int screenHeight_;
    SDL_Renderer* renderer_;

    // Character data
    std::vector<CharacterClass> characters_;
    int selectedIndex_;
    int hoveredIndex_;

    // State
    bool customizationEnabled_;
    bool gamepadActive_;
    float modelRotation_;
    float modelRotationSpeed_;

    // Animations
    float transitionTimer_;
    float cardAnimTimer_;
    float statsRevealTimer_;
    std::vector<StatBar> currentStats_;

    // Layout
    int gridColumns_;
    int gridRows_;
    int cardWidth_;
    int cardHeight_;
    int cardSpacing_;

    // Callbacks
    std::function<void(const std::string&)> onCharacterConfirmed_;
    std::function<void()> onBack_;

    // Visual effects
    bool showParticles_;
    float glowIntensity_;
};

} // namespace Launcher
