#include "LightingManager.h"
#include "DayNightCycle.h"
#include "WeatherSystem.h"
#include "SeasonSystem.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Environment {

// Random number utilities
static std::random_device s_rd;
static std::mt19937 s_gen(s_rd());

static float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(s_gen);
}

LightingManager::LightingManager()
    : m_dayNightCycle(nullptr)
    , m_weatherSystem(nullptr)
    , m_seasonSystem(nullptr)
    , m_nextLightId(1)
    , m_ambientColor(0.3f, 0.3f, 0.4f)
    , m_ambientIntensity(0.3f)
    , m_directionalDirection(0.0f, -1.0f, 0.5f)
    , m_directionalColor(1.0f, 0.95f, 0.8f)
    , m_directionalIntensity(1.0f)
    , m_finalAmbientColor(0.3f, 0.3f, 0.4f)
    , m_finalAmbientIntensity(0.3f)
    , m_finalDirectionalColor(1.0f, 0.95f, 0.8f)
    , m_finalDirectionalIntensity(1.0f)
    , m_playerPosition(0.0f)
    , m_isIndoors(false)
    , m_currentZone(nullptr)
    , m_sunLightId(-1)
    , m_moonLightId(-1)
    , m_debugMode(false)
{
}

LightingManager::~LightingManager() {
    shutdown();
}

bool LightingManager::initialize(const LightingConfig& config) {
    m_config = config;

    // Set initial ambient
    m_ambientColor = config.ambientColor;
    m_ambientIntensity = config.ambientIntensity;

    // Create sun and moon lights
    m_sunLightId = createSunLight();
    m_moonLightId = createMoonLight();

    // Initially disable moon
    if (m_moonLightId >= 0) {
        enableLight(m_moonLightId, false);
    }

    return true;
}

void LightingManager::shutdown() {
    m_lights.clear();
    m_visibleLightIds.clear();
    m_temporaryLights.clear();
    m_lightningFlashes.clear();
    m_zones.clear();
}

void LightingManager::update(float deltaTime) {
    // Update time-based lighting
    if (m_dayNightCycle) {
        updateTimeBasedLighting();
    }

    // Update weather effects
    if (m_weatherSystem) {
        updateWeatherLighting();
    }

    // Update seasonal effects
    if (m_seasonSystem) {
        updateSeasonalLighting();
    }

    // Update flickering lights
    updateFlickeringLights(deltaTime);

    // Update temporary lights
    updateTemporaryLights(deltaTime);

    // Update lighting zones
    updateLightingZones();

    // Calculate final combined lighting
    calculateCombinedLighting();

    // Update lightning flashes
    for (auto it = m_lightningFlashes.begin(); it != m_lightningFlashes.end();) {
        it->timer += deltaTime;
        if (it->timer >= it->duration) {
            it = m_lightningFlashes.erase(it);
        } else {
            ++it;
        }
    }
}

void LightingManager::updateTimeBasedLighting() {
    if (!m_dayNightCycle) return;

    // Get lighting from day/night cycle
    const auto& lighting = m_dayNightCycle->getCurrentLighting();

    // Update ambient
    m_ambientColor = lighting.ambientColor;
    m_ambientIntensity = lighting.ambientIntensity;

    // Update sun
    if (m_sunLightId >= 0) {
        Light* sunLight = getLight(m_sunLightId);
        if (sunLight) {
            sunLight->direction = m_dayNightCycle->getSunDirection();
            sunLight->color = lighting.sunColor;
            sunLight->intensity = lighting.sunIntensity;
            sunLight->enabled = m_dayNightCycle->isDay();
        }
    }

    // Update moon
    if (m_moonLightId >= 0) {
        Light* moonLight = getLight(m_moonLightId);
        if (moonLight) {
            moonLight->direction = m_dayNightCycle->getMoonDirection();
            moonLight->color = glm::vec3(0.7f, 0.8f, 1.0f);
            moonLight->intensity = m_dayNightCycle->getMoonBrightness() * 0.4f;
            moonLight->enabled = m_dayNightCycle->isNight();
        }
    }

    // Update directional light (combine sun/moon)
    if (m_dayNightCycle->isDay()) {
        m_directionalDirection = m_dayNightCycle->getSunDirection();
        m_directionalColor = lighting.sunColor;
        m_directionalIntensity = lighting.sunIntensity;
    } else {
        m_directionalDirection = m_dayNightCycle->getMoonDirection();
        m_directionalColor = glm::vec3(0.7f, 0.8f, 1.0f);
        m_directionalIntensity = m_dayNightCycle->getMoonBrightness() * 0.4f;
    }
}

