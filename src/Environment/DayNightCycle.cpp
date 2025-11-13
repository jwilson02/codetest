#include "DayNightCycle.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Environment {

DayNightCycle::DayNightCycle()
    : m_currentHour(8.0f)
    , m_currentDay(1)
    , m_currentMonth(1)
    , m_currentYear(1)
    , m_timeAccumulator(0.0f)
    , m_timeScale(1.0f)
    , m_paused(false)
    , m_previousTimeOfDay(TimeOfDay::Morning)
    , m_previousHour(8)
    , m_previousDay(1)
    , m_currentEvent(CelestialEvent::None)
    , m_eventDuration(0.0f)
    , m_eventTimer(0.0f)
{
}

void DayNightCycle::initialize(const TimeConfig& config) {
    m_config = config;
    m_currentHour = config.startHour;
    updateLighting();
}

void DayNightCycle::shutdown() {
    // Cleanup if needed
}

void DayNightCycle::update(float deltaTime) {
    if (m_paused) {
        return;
    }

    updateTime(deltaTime);
    updateLighting();
    updateCelestialEvent(deltaTime);
    checkTimeTransitions();
}

void DayNightCycle::updateTime(float deltaTime) {
    // Convert real time to game time
    float gameTimeMultiplier = 24.0f / (m_config.minutesPerGameDay * 60.0f);
    m_timeAccumulator += deltaTime * m_timeScale * gameTimeMultiplier;

    // Add hours
    m_currentHour += m_timeAccumulator;
    m_timeAccumulator = 0.0f;

    // Handle day rollover
    if (m_currentHour >= 24.0f) {
        m_currentHour -= 24.0f;
        m_currentDay++;

        // Handle month rollover (simplified - 30 days per month)
        if (m_currentDay > 30) {
            m_currentDay = 1;
            m_currentMonth++;

            // Handle year rollover
            if (m_currentMonth > 12) {
                m_currentMonth = 1;
                m_currentYear++;
            }
        }
    }
}

void DayNightCycle::updateLighting() {
    m_currentLighting = calculateLighting(m_currentHour);

    // Apply celestial event modifiers
    if (hasActiveEvent()) {
        applyCelestialEventEffects();
    }
}

void DayNightCycle::updateCelestialEvent(float deltaTime) {
    if (!hasActiveEvent()) {
        return;
    }

    m_eventTimer += deltaTime;

    if (m_eventTimer >= m_eventDuration) {
        // Event ended
        if (m_onCelestialEvent) {
            m_onCelestialEvent(m_currentEvent, false);
        }
        m_currentEvent = CelestialEvent::None;
        m_eventTimer = 0.0f;
        m_eventDuration = 0.0f;
    }
}

void DayNightCycle::checkTimeTransitions() {
    // Check time of day transitions
    TimeOfDay currentTimeOfDay = getTimeOfDay();
    if (currentTimeOfDay != m_previousTimeOfDay) {
        if (m_onTimeOfDayChanged) {
            m_onTimeOfDayChanged(currentTimeOfDay);
        }
        m_previousTimeOfDay = currentTimeOfDay;
    }

    // Check hour transitions
    int currentHourInt = static_cast<int>(m_currentHour);
    if (currentHourInt != m_previousHour) {
        if (m_onHourChanged) {
            m_onHourChanged(currentHourInt);
        }
        m_previousHour = currentHourInt;
    }

    // Check day transitions
    if (m_currentDay != m_previousDay) {
        if (m_onDayChanged) {
            m_onDayChanged(m_currentDay);
        }
        m_previousDay = m_currentDay;
    }
}

