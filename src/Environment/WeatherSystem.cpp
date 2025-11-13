#include "WeatherSystem.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <sstream>

namespace Environment {

// Random number generator
static std::random_device s_rd;
static std::mt19937 s_gen(s_rd());

static float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(s_gen);
}

static int randomInt(int min, int max) {
    std::uniform_int_distribution<int> dis(min, max);
    return dis(s_gen);
}

WeatherSystem::WeatherSystem()
    : m_currentWeather(WeatherType::Clear)
    , m_currentIntensity(WeatherIntensity::None)
    , m_weatherTimer(0.0f)
    , m_weatherDuration(600.0f)
    , m_isTransitioning(false)
    , m_targetWeather(WeatherType::Clear)
    , m_targetIntensity(WeatherIntensity::None)
    , m_transitionTimer(0.0f)
    , m_transitionDuration(30.0f)
    , m_currentRegion("default")
    , m_windTimer(0.0f)
    , m_particleSpawnTimer(0.0f)
    , m_rainbowActive(false)
    , m_rainbowTimer(0.0f)
    , m_rainbowDuration(0.0f)
    , m_randomAccumulator(0.0f)
{
}

WeatherSystem::~WeatherSystem() {
    shutdown();
}

bool WeatherSystem::initialize(const WeatherConfig& config) {
    m_config = config;

    // Initialize default weather patterns
    // These will be overridden if loaded from JSON
    WeatherPattern clearPattern;
    clearPattern.name = "Clear";
    clearPattern.type = WeatherType::Clear;
    clearPattern.intensity = WeatherIntensity::None;
    clearPattern.visuals.skyTint = glm::vec3(1.0f);
    clearPattern.visuals.skyBrightness = 1.0f;
    clearPattern.visuals.fogDensity = 0.0f;
    clearPattern.visuals.particleDensity = 0;
    clearPattern.probability = 0.4f;
    m_weatherPatterns[WeatherType::Clear] = clearPattern;

    // Set initial weather
    m_currentWeather = WeatherType::Clear;
    updateEffects();

    return true;
}

void WeatherSystem::shutdown() {
    clearWeatherParticles();
    m_weatherPatterns.clear();
    m_regionalWeather.clear();
}

void WeatherSystem::update(float deltaTime) {
    updateWeatherTimer(deltaTime);
    updateTransition(deltaTime);
    updateWind(deltaTime);
    updateParticles(deltaTime);

    // Update lightning strikes
    for (auto it = m_lightningStrikes.begin(); it != m_lightningStrikes.end();) {
        it->timer += deltaTime;
        if (it->timer >= it->duration) {
            it = m_lightningStrikes.erase(it);
        } else {
            ++it;
        }
    }

    // Update rainbow
    if (m_rainbowActive) {
        m_rainbowTimer += deltaTime;
        if (m_rainbowTimer >= m_rainbowDuration) {
            m_rainbowActive = false;
        }
    }

    // Check for dynamic weather changes
    if (m_config.enableDynamicWeather && !m_isTransitioning) {
        checkForDynamicChange();
    }
}

void WeatherSystem::updateWeatherTimer(float deltaTime) {
    if (m_isTransitioning) {
        return;
    }

    m_weatherTimer += deltaTime;

    if (m_weatherTimer >= m_weatherDuration && m_config.enableDynamicWeather) {
        triggerRandomWeatherChange();
    }
}

