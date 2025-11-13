#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Rendering {
    class ParticleSystem;
}

namespace Environment {

/**
 * @brief Weather types
 */
enum class WeatherType {
    Clear,
    PartlyCloudy,
    Cloudy,
    Overcast,
    LightRain,
    Rain,
    HeavyRain,
    Thunderstorm,
    LightSnow,
    Snow,
    HeavySnow,
    Blizzard,
    Fog,
    Mist,
    Sandstorm,
    Hail
};

/**
 * @brief Weather intensity
 */
enum class WeatherIntensity {
    None,
    Light,
    Moderate,
    Heavy,
    Extreme
};

/**
 * @brief Weather configuration
 */
struct WeatherConfig {
    bool enableDynamicWeather = true;
    bool enableWeatherTransitions = true;
    float transitionDuration = 30.0f;       // Seconds for weather transitions
    float minWeatherDuration = 300.0f;      // Minimum duration of a weather state
    float maxWeatherDuration = 1800.0f;     // Maximum duration of a weather state
    bool enableRegionalWeather = true;      // Different weather in different regions
    bool enableSeasonalInfluence = true;    // Seasons affect weather patterns
    int maxParticles = 2000;                // Max weather particles
    float windUpdateFrequency = 2.0f;       // Seconds between wind updates
};

/**
 * @brief Weather effects on gameplay
 */
struct WeatherGameplayEffects {
    float movementSpeedMultiplier = 1.0f;
    float visibilityRange = 1.0f;           // 1.0 = normal, 0.5 = 50% visibility
    float staminaDrainMultiplier = 1.0f;
    float accuracyMultiplier = 1.0f;        // Affects ranged attacks
    bool enablesHydration = false;          // Rain hydrates player
    bool causesCold = false;                // Requires warm clothing
    bool causesHeat = false;                // Requires cooling
    float damagePerSecond = 0.0f;           // Environmental damage
    float healthRegenMultiplier = 1.0f;
    float magicPowerMultiplier = 1.0f;      // Some weather boosts magic
};

/**
 * @brief Weather visual parameters
 */
struct WeatherVisuals {
    glm::vec3 skyTint;
    float skyBrightness;
    glm::vec3 fogColor;
    float fogDensity;
    float fogStart;
    float fogEnd;
    glm::vec3 ambientLightModifier;
    float ambientIntensityModifier;
    float shadowIntensityModifier;
    int particleDensity;                    // Particles per spawn
    glm::vec3 particleColor;
    glm::vec2 particleSize;                 // min, max
    glm::vec2 particleVelocity;             // base velocity
    float particleLifetime;
    std::string particleTexture;
};

/**
 * @brief Wind parameters
 */
struct Wind {
    glm::vec2 direction;                    // Normalized direction
    float speed;                            // Wind speed
    float gustStrength;                     // Occasional gusts
    float turbulence;                       // Randomness in direction

    Wind() : direction(1.0f, 0.0f), speed(0.0f), gustStrength(0.0f), turbulence(0.0f) {}
};

/**
 * @brief Weather pattern definition
 */
struct WeatherPattern {
    std::string name;
    WeatherType type;
    WeatherIntensity intensity;
    WeatherGameplayEffects gameplayEffects;
    WeatherVisuals visuals;
    Wind wind;
    float probability;                      // Chance of occurring (0-1)
    std::vector<WeatherType> canTransitionTo;
    std::vector<std::string> soundEffects;  // Ambient sounds
    float soundVolume;
};

/**
 * @brief Regional weather biases
 */
struct RegionalWeather {
    std::string regionName;
    std::unordered_map<WeatherType, float> weatherProbabilities;
    float temperatureModifier;              // Base temperature adjustment
    float humidityModifier;                 // Base humidity adjustment
};

/**
 * @brief Main weather system manager
 * Handles weather patterns, transitions, and effects
 */
class WeatherSystem {
public:
    WeatherSystem();
    ~WeatherSystem();

    // Initialization
    bool initialize(const WeatherConfig& config = WeatherConfig());
    void shutdown();

    // Update (call once per frame)
    void update(float deltaTime);

    // Weather control
    void setWeather(WeatherType type, WeatherIntensity intensity = WeatherIntensity::Moderate);
    void transitionToWeather(WeatherType type, float duration = -1.0f); // -1 uses default
    WeatherType getCurrentWeather() const { return m_currentWeather; }
    WeatherIntensity getCurrentIntensity() const { return m_currentIntensity; }
    bool isTransitioning() const { return m_isTransitioning; }
    float getTransitionProgress() const;

    // Weather patterns
    bool loadWeatherPatterns(const std::string& jsonPath);
    void addWeatherPattern(const WeatherPattern& pattern);
    const WeatherPattern* getWeatherPattern(WeatherType type) const;
    const WeatherPattern* getCurrentPattern() const;