TimeOfDayLighting DayNightCycle::calculateLighting(float hour) const {
    TimeOfDayLighting lighting;

    // Define key lighting states
    struct LightingKeyframe {
        float hour;
        TimeOfDayLighting lighting;
    };

    std::vector<LightingKeyframe> keyframes = {
        // Night (0:00)
        {0.0f, {
            glm::vec3(0.05f, 0.08f, 0.15f),  // ambient
            0.15f,                             // ambient intensity
            glm::vec3(0.8f, 0.85f, 1.0f),     // sun (not visible)
            0.0f,                              // sun intensity
            glm::vec3(0.01f, 0.02f, 0.08f),   // sky
            glm::vec3(0.02f, 0.04f, 0.12f),   // horizon
            0.4f,                              // fog density
            glm::vec3(0.05f, 0.08f, 0.15f)    // fog color
        }},
        // Dawn (6:00)
        {6.0f, {
            glm::vec3(0.4f, 0.35f, 0.45f),
            0.4f,
            glm::vec3(1.0f, 0.7f, 0.5f),
            0.3f,
            glm::vec3(0.5f, 0.35f, 0.4f),
            glm::vec3(1.0f, 0.5f, 0.3f),
            0.3f,
            glm::vec3(0.6f, 0.5f, 0.6f)
        }},
        // Morning (9:00)
        {9.0f, {
            glm::vec3(0.6f, 0.65f, 0.7f),
            0.7f,
            glm::vec3(1.0f, 0.95f, 0.9f),
            0.9f,
            glm::vec3(0.4f, 0.6f, 0.9f),
            glm::vec3(0.7f, 0.8f, 0.95f),
            0.1f,
            glm::vec3(0.7f, 0.8f, 0.9f)
        }},
        // Noon (12:00)
        {12.0f, {
            glm::vec3(0.7f, 0.75f, 0.8f),
            1.0f,
            glm::vec3(1.0f, 1.0f, 0.95f),
            1.0f,
            glm::vec3(0.3f, 0.5f, 0.9f),
            glm::vec3(0.6f, 0.75f, 0.95f),
            0.05f,
            glm::vec3(0.75f, 0.82f, 0.9f)
        }},
        // Afternoon (15:00)
        {15.0f, {
            glm::vec3(0.65f, 0.7f, 0.75f),
            0.9f,
            glm::vec3(1.0f, 0.95f, 0.85f),
            0.95f,
            glm::vec3(0.4f, 0.55f, 0.85f),
            glm::vec3(0.7f, 0.75f, 0.9f),
            0.08f,
            glm::vec3(0.72f, 0.8f, 0.88f)
        }},
        // Dusk (18:00)
        {18.0f, {
            glm::vec3(0.5f, 0.4f, 0.5f),
            0.5f,
            glm::vec3(1.0f, 0.5f, 0.3f),
            0.4f,
            glm::vec3(0.3f, 0.25f, 0.4f),
            glm::vec3(1.0f, 0.4f, 0.2f),
            0.25f,
            glm::vec3(0.6f, 0.5f, 0.65f)
        }},
        // Night (21:00)
        {21.0f, {
            glm::vec3(0.05f, 0.08f, 0.15f),
            0.2f,
            glm::vec3(0.8f, 0.85f, 1.0f),
            0.0f,
            glm::vec3(0.01f, 0.02f, 0.08f),
            glm::vec3(0.02f, 0.04f, 0.12f),
            0.35f,
            glm::vec3(0.08f, 0.1f, 0.18f)
        }},
        // Midnight (24:00)
        {24.0f, {
            glm::vec3(0.05f, 0.08f, 0.15f),
            0.15f,
            glm::vec3(0.8f, 0.85f, 1.0f),
            0.0f,
            glm::vec3(0.01f, 0.02f, 0.08f),
            glm::vec3(0.02f, 0.04f, 0.12f),
            0.4f,
            glm::vec3(0.05f, 0.08f, 0.15f)
        }}
    };

    // Find surrounding keyframes
    size_t nextIndex = 0;
    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (hour < keyframes[i].hour) {
            nextIndex = i;
            break;
        }
    }

    size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : keyframes.size() - 1;

    // Handle wrap-around
    float prevHour = keyframes[prevIndex].hour;
    float nextHour = keyframes[nextIndex].hour;
    if (nextHour < prevHour) {
        nextHour += 24.0f;
    }
    float currentHour = hour;
    if (currentHour < prevHour) {
        currentHour += 24.0f;
    }

    // Interpolate
    float t = (currentHour - prevHour) / (nextHour - prevHour);
    t = glm::clamp(t, 0.0f, 1.0f);

    const auto& prev = keyframes[prevIndex].lighting;
    const auto& next = keyframes[nextIndex].lighting;

    lighting.ambientColor = interpolateColor(prev.ambientColor, next.ambientColor, t);
    lighting.ambientIntensity = prev.ambientIntensity + (next.ambientIntensity - prev.ambientIntensity) * t;
    lighting.sunColor = interpolateColor(prev.sunColor, next.sunColor, t);
    lighting.sunIntensity = prev.sunIntensity + (next.sunIntensity - prev.sunIntensity) * t;
    lighting.skyColor = interpolateColor(prev.skyColor, next.skyColor, t);
    lighting.horizonColor = interpolateColor(prev.horizonColor, next.horizonColor, t);
    lighting.fogDensity = prev.fogDensity + (next.fogDensity - prev.fogDensity) * t;
    lighting.fogColor = interpolateColor(prev.fogColor, next.fogColor, t);

    // Calculate sun position
    float sunAngle = calculateSunAngle();
    lighting.sunPosition = glm::vec2(std::cos(sunAngle), std::sin(sunAngle));

    return lighting;
}

