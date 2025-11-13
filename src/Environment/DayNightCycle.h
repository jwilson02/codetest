#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <vector>

namespace Environment {

/**
 * @brief Time of day periods
 */
enum class TimeOfDay {
    Dawn,      // 5:00 - 7:00
    Morning,   // 7:00 - 12:00
    Afternoon, // 12:00 - 17:00
    Dusk,      // 17:00 - 19:00
    Night,     // 19:00 - 5:00
    Midnight   // 23:00 - 1:00
};

/**
 * @brief Moon phases affecting lighting and gameplay
 */
enum class MoonPhase {
    NewMoon,
    WaxingCrescent,
    FirstQuarter,
    WaxingGibbous,
    FullMoon,
    WaningGibbous,
    LastQuarter,
    WaningCrescent
};

/**
 * @brief Configuration for time system
 */
struct TimeConfig {
    float minutesPerGameDay = 24.0f;      // Real minutes for one game day
    float startHour = 8.0f;               // Starting hour (0-24)
    bool pauseTimeInInterior = true;      // Pause time when indoors
    bool enableCelestialEvents = true;    // Enable eclipses, meteor showers, etc.
    float sunriseHour = 6.0f;             // Sunrise time
    float sunsetHour = 18.0f;             // Sunset time
    float moonCycleDays = 28.0f;          // Days for complete moon cycle
    bool enableDynamicShadows = true;     // Dynamic shadows based on sun position
};

/**
 * @brief Lighting parameters for different times of day
 */
struct TimeOfDayLighting {
    glm::vec3 ambientColor;
    float ambientIntensity;
    glm::vec3 sunColor;
    float sunIntensity;
    glm::vec3 skyColor;
    glm::vec3 horizonColor;
    float fogDensity;
    glm::vec3 fogColor;
    glm::vec2 sunPosition;  // Normalized position on sky sphere
};

/**
 * @brief Celestial event types
 */
enum class CelestialEvent {
    None,
    SolarEclipse,
    LunarEclipse,
    MeteorShower,
    Aurora,
    CometPassing,
    BloodMoon
};

/**
 * @brief Main day/night cycle controller
 * Manages game time, lighting transitions, and celestial events
 */
class DayNightCycle {
public:
    DayNightCycle();
    ~DayNightCycle() = default;

    // Initialization
    void initialize(const TimeConfig& config = TimeConfig());
    void shutdown();

    // Update (call once per frame)
    void update(float deltaTime);

    // Time control
    void setTimeScale(float scale);          // 0 = paused, 1 = normal, 2 = 2x speed
    float getTimeScale() const { return m_timeScale; }
    void pauseTime();
    void resumeTime();
    bool isPaused() const { return m_paused; }

    // Current time
    void setTime(float hour);                // Set time (0-24)
    float getHour() const { return m_currentHour; }
    int getDay() const { return m_currentDay; }
    int getMonth() const { return m_currentMonth; }
    int getYear() const { return m_currentYear; }
    std::string getTimeString() const;       // Returns formatted time "HH:MM"
    std::string getDateString() const;       // Returns formatted date

    // Time of day
    TimeOfDay getTimeOfDay() const;
    bool isDay() const;
    bool isNight() const;
    bool isDawn() const { return getTimeOfDay() == TimeOfDay::Dawn; }
    bool isDusk() const { return getTimeOfDay() == TimeOfDay::Dusk; }

    // Moon phase
    MoonPhase getMoonPhase() const;
    float getMoonBrightness() const;         // 0.0 (new moon) to 1.0 (full moon)
    float getMoonVisibility() const;         // 0.0 (day/cloudy) to 1.0 (clear night)

    // Lighting
    const TimeOfDayLighting& getCurrentLighting() const { return m_currentLighting; }
    glm::vec3 getSunDirection() const;
    glm::vec3 getMoonDirection() const;
    float getShadowIntensity() const;        // Based on time and weather

    // Celestial events
    void triggerCelestialEvent(CelestialEvent event, float duration = 60.0f);
    CelestialEvent getCurrentEvent() const { return m_currentEvent; }
    bool hasActiveEvent() const { return m_currentEvent != CelestialEvent::None; }
    float getEventProgress() const;          // 0.0 to 1.0

    // Configuration
    const TimeConfig& getConfig() const { return m_config; }
    void setConfig(const TimeConfig& config);

    // Callbacks
    void onTimeOfDayChanged(std::function<void(TimeOfDay)> callback);
    void onHourChanged(std::function<void(int)> callback);
    void onDayChanged(std::function<void(int)> callback);
    void onCelestialEvent(std::function<void(CelestialEvent, bool)> callback); // event, started/ended

    // Utility
    float getNormalizedDayTime() const;      // 0.0 to 1.0 (0 = midnight, 0.5 = noon)
    bool isWithinTimeRange(float startHour, float endHour) const;

private:
    // Update helpers
    void updateTime(float deltaTime);
    void updateLighting();
    void updateCelestialEvent(float deltaTime);
    void checkTimeTransitions();

    // Lighting calculation
    TimeOfDayLighting calculateLighting(float hour) const;
    glm::vec3 interpolateColor(const glm::vec3& a, const glm::vec3& b, float t) const;
    float calculateSunAngle() const;
    float calculateMoonAngle() const;

    // Event helpers
    void applyCelestialEventEffects();

private:
    TimeConfig m_config;

    // Time state
    float m_currentHour;
    int m_currentDay;
    int m_currentMonth;
    int m_currentYear;
    float m_timeAccumulator;
    float m_timeScale;
    bool m_paused;

    // Previous state for transition detection
    TimeOfDay m_previousTimeOfDay;
    int m_previousHour;
    int m_previousDay;

    // Lighting
    TimeOfDayLighting m_currentLighting;

    // Celestial events
    CelestialEvent m_currentEvent;
    float m_eventDuration;
    float m_eventTimer;

    // Callbacks
    std::function<void(TimeOfDay)> m_onTimeOfDayChanged;
    std::function<void(int)> m_onHourChanged;
    std::function<void(int)> m_onDayChanged;
    std::function<void(CelestialEvent, bool)> m_onCelestialEvent;
};

} // namespace Environment