void WeatherSystem::updateTransition(float deltaTime) {
    if (!m_isTransitioning) {
        return;
    }

    m_transitionTimer += deltaTime;
    float t = glm::clamp(m_transitionTimer / m_transitionDuration, 0.0f, 1.0f);

    if (m_transitionTimer >= m_transitionDuration) {
        // Transition complete
        m_isTransitioning = false;
        m_currentWeather = m_targetWeather;
        m_currentIntensity = m_targetIntensity;
        m_transitionTimer = 0.0f;

        updateEffects();

        if (m_onWeatherChanged) {
            m_onWeatherChanged(m_currentWeather, m_targetWeather);
        }
    } else {
        // Interpolate effects during transition
        const auto* currentPattern = getWeatherPattern(m_currentWeather);
        const auto* targetPattern = getWeatherPattern(m_targetWeather);

        if (currentPattern && targetPattern) {
            // Interpolate visuals
            m_currentVisuals.skyTint = lerpColor(currentPattern->visuals.skyTint,
                                                  targetPattern->visuals.skyTint, t);
            m_currentVisuals.fogDensity = lerpFloat(currentPattern->visuals.fogDensity,
                                                     targetPattern->visuals.fogDensity, t);
            m_currentVisuals.fogColor = lerpColor(currentPattern->visuals.fogColor,
                                                   targetPattern->visuals.fogColor, t);

            // Interpolate gameplay effects
            m_currentEffects.movementSpeedMultiplier = lerpFloat(
                currentPattern->gameplayEffects.movementSpeedMultiplier,
                targetPattern->gameplayEffects.movementSpeedMultiplier, t);
            m_currentEffects.visibilityRange = lerpFloat(
                currentPattern->gameplayEffects.visibilityRange,
                targetPattern->gameplayEffects.visibilityRange, t);
        }
    }
}

void WeatherSystem::updateWind(float deltaTime) {
    m_windTimer += deltaTime;

    if (m_windTimer >= m_config.windUpdateFrequency) {
        m_windTimer = 0.0f;

        // Add turbulence
        float angleOffset = randomFloat(-m_currentWind.turbulence, m_currentWind.turbulence);
        float currentAngle = std::atan2(m_currentWind.direction.y, m_currentWind.direction.x);
        float newAngle = currentAngle + angleOffset;

        m_currentWind.direction = glm::vec2(std::cos(newAngle), std::sin(newAngle));

        // Add gusts
        if (m_currentWind.gustStrength > 0.0f && randomFloat(0.0f, 1.0f) < 0.1f) {
            m_currentWind.speed *= (1.0f + randomFloat(0.5f, m_currentWind.gustStrength));
        }
    }
}

void WeatherSystem::updateParticles(float deltaTime) {
    const auto* pattern = getCurrentPattern();
    if (!pattern || pattern->visuals.particleDensity == 0) {
        return;
    }

    // Spawn new particles
    m_particleSpawnTimer += deltaTime;
    float spawnInterval = 1.0f / (pattern->visuals.particleDensity * 10.0f);

    while (m_particleSpawnTimer >= spawnInterval && m_particles.size() < static_cast<size_t>(m_config.maxParticles)) {
        m_particleSpawnTimer -= spawnInterval;

        WeatherParticle particle;
        // Spawn across screen width
        particle.position = glm::vec2(randomFloat(-100.0f, 100.0f), 100.0f);

        // Calculate velocity based on wind and weather type
        glm::vec2 baseVelocity = pattern->visuals.particleVelocity;
        glm::vec2 windForce = getWindForce();
        particle.velocity = baseVelocity + windForce * 0.5f;

        particle.lifetime = 0.0f;
        particle.maxLifetime = pattern->visuals.particleLifetime;
        particle.size = randomFloat(pattern->visuals.particleSize.x, pattern->visuals.particleSize.y);

        glm::vec3 color3 = pattern->visuals.particleColor;
        particle.color = glm::vec4(color3.r, color3.g, color3.b, 1.0f);

        m_particles.push_back(particle);
    }

    // Update existing particles
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->lifetime += deltaTime;
        it->position += it->velocity * deltaTime;

        // Fade out near end of life
        float lifetimeRatio = it->lifetime / it->maxLifetime;
        if (lifetimeRatio > 0.7f) {
            it->color.a = 1.0f - (lifetimeRatio - 0.7f) / 0.3f;
        }

        // Remove dead particles or those that fell off screen
        if (it->lifetime >= it->maxLifetime || it->position.y < -100.0f) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }
}

void WeatherSystem::checkForDynamicChange() {
    // Accumulate random chance
    m_randomAccumulator += 0.001f;

    if (randomFloat(0.0f, 1.0f) < m_randomAccumulator) {
        triggerRandomWeatherChange();
        m_randomAccumulator = 0.0f;
    }
}