glm::vec3 DayNightCycle::interpolateColor(const glm::vec3& a, const glm::vec3& b, float t) const {
    // Smooth interpolation
    float smoothT = t * t * (3.0f - 2.0f * t);
    return glm::mix(a, b, smoothT);
}

float DayNightCycle::calculateSunAngle() const {
    // Sun moves from east (-π/2) at sunrise to west (π/2) at sunset
    float dayProgress = (m_currentHour - m_config.sunriseHour) /
                        (m_config.sunsetHour - m_config.sunriseHour);
    dayProgress = glm::clamp(dayProgress, 0.0f, 1.0f);

    return -glm::half_pi<float>() + dayProgress * glm::pi<float>();
}

float DayNightCycle::calculateMoonAngle() const {
    // Moon is opposite to sun
    return calculateSunAngle() + glm::pi<float>();
}

void DayNightCycle::applyCelestialEventEffects() {
    float eventT = getEventProgress();

    switch (m_currentEvent) {
        case CelestialEvent::SolarEclipse: {
            // Darken the world, add eerie tint
            float eclipseIntensity = 1.0f - std::abs(eventT - 0.5f) * 2.0f; // Peak at 50%
            m_currentLighting.ambientIntensity *= (1.0f - eclipseIntensity * 0.7f);
            m_currentLighting.sunIntensity *= (1.0f - eclipseIntensity * 0.9f);
            m_currentLighting.skyColor = glm::mix(m_currentLighting.skyColor,
                                                   glm::vec3(0.2f, 0.15f, 0.3f),
                                                   eclipseIntensity * 0.6f);
            break;
        }
        case CelestialEvent::LunarEclipse: {
            // Blood moon effect
            if (isNight()) {
                m_currentLighting.ambientColor = glm::mix(m_currentLighting.ambientColor,
                                                           glm::vec3(0.3f, 0.05f, 0.05f),
                                                           eventT * 0.5f);
            }
            break;
        }
        case CelestialEvent::MeteorShower: {
            // Slight brightness increase at night
            if (isNight()) {
                m_currentLighting.ambientIntensity *= 1.0f + eventT * 0.2f;
            }
            break;
        }
        case CelestialEvent::Aurora: {
            // Add greenish/purplish tint to night sky
            if (isNight()) {
                glm::vec3 auroraColor(0.2f, 0.8f, 0.6f);
                m_currentLighting.skyColor = glm::mix(m_currentLighting.skyColor,
                                                       auroraColor,
                                                       eventT * 0.4f);
                m_currentLighting.ambientColor = glm::mix(m_currentLighting.ambientColor,
                                                           auroraColor,
                                                           eventT * 0.2f);
            }
            break;
        }
        case CelestialEvent::BloodMoon: {
            // Deep red moon
            if (isNight()) {
                m_currentLighting.ambientColor = glm::mix(m_currentLighting.ambientColor,
                                                           glm::vec3(0.4f, 0.05f, 0.05f),
                                                           0.6f);
                m_currentLighting.ambientIntensity *= 1.3f;
            }
            break;
        }
        default:
            break;
    }
}

// Time control methods
void DayNightCycle::setTimeScale(float scale) {
    m_timeScale = glm::max(0.0f, scale);
}

void DayNightCycle::pauseTime() {
    m_paused = true;
}

void DayNightCycle::resumeTime() {
    m_paused = false;
}

void DayNightCycle::setTime(float hour) {
    m_currentHour = glm::clamp(hour, 0.0f, 24.0f);
    updateLighting();
}

std::string DayNightCycle::getTimeString() const {
    int hours = static_cast<int>(m_currentHour);
    int minutes = static_cast<int>((m_currentHour - hours) * 60.0f);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes;
    return ss.str();
}

std::string DayNightCycle::getDateString() const {
    std::stringstream ss;
    ss << "Day " << m_currentDay << ", Month " << m_currentMonth << ", Year " << m_currentYear;
    return ss.str();
}

