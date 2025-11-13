#include "DayNightCycle.h"
#include "WeatherSystem.h"
#include "SeasonSystem.h"
#include "LightingManager.h"
#include <iostream>
#include <iomanip>

/**
 * Example demonstrating the Environment System
 * Shows how to integrate day/night cycle, weather, seasons, and lighting
 */

namespace Environment {

class EnvironmentExample {
public:
    void initialize() {
        std::cout << "=== Initializing Environment System ===" << std::endl;

        // Initialize day/night cycle
        TimeConfig timeConfig;
        timeConfig.minutesPerGameDay = 12.0f;  // Fast cycle for demo
        timeConfig.startHour = 8.0f;
        timeConfig.enableCelestialEvents = true;
        m_dayNightCycle.initialize(timeConfig);

        // Set up time callbacks
        m_dayNightCycle.onTimeOfDayChanged([](TimeOfDay tod) {
            std::cout << "Time of day changed to: ";
            switch (tod) {
                case TimeOfDay::Dawn: std::cout << "Dawn"; break;
                case TimeOfDay::Morning: std::cout << "Morning"; break;
                case TimeOfDay::Afternoon: std::cout << "Afternoon"; break;
                case TimeOfDay::Dusk: std::cout << "Dusk"; break;
                case TimeOfDay::Night: std::cout << "Night"; break;
                case TimeOfDay::Midnight: std::cout << "Midnight"; break;
            }
            std::cout << std::endl;
        });

        m_dayNightCycle.onCelestialEvent([](CelestialEvent event, bool started) {
            if (started) {
                std::cout << "Celestial event started: ";
                switch (event) {
                    case CelestialEvent::SolarEclipse: std::cout << "Solar Eclipse"; break;
                    case CelestialEvent::LunarEclipse: std::cout << "Lunar Eclipse"; break;
                    case CelestialEvent::MeteorShower: std::cout << "Meteor Shower"; break;
                    case CelestialEvent::Aurora: std::cout << "Aurora"; break;
                    case CelestialEvent::BloodMoon: std::cout << "Blood Moon"; break;
                    default: break;
                }
                std::cout << std::endl;
            }
        });

        // Initialize weather system
        WeatherConfig weatherConfig;
        weatherConfig.enableDynamicWeather = true;
        weatherConfig.transitionDuration = 10.0f;  // Fast transitions for demo
        weatherConfig.minWeatherDuration = 30.0f;
        weatherConfig.maxWeatherDuration = 60.0f;
        m_weatherSystem.initialize(weatherConfig);
        m_weatherSystem.loadWeatherPatterns("data/environment/weather_patterns.json");

        m_weatherSystem.onWeatherChanged([](WeatherType oldWeather, WeatherType newWeather) {
            std::cout << "Weather changing to: " << getWeatherTypeName(newWeather) << std::endl;
        });

        m_weatherSystem.onLightningStrike([](glm::vec2 pos) {
            std::cout << "Lightning strike at (" << pos.x << ", " << pos.y << ")!" << std::endl;
        });

        // Initialize season system
        SeasonConfig seasonConfig;
        seasonConfig.daysPerSeason = 5.0f;  // Short seasons for demo
        seasonConfig.enableSeasonalCycle = true;
        seasonConfig.transitionDays = 1.0f;
        m_seasonSystem.initialize(seasonConfig);

        m_seasonSystem.onSeasonChanged([](Season oldSeason, Season newSeason) {
            std::cout << "Season changed to: " << SeasonSystem::seasonToString(newSeason) << std::endl;
        });

        m_seasonSystem.onSeasonalEvent([](const std::string& eventName) {
            std::cout << "Seasonal event: " << eventName << std::endl;
        });

        // Initialize lighting manager
        LightingConfig lightConfig;
        lightConfig.maxLights = 32;
        lightConfig.enableDynamicLighting = true;
        lightConfig.enableShadows = true;
        lightConfig.enableHDR = true;
        m_lightingManager.initialize(lightConfig);

        // Connect environment systems to lighting
        m_lightingManager.setDayNightCycle(&m_dayNightCycle);
        m_lightingManager.setWeatherSystem(&m_weatherSystem);
        m_lightingManager.setSeasonSystem(&m_seasonSystem);

        // Create some example lights
        createExampleLights();

        std::cout << "Environment system initialized successfully!" << std::endl << std::endl;
    }

    void update(float deltaTime) {
        // Update all systems
        m_dayNightCycle.update(deltaTime);
        m_weatherSystem.update(deltaTime);
        m_seasonSystem.update(deltaTime, m_dayNightCycle.getDay());
        m_lightingManager.update(deltaTime);

        // Print status every 5 seconds
        m_statusTimer += deltaTime;
        if (m_statusTimer >= 5.0f) {
            m_statusTimer = 0.0f;
            printStatus();
        }
    }

