# Environment System Integration Guide

This document explains how to integrate the Environment System with your existing ARPG engine components.

## Integration with Existing Systems

### 1. Rendering System Integration

The Environment System needs to provide lighting data to your Rendering system:

```cpp
// In your main render loop
void GameRenderer::render() {
    // Get environmental lighting
    auto& lighting = m_environmentSystem.getLightingManager();

    // Apply ambient lighting
    glm::vec3 ambientColor = lighting.getAmbientColor();
    float ambientIntensity = lighting.getAmbientIntensity();

    // Apply to sprite batch shader
    m_spriteBatch.setAmbientLight(ambientColor, ambientIntensity);

    // Apply directional light (sun/moon)
    glm::vec3 dirColor = lighting.getDirectionalLightColor();
    glm::vec3 dirDirection = lighting.getDirectionalLightDirection();
    float dirIntensity = lighting.getDirectionalLightIntensity();

    m_shader->setUniform("u_DirectionalLight.color", dirColor);
    m_shader->setUniform("u_DirectionalLight.direction", dirDirection);
    m_shader->setUniform("u_DirectionalLight.intensity", dirIntensity);

    // Apply fog for weather effects
    float fogDensity = lighting.getCurrentFogDensity();
    glm::vec3 fogColor = lighting.getCurrentFogColor();

    m_shader->setUniform("u_FogDensity", fogDensity);
    m_shader->setUniform("u_FogColor", fogColor);

    // Apply seasonal color tint
    glm::vec3 seasonalTint = lighting.getSeasonalTint();
    m_shader->setUniform("u_ColorTint", seasonalTint);

    // Render all visible lights
    auto visibleLights = lighting.getVisibleLights();
    for (size_t i = 0; i < visibleLights.size() && i < 32; ++i) {
        applyLightToShader(visibleLights[i], i);
    }
}
```

### 2. Audio System Integration

Weather and time of day should affect ambient audio:

```cpp
// Update audio based on environment
void updateEnvironmentalAudio() {
    auto& audio = Audio::AudioSystem::getInstance();
    auto& weather = m_environmentSystem.getWeatherSystem();
    auto& dayNight = m_environmentSystem.getDayNightCycle();

    // Play weather sounds
    const auto& weatherSounds = weather.getCurrentSounds();
    for (const auto& sound : weatherSounds) {
        if (!audio.isPlaying(sound)) {
            audio.playAmbient(sound, weather.getCurrentPattern()->soundVolume);
        }
    }

    // Adjust ambient based on time
    if (dayNight.isNight()) {
        audio.playAmbient("night_crickets", 0.3f);
        audio.playAmbient("owl_hoot", 0.2f);
    } else {
        audio.stopAmbient("night_crickets");
        audio.playAmbient("birds_chirping", 0.4f);
    }

    // Wind sounds based on weather
    float windSpeed = weather.getWind().speed;
    if (windSpeed > 10.0f) {
        audio.playAmbient("wind_strong", windSpeed / 30.0f);
    }
}
```

### 3. Player/Character Integration

Apply environmental effects to player stats:

```cpp
class Player {
public:
    void updateEnvironmentalEffects(float deltaTime) {
        auto& weather = Environment::getWeatherSystem();
        auto& dayNight = Environment::getDayNightCycle();
        auto& seasons = Environment::getSeasonSystem();

        // Movement speed
        float speedMult = weather.getMovementSpeedMultiplier();
        m_currentSpeed = m_baseSpeed * speedMult;

        // Stamina drain
        float staminaMult = weather.getGameplayEffects().staminaDrainMultiplier;
        m_staminaDrain = m_baseStaminaDrain * staminaMult;

        // Environmental damage
        float damage = weather.getGameplayEffects().damagePerSecond;
        if (damage > 0.0f) {
            // Check if player has protection
            if (!hasWeatherProtection()) {
                takeDamage(damage * deltaTime);
            }
        }

        // Temperature effects
        float temperature = weather.getTemperature() + seasons.getTemperatureModifier();
        if (temperature < 0.0f && !hasWarmClothing()) {
            // Freezing damage
            takeDamage(abs(temperature) * 0.1f * deltaTime);
            showStatusEffect("Freezing");
        } else if (temperature > 35.0f && !hasCoolingItems()) {
            // Heat exhaustion
            m_staminaDrain *= 1.5f;
            showStatusEffect("Heat Exhaustion");
        }

        // Visibility affects stealth
        float visibility = weather.getVisibilityRange();
        m_stealthMultiplier = 1.0f + (1.0f - visibility) * 0.5f;

        // Night time stealth bonus
        if (dayNight.isNight()) {
            m_stealthMultiplier *= 1.3f;
        }
    }

    bool hasWeatherProtection() {
        // Check for umbrella, raincoat, etc.
        return hasItem("raincoat") || hasItem("magical_barrier");
    }

    bool hasWarmClothing() {
        return hasItem("fur_coat") || hasItem("winter_cloak");
    }
};
```

### 4. AI/Enemy Integration

Enemies react to environmental conditions:

```cpp
class Enemy {
public:
    void updateAI(float deltaTime) {
        auto& weather = Environment::getWeatherSystem();
        auto& dayNight = Environment::getDayNightCycle();

        // Some enemies only appear at night
        if (m_type == EnemyType::Vampire || m_type == EnemyType::Ghost) {
            if (dayNight.isDay()) {
                // Flee to shelter or take damage
                flee();
                if (isInSunlight()) {
                    takeDamage(10.0f * deltaTime);
                }
            }
        }

        // Wolves are more aggressive during full moon
        if (m_type == EnemyType::Wolf) {
            if (dayNight.getMoonPhase() == MoonPhase::FullMoon) {
                m_aggressionMultiplier = 2.0f;
                m_detectionRange *= 1.5f;
            }
        }

        // Reduced vision in fog
        if (weather.getCurrentWeather() == WeatherType::Fog) {
            m_visionRange *= weather.getVisibilityRange();
        }

        // Seek shelter in extreme weather
        if (weather.isStorming() && !isIndoors()) {
            seekShelter();
        }
    }
};
```

### 5. Quest System Integration

Quests can have time and weather requirements:

```cpp
struct Quest {
    std::string id;
    std::vector<TimeOfDay> validTimes;
    std::vector<WeatherType> validWeather;
    std::vector<Season> validSeasons;

    bool canComplete() {
        auto& env = Environment::getEnvironmentSystem();

        // Check time requirements
        if (!validTimes.empty()) {
            TimeOfDay current = env.getDayNightCycle().getTimeOfDay();
            if (std::find(validTimes.begin(), validTimes.end(), current) == validTimes.end()) {
                return false;
            }
        }

        // Check weather requirements
        if (!validWeather.empty()) {
            WeatherType current = env.getWeatherSystem().getCurrentWeather();
            if (std::find(validWeather.begin(), validWeather.end(), current) == validWeather.end()) {
                return false;
            }
        }

        // Check season requirements
        if (!validSeasons.empty()) {
            Season current = env.getSeasonSystem().getCurrentSeason();
            if (std::find(validSeasons.begin(), validSeasons.end(), current) == validSeasons.end()) {
                return false;
            }
        }

        return true;
    }
};

// Example: Quest that requires thunderstorm at midnight
Quest midnightStormQuest;
midnightStormQuest.id = "storm_ritual";
midnightStormQuest.validTimes = {TimeOfDay::Midnight};
midnightStormQuest.validWeather = {WeatherType::Thunderstorm};
```

### 6. Crafting System Integration

Seasonal resources and weather-dependent crafting:

```cpp
class CraftingSystem {
public:
    bool canCraft(const Recipe& recipe) {
        auto& seasons = Environment::getSeasonSystem();

        // Check if all ingredients are available
        for (const auto& ingredient : recipe.ingredients) {
            if (ingredient.seasonal) {
                if (!seasons.isResourceAvailable(ingredient.name)) {
                    return false;  // Not available this season
                }
            }
        }

        // Some recipes require specific weather
        if (!recipe.requiredWeather.empty()) {
            auto& weather = Environment::getWeatherSystem();
            WeatherType current = weather.getCurrentWeather();

            if (std::find(recipe.requiredWeather.begin(),
                         recipe.requiredWeather.end(),
                         current) == recipe.requiredWeather.end()) {
                return false;
            }
        }

        return true;
    }

    void updateQuality(Item& item) {
        auto& weather = Environment::getWeatherSystem();

        // Weather affects crafting quality
        if (weather.isRaining()) {
            // Metal working suffers in rain
            if (item.category == ItemCategory::Weapon) {
                item.quality *= 0.8f;
            }
        }

        // Clear weather gives bonus
        if (weather.isClearSky()) {
            item.quality *= 1.1f;
        }
    }
};
```

### 7. World/Map Integration

Apply visual effects to world rendering:

```cpp
class WorldRenderer {
public:
    void renderTerrain() {
        auto& seasons = Environment::getSeasonSystem();
        auto& weather = Environment::getWeatherSystem();

        // Apply seasonal foliage color
        glm::vec3 foliageColor = seasons.getFoliageColor();
        m_terrainShader->setUniform("u_FoliageColor", foliageColor);

        // Apply snow coverage
        float snowCoverage = seasons.getSnowCoverage();
        m_terrainShader->setUniform("u_SnowCoverage", snowCoverage);

        // Apply foliage density
        float foliageDensity = seasons.getFoliageDensity();
        m_terrainShader->setUniform("u_FoliageDensity", foliageDensity);

        // Render terrain...
    }

    void renderWeatherEffects() {
        auto& weather = Environment::getWeatherSystem();

        // Render weather particles
        const auto& visuals = weather.getVisuals();
        if (visuals.particleDensity > 0) {
            renderWeatherParticles();
        }

        // Apply fog
        m_fogShader->setUniform("u_FogDensity", visuals.fogDensity);
        m_fogShader->setUniform("u_FogColor", visuals.fogColor);
    }
};
```

### 8. UI Integration

Display environmental information:

```cpp
class HUD {
public:
    void renderEnvironmentalInfo() {
        auto& dayNight = Environment::getDayNightCycle();
        auto& weather = Environment::getWeatherSystem();
        auto& seasons = Environment::getSeasonSystem();

        // Time display
        std::string timeStr = dayNight.getTimeString();
        std::string dateStr = dayNight.getDateString();
        drawText(timeStr, glm::vec2(10, 10));
        drawText(dateStr, glm::vec2(10, 30));

        // Weather icon
        WeatherType current = weather.getCurrentWeather();
        drawWeatherIcon(getWeatherIcon(current), glm::vec2(10, 50));

        // Temperature
        float temp = weather.getTemperature();
        drawText(std::to_string((int)temp) + "°C", glm::vec2(40, 50));

        // Season indicator
        drawText(seasons.getSeasonName(), glm::vec2(10, 70));

        // Moon phase (at night)
        if (dayNight.isNight()) {
            MoonPhase phase = dayNight.getMoonPhase();
            drawMoonIcon(getMoonIcon(phase), glm::vec2(screenWidth - 50, 10));
        }

        // Active celestial events
        if (dayNight.hasActiveEvent()) {
            CelestialEvent event = dayNight.getCurrentEvent();
            drawEventNotification(getEventName(event), getEventIcon(event));
        }

        // Weather warnings
        const auto& effects = weather.getGameplayEffects();
        if (effects.damagePerSecond > 0.0f) {
            drawWarning("Hazardous weather! Seek shelter!");
        }
        if (effects.causesCold) {
            drawWarning("Cold weather - wear warm clothing");
        }
    }

    void renderEnvironmentalBuffs() {
        // Show active environmental buffs/debuffs
        std::vector<BuffInfo> buffs;

        // Weather buffs
        if (weather.isRaining()) {
            buffs.push_back({"Rain", "Movement -10%", BuffType::Debuff});
            buffs.push_back({"Wet", "Fire resistance +20%", BuffType::Buff});
        }

        // Time buffs
        if (dayNight.isNight()) {
            buffs.push_back({"Night", "Stealth +30%", BuffType::Buff});
        }

        // Moon phase buffs
        if (dayNight.getMoonPhase() == MoonPhase::FullMoon) {
            buffs.push_back({"Full Moon", "Magic power +20%", BuffType::Buff});
        }

        // Render buff icons
        for (size_t i = 0; i < buffs.size(); ++i) {
            drawBuff(buffs[i], glm::vec2(screenWidth - 100, 100 + i * 40));
        }
    }
};
```