void LightingManager::updateWeatherLighting() {
    if (!m_weatherSystem) return;

    // Weather modifies lighting but doesn't replace it
    const auto& visuals = m_weatherSystem->getVisuals();

    // Apply weather tint
    m_ambientColor *= visuals.skyTint;
    m_ambientIntensity *= visuals.skyBrightness;

    // Reduce directional light in bad weather
    const auto& effects = m_weatherSystem->getGameplayEffects();
    m_directionalIntensity *= effects.visibilityRange;
}

void LightingManager::updateSeasonalLighting() {
    if (!m_seasonSystem) return;

    const auto& visuals = m_seasonSystem->getCurrentVisuals();

    // Apply seasonal tint
    m_ambientColor *= visuals.ambientTint;

    // Adjust saturation (affects all colors)
    // This would be applied in shader
}

void LightingManager::updateFlickeringLights(float deltaTime) {
    for (auto& pair : m_lights) {
        Light& light = pair.second;

        if (light.flickerAmount > 0.0f && light.enabled) {
            float flickerValue = std::sin(deltaTime * light.flickerSpeed * 10.0f) * light.flickerAmount;
            // Flicker is applied as a multiplier in shader or when getting intensity
            // Store base intensity and apply flicker when needed
        }
    }
}

void LightingManager::updateTemporaryLights(float deltaTime) {
    for (auto it = m_temporaryLights.begin(); it != m_temporaryLights.end();) {
        it->timer += deltaTime;

        if (it->timer >= it->duration) {
            // Remove light
            removeLight(it->lightId);
            it = m_temporaryLights.erase(it);
        } else {
            // Fade out if requested
            if (it->fadeOut) {
                float t = it->timer / it->duration;
                float intensity = it->initialIntensity * (1.0f - t);

                Light* light = getLight(it->lightId);
                if (light) {
                    light->intensity = intensity;
                }
            }
            ++it;
        }
    }
}

void LightingManager::updateLightingZones() {
    m_isIndoors = false;
    m_currentZone = nullptr;

    // Check which zone player is in
    for (const auto& zone : m_zones) {
        glm::vec2 min = zone.position - zone.size * 0.5f;
        glm::vec2 max = zone.position + zone.size * 0.5f;

        if (m_playerPosition.x >= min.x && m_playerPosition.x <= max.x &&
            m_playerPosition.y >= min.y && m_playerPosition.y <= max.y) {
            m_isIndoors = zone.isIndoor;
            m_currentZone = &zone;
            break;
        }
    }
}

void LightingManager::calculateCombinedLighting() {
    // Start with base lighting
    m_finalAmbientColor = m_ambientColor;
    m_finalAmbientIntensity = m_ambientIntensity;
    m_finalDirectionalColor = m_directionalColor;
    m_finalDirectionalIntensity = m_directionalIntensity;

    // Apply zone overrides if indoors
    if (m_isIndoors && m_currentZone) {
        m_finalAmbientColor = m_currentZone->ambientOverride;
        m_finalAmbientIntensity = m_currentZone->ambientIntensityOverride;
        // Indoor areas might not have strong directional light
        m_finalDirectionalIntensity *= 0.3f;
    }

    // Apply lightning flash if active
    for (const auto& flash : m_lightningFlashes) {
        float flashT = flash.timer / flash.duration;
        float flashIntensity = (1.0f - flashT) * flash.intensity;

        m_finalAmbientColor += glm::vec3(flashIntensity * 0.8f, flashIntensity * 0.9f, flashIntensity);
        m_finalAmbientIntensity += flashIntensity * 0.5f;
    }

    // Clamp values
    m_finalAmbientIntensity = glm::clamp(m_finalAmbientIntensity, 0.0f, 2.0f);
    m_finalDirectionalIntensity = glm::clamp(m_finalDirectionalIntensity, 0.0f, 2.0f);
}