void WeatherSystem::setWeather(WeatherType type, WeatherIntensity intensity) {
    m_currentWeather = type;
    m_currentIntensity = intensity;
    m_isTransitioning = false;
    m_weatherTimer = 0.0f;
    m_weatherDuration = randomFloat(m_config.minWeatherDuration, m_config.maxWeatherDuration);

    updateEffects();

    if (m_onWeatherChanged) {
        m_onWeatherChanged(m_currentWeather, type);
    }
}

void WeatherSystem::transitionToWeather(WeatherType type, float duration) {
    if (type == m_currentWeather) {
        return;
    }

    m_targetWeather = type;
    m_targetIntensity = selectIntensity(type);
    m_isTransitioning = true;
    m_transitionTimer = 0.0f;
    m_transitionDuration = (duration > 0.0f) ? duration : m_config.transitionDuration;

    m_weatherTimer = 0.0f;
    m_weatherDuration = randomFloat(m_config.minWeatherDuration, m_config.maxWeatherDuration);
}

float WeatherSystem::getTransitionProgress() const {
    if (!m_isTransitioning || m_transitionDuration <= 0.0f) {
        return 1.0f;
    }
    return glm::clamp(m_transitionTimer / m_transitionDuration, 0.0f, 1.0f);
}

bool WeatherSystem::loadWeatherPatterns(const std::string& jsonPath) {
    // TODO: Implement JSON loading
    // For now, create some default patterns

    // Rain pattern
    WeatherPattern rain;
    rain.name = "Rain";
    rain.type = WeatherType::Rain;
    rain.intensity = WeatherIntensity::Moderate;
    rain.visuals.skyTint = glm::vec3(0.6f, 0.65f, 0.7f);
    rain.visuals.skyBrightness = 0.7f;
    rain.visuals.fogDensity = 0.3f;
    rain.visuals.fogColor = glm::vec3(0.5f, 0.55f, 0.6f);
    rain.visuals.particleDensity = 100;
    rain.visuals.particleColor = glm::vec3(0.7f, 0.8f, 0.9f);
    rain.visuals.particleSize = glm::vec2(1.0f, 2.0f);
    rain.visuals.particleVelocity = glm::vec2(0.0f, -50.0f);
    rain.visuals.particleLifetime = 2.0f;
    rain.gameplayEffects.movementSpeedMultiplier = 0.9f;
    rain.gameplayEffects.visibilityRange = 0.7f;
    rain.wind.speed = 5.0f;
    rain.probability = 0.2f;
    rain.soundEffects = {"rain_ambient"};
    m_weatherPatterns[WeatherType::Rain] = rain;

    // Snow pattern
    WeatherPattern snow;
    snow.name = "Snow";
    snow.type = WeatherType::Snow;
    snow.intensity = WeatherIntensity::Moderate;
    snow.visuals.skyTint = glm::vec3(0.85f, 0.88f, 0.9f);
    snow.visuals.skyBrightness = 0.9f;
    snow.visuals.fogDensity = 0.4f;
    snow.visuals.fogColor = glm::vec3(0.9f, 0.92f, 0.95f);
    snow.visuals.particleDensity = 50;
    snow.visuals.particleColor = glm::vec3(1.0f);
    snow.visuals.particleSize = glm::vec2(2.0f, 4.0f);
    snow.visuals.particleVelocity = glm::vec2(0.0f, -20.0f);
    snow.visuals.particleLifetime = 4.0f;
    snow.gameplayEffects.movementSpeedMultiplier = 0.8f;
    snow.gameplayEffects.visibilityRange = 0.6f;
    snow.gameplayEffects.causesCold = true;
    snow.wind.speed = 3.0f;
    snow.probability = 0.15f;
    snow.soundEffects = {"wind_cold"};
    m_weatherPatterns[WeatherType::Snow] = snow;

    // Thunderstorm pattern
    WeatherPattern storm;
    storm.name = "Thunderstorm";
    storm.type = WeatherType::Thunderstorm;
    storm.intensity = WeatherIntensity::Heavy;
    storm.visuals.skyTint = glm::vec3(0.3f, 0.35f, 0.4f);
    storm.visuals.skyBrightness = 0.4f;
    storm.visuals.fogDensity = 0.5f;
    storm.visuals.fogColor = glm::vec3(0.25f, 0.3f, 0.35f);
    storm.visuals.particleDensity = 150;
    storm.visuals.particleColor = glm::vec3(0.6f, 0.7f, 0.8f);
    storm.visuals.particleSize = glm::vec2(1.5f, 3.0f);
    storm.visuals.particleVelocity = glm::vec2(5.0f, -60.0f);
    storm.visuals.particleLifetime = 1.5f;
    storm.gameplayEffects.movementSpeedMultiplier = 0.75f;
    storm.gameplayEffects.visibilityRange = 0.5f;
    storm.gameplayEffects.accuracyMultiplier = 0.8f;
    storm.wind.speed = 15.0f;
    storm.wind.gustStrength = 1.5f;
    storm.wind.turbulence = 0.3f;
    storm.probability = 0.05f;
    storm.soundEffects = {"thunder", "rain_heavy", "wind_strong"};
    m_weatherPatterns[WeatherType::Thunderstorm] = storm;

    // Fog pattern
    WeatherPattern fog;
    fog.name = "Fog";
    fog.type = WeatherType::Fog;
    fog.intensity = WeatherIntensity::Moderate;
    fog.visuals.skyTint = glm::vec3(0.7f, 0.72f, 0.75f);
    fog.visuals.skyBrightness = 0.6f;
    fog.visuals.fogDensity = 0.8f;
    fog.visuals.fogColor = glm::vec3(0.7f, 0.75f, 0.8f);
    fog.visuals.particleDensity = 0;
    fog.gameplayEffects.movementSpeedMultiplier = 0.95f;
    fog.gameplayEffects.visibilityRange = 0.3f;
    fog.gameplayEffects.accuracyMultiplier = 0.7f;
    fog.wind.speed = 1.0f;
    fog.probability = 0.1f;
    fog.soundEffects = {"ambient_eerie"};
    m_weatherPatterns[WeatherType::Fog] = fog;

    return true;
}