### 9. Save System Integration

Persist environmental state:

```cpp
struct EnvironmentSaveData {
    // Time data
    float currentHour;
    int currentDay;
    int currentMonth;
    int currentYear;

    // Weather data
    int currentWeatherType;
    int currentIntensity;
    bool isTransitioning;

    // Season data
    int currentSeason;
    int daysIntoSeason;

    // Active events
    std::vector<std::string> activeEvents;
    int currentCelestialEvent;
    float celestialEventTimer;

    // Lighting data
    std::vector<LightSaveData> activeLights;
};

void saveEnvironment(SaveFile& save) {
    EnvironmentSaveData data;

    // Save time
    data.currentHour = m_dayNight.getHour();
    data.currentDay = m_dayNight.getDay();
    data.currentMonth = m_dayNight.getMonth();
    data.currentYear = m_dayNight.getYear();

    // Save weather
    data.currentWeatherType = (int)m_weather.getCurrentWeather();
    data.currentIntensity = (int)m_weather.getCurrentIntensity();
    data.isTransitioning = m_weather.isTransitioning();

    // Save season
    data.currentSeason = (int)m_seasons.getCurrentSeason();
    data.daysIntoSeason = m_seasons.getDaysIntoSeason();

    // Write to save file
    save.write(data);
}

void loadEnvironment(const SaveFile& save) {
    EnvironmentSaveData data = save.read<EnvironmentSaveData>();

    // Restore time
    m_dayNight.setTime(data.currentHour);
    // Set day/month/year...

    // Restore weather
    m_weather.setWeather((WeatherType)data.currentWeatherType,
                         (WeatherIntensity)data.currentIntensity);

    // Restore season
    m_seasons.setSeason((Season)data.currentSeason);
}
```

### 10. Performance Monitoring

Monitor and optimize environment systems:

```cpp
class EnvironmentProfiler {
public:
    void update() {
        auto start = std::chrono::high_resolution_clock::now();

        // Time day/night update
        auto t1 = std::chrono::high_resolution_clock::now();
        m_dayNight.update(deltaTime);
        auto t2 = std::chrono::high_resolution_clock::now();
        m_dayNightTime = std::chrono::duration<float, std::milli>(t2 - t1).count();

        // Time weather update
        t1 = std::chrono::high_resolution_clock::now();
        m_weather.update(deltaTime);
        t2 = std::chrono::high_resolution_clock::now();
        m_weatherTime = std::chrono::duration<float, std::milli>(t2 - t1).count();

        // Time lighting update
        t1 = std::chrono::high_resolution_clock::now();
        m_lighting.update(deltaTime);
        t2 = std::chrono::high_resolution_clock::now();
        m_lightingTime = std::chrono::duration<float, std::milli>(t2 - t1).count();

        auto end = std::chrono::high_resolution_clock::now();
        m_totalTime = std::chrono::duration<float, std::milli>(end - start).count();
    }

    void printStats() {
        std::cout << "Environment Performance:" << std::endl;
        std::cout << "  Day/Night: " << m_dayNightTime << "ms" << std::endl;
        std::cout << "  Weather: " << m_weatherTime << "ms" << std::endl;
        std::cout << "  Lighting: " << m_lightingTime << "ms" << std::endl;
        std::cout << "  Total: " << m_totalTime << "ms" << std::endl;
        std::cout << "  Active Lights: " << m_lighting.getActiveLightCount() << std::endl;
        std::cout << "  Weather Particles: " << m_weather.getParticleCount() << std::endl;
    }

private:
    float m_dayNightTime = 0.0f;
    float m_weatherTime = 0.0f;
    float m_lightingTime = 0.0f;
    float m_totalTime = 0.0f;
};
```