int LightingManager::addLight(const Light& light) {
    int id = m_nextLightId++;
    m_lights[id] = light;
    return id;
}

void LightingManager::removeLight(int lightId) {
    m_lights.erase(lightId);

    // Remove from visible list
    auto it = std::find(m_visibleLightIds.begin(), m_visibleLightIds.end(), lightId);
    if (it != m_visibleLightIds.end()) {
        m_visibleLightIds.erase(it);
    }
}

void LightingManager::updateLight(int lightId, const Light& light) {
    auto it = m_lights.find(lightId);
    if (it != m_lights.end()) {
        it->second = light;
    }
}

Light* LightingManager::getLight(int lightId) {
    auto it = m_lights.find(lightId);
    if (it != m_lights.end()) {
        return &it->second;
    }
    return nullptr;
}

const Light* LightingManager::getLight(int lightId) const {
    auto it = m_lights.find(lightId);
    if (it != m_lights.end()) {
        return &it->second;
    }
    return nullptr;
}

void LightingManager::enableLight(int lightId, bool enabled) {
    Light* light = getLight(lightId);
    if (light) {
        light->enabled = enabled;
    }
}

void LightingManager::clearAllLights() {
    m_lights.clear();
    m_visibleLightIds.clear();
    m_sunLightId = -1;
    m_moonLightId = -1;
}

int LightingManager::createSunLight() {
    Light sun;
    sun.name = "Sun";
    sun.type = LightType::Directional;
    sun.direction = glm::vec3(0.0f, -1.0f, 0.5f);
    sun.color = glm::vec3(1.0f, 0.95f, 0.85f);
    sun.intensity = 1.0f;
    sun.castsShadows = m_config.enableShadows;
    sun.priority = 1000;
    return addLight(sun);
}

int LightingManager::createMoonLight() {
    Light moon;
    moon.name = "Moon";
    moon.type = LightType::Directional;
    moon.direction = glm::vec3(0.0f, -1.0f, -0.5f);
    moon.color = glm::vec3(0.7f, 0.8f, 1.0f);
    moon.intensity = 0.3f;
    moon.castsShadows = false;
    moon.priority = 900;
    moon.enabled = false;
    return addLight(moon);
}

int LightingManager::createTorchLight(const glm::vec3& position) {
    Light torch;
    torch.name = "Torch";
    torch.type = LightType::Point;
    torch.position = position;
    torch.color = glm::vec3(1.0f, 0.7f, 0.4f);
    torch.intensity = 1.5f;
    torch.radius = 8.0f;
    torch.castsShadows = false;
    torch.priority = 10;
    torch.flickerAmount = 0.15f;
    torch.flickerSpeed = 2.0f;
    return addLight(torch);
}

int LightingManager::createCampfireLight(const glm::vec3& position) {
    Light fire;
    fire.name = "Campfire";
    fire.type = LightType::Point;
    fire.position = position;
    fire.color = glm::vec3(1.0f, 0.6f, 0.2f);
    fire.intensity = 2.0f;
    fire.radius = 12.0f;
    fire.castsShadows = true;
    fire.priority = 20;
    fire.flickerAmount = 0.25f;
    fire.flickerSpeed = 3.0f;
    return addLight(fire);
}

int LightingManager::createMagicLight(const glm::vec3& position, const glm::vec3& color) {
    Light magic;
    magic.name = "Magic";
    magic.type = LightType::Point;
    magic.position = position;
    magic.color = color;
    magic.intensity = 1.8f;
    magic.radius = 10.0f;
    magic.castsShadows = false;
    magic.priority = 15;
    magic.flickerAmount = 0.1f;
    magic.flickerSpeed = 4.0f;
    return addLight(magic);
}

