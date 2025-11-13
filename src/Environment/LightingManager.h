#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace Environment {
    class DayNightCycle;
    class WeatherSystem;
    class SeasonSystem;
}

namespace Rendering {
    class Shader;
}

namespace Environment {

/**
 * @brief Light source types
 */
enum class LightType {
    Directional,    // Sun, moon
    Point,          // Torches, lamps
    Spot,           // Focused beams
    Ambient,        // Overall scene lighting
    Area            // Large soft lights
};

/**
 * @brief Light properties
 */
struct Light {
    std::string name;
    LightType type;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float radius;               // For point/area lights
    float innerConeAngle;       // For spot lights
    float outerConeAngle;       // For spot lights
    bool castsShadows;
    bool enabled;
    int priority;               // For culling (higher = more important)
    float flickerAmount;        // 0.0 = stable, 1.0 = max flicker
    float flickerSpeed;

    Light()
        : type(LightType::Point)
        , position(0.0f)
        , direction(0.0f, -1.0f, 0.0f)
        , color(1.0f)
        , intensity(1.0f)
        , radius(10.0f)
        , innerConeAngle(15.0f)
        , outerConeAngle(30.0f)
        , castsShadows(false)
        , enabled(true)
        , priority(0)
        , flickerAmount(0.0f)
        , flickerSpeed(1.0f)
    {}
};

/**
 * @brief Shadow configuration
 */
struct ShadowConfig {
    bool enabled = true;
    int resolution = 1024;          // Shadow map resolution
    float bias = 0.005f;            // Shadow acne prevention
    float normalBias = 0.01f;
    float maxDistance = 100.0f;     // Shadow draw distance
    int cascadeCount = 3;           // For directional lights
    float cascadeSplitLambda = 0.5f;
    bool softShadows = true;        // PCF filtering
    int pcfSamples = 4;             // PCF sample count
};

/**
 * @brief Lighting configuration
 */
struct LightingConfig {
    int maxLights = 32;             // Maximum active lights
    bool enableDynamicLighting = true;
    bool enableShadows = true;
    bool enableAmbientOcclusion = false;
    bool enableGlobalIllumination = false;
    float ambientIntensity = 0.3f;
    glm::vec3 ambientColor = glm::vec3(1.0f);
    bool enableHDR = true;
    float exposure = 1.0f;
    bool enableBloom = true;
    float bloomThreshold = 1.0f;
    bool enableColorGrading = true;
};

/**
 * @brief Indoor/Outdoor detection zone
 */
struct LightingZone {
    std::string name;
    glm::vec2 position;
    glm::vec2 size;
    bool isIndoor;
    glm::vec3 ambientOverride;
    float ambientIntensityOverride;
    std::vector<std::string> zoneLights; // Lights active in this zone
};

/**
 * @brief Main lighting manager
 * Coordinates all lighting systems and integrates with environment
 */
class LightingManager {
public:
    LightingManager();
    ~LightingManager();

    // Initialization
    bool initialize(const LightingConfig& config = LightingConfig());
    void shutdown();

    // Update (call once per frame)
    void update(float deltaTime);

    // Integration with environment systems
    void setDayNightCycle(DayNightCycle* cycle) { m_dayNightCycle = cycle; }
    void setWeatherSystem(WeatherSystem* weather) { m_weatherSystem = weather; }
    void setSeasonSystem(SeasonSystem* season) { m_seasonSystem = season; }

    // Light management
    int addLight(const Light& light);
    void removeLight(int lightId);
    void updateLight(int lightId, const Light& light);
    Light* getLight(int lightId);
    const Light* getLight(int lightId) const;
    void enableLight(int lightId, bool enabled);
    void clearAllLights();

    // Predefined lights
    int createSunLight();
    int createMoonLight();
    int createTorchLight(const glm::vec3& position);
    int createCampfireLight(const glm::vec3& position);
    int createMagicLight(const glm::vec3& position, const glm::vec3& color);

    // Ambient lighting
    void setAmbientLight(const glm::vec3& color, float intensity);
    glm::vec3 getAmbientColor() const { return m_ambientColor; }
    float getAmbientIntensity() const { return m_ambientIntensity; }

    // Directional light (sun/moon)
    void setDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity);
    glm::vec3 getDirectionalLightDirection() const { return m_directionalDirection; }
    glm::vec3 getDirectionalLightColor() const { return m_directionalColor; }
    float getDirectionalLightIntensity() const { return m_directionalIntensity; }