## Main Game Loop Integration

Complete example of environment integration in main game loop:

```cpp
class Game {
public:
    void initialize() {
        // Initialize all systems
        m_renderer.initialize();
        m_audioSystem.initialize();

        // Initialize environment
        m_dayNight.initialize();
        m_weather.initialize();
        m_weather.loadWeatherPatterns("data/environment/weather_patterns.json");
        m_seasons.initialize();
        m_lighting.initialize();

        // Connect systems
        m_lighting.setDayNightCycle(&m_dayNight);
        m_lighting.setWeatherSystem(&m_weather);
        m_lighting.setSeasonSystem(&m_seasons);

        // Set up callbacks
        setupEnvironmentCallbacks();
    }

    void update(float deltaTime) {
        // Update environment
        m_dayNight.update(deltaTime);
        m_weather.update(deltaTime);
        m_seasons.update(deltaTime, m_dayNight.getDay());
        m_lighting.update(deltaTime);

        // Update world based on environment
        updateEnvironmentalEffects(deltaTime);

        // Update game systems
        m_player.update(deltaTime);
        m_enemyManager.update(deltaTime);
        m_questSystem.update(deltaTime);
        m_craftingSystem.update(deltaTime);
    }

    void render() {
        // Apply environmental lighting
        applyEnvironmentalLighting();

        // Render world
        m_worldRenderer.render();

        // Render weather effects
        renderWeatherEffects();

        // Render game objects
        m_player.render();
        m_enemyManager.render();

        // Render UI with environmental info
        m_hud.render();
    }

private:
    // Environment systems
    Environment::DayNightCycle m_dayNight;
    Environment::WeatherSystem m_weather;
    Environment::SeasonSystem m_seasons;
    Environment::LightingManager m_lighting;

    // Game systems
    Renderer m_renderer;
    Player m_player;
    EnemyManager m_enemyManager;
    // ... etc
};
```

## Shader Integration Examples

### Fragment Shader with Environment

```glsl
#version 330 core

// Environmental uniforms
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;
uniform vec3 u_DirectionalLightDir;
uniform vec3 u_DirectionalLightColor;
uniform float u_DirectionalLightIntensity;

// Fog
uniform float u_FogDensity;
uniform vec3 u_FogColor;

// Seasonal tint
uniform vec3 u_SeasonalTint;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;

out vec4 FragColor;

void main() {
    // Sample base texture
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // Ambient lighting
    vec3 ambient = u_AmbientColor * u_AmbientIntensity;

    // Directional lighting (sun/moon)
    float diff = max(dot(v_Normal, -u_DirectionalLightDir), 0.0);
    vec3 diffuse = diff * u_DirectionalLightColor * u_DirectionalLightIntensity;

    // Combine lighting
    vec3 lighting = ambient + diffuse;
    vec3 result = texColor.rgb * lighting;

    // Apply seasonal tint
    result *= u_SeasonalTint;

    // Apply fog
    float fogFactor = exp(-u_FogDensity * length(v_FragPos));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    result = mix(u_FogColor, result, fogFactor);

    FragColor = vec4(result, texColor.a);
}
```

This integration guide provides everything needed to seamlessly incorporate the Environment System into your ARPG engine!