    void demonstrateFeatures() {
        std::cout << "\n=== Demonstrating Features ===" << std::endl;

        // Time control
        std::cout << "\n1. Time Control:" << std::endl;
        std::cout << "   Current time: " << m_dayNightCycle.getTimeString() << std::endl;
        std::cout << "   Is day: " << (m_dayNightCycle.isDay() ? "Yes" : "No") << std::endl;
        std::cout << "   Moon phase: " << getMoonPhaseName(m_dayNightCycle.getMoonPhase()) << std::endl;

        // Weather control
        std::cout << "\n2. Weather System:" << std::endl;
        std::cout << "   Current weather: " << m_weatherSystem.getWeatherName() << std::endl;
        std::cout << "   Temperature: " << m_weatherSystem.getTemperature() << "°C" << std::endl;
        std::cout << "   Visibility: " << (m_weatherSystem.getVisibilityRange() * 100) << "%" << std::endl;

        // Season info
        std::cout << "\n3. Seasonal System:" << std::endl;
        std::cout << "   Current season: " << m_seasonSystem.getSeasonName() << std::endl;
        std::cout << "   Days into season: " << m_seasonSystem.getDaysIntoSeason() << std::endl;
        std::cout << "   Snow coverage: " << (m_seasonSystem.getSnowCoverage() * 100) << "%" << std::endl;

        // Lighting info
        std::cout << "\n4. Lighting System:" << std::endl;
        std::cout << "   Active lights: " << m_lightingManager.getActiveLightCount() << std::endl;
        std::cout << "   Shadow intensity: " << m_lightingManager.getCurrentShadowIntensity() << std::endl;
        std::cout << "   Is indoors: " << (m_lightingManager.isPlayerIndoors() ? "Yes" : "No") << std::endl;

        // Trigger special events
        std::cout << "\n5. Special Events:" << std::endl;
        std::cout << "   Triggering solar eclipse..." << std::endl;
        m_dayNightCycle.triggerCelestialEvent(CelestialEvent::SolarEclipse, 60.0f);

        std::cout << "   Triggering thunderstorm..." << std::endl;
        m_weatherSystem.transitionToWeather(WeatherType::Thunderstorm, 5.0f);

        std::cout << "   Triggering lightning..." << std::endl;
        m_weatherSystem.triggerLightningStrike(glm::vec2(10.0f, 20.0f));
        m_lightingManager.triggerLightningFlash(glm::vec2(10.0f, 20.0f));
    }

    void demonstrateIndoorOutdoor() {
        std::cout << "\n=== Indoor/Outdoor Detection ===" << std::endl;

        // Create lighting zones
        LightingZone outdoorZone;
        outdoorZone.name = "forest_clearing";
        outdoorZone.position = glm::vec2(0.0f, 0.0f);
        outdoorZone.size = glm::vec2(100.0f, 100.0f);
        outdoorZone.isIndoor = false;
        m_lightingManager.addLightingZone(outdoorZone);

        LightingZone indoorZone;
        indoorZone.name = "tavern";
        indoorZone.position = glm::vec2(150.0f, 0.0f);
        indoorZone.size = glm::vec2(30.0f, 30.0f);
        indoorZone.isIndoor = true;
        indoorZone.ambientOverride = glm::vec3(0.8f, 0.6f, 0.4f);
        indoorZone.ambientIntensityOverride = 0.6f;
        m_lightingManager.addLightingZone(indoorZone);

        // Test player positions
        std::cout << "Player outdoors:" << std::endl;
        m_lightingManager.setPlayerPosition(glm::vec2(0.0f, 0.0f));
        m_lightingManager.update(0.0f);
        std::cout << "   Indoors: " << (m_lightingManager.isPlayerIndoors() ? "Yes" : "No") << std::endl;

        std::cout << "Player enters tavern:" << std::endl;
        m_lightingManager.setPlayerPosition(glm::vec2(150.0f, 0.0f));
        m_lightingManager.update(0.0f);
        std::cout << "   Indoors: " << (m_lightingManager.isPlayerIndoors() ? "Yes" : "No") << std::endl;
    }