    // Regional weather
    void setRegion(const std::string& regionName);
    std::string getCurrentRegion() const { return m_currentRegion; }
    void addRegionalWeather(const RegionalWeather& regional);

    // Dynamic weather
    void enableDynamicWeather(bool enable);
    bool isDynamicWeatherEnabled() const { return m_config.enableDynamicWeather; }
    void triggerRandomWeatherChange();

    // Wind
    const Wind& getWind() const { return m_currentWind; }
    glm::vec2 getWindForce() const;         // Direction * speed
    void setWind(const Wind& wind);

    // Gameplay effects
    const WeatherGameplayEffects& getGameplayEffects() const { return m_currentEffects; }
    float getMovementSpeedMultiplier() const { return m_currentEffects.movementSpeedMultiplier; }
    float getVisibilityRange() const { return m_currentEffects.visibilityRange; }
    bool shouldTakeDamage() const { return m_currentEffects.damagePerSecond > 0.0f; }

    // Visual effects
    const WeatherVisuals& getVisuals() const { return m_currentVisuals; }
    void setVisibilityMultiplier(float multiplier); // For time of day

    // Particle effects
    void spawnWeatherParticles(int count, const glm::vec2& spawnArea);
    void clearWeatherParticles();

    // Audio
    void updateWeatherAudio();
    const std::vector<std::string>& getCurrentSounds() const;

    // Temperature and conditions
    float getTemperature() const;           // In Celsius
    float getHumidity() const;              // 0.0 to 1.0
    bool isRaining() const;
    bool isSnowing() const;
    bool isStorming() const;
    bool isClearSky() const;

    // Configuration
    const WeatherConfig& getConfig() const { return m_config; }
    void setConfig(const WeatherConfig& config);

    // Callbacks
    void onWeatherChanged(std::function<void(WeatherType, WeatherType)> callback); // old, new
    void onWeatherIntensityChanged(std::function<void(WeatherIntensity)> callback);
    void onLightningStrike(std::function<void(glm::vec2)> callback);

    // Special weather events
    void triggerLightningStrike(const glm::vec2& position);
    void createTornado(const glm::vec2& position, float duration = 60.0f);
    void createRainbow(float duration = 120.0f);

    // Utility
    std::string getWeatherName() const;
    std::string getWeatherDescription() const;

private:
    // Update helpers
    void updateWeatherTimer(float deltaTime);
    void updateTransition(float deltaTime);
    void updateWind(float deltaTime);
    void updateEffects();
    void updateParticles(float deltaTime);
    void checkForDynamicChange();

    // Weather selection
    WeatherType selectNextWeather() const;
    WeatherIntensity selectIntensity(WeatherType type) const;
    float getWeatherProbability(WeatherType type) const;

    // Effect calculation
    WeatherGameplayEffects calculateEffects() const;
    WeatherVisuals calculateVisuals() const;
    Wind calculateWind() const;

    // Interpolation
    glm::vec3 lerpColor(const glm::vec3& a, const glm::vec3& b, float t) const;
    float lerpFloat(float a, float b, float t) const;

private:
    WeatherConfig m_config;

    // Current weather state
    WeatherType m_currentWeather;
    WeatherIntensity m_currentIntensity;
    float m_weatherTimer;
    float m_weatherDuration;

    // Transition state
    bool m_isTransitioning;
    WeatherType m_targetWeather;
    WeatherIntensity m_targetIntensity;
    float m_transitionTimer;
    float m_transitionDuration;

    // Weather patterns
    std::unordered_map<WeatherType, WeatherPattern> m_weatherPatterns;
    std::string m_currentRegion;
    std::unordered_map<std::string, RegionalWeather> m_regionalWeather;

    // Current effects
    WeatherGameplayEffects m_currentEffects;
    WeatherVisuals m_currentVisuals;
    Wind m_currentWind;
    Wind m_targetWind;
    float m_windTimer;

    // Particle systems
    struct WeatherParticle {
        glm::vec2 position;
        glm::vec2 velocity;
        float lifetime;
        float maxLifetime;
        float size;
        glm::vec4 color;
    };
    std::vector<WeatherParticle> m_particles;
    float m_particleSpawnTimer;

    // Special effects
    struct Lightning {
        glm::vec2 position;
        float duration;
        float timer;
    };
    std::vector<Lightning> m_lightningStrikes;

    bool m_rainbowActive;
    float m_rainbowTimer;
    float m_rainbowDuration;

    // Callbacks
    std::function<void(WeatherType, WeatherType)> m_onWeatherChanged;
    std::function<void(WeatherIntensity)> m_onWeatherIntensityChanged;
    std::function<void(glm::vec2)> m_onLightningStrike;

    // Randomness
    float m_randomAccumulator;
};

} // namespace Environment