void LightingManager::setAmbientLight(const glm::vec3& color, float intensity) {
    m_ambientColor = color;
    m_ambientIntensity = intensity;
}

void LightingManager::setDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity) {
    m_directionalDirection = glm::normalize(direction);
    m_directionalColor = color;
    m_directionalIntensity = intensity;
}

void LightingManager::setShadowConfig(const ShadowConfig& config) {
    m_shadowConfig = config;
}

void LightingManager::enableShadows(bool enabled) {
    m_shadowConfig.enabled = enabled;
}

bool LightingManager::areShadowsEnabled() const {
    return m_shadowConfig.enabled && m_config.enableShadows;
}

void LightingManager::addLightingZone(const LightingZone& zone) {
    m_zones.push_back(zone);
}

void LightingManager::removeLightingZone(const std::string& name) {
    m_zones.erase(
        std::remove_if(m_zones.begin(), m_zones.end(),
            [&name](const LightingZone& zone) { return zone.name == name; }),
        m_zones.end()
    );
}

void LightingManager::setPlayerPosition(const glm::vec2& position) {
    m_playerPosition = position;
}

const LightingZone* LightingManager::getCurrentZone() const {
    return m_currentZone;
}

void LightingManager::cullLights(const glm::vec2& cameraPosition, float viewDistance) {
    m_visibleLightIds.clear();

    // Build list of visible lights with priorities
    struct LightScore {
        int id;
        float score;
    };
    std::vector<LightScore> scoredLights;

    for (const auto& pair : m_lights) {
        const Light& light = pair.second;

        if (!light.enabled) continue;

        if (isLightVisible(light, cameraPosition, viewDistance)) {
            float score = calculateLightPriority(light, cameraPosition);
            scoredLights.push_back({pair.first, score});
        }
    }

    // Sort by priority score
    std::sort(scoredLights.begin(), scoredLights.end(),
        [](const LightScore& a, const LightScore& b) { return a.score > b.score; });

    // Take top N lights
    int maxLights = std::min(m_config.maxLights, static_cast<int>(scoredLights.size()));
    for (int i = 0; i < maxLights; ++i) {
        m_visibleLightIds.push_back(scoredLights[i].id);
    }
}

std::vector<const Light*> LightingManager::getVisibleLights() const {
    std::vector<const Light*> lights;
    for (int id : m_visibleLightIds) {
        const Light* light = getLight(id);
        if (light) {
            lights.push_back(light);
        }
    }
    return lights;
}

int LightingManager::getActiveLightCount() const {
    int count = 0;
    for (const auto& pair : m_lights) {
        if (pair.second.enabled) {
            count++;
        }
    }
    return count;
}

bool LightingManager::isLightVisible(const Light& light, const glm::vec2& cameraPos, float viewDist) const {
    if (light.type == LightType::Directional || light.type == LightType::Ambient) {
        return true; // Always visible
    }

    glm::vec2 lightPos2D(light.position.x, light.position.y);
    float distance = glm::distance(lightPos2D, cameraPos);

    return distance <= (viewDist + light.radius);
}

float LightingManager::calculateLightPriority(const Light& light, const glm::vec2& cameraPos) const {
    float score = static_cast<float>(light.priority);

    // Higher intensity = higher priority
    score += light.intensity * 10.0f;

    // Closer lights = higher priority
    if (light.type != LightType::Directional && light.type != LightType::Ambient) {
        glm::vec2 lightPos2D(light.position.x, light.position.y);
        float distance = glm::distance(lightPos2D, cameraPos);
        float distanceScore = 1.0f / (1.0f + distance * 0.1f);
        score += distanceScore * 50.0f;
    }

    return score;
}

float LightingManager::getCurrentShadowIntensity() const {
    float intensity = 1.0f;

    if (m_dayNightCycle) {
        intensity = m_dayNightCycle->getShadowIntensity();
    }

    if (m_weatherSystem) {
        intensity *= m_weatherSystem->getVisibilityRange();
    }

    return intensity;
}

