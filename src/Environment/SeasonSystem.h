#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace Environment {

/**
 * @brief Season types
 */
enum class Season {
    Spring,
    Summer,
    Autumn,
    Winter
};

/**
 * @brief Season configuration
 */
struct SeasonConfig {
    bool enableSeasonalCycle = true;
    float daysPerSeason = 30.0f;            // Game days per season
    bool enableSeasonalWeather = true;      // Seasons affect weather
    bool enableSeasonalVisuals = true;      // Change environment visuals
    bool enableSeasonalGameplay = true;     // Seasons affect gameplay
    bool startInSpring = true;              // Start game in spring
    bool enableTransitions = true;          // Gradual season transitions
    float transitionDays = 3.0f;            // Days for season transition
};

/**
 * @brief Seasonal visual effects
 */
struct SeasonalVisuals {
    glm::vec3 foliageColor;                 // Tree/grass tint
    glm::vec3 groundColor;                  // Ground/terrain tint
    glm::vec3 waterColor;                   // Water body color
    float foliageDensity;                   // 0.0 to 1.0 (winter = less)
    float snowCoverage;                     // 0.0 to 1.0
    float bloomAmount;                      // Flower/bloom density
    glm::vec3 ambientTint;                  // Overall color tint
    float saturation;                       // Color saturation modifier
    std::string particleEffect;             // Falling leaves, petals, etc.
    int particleDensity;                    // Particles per second
};

/**
 * @brief Seasonal gameplay modifiers
 */
struct SeasonalGameplay {
    float temperatureModifier;              // Average temp change
    float cropGrowthMultiplier;             // Farming speed
    float animalSpawnRate;                  // Wildlife activity
    float fishAvailability;                 // Fishing success
    float staminaDrainMultiplier;           // Physical activity cost
    float healthRegenModifier;              // Natural healing rate
    std::vector<std::string> availableResources;  // Season-specific items
    std::vector<std::string> activeCreatures;     // Season-specific enemies
    float dayLengthMultiplier;              // Longer/shorter days
    bool allowsFreeze;                      // Can water freeze?
};

/**
 * @brief Weather probability changes by season
 */
struct SeasonalWeatherInfluence {
    float clearProbability = 0.4f;
    float rainProbability = 0.2f;
    float snowProbability = 0.0f;
    float stormProbability = 0.1f;
    float fogProbability = 0.1f;
    float temperatureRange[2] = {10.0f, 25.0f}; // Min, Max temp
};

/**
 * @brief Complete season definition
 */
struct SeasonDefinition {
    std::string name;
    Season season;
    SeasonalVisuals visuals;
    SeasonalGameplay gameplay;
    SeasonalWeatherInfluence weatherInfluence;
    std::string description;
    std::vector<std::string> events;        // Season-specific events
};

/**
 * @brief Main season system manager
 * Manages seasonal cycles, transitions, and their effects on the world
 */
class SeasonSystem {
public:
    SeasonSystem();
    ~SeasonSystem() = default;

    // Initialization
    void initialize(const SeasonConfig& config = SeasonConfig());
    void shutdown();

    // Update (call once per frame)
    void update(float deltaTime, int currentDay);

    // Season control
    void setSeason(Season season);
    Season getCurrentSeason() const { return m_currentSeason; }
    Season getNextSeason() const;
    bool isTransitioning() const { return m_isTransitioning; }
    float getTransitionProgress() const;

    // Season information
    const SeasonDefinition* getSeasonDefinition(Season season) const;
    const SeasonDefinition* getCurrentDefinition() const;
    std::string getSeasonName() const;
    std::string getSeasonDescription() const;
    int getDaysIntoSeason() const { return m_daysIntoSeason; }
    int getDaysUntilNextSeason() const;

    // Visual effects
    const SeasonalVisuals& getCurrentVisuals() const { return m_currentVisuals; }
    glm::vec3 getFoliageColor() const { return m_currentVisuals.foliageColor; }
    float getSnowCoverage() const { return m_currentVisuals.snowCoverage; }
    float getFoliageDensity() const { return m_currentVisuals.foliageDensity; }

    // Gameplay effects
    const SeasonalGameplay& getCurrentGameplay() const { return m_currentGameplay; }
    float getTemperatureModifier() const { return m_currentGameplay.temperatureModifier; }
    float getCropGrowthRate() const { return m_currentGameplay.cropGrowthMultiplier; }
    bool canWaterFreeze() const { return m_currentGameplay.allowsFreeze; }

    // Weather influence
    const SeasonalWeatherInfluence& getWeatherInfluence() const { return m_currentWeatherInfluence; }
    float getWeatherProbability(const std::string& weatherType) const;

    // Seasonal resources and creatures
    const std::vector<std::string>& getAvailableResources() const;
    const std::vector<std::string>& getActiveCreatures() const;
    bool isResourceAvailable(const std::string& resourceName) const;
    bool isCreatureActive(const std::string& creatureName) const;

    // Seasonal events
    void triggerSeasonalEvent(const std::string& eventName);
    const std::vector<std::string>& getCurrentEvents() const;
    bool hasEvent(const std::string& eventName) const;

    // Configuration
    const SeasonConfig& getConfig() const { return m_config; }
    void setConfig(const SeasonConfig& config);

    // Custom season definitions
    void addSeasonDefinition(const SeasonDefinition& definition);
    bool loadSeasonDefinitions(const std::string& jsonPath);

    // Callbacks
    void onSeasonChanged(std::function<void(Season, Season)> callback); // old, new
    void onSeasonalEvent(std::function<void(const std::string&)> callback);

    // Utility
    static std::string seasonToString(Season season);
    static Season stringToSeason(const std::string& str);
    float getYearProgress() const;          // 0.0 to 1.0 through the year

private:
    // Update helpers
    void checkSeasonTransition(int currentDay);
    void updateTransition(float progress);
    void updateVisuals();
    void updateGameplay();
    void updateWeatherInfluence();

    // Interpolation for smooth transitions
    glm::vec3 lerpColor(const glm::vec3& a, const glm::vec3& b, float t) const;
    float lerpFloat(float a, float b, float t) const;

    // Season progression
    void advanceSeason();

private:
    SeasonConfig m_config;

    // Current season state
    Season m_currentSeason;
    int m_daysIntoSeason;
    int m_lastDayUpdate;

    // Transition state
    bool m_isTransitioning;
    Season m_targetSeason;
    float m_transitionProgress;

    // Current effects
    SeasonalVisuals m_currentVisuals;
    SeasonalGameplay m_currentGameplay;
    SeasonalWeatherInfluence m_currentWeatherInfluence;

    // Season definitions
    std::unordered_map<Season, SeasonDefinition> m_seasonDefinitions;

    // Active seasonal events
    std::vector<std::string> m_activeEvents;

    // Callbacks
    std::function<void(Season, Season)> m_onSeasonChanged;
    std::function<void(const std::string&)> m_onSeasonalEvent;
};

} // namespace Environment