TimeOfDay DayNightCycle::getTimeOfDay() const {
    if (m_currentHour >= 5.0f && m_currentHour < 7.0f) {
        return TimeOfDay::Dawn;
    } else if (m_currentHour >= 7.0f && m_currentHour < 12.0f) {
        return TimeOfDay::Morning;
    } else if (m_currentHour >= 12.0f && m_currentHour < 17.0f) {
        return TimeOfDay::Afternoon;
    } else if (m_currentHour >= 17.0f && m_currentHour < 19.0f) {
        return TimeOfDay::Dusk;
    } else if (m_currentHour >= 23.0f || m_currentHour < 1.0f) {
        return TimeOfDay::Midnight;
    } else {
        return TimeOfDay::Night;
    }
}

bool DayNightCycle::isDay() const {
    return m_currentHour >= m_config.sunriseHour && m_currentHour < m_config.sunsetHour;
}

bool DayNightCycle::isNight() const {
    return !isDay();
}

MoonPhase DayNightCycle::getMoonPhase() const {
    // Calculate based on days elapsed
    int totalDays = (m_currentYear - 1) * 360 + (m_currentMonth - 1) * 30 + m_currentDay;
    float moonCycleProgress = std::fmod(static_cast<float>(totalDays), m_config.moonCycleDays) / m_config.moonCycleDays;

    if (moonCycleProgress < 0.0625f) return MoonPhase::NewMoon;
    if (moonCycleProgress < 0.1875f) return MoonPhase::WaxingCrescent;
    if (moonCycleProgress < 0.3125f) return MoonPhase::FirstQuarter;
    if (moonCycleProgress < 0.4375f) return MoonPhase::WaxingGibbous;
    if (moonCycleProgress < 0.5625f) return MoonPhase::FullMoon;
    if (moonCycleProgress < 0.6875f) return MoonPhase::WaningGibbous;
    if (moonCycleProgress < 0.8125f) return MoonPhase::LastQuarter;
    return MoonPhase::WaningCrescent;
}

float DayNightCycle::getMoonBrightness() const {
    MoonPhase phase = getMoonPhase();
    switch (phase) {
        case MoonPhase::NewMoon: return 0.0f;
        case MoonPhase::WaxingCrescent: return 0.25f;
        case MoonPhase::FirstQuarter: return 0.5f;
        case MoonPhase::WaxingGibbous: return 0.75f;
        case MoonPhase::FullMoon: return 1.0f;
        case MoonPhase::WaningGibbous: return 0.75f;
        case MoonPhase::LastQuarter: return 0.5f;
        case MoonPhase::WaningCrescent: return 0.25f;
        default: return 0.5f;
    }
}

float DayNightCycle::getMoonVisibility() const {
    if (isDay()) {
        return 0.0f;
    }
    return getMoonBrightness();
}

glm::vec3 DayNightCycle::getSunDirection() const {
    float angle = calculateSunAngle();
    return glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
}

glm::vec3 DayNightCycle::getMoonDirection() const {
    float angle = calculateMoonAngle();
    return glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
}

float DayNightCycle::getShadowIntensity() const {
    if (isNight()) {
        return 0.2f * getMoonBrightness();
    }
    return m_currentLighting.sunIntensity * 0.8f;
}

void DayNightCycle::triggerCelestialEvent(CelestialEvent event, float duration) {
    m_currentEvent = event;
    m_eventDuration = duration;
    m_eventTimer = 0.0f;

    if (m_onCelestialEvent) {
        m_onCelestialEvent(event, true);
    }
}

float DayNightCycle::getEventProgress() const {
    if (!hasActiveEvent() || m_eventDuration <= 0.0f) {
        return 0.0f;
    }
    return glm::clamp(m_eventTimer / m_eventDuration, 0.0f, 1.0f);
}

void DayNightCycle::setConfig(const TimeConfig& config) {
    m_config = config;
}

float DayNightCycle::getNormalizedDayTime() const {
    return m_currentHour / 24.0f;
}

bool DayNightCycle::isWithinTimeRange(float startHour, float endHour) const {
    if (startHour <= endHour) {
        return m_currentHour >= startHour && m_currentHour < endHour;
    } else {
        // Handle overnight range (e.g., 22:00 to 6:00)
        return m_currentHour >= startHour || m_currentHour < endHour;
    }
}

// Callback setters
void DayNightCycle::onTimeOfDayChanged(std::function<void(TimeOfDay)> callback) {
    m_onTimeOfDayChanged = callback;
}

void DayNightCycle::onHourChanged(std::function<void(int)> callback) {
    m_onHourChanged = callback;
}

void DayNightCycle::onDayChanged(std::function<void(int)> callback) {
    m_onDayChanged = callback;
}

void DayNightCycle::onCelestialEvent(std::function<void(CelestialEvent, bool)> callback) {
    m_onCelestialEvent = callback;
}

} // namespace Environment