void WeatherSystem::addWeatherPattern(const WeatherPattern& pattern) {
    m_weatherPatterns[pattern.type] = pattern;
}

const WeatherPattern* WeatherSystem::getWeatherPattern(WeatherType type) const {
    auto it = m_weatherPatterns.find(type);
    if (it != m_weatherPatterns.end()) {
        return &it->second;
    }
    return nullptr;
}

const WeatherPattern* WeatherSystem::getCurrentPattern() const {
    return getWeatherPattern(m_currentWeather);
}

void WeatherSystem::setRegion(const std::string& regionName) {
    m_currentRegion = regionName;
}

void WeatherSystem::addRegionalWeather(const RegionalWeather& regional) {
    m_regionalWeather[regional.regionName] = regional;
}

void WeatherSystem::enableDynamicWeather(bool enable) {
    m_config.enableDynamicWeather = enable;
}

void WeatherSystem::triggerRandomWeatherChange() {
    WeatherType nextWeather = selectNextWeather();
    transitionToWeather(nextWeather);
}

WeatherType WeatherSystem::selectNextWeather() const {
    // Build probability table
    std::vector<std::pair<WeatherType, float>> probabilities;
    float totalProb = 0.0f;

    for (const auto& pair : m_weatherPatterns) {
        float prob = getWeatherProbability(pair.first);
        probabilities.push_back({pair.first, prob});
        totalProb += prob;
    }

    // Select based on probability
    float roll = randomFloat(0.0f, totalProb);
    float accumulator = 0.0f;

    for (const auto& pair : probabilities) {
        accumulator += pair.second;
        if (roll <= accumulator) {
            return pair.first;
        }
    }

    return WeatherType::Clear;
}

