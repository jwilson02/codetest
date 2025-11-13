# Environment System Guide

A comprehensive dynamic weather and day/night cycle system for immersive ARPG gameplay.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Components](#system-components)
- [Quick Start](#quick-start)
- [Integration Guide](#integration-guide)
- [Configuration](#configuration)
- [Performance Optimization](#performance-optimization)
- [Examples](#examples)

## Overview

The Environment System provides a fully integrated suite of atmospheric and lighting systems that work together to create an immersive, dynamic world. The system includes:

- **Day/Night Cycle**: Dynamic time progression with realistic lighting transitions
- **Weather System**: Multiple weather types with smooth transitions and gameplay effects
- **Season System**: Four seasons with visual and gameplay changes
- **Lighting Manager**: Unified lighting control that integrates all environmental factors

## Features

### Day/Night Cycle
- ✅ Smooth 24-hour time progression
- ✅ Configurable time scale (real-time to accelerated)
- ✅ Dynamic sun and moon positioning
- ✅ Realistic lighting transitions (dawn, day, dusk, night)
- ✅ Moon phases with gameplay effects
- ✅ Celestial events (eclipses, auroras, meteor showers, blood moons)
- ✅ Time-based callbacks and events
- ✅ Indoor time pausing option

### Weather System
- ✅ 10+ weather types (clear, rain, snow, fog, storms, etc.)
- ✅ Smooth weather transitions
- ✅ Dynamic weather particles (rain drops, snowflakes)
- ✅ Wind simulation with gusts and turbulence
- ✅ Regional weather variations
- ✅ Seasonal weather influences
- ✅ Gameplay effects (movement, visibility, stamina)
- ✅ Special effects (lightning strikes, rainbows)
- ✅ Temperature and humidity simulation

### Season System
- ✅ Four seasons with distinct characteristics
- ✅ Smooth seasonal transitions
- ✅ Visual changes (foliage color, snow coverage, bloom)
- ✅ Seasonal resource availability
- ✅ Season-specific creatures and events
- ✅ Gameplay modifiers (crop growth, animal spawns, fishing)
- ✅ Seasonal weather patterns
- ✅ Holiday/festival events

### Lighting Manager
- ✅ Multiple light types (directional, point, spot, area)
- ✅ Dynamic sun and moon lighting
- ✅ Indoor/outdoor detection zones
- ✅ Automatic light culling and optimization
- ✅ Shadow system with configurable quality
- ✅ Flickering lights (torches, campfires)
- ✅ Temporary lights (explosions, spells)
- ✅ Weather and time integration
- ✅ HDR and bloom support

## System Components

### File Structure

```
src/Environment/
├── DayNightCycle.h/cpp       - Time system and celestial events
├── WeatherSystem.h/cpp       - Weather patterns and effects
├── SeasonSystem.h/cpp        - Seasonal cycles and changes
├── LightingManager.h/cpp     - Dynamic lighting controller
└── EnvironmentExample.cpp    - Usage examples

data/environment/
├── weather_patterns.json     - Weather configuration
└── time_settings.json        - Time and celestial event config
```

## Quick Start

### Basic Setup

```cpp
#include "Environment/DayNightCycle.h"
#include "Environment/WeatherSystem.h"
#include "Environment/SeasonSystem.h"
#include "Environment/LightingManager.h"

// Create systems
Environment::DayNightCycle dayNight;
Environment::WeatherSystem weather;
Environment::SeasonSystem seasons;
Environment::LightingManager lighting;

// Initialize
dayNight.initialize();
weather.initialize();
seasons.initialize();
lighting.initialize();

// Connect systems
lighting.setDayNightCycle(&dayNight);
lighting.setWeatherSystem(&weather);
lighting.setSeasonSystem(&seasons);

// Update in game loop (call each frame)
void update(float deltaTime) {
    dayNight.update(deltaTime);
    weather.update(deltaTime);
    seasons.update(deltaTime, dayNight.getDay());
    lighting.update(deltaTime);
}
```

## Integration Guide

### Step 1: Initialize Systems

```cpp
// Configure day/night cycle
Environment::TimeConfig timeConfig;
timeConfig.minutesPerGameDay = 24.0f;  // 24 real minutes = 1 game day
timeConfig.startHour = 8.0f;           // Start at 8 AM
timeConfig.enableCelestialEvents = true;
dayNight.initialize(timeConfig);

// Configure weather
Environment::WeatherConfig weatherConfig;
weatherConfig.enableDynamicWeather = true;
weatherConfig.minWeatherDuration = 300.0f;  // 5 minutes minimum
weatherConfig.maxWeatherDuration = 1800.0f; // 30 minutes maximum
weather.initialize(weatherConfig);
weather.loadWeatherPatterns("data/environment/weather_patterns.json");

// Configure seasons
Environment::SeasonConfig seasonConfig;
seasonConfig.daysPerSeason = 30.0f;  // 30 game days per season
seasonConfig.enableSeasonalCycle = true;
seasons.initialize(seasonConfig);

// Configure lighting
Environment::LightingConfig lightConfig;
lightConfig.maxLights = 32;
lightConfig.enableShadows = true;
lightConfig.enableHDR = true;
lighting.initialize(lightConfig);
```

### Step 2: Set Up Event Callbacks

```cpp
// Time of day changes
dayNight.onTimeOfDayChanged([](Environment::TimeOfDay tod) {
    if (tod == Environment::TimeOfDay::Night) {
        // Spawn nocturnal creatures
        spawnNightCreatures();
    }
});

// Weather changes
weather.onWeatherChanged([](Environment::WeatherType old, Environment::WeatherType newType) {
    if (newType == Environment::WeatherType::Thunderstorm) {
        // Start playing storm sounds
        playStormAmbience();
    }
});

// Season changes
seasons.onSeasonChanged([](Environment::Season old, Environment::Season newSeason) {
    // Update world visuals
    updateWorldTextures(newSeason);
});

// Lightning strikes
weather.onLightningStrike([](glm::vec2 position) {
    // Create lightning visual effect
    createLightningEffect(position);
    // Play thunder sound
    playThunderSound();
});
```

### Step 3: Create Dynamic Lights

```cpp
// Sun and moon (automatically created)
// The lighting manager creates these internally

// Player torch
int torchId = lighting.createTorchLight(player.getPosition());
lighting.getLight(torchId)->flickerAmount = 0.2f;

// Campfire
int campfireId = lighting.createCampfireLight(campfirePos);

// Magic spell
int spellLight = lighting.createMagicLight(
    spellPosition,
    glm::vec3(0.3f, 0.5f, 1.0f)  // Blue magic
);

// Explosion (temporary)
lighting.createExplosionLight(explosionPos, 2.0f);  // Lasts 2 seconds
```

### Step 4: Set Up Indoor/Outdoor Zones

```cpp
// Create outdoor zone
Environment::LightingZone forest;
forest.name = "forest";
forest.position = glm::vec2(0, 0);
forest.size = glm::vec2(200, 200);
forest.isIndoor = false;
lighting.addLightingZone(forest);

// Create indoor zone (tavern)
Environment::LightingZone tavern;
tavern.name = "tavern";
tavern.position = glm::vec2(100, 50);
tavern.size = glm::vec2(30, 20);
tavern.isIndoor = true;
tavern.ambientOverride = glm::vec3(0.8f, 0.6f, 0.4f);  // Warm lighting
tavern.ambientIntensityOverride = 0.6f;
lighting.addLightingZone(tavern);

// Update player position
lighting.setPlayerPosition(player.getPosition());
```

### Step 5: Apply Gameplay Effects

```cpp
// Get weather effects
const auto& weatherEffects = weather.getGameplayEffects();

// Apply movement penalty
player.setSpeedMultiplier(weatherEffects.movementSpeedMultiplier);

// Apply visibility
renderer.setViewDistance(baseViewDistance * weatherEffects.visibilityRange);

// Check for environmental damage
if (weatherEffects.damagePerSecond > 0) {
    player.takeDamage(weatherEffects.damagePerSecond * deltaTime);
}

// Get seasonal effects
const auto& seasonalEffects = seasons.getCurrentGameplay();

// Modify crop growth
if (player.hasCrops()) {
    float growthRate = baseGrowthRate * seasonalEffects.cropGrowthMultiplier;
    player.updateCrops(growthRate, deltaTime);
}

// Check resource availability
if (seasons.isResourceAvailable("spring_herbs")) {
    // Player can gather spring herbs
}

// Check creature spawning
if (seasons.isCreatureActive("wolves")) {
    // Wolves can spawn in current season
}
```

## Configuration

### Weather Patterns (JSON)

```json
{
  "name": "Heavy Rain",
  "type": "heavy_rain",
  "intensity": "heavy",
  "probability": 0.1,
  "visuals": {
    "skyTint": [0.5, 0.55, 0.6],
    "particleDensity": 150,
    "fogDensity": 0.4
  },
  "gameplayEffects": {
    "movementSpeedMultiplier": 0.8,
    "visibilityRange": 0.6,
    "damagePerSecond": 0.0
  },
  "wind": {
    "speed": 12.0,
    "gustStrength": 2.0
  }
}
```

### Time Settings (JSON)

```json
{
  "timeConfig": {
    "minutesPerGameDay": 24.0,
    "startHour": 8.0,
    "sunriseHour": 6.0,
    "sunsetHour": 18.0,
    "moonCycleDays": 28.0
  },
  "celestialEvents": [
    {
      "name": "Solar Eclipse",
      "type": "solar_eclipse",
      "duration": 300.0,
      "rarity": "rare"
    }
  ]
}
```

## Performance Optimization

### Light Culling

```cpp
// Cull lights based on camera position
lighting.cullLights(camera.getPosition(), camera.getViewDistance());

// Get only visible lights for rendering
auto visibleLights = lighting.getVisibleLights();
```

### Weather Particle Management

```cpp
// Adjust particle count based on performance
WeatherConfig config;
config.maxParticles = 1000;  // Lower for mobile/weak hardware
weather.setConfig(config);

// Spawn particles in batches
weather.spawnWeatherParticles(50, screenArea);
```

### Shadow Quality

```cpp
// Configure shadow quality
ShadowConfig shadowConfig;
shadowConfig.resolution = 1024;     // Lower for performance
shadowConfig.softShadows = false;   // Disable for speed
shadowConfig.maxDistance = 50.0f;   // Reduce shadow distance
lighting.setShadowConfig(shadowConfig);
```

### Zone Optimization

```cpp
// Use fewer, larger zones instead of many small zones
// Bad: 100 small 5x5 zones
// Good: 10 large 50x50 zones

// Disable time updates when paused
if (game.isPaused()) {
    dayNight.pauseTime();
}
```

## Examples

### Example 1: Time-based Quest

```cpp
// Quest that requires player to visit location at midnight
void checkMidnightQuest() {
    if (dayNight.getTimeOfDay() == TimeOfDay::Midnight) {
        if (player.isAtLocation(mysteriousShrine)) {
            triggerSecretEvent();
        }
    }
}
```

### Example 2: Weather-dependent Combat

```cpp
// Fire spells weaker in rain, lightning stronger in storms
float getSpellMultiplier(SpellType spell) {
    if (spell == SpellType::Fire && weather.isRaining()) {
        return 0.5f;  // 50% damage in rain
    }
    if (spell == SpellType::Lightning && weather.isStorming()) {
        return 1.5f;  // 50% bonus in storms
    }
    return 1.0f;
}
```

### Example 3: Seasonal Fishing

```cpp
// Different fish available in different seasons
std::vector<Fish> getAvailableFish() {
    std::vector<Fish> available;

    Season current = seasons.getCurrentSeason();

    if (current == Season::Spring) {
        available.push_back(Fish::Trout);
        available.push_back(Fish::Bass);
    } else if (current == Season::Summer) {
        available.push_back(Fish::Catfish);
        available.push_back(Fish::Pike);
    }
    // ... etc

    return available;
}
```

### Example 4: Dynamic Stealth

```cpp
// Stealth bonus at night, penalty in daylight
float getStealthMultiplier() {
    float multiplier = 1.0f;

    // Time of day
    if (dayNight.isNight()) {
        multiplier *= 1.5f;  // 50% bonus at night
    }

    // Moon phase
    if (dayNight.getMoonPhase() == MoonPhase::NewMoon) {
        multiplier *= 1.3f;  // Extra bonus on new moon
    }

    // Weather
    if (weather.getCurrentWeather() == WeatherType::Fog) {
        multiplier *= 1.4f;  // Fog helps stealth
    }

    // Indoor vs outdoor
    if (!lighting.isPlayerIndoors()) {
        multiplier *= 0.8f;  // Harder to hide outdoors
    }

    return multiplier;
}
```

### Example 5: Celestial Event Triggers

```cpp
// Spawn rare boss during blood moon
dayNight.onCelestialEvent([](CelestialEvent event, bool started) {
    if (event == CelestialEvent::BloodMoon && started) {
        spawnBloodMoonBoss();
        showMessage("The Blood Moon rises! Ancient evil awakens...");
    }
});
```

### Example 6: Weather Transitions

```cpp
// Gradually transition to storm when player enters cursed area
void enterCursedArea() {
    weather.transitionToWeather(WeatherType::Thunderstorm, 30.0f);  // 30 sec transition
}
```

## Advanced Features

### Custom Weather Patterns

```cpp
// Create custom weather pattern
WeatherPattern acidRain;
acidRain.name = "Acid Rain";
acidRain.type = WeatherType::Rain;  // Base type
acidRain.visuals.particleColor = glm::vec3(0.5f, 1.0f, 0.3f);  // Green
acidRain.gameplayEffects.damagePerSecond = 5.0f;  // Hurts player
acidRain.gameplayEffects.corrodeArmor = true;
weather.addWeatherPattern(acidRain);
```

### Custom Celestial Events

```cpp
// Trigger custom event
dayNight.triggerCelestialEvent(CelestialEvent::CometPassing, 600.0f);

// Check if event is active
if (dayNight.hasActiveEvent()) {
    float progress = dayNight.getEventProgress();
    // Update event visuals based on progress
}
```

### Dynamic Light Animations

```cpp
// Animate torch flickering
void updateTorch(int torchId, float time) {
    Light* torch = lighting.getLight(torchId);
    if (torch) {
        // Vary intensity over time
        torch->intensity = 1.0f + 0.3f * sin(time * 3.0f);

        // Add random flicker
        if (rand() % 100 < 10) {
            torch->intensity *= 0.7f;
        }
    }
}
```

## Troubleshooting

### Issue: Weather not changing
- Check `enableDynamicWeather` is true
- Verify weather patterns are loaded
- Ensure update() is being called

### Issue: Lighting too dark
- Increase ambient intensity
- Check if player is in indoor zone
- Verify time of day lighting values

### Issue: Performance problems
- Reduce max particles
- Lower shadow resolution
- Increase light culling distance
- Use fewer active lights

### Issue: Seasons not transitioning
- Check `enableSeasonalCycle` is true
- Verify days are advancing
- Ensure update() receives correct day count

## API Reference

See individual header files for complete API documentation:
- `DayNightCycle.h` - Time and celestial systems
- `WeatherSystem.h` - Weather and atmospheric effects
- `SeasonSystem.h` - Seasonal cycles and changes
- `LightingManager.h` - Dynamic lighting control

## Credits

Environment System for SCUMM VM ARPG
Integrates seamlessly with existing Audio, Rendering, and Game systems
Optimized for performance and visual quality