    // Shadow system
    void setShadowConfig(const ShadowConfig& config);
    const ShadowConfig& getShadowConfig() const { return m_shadowConfig; }
    void enableShadows(bool enabled);
    bool areShadowsEnabled() const;

    // Indoor/Outdoor system
    void addLightingZone(const LightingZone& zone);
    void removeLightingZone(const std::string& name);
    void setPlayerPosition(const glm::vec2& position);
    bool isPlayerIndoors() const { return m_isIndoors; }
    const LightingZone* getCurrentZone() const;

    // Light culling and optimization
    void cullLights(const glm::vec2& cameraPosition, float viewDistance);
    std::vector<const Light*> getVisibleLights() const;
    int getActiveLightCount() const;

    // Time-based effects
    void updateTimeBasedLighting();
    float getCurrentShadowIntensity() const;
    glm::vec3 getCurrentSkyColor() const;
    glm::vec3 getCurrentFogColor() const;
    float getCurrentFogDensity() const;

    // Weather-based effects
    void updateWeatherLighting();
    void applyWeatherModifiers();

    // Seasonal effects
    void updateSeasonalLighting();
    glm::vec3 getSeasonalTint() const;

    // Special lighting effects
    void triggerLightningFlash(const glm::vec2& position, float duration = 0.3f);
    void createFireLight(const glm::vec3& position, float intensity = 1.0f);
    void createExplosionLight(const glm::vec3& position, float duration = 1.0f);

    // HDR and post-processing
    void setExposure(float exposure);
    float getExposure() const { return m_config.exposure; }
    void setBloomThreshold(float threshold);
    float getBloomThreshold() const { return m_config.bloomThreshold; }

    // Configuration
    const LightingConfig& getConfig() const { return m_config; }
    void setConfig(const LightingConfig& config);

    // Shader uniforms helper
    void applyLightingToShader(Rendering::Shader* shader) const;
    void applyShadowsToShader(Rendering::Shader* shader) const;

    // Debug
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }
    void drawDebugLights();

private:
    // Update helpers
    void updateFlickeringLights(float deltaTime);
    void updateTemporaryLights(float deltaTime);
    void updateLightingZones();
    void calculateCombinedLighting();

    // Light culling
    bool isLightVisible(const Light& light, const glm::vec2& cameraPos, float viewDist) const;
    float calculateLightPriority(const Light& light, const glm::vec2& cameraPos) const;

    // Effect helpers
    glm::vec3 calculateAmbientWithEnvironment() const;
    glm::vec3 calculateDirectionalWithEnvironment() const;
    float calculateShadowIntensityWithEnvironment() const;

private:
    LightingConfig m_config;
    ShadowConfig m_shadowConfig;

    // Environment system references
    DayNightCycle* m_dayNightCycle;
    WeatherSystem* m_weatherSystem;
    SeasonSystem* m_seasonSystem;

    // Light storage
    std::unordered_map<int, Light> m_lights;
    int m_nextLightId;
    std::vector<int> m_visibleLightIds;

    // Global lighting
    glm::vec3 m_ambientColor;
    float m_ambientIntensity;
    glm::vec3 m_directionalDirection;
    glm::vec3 m_directionalColor;
    float m_directionalIntensity;

    // Combined/final lighting (after all modifiers)
    glm::vec3 m_finalAmbientColor;
    float m_finalAmbientIntensity;
    glm::vec3 m_finalDirectionalColor;
    float m_finalDirectionalIntensity;

    // Indoor/Outdoor system
    std::vector<LightingZone> m_zones;
    glm::vec2 m_playerPosition;
    bool m_isIndoors;
    const LightingZone* m_currentZone;

    // Temporary/special lights
    struct TemporaryLight {
        int lightId;
        float duration;
        float timer;
        float initialIntensity;
        bool fadeOut;
    };
    std::vector<TemporaryLight> m_temporaryLights;

    // Lightning flashes
    struct LightningFlash {
        glm::vec2 position;
        float duration;
        float timer;
        float intensity;
    };
    std::vector<LightningFlash> m_lightningFlashes;

    // Sun and moon light IDs
    int m_sunLightId;
    int m_moonLightId;

    // Debug
    bool m_debugMode;
};

} // namespace Environment