    void demonstrateGameplayEffects() {
        std::cout << "\n=== Gameplay Effects ===" << std::endl;

        const auto& weatherEffects = m_weatherSystem.getGameplayEffects();
        const auto& seasonalEffects = m_seasonSystem.getCurrentGameplay();

        std::cout << "Weather Effects:" << std::endl;
        std::cout << "   Movement speed: " << (weatherEffects.movementSpeedMultiplier * 100) << "%" << std::endl;
        std::cout << "   Visibility: " << (weatherEffects.visibilityRange * 100) << "%" << std::endl;
        std::cout << "   Stamina drain: " << (weatherEffects.staminaDrainMultiplier * 100) << "%" << std::endl;

        if (weatherEffects.causesCold) {
            std::cout << "   WARNING: Cold weather - warm clothing recommended!" << std::endl;
        }

        if (weatherEffects.damagePerSecond > 0) {
            std::cout << "   WARNING: Taking " << weatherEffects.damagePerSecond << " damage/sec!" << std::endl;
        }

        std::cout << "\nSeasonal Effects:" << std::endl;
        std::cout << "   Crop growth: " << (seasonalEffects.cropGrowthMultiplier * 100) << "%" << std::endl;
        std::cout << "   Animal spawns: " << (seasonalEffects.animalSpawnRate * 100) << "%" << std::endl;
        std::cout << "   Temperature: " << seasonalEffects.temperatureModifier << "°C modifier" << std::endl;

        std::cout << "\nAvailable resources this season:" << std::endl;
        for (const auto& resource : m_seasonSystem.getAvailableResources()) {
            std::cout << "   - " << resource << std::endl;
        }

        std::cout << "\nActive creatures this season:" << std::endl;
        for (const auto& creature : m_seasonSystem.getActiveCreatures()) {
            std::cout << "   - " << creature << std::endl;
        }
    }

private:
    void createExampleLights() {
        // Create a campfire
        int campfireId = m_lightingManager.createCampfireLight(glm::vec3(10.0f, 10.0f, 0.0f));
        std::cout << "Created campfire light (ID: " << campfireId << ")" << std::endl;

        // Create some torches
        for (int i = 0; i < 4; ++i) {
            float angle = (i / 4.0f) * 2.0f * 3.14159f;
            float x = 20.0f * std::cos(angle);
            float y = 20.0f * std::sin(angle);
            int torchId = m_lightingManager.createTorchLight(glm::vec3(x, y, 0.0f));
            std::cout << "Created torch light " << (i + 1) << " (ID: " << torchId << ")" << std::endl;
        }

        // Create a magic light
        int magicId = m_lightingManager.createMagicLight(
            glm::vec3(0.0f, 30.0f, 0.0f),
            glm::vec3(0.5f, 0.3f, 1.0f)  // Purple magic
        );
        std::cout << "Created magic light (ID: " << magicId << ")" << std::endl;
    }

    void printStatus() {
        std::cout << "\n--- Environment Status ---" << std::endl;
        std::cout << "Time: " << m_dayNightCycle.getTimeString()
                  << " | Day " << m_dayNightCycle.getDay() << std::endl;
        std::cout << "Weather: " << m_weatherSystem.getWeatherDescription() << std::endl;
        std::cout << "Season: " << m_seasonSystem.getSeasonName()
                  << " (Day " << m_seasonSystem.getDaysIntoSeason() << ")" << std::endl;
        std::cout << "Lights: " << m_lightingManager.getActiveLightCount() << " active" << std::endl;
        std::cout << "-------------------------\n" << std::endl;
    }

    static std::string getWeatherTypeName(WeatherType type) {
        switch (type) {
            case WeatherType::Clear: return "Clear";
            case WeatherType::PartlyCloudy: return "Partly Cloudy";
            case WeatherType::Cloudy: return "Cloudy";
            case WeatherType::Rain: return "Rain";
            case WeatherType::Thunderstorm: return "Thunderstorm";
            case WeatherType::Snow: return "Snow";
            case WeatherType::Blizzard: return "Blizzard";
            case WeatherType::Fog: return "Fog";
            default: return "Unknown";
        }
    }

    static std::string getMoonPhaseName(MoonPhase phase) {
        switch (phase) {
            case MoonPhase::NewMoon: return "New Moon";
            case MoonPhase::WaxingCrescent: return "Waxing Crescent";
            case MoonPhase::FirstQuarter: return "First Quarter";
            case MoonPhase::WaxingGibbous: return "Waxing Gibbous";
            case MoonPhase::FullMoon: return "Full Moon";
            case MoonPhase::WaningGibbous: return "Waning Gibbous";
            case MoonPhase::LastQuarter: return "Last Quarter";
            case MoonPhase::WaningCrescent: return "Waning Crescent";
            default: return "Unknown";
        }
    }

private:
    DayNightCycle m_dayNightCycle;
    WeatherSystem m_weatherSystem;
    SeasonSystem m_seasonSystem;
    LightingManager m_lightingManager;
    float m_statusTimer = 0.0f;
};

} // namespace Environment

// Example main function
int main() {
    Environment::EnvironmentExample example;

    example.initialize();
    example.demonstrateFeatures();
    example.demonstrateIndoorOutdoor();
    example.demonstrateGameplayEffects();

    std::cout << "\n=== Running simulation for 60 seconds ===" << std::endl;

    // Simulate 60 seconds of game time
    float totalTime = 0.0f;
    float dt = 0.016f;  // ~60 FPS

    while (totalTime < 60.0f) {
        example.update(dt);
        totalTime += dt;

        // Sleep to simulate real-time (optional)
        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "\nSimulation complete!" << std::endl;
    return 0;
}
