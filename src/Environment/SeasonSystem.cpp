#include "SeasonSystem.h"
#include <algorithm>
#include <cmath>

namespace Environment {

SeasonSystem::SeasonSystem()
    : m_currentSeason(Season::Spring)
    , m_daysIntoSeason(0)
    , m_lastDayUpdate(0)
    , m_isTransitioning(false)
    , m_targetSeason(Season::Spring)
    , m_transitionProgress(0.0f)
{
}

void SeasonSystem::initialize(const SeasonConfig& config) {
    m_config = config;

    // Initialize default season definitions
    // Spring
    SeasonDefinition spring;
    spring.name = "Spring";
    spring.season = Season::Spring;
    spring.description = "A season of renewal, with blooming flowers and fresh growth.";

    spring.visuals.foliageColor = glm::vec3(0.4f, 0.9f, 0.3f);
    spring.visuals.groundColor = glm::vec3(0.5f, 0.7f, 0.3f);
    spring.visuals.waterColor = glm::vec3(0.3f, 0.5f, 0.8f);
    spring.visuals.foliageDensity = 0.9f;
    spring.visuals.snowCoverage = 0.0f;
    spring.visuals.bloomAmount = 1.0f;
    spring.visuals.ambientTint = glm::vec3(1.0f, 1.05f, 1.0f);
    spring.visuals.saturation = 1.2f;
    spring.visuals.particleEffect = "falling_petals";
    spring.visuals.particleDensity = 20;

    spring.gameplay.temperatureModifier = 15.0f;
    spring.gameplay.cropGrowthMultiplier = 1.2f;
    spring.gameplay.animalSpawnRate = 1.5f;
    spring.gameplay.fishAvailability = 1.1f;
    spring.gameplay.staminaDrainMultiplier = 1.0f;
    spring.gameplay.healthRegenModifier = 1.1f;
    spring.gameplay.availableResources = {"spring_herbs", "wild_flowers", "bird_eggs"};
    spring.gameplay.activeCreatures = {"rabbits", "deer", "songbirds"};
    spring.gameplay.dayLengthMultiplier = 1.0f;
    spring.gameplay.allowsFreeze = false;

    spring.weatherInfluence.clearProbability = 0.4f;
    spring.weatherInfluence.rainProbability = 0.35f;
    spring.weatherInfluence.snowProbability = 0.05f;
    spring.weatherInfluence.stormProbability = 0.15f;
    spring.weatherInfluence.fogProbability = 0.05f;
    spring.weatherInfluence.temperatureRange[0] = 10.0f;
    spring.weatherInfluence.temperatureRange[1] = 20.0f;

    spring.events = {"cherry_blossom_festival", "spring_awakening", "first_bloom"};

    m_seasonDefinitions[Season::Spring] = spring;

    // Summer
    SeasonDefinition summer;
    summer.name = "Summer";
    summer.season = Season::Summer;
    summer.description = "The warmest season, with long days and abundant sunshine.";

    summer.visuals.foliageColor = glm::vec3(0.3f, 0.7f, 0.2f);
    summer.visuals.groundColor = glm::vec3(0.6f, 0.6f, 0.3f);
    summer.visuals.waterColor = glm::vec3(0.2f, 0.4f, 0.7f);
    summer.visuals.foliageDensity = 1.0f;
    summer.visuals.snowCoverage = 0.0f;
    summer.visuals.bloomAmount = 0.5f;
    summer.visuals.ambientTint = glm::vec3(1.1f, 1.05f, 0.95f);
    summer.visuals.saturation = 1.1f;
    summer.visuals.particleEffect = "fireflies";
    summer.visuals.particleDensity = 30;

    summer.gameplay.temperatureModifier = 28.0f;
    summer.gameplay.cropGrowthMultiplier = 1.5f;
    summer.gameplay.animalSpawnRate = 1.0f;
    summer.gameplay.fishAvailability = 0.8f;
    summer.gameplay.staminaDrainMultiplier = 1.3f;
    summer.gameplay.healthRegenModifier = 1.0f;
    summer.gameplay.availableResources = {"summer_fruits", "honey", "medicinal_herbs"};
    summer.gameplay.activeCreatures = {"butterflies", "bees", "lizards"};
    summer.gameplay.dayLengthMultiplier = 1.2f;
    summer.gameplay.allowsFreeze = false;

    summer.weatherInfluence.clearProbability = 0.6f;
    summer.weatherInfluence.rainProbability = 0.15f;
    summer.weatherInfluence.snowProbability = 0.0f;
    summer.weatherInfluence.stormProbability = 0.2f;
    summer.weatherInfluence.fogProbability = 0.05f;
    summer.weatherInfluence.temperatureRange[0] = 22.0f;
    summer.weatherInfluence.temperatureRange[1] = 35.0f;

    summer.events = {"midsummer_celebration", "heat_wave", "firefly_night"};

    m_seasonDefinitions[Season::Summer] = summer;

    // Autumn
    SeasonDefinition autumn;
    autumn.name = "Autumn";
    autumn.season = Season::Autumn;
    autumn.description = "A season of harvest, with leaves turning golden and red.";

    autumn.visuals.foliageColor = glm::vec3(0.9f, 0.5f, 0.2f);
    autumn.visuals.groundColor = glm::vec3(0.7f, 0.5f, 0.3f);
    autumn.visuals.waterColor = glm::vec3(0.3f, 0.4f, 0.6f);
    autumn.visuals.foliageDensity = 0.7f;
    autumn.visuals.snowCoverage = 0.0f;
    autumn.visuals.bloomAmount = 0.2f;
    autumn.visuals.ambientTint = glm::vec3(1.05f, 0.95f, 0.9f);
    autumn.visuals.saturation = 1.0f;
    autumn.visuals.particleEffect = "falling_leaves";
    autumn.visuals.particleDensity = 50;

    autumn.gameplay.temperatureModifier = 12.0f;
    autumn.gameplay.cropGrowthMultiplier = 0.8f;
    autumn.gameplay.animalSpawnRate = 0.8f;
    autumn.gameplay.fishAvailability = 1.2f;
    autumn.gameplay.staminaDrainMultiplier = 1.0f;
    autumn.gameplay.healthRegenModifier = 0.95f;
    autumn.gameplay.availableResources = {"mushrooms", "nuts", "autumn_berries"};
    autumn.gameplay.activeCreatures = {"squirrels", "boars", "ravens"};
    autumn.gameplay.dayLengthMultiplier = 0.9f;
    autumn.gameplay.allowsFreeze = false;

    autumn.weatherInfluence.clearProbability = 0.35f;
    autumn.weatherInfluence.rainProbability = 0.3f;
    autumn.weatherInfluence.snowProbability = 0.1f;
    autumn.weatherInfluence.stormProbability = 0.15f;
    autumn.weatherInfluence.fogProbability = 0.1f;
    autumn.weatherInfluence.temperatureRange[0] = 5.0f;
    autumn.weatherInfluence.temperatureRange[1] = 18.0f;

    autumn.events = {"harvest_festival", "autumn_equinox", "migration"};

    m_seasonDefinitions[Season::Autumn] = autumn;

    // Winter
    SeasonDefinition winter;
    winter.name = "Winter";
    winter.season = Season::Winter;
    winter.description = "The coldest season, with snow and frost covering the land.";

    winter.visuals.foliageColor = glm::vec3(0.5f, 0.6f, 0.6f);
    winter.visuals.groundColor = glm::vec3(0.9f, 0.95f, 1.0f);
    winter.visuals.waterColor = glm::vec3(0.7f, 0.8f, 0.9f);
    winter.visuals.foliageDensity = 0.3f;
    winter.visuals.snowCoverage = 0.8f;
    winter.visuals.bloomAmount = 0.0f;
    winter.visuals.ambientTint = glm::vec3(0.95f, 0.98f, 1.05f);
    winter.visuals.saturation = 0.8f;
    winter.visuals.particleEffect = "snow_sparkles";
    winter.visuals.particleDensity = 15;

    winter.gameplay.temperatureModifier = -5.0f;
    winter.gameplay.cropGrowthMultiplier = 0.2f;
    winter.gameplay.animalSpawnRate = 0.3f;
    winter.gameplay.fishAvailability = 0.5f;
    winter.gameplay.staminaDrainMultiplier = 1.5f;
    winter.gameplay.healthRegenModifier = 0.8f;
    winter.gameplay.availableResources = {"winter_roots", "ice_crystals", "pine_cones"};
    winter.gameplay.activeCreatures = {"wolves", "foxes", "owls"};
    winter.gameplay.dayLengthMultiplier = 0.7f;
    winter.gameplay.allowsFreeze = true;

    winter.weatherInfluence.clearProbability = 0.3f;
    winter.weatherInfluence.rainProbability = 0.1f;
    winter.weatherInfluence.snowProbability = 0.45f;
    winter.weatherInfluence.stormProbability = 0.1f;
    winter.weatherInfluence.fogProbability = 0.05f;
    winter.weatherInfluence.temperatureRange[0] = -10.0f;
    winter.weatherInfluence.temperatureRange[1] = 5.0f;

    winter.events = {"winter_solstice", "aurora_nights", "frozen_lake"};

    m_seasonDefinitions[Season::Winter] = winter;

    // Set initial season
    if (config.startInSpring) {
        m_currentSeason = Season::Spring;
    }

    updateVisuals();
    updateGameplay();
    updateWeatherInfluence();
}

void SeasonSystem::shutdown() {
    m_seasonDefinitions.clear();
    m_activeEvents.clear();
}

void SeasonSystem::update(float deltaTime, int currentDay) {
    if (!m_config.enableSeasonalCycle) {
        return;
    }

    // Check if day changed
    if (currentDay != m_lastDayUpdate) {
        m_daysIntoSeason++;
        m_lastDayUpdate = currentDay;

        checkSeasonTransition(currentDay);
    }

    // Update transition if active
    if (m_isTransitioning && m_config.enableTransitions) {
        float daysElapsed = static_cast<float>(m_daysIntoSeason);
        float progress = daysElapsed / m_config.transitionDays;
        progress = glm::clamp(progress, 0.0f, 1.0f);

        updateTransition(progress);

        if (progress >= 1.0f) {
            m_isTransitioning = false;
            m_currentSeason = m_targetSeason;
            updateVisuals();
            updateGameplay();
            updateWeatherInfluence();
        }
    }
}

void SeasonSystem::checkSeasonTransition(int currentDay) {
    if (m_daysIntoSeason >= static_cast<int>(m_config.daysPerSeason)) {
        advanceSeason();
    }
}

void SeasonSystem::advanceSeason() {
    Season nextSeason = getNextSeason();

    if (m_onSeasonChanged) {
        m_onSeasonChanged(m_currentSeason, nextSeason);
    }

    if (m_config.enableTransitions) {
        m_isTransitioning = true;
        m_targetSeason = nextSeason;
        m_transitionProgress = 0.0f;
    } else {
        m_currentSeason = nextSeason;
        updateVisuals();
        updateGameplay();
        updateWeatherInfluence();
    }

    m_daysIntoSeason = 0;

    // Clear and trigger new seasonal events
    m_activeEvents.clear();
    const auto* def = getSeasonDefinition(nextSeason);
    if (def && !def->events.empty()) {
        // Trigger first event of the season
        triggerSeasonalEvent(def->events[0]);
    }
}

void SeasonSystem::updateTransition(float progress) {
    m_transitionProgress = progress;

    const auto* currentDef = getSeasonDefinition(m_currentSeason);
    const auto* targetDef = getSeasonDefinition(m_targetSeason);

    if (!currentDef || !targetDef) {
        return;
    }

    // Interpolate visuals
    m_currentVisuals.foliageColor = lerpColor(currentDef->visuals.foliageColor,
                                               targetDef->visuals.foliageColor, progress);
    m_currentVisuals.groundColor = lerpColor(currentDef->visuals.groundColor,
                                              targetDef->visuals.groundColor, progress);
    m_currentVisuals.waterColor = lerpColor(currentDef->visuals.waterColor,
                                             targetDef->visuals.waterColor, progress);
    m_currentVisuals.foliageDensity = lerpFloat(currentDef->visuals.foliageDensity,
                                                 targetDef->visuals.foliageDensity, progress);
    m_currentVisuals.snowCoverage = lerpFloat(currentDef->visuals.snowCoverage,
                                               targetDef->visuals.snowCoverage, progress);
    m_currentVisuals.bloomAmount = lerpFloat(currentDef->visuals.bloomAmount,
                                              targetDef->visuals.bloomAmount, progress);
    m_currentVisuals.saturation = lerpFloat(currentDef->visuals.saturation,
                                             targetDef->visuals.saturation, progress);

    // Interpolate gameplay effects
    m_currentGameplay.temperatureModifier = lerpFloat(currentDef->gameplay.temperatureModifier,
                                                       targetDef->gameplay.temperatureModifier, progress);
    m_currentGameplay.cropGrowthMultiplier = lerpFloat(currentDef->gameplay.cropGrowthMultiplier,
                                                        targetDef->gameplay.cropGrowthMultiplier, progress);
}

void SeasonSystem::updateVisuals() {
    const auto* def = getCurrentDefinition();
    if (def) {
        m_currentVisuals = def->visuals;
    }
}

void SeasonSystem::updateGameplay() {
    const auto* def = getCurrentDefinition();
    if (def) {
        m_currentGameplay = def->gameplay;
    }
}

void SeasonSystem::updateWeatherInfluence() {
    const auto* def = getCurrentDefinition();
    if (def) {
        m_currentWeatherInfluence = def->weatherInfluence;
    }
}

void SeasonSystem::setSeason(Season season) {
    Season oldSeason = m_currentSeason;
    m_currentSeason = season;
    m_daysIntoSeason = 0;
    m_isTransitioning = false;

    updateVisuals();
    updateGameplay();
    updateWeatherInfluence();

    if (m_onSeasonChanged) {
        m_onSeasonChanged(oldSeason, season);
    }

    // Trigger first seasonal event
    const auto* def = getCurrentDefinition();
    if (def && !def->events.empty()) {
        m_activeEvents.clear();
        triggerSeasonalEvent(def->events[0]);
    }
}

Season SeasonSystem::getNextSeason() const {
    switch (m_currentSeason) {
        case Season::Spring: return Season::Summer;
        case Season::Summer: return Season::Autumn;
        case Season::Autumn: return Season::Winter;
        case Season::Winter: return Season::Spring;
        default: return Season::Spring;
    }
}

float SeasonSystem::getTransitionProgress() const {
    return m_transitionProgress;
}

const SeasonDefinition* SeasonSystem::getSeasonDefinition(Season season) const {
    auto it = m_seasonDefinitions.find(season);
    if (it != m_seasonDefinitions.end()) {
        return &it->second;
    }
    return nullptr;
}

const SeasonDefinition* SeasonSystem::getCurrentDefinition() const {
    return getSeasonDefinition(m_currentSeason);
}

std::string SeasonSystem::getSeasonName() const {
    const auto* def = getCurrentDefinition();
    if (def) {
        return def->name;
    }
    return "Unknown";
}

std::string SeasonSystem::getSeasonDescription() const {
    const auto* def = getCurrentDefinition();
    if (def) {
        return def->description;
    }
    return "";
}

int SeasonSystem::getDaysUntilNextSeason() const {
    return static_cast<int>(m_config.daysPerSeason) - m_daysIntoSeason;
}

const std::vector<std::string>& SeasonSystem::getAvailableResources() const {
    static std::vector<std::string> empty;
    const auto* def = getCurrentDefinition();
    if (def) {
        return def->gameplay.availableResources;
    }
    return empty;
}

const std::vector<std::string>& SeasonSystem::getActiveCreatures() const {
    static std::vector<std::string> empty;
    const auto* def = getCurrentDefinition();
    if (def) {
        return def->gameplay.activeCreatures;
    }
    return empty;
}

bool SeasonSystem::isResourceAvailable(const std::string& resourceName) const {
    const auto& resources = getAvailableResources();
    return std::find(resources.begin(), resources.end(), resourceName) != resources.end();
}

bool SeasonSystem::isCreatureActive(const std::string& creatureName) const {
    const auto& creatures = getActiveCreatures();
    return std::find(creatures.begin(), creatures.end(), creatureName) != creatures.end();
}

void SeasonSystem::triggerSeasonalEvent(const std::string& eventName) {
    if (std::find(m_activeEvents.begin(), m_activeEvents.end(), eventName) == m_activeEvents.end()) {
        m_activeEvents.push_back(eventName);

        if (m_onSeasonalEvent) {
            m_onSeasonalEvent(eventName);
        }
    }
}

const std::vector<std::string>& SeasonSystem::getCurrentEvents() const {
    return m_activeEvents;
}

bool SeasonSystem::hasEvent(const std::string& eventName) const {
    return std::find(m_activeEvents.begin(), m_activeEvents.end(), eventName) != m_activeEvents.end();
}

void SeasonSystem::setConfig(const SeasonConfig& config) {
    m_config = config;
}

void SeasonSystem::addSeasonDefinition(const SeasonDefinition& definition) {
    m_seasonDefinitions[definition.season] = definition;
}

bool SeasonSystem::loadSeasonDefinitions(const std::string& jsonPath) {
    // TODO: Implement JSON loading
    return false;
}

void SeasonSystem::onSeasonChanged(std::function<void(Season, Season)> callback) {
    m_onSeasonChanged = callback;
}

void SeasonSystem::onSeasonalEvent(std::function<void(const std::string&)> callback) {
    m_onSeasonalEvent = callback;
}

std::string SeasonSystem::seasonToString(Season season) {
    switch (season) {
        case Season::Spring: return "Spring";
        case Season::Summer: return "Summer";
        case Season::Autumn: return "Autumn";
        case Season::Winter: return "Winter";
        default: return "Unknown";
    }
}

Season SeasonSystem::stringToSeason(const std::string& str) {
    if (str == "Spring") return Season::Spring;
    if (str == "Summer") return Season::Summer;
    if (str == "Autumn") return Season::Autumn;
    if (str == "Winter") return Season::Winter;
    return Season::Spring;
}

float SeasonSystem::getYearProgress() const {
    int seasonIndex = static_cast<int>(m_currentSeason);
    float totalDays = m_config.daysPerSeason * 4.0f;
    float currentDayInYear = seasonIndex * m_config.daysPerSeason + m_daysIntoSeason;
    return currentDayInYear / totalDays;
}

float SeasonSystem::getWeatherProbability(const std::string& weatherType) const {
    if (weatherType == "clear") return m_currentWeatherInfluence.clearProbability;
    if (weatherType == "rain") return m_currentWeatherInfluence.rainProbability;
    if (weatherType == "snow") return m_currentWeatherInfluence.snowProbability;
    if (weatherType == "storm") return m_currentWeatherInfluence.stormProbability;
    if (weatherType == "fog") return m_currentWeatherInfluence.fogProbability;
    return 0.0f;
}

glm::vec3 SeasonSystem::lerpColor(const glm::vec3& a, const glm::vec3& b, float t) const {
    float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep
    return glm::mix(a, b, smoothT);
}

float SeasonSystem::lerpFloat(float a, float b, float t) const {
    float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep
    return a + (b - a) * smoothT;
}

} // namespace Environment