WeatherIntensity WeatherSystem::selectIntensity(WeatherType type) const {
    // Simple random intensity selection
    int roll = randomInt(1, 100);

    if (roll <= 50) return WeatherIntensity::Light;
    if (roll <= 85) return WeatherIntensity::Moderate;
    if (roll <= 95) return WeatherIntensity::Heavy;
    return WeatherIntensity::Extreme;
}

float WeatherSystem::getWeatherProbability(WeatherType type) const {
    const auto* pattern = getWeatherPattern(type);
    if (!pattern) {
        return 0.0f;
    }

    float baseProbability = pattern->probability;

    // Apply regional modifiers
    if (m_config.enableRegionalWeather) {
        auto it = m_regionalWeather.find(m_currentRegion);
        if (it != m_regionalWeather.end()) {
            auto probIt = it->second.weatherProbabilities.find(type);
            if (probIt != it->second.weatherProbabilities.end()) {
                baseProbability *= probIt->second;
            }
        }
    }

    return baseProbability;
}

WeatherGameplayEffects WeatherSystem::calculateEffects() const {
    const auto* pattern = getCurrentPattern();
    if (!pattern) {
        return WeatherGameplayEffects();
    }

    // Could apply intensity modifiers here
    return pattern->gameplayEffects;
}

WeatherVisuals WeatherSystem::calculateVisuals() const {
    const auto* pattern = getCurrentPattern();
    if (!pattern) {
        return WeatherVisuals();
    }

    return pattern->visuals;
}

Wind WeatherSystem::calculateWind() const {
    const auto* pattern = getCurrentPattern();
    if (!pattern) {
        return Wind();
    }

    return pattern->wind;
}

void WeatherSystem::updateEffects() {
    m_currentEffects = calculateEffects();
    m_currentVisuals = calculateVisuals();
    m_currentWind = calculateWind();
    m_targetWind = m_currentWind;
}

glm::vec2 WeatherSystem::getWindForce() const {
    return m_currentWind.direction * m_currentWind.speed;
}

void WeatherSystem::setWind(const Wind& wind) {
    m_currentWind = wind;
}

void WeatherSystem::setVisibilityMultiplier(float multiplier) {
    m_currentEffects.visibilityRange *= multiplier;
}

void WeatherSystem::spawnWeatherParticles(int count, const glm::vec2& spawnArea) {
    const auto* pattern = getCurrentPattern();
    if (!pattern) {
        return;
    }

    for (int i = 0; i < count && m_particles.size() < static_cast<size_t>(m_config.maxParticles); ++i) {
        WeatherParticle particle;
        particle.position = glm::vec2(
            randomFloat(-spawnArea.x / 2, spawnArea.x / 2),
            randomFloat(0.0f, spawnArea.y)
        );
        particle.velocity = pattern->visuals.particleVelocity + getWindForce() * 0.5f;
        particle.lifetime = 0.0f;
        particle.maxLifetime = pattern->visuals.particleLifetime;
        particle.size = randomFloat(pattern->visuals.particleSize.x, pattern->visuals.particleSize.y);

        glm::vec3 color3 = pattern->visuals.particleColor;
        particle.color = glm::vec4(color3.r, color3.g, color3.b, 1.0f);

        m_particles.push_back(particle);
    }
}

void WeatherSystem::clearWeatherParticles() {
    m_particles.clear();
}

void WeatherSystem::updateWeatherAudio() {
    // TODO: Update audio system with current weather sounds
}

const std::vector<std::string>& WeatherSystem::getCurrentSounds() const {
    const auto* pattern = getCurrentPattern();
    if (pattern) {
        return pattern->soundEffects;
    }

    static std::vector<std::string> empty;
    return empty;
}

float WeatherSystem::getTemperature() const {
    // Base temperature
    float temp = 20.0f; // 20°C base

    // Modify by weather
    switch (m_currentWeather) {
        case WeatherType::Snow:
        case WeatherType::HeavySnow:
        case WeatherType::Blizzard:
            temp = -5.0f;
            break;
        case WeatherType::Clear:
            temp = 25.0f;
            break;
        case WeatherType::Rain:
            temp = 15.0f;
            break;
        case WeatherType::Thunderstorm:
            temp = 12.0f;
            break;
        case WeatherType::Fog:
            temp = 10.0f;
            break;
        default:
            break;
    }

    // Regional modifier
    auto it = m_regionalWeather.find(m_currentRegion);
    if (it != m_regionalWeather.end()) {
        temp += it->second.temperatureModifier;
    }

    return temp;
}