glm::vec3 LightingManager::getCurrentSkyColor() const {
    if (m_dayNightCycle) {
        return m_dayNightCycle->getCurrentLighting().skyColor;
    }
    return glm::vec3(0.5f, 0.7f, 1.0f);
}

glm::vec3 LightingManager::getCurrentFogColor() const {
    glm::vec3 fogColor(0.7f, 0.75f, 0.8f);

    if (m_dayNightCycle) {
        fogColor = m_dayNightCycle->getCurrentLighting().fogColor;
    }

    if (m_weatherSystem) {
        fogColor = m_weatherSystem->getVisuals().fogColor;
    }

    return fogColor;
}

float LightingManager::getCurrentFogDensity() const {
    float density = 0.1f;

    if (m_dayNightCycle) {
        density = m_dayNightCycle->getCurrentLighting().fogDensity;
    }

    if (m_weatherSystem) {
        density = glm::max(density, m_weatherSystem->getVisuals().fogDensity);
    }

    return density;
}

glm::vec3 LightingManager::getSeasonalTint() const {
    if (m_seasonSystem) {
        return m_seasonSystem->getCurrentVisuals().ambientTint;
    }
    return glm::vec3(1.0f);
}

void LightingManager::triggerLightningFlash(const glm::vec2& position, float duration) {
    LightningFlash flash;
    flash.position = position;
    flash.duration = duration;
    flash.timer = 0.0f;
    flash.intensity = 2.0f;
    m_lightningFlashes.push_back(flash);
}

void LightingManager::createFireLight(const glm::vec3& position, float intensity) {
    Light fire;
    fire.type = LightType::Point;
    fire.position = position;
    fire.color = glm::vec3(1.0f, 0.6f, 0.2f);
    fire.intensity = intensity;
    fire.radius = 10.0f * intensity;
    fire.flickerAmount = 0.3f;
    fire.flickerSpeed = 4.0f;

    int lightId = addLight(fire);

    TemporaryLight temp;
    temp.lightId = lightId;
    temp.duration = 60.0f; // Fire lasts 60 seconds
    temp.timer = 0.0f;
    temp.initialIntensity = intensity;
    temp.fadeOut = true;
    m_temporaryLights.push_back(temp);
}

void LightingManager::createExplosionLight(const glm::vec3& position, float duration) {
    Light explosion;
    explosion.type = LightType::Point;
    explosion.position = position;
    explosion.color = glm::vec3(1.0f, 0.8f, 0.4f);
    explosion.intensity = 5.0f;
    explosion.radius = 20.0f;

    int lightId = addLight(explosion);

    TemporaryLight temp;
    temp.lightId = lightId;
    temp.duration = duration;
    temp.timer = 0.0f;
    temp.initialIntensity = 5.0f;
    temp.fadeOut = true;
    m_temporaryLights.push_back(temp);
}

void LightingManager::setExposure(float exposure) {
    m_config.exposure = glm::clamp(exposure, 0.1f, 5.0f);
}

void LightingManager::setBloomThreshold(float threshold) {
    m_config.bloomThreshold = glm::clamp(threshold, 0.0f, 2.0f);
}

void LightingManager::setConfig(const LightingConfig& config) {
    m_config = config;
}

void LightingManager::applyLightingToShader(Rendering::Shader* shader) const {
    // TODO: Implement shader uniform application
    // This would set uniforms for:
    // - Ambient light
    // - Directional light
    // - Point lights array
    // - Light count
}

void LightingManager::applyShadowsToShader(Rendering::Shader* shader) const {
    // TODO: Implement shadow shader uniforms
    // This would set uniforms for:
    // - Shadow maps
    // - Shadow matrices
    // - Shadow parameters
}

void LightingManager::drawDebugLights() {
    // TODO: Implement debug visualization
    // Draw spheres for point lights
    // Draw direction indicators for directional lights
    // Show light ranges, intensities, etc.
}

} // namespace Environment