float WeatherSystem::getHumidity() const {
    switch (m_currentWeather) {
        case WeatherType::Rain:
        case WeatherType::HeavyRain:
        case WeatherType::Thunderstorm:
            return 0.9f;
        case WeatherType::Fog:
        case WeatherType::Mist:
            return 0.95f;
        case WeatherType::Clear:
            return 0.4f;
        case WeatherType::Snow:
            return 0.7f;
        default:
            return 0.5f;
    }
}

bool WeatherSystem::isRaining() const {
    return m_currentWeather == WeatherType::LightRain ||
           m_currentWeather == WeatherType::Rain ||
           m_currentWeather == WeatherType::HeavyRain ||
           m_currentWeather == WeatherType::Thunderstorm;
}

bool WeatherSystem::isSnowing() const {
    return m_currentWeather == WeatherType::LightSnow ||
           m_currentWeather == WeatherType::Snow ||
           m_currentWeather == WeatherType::HeavySnow ||
           m_currentWeather == WeatherType::Blizzard;
}

bool WeatherSystem::isStorming() const {
    return m_currentWeather == WeatherType::Thunderstorm ||
           m_currentWeather == WeatherType::Blizzard ||
           m_currentWeather == WeatherType::Sandstorm;
}

bool WeatherSystem::isClearSky() const {
    return m_currentWeather == WeatherType::Clear ||
           m_currentWeather == WeatherType::PartlyCloudy;
}

void WeatherSystem::setConfig(const WeatherConfig& config) {
    m_config = config;
}

void WeatherSystem::onWeatherChanged(std::function<void(WeatherType, WeatherType)> callback) {
    m_onWeatherChanged = callback;
}

void WeatherSystem::onWeatherIntensityChanged(std::function<void(WeatherIntensity)> callback) {
    m_onWeatherIntensityChanged = callback;
}

void WeatherSystem::onLightningStrike(std::function<void(glm::vec2)> callback) {
    m_onLightningStrike = callback;
}

void WeatherSystem::triggerLightningStrike(const glm::vec2& position) {
    Lightning strike;
    strike.position = position;
    strike.duration = 0.3f;
    strike.timer = 0.0f;
    m_lightningStrikes.push_back(strike);

    if (m_onLightningStrike) {
        m_onLightningStrike(position);
    }
}

void WeatherSystem::createTornado(const glm::vec2& position, float duration) {
    // TODO: Implement tornado system
}

void WeatherSystem::createRainbow(float duration) {
    m_rainbowActive = true;
    m_rainbowTimer = 0.0f;
    m_rainbowDuration = duration;
}

std::string WeatherSystem::getWeatherName() const {
    const auto* pattern = getCurrentPattern();
    if (pattern) {
        return pattern->name;
    }
    return "Unknown";
}

std::string WeatherSystem::getWeatherDescription() const {
    std::stringstream ss;
    ss << getWeatherName();

    if (m_currentIntensity != WeatherIntensity::None) {
        ss << " (";
        switch (m_currentIntensity) {
            case WeatherIntensity::Light: ss << "Light"; break;
            case WeatherIntensity::Moderate: ss << "Moderate"; break;
            case WeatherIntensity::Heavy: ss << "Heavy"; break;
            case WeatherIntensity::Extreme: ss << "Extreme"; break;
            default: break;
        }
        ss << ")";
    }

    return ss.str();
}

glm::vec3 WeatherSystem::lerpColor(const glm::vec3& a, const glm::vec3& b, float t) const {
    return glm::mix(a, b, t);
}

float WeatherSystem::lerpFloat(float a, float b, float t) const {
    return a + (b - a) * t;
}

} // namespace Environment
