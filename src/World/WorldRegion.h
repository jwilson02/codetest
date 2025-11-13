#pragma once

#include "TileMap.h"
#include "MapSystem.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace World {

// Region bounds
struct RegionBounds {
    int x, y;
    int width, height;

    RegionBounds() : x(0), y(0), width(0), height(0) {}
    RegionBounds(int _x, int _y, int _w, int _h)
        : x(_x), y(_y), width(_w), height(_h) {}

    bool contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }

    bool intersects(const RegionBounds& other) const {
        return !(x + width <= other.x ||
                 other.x + other.width <= x ||
                 y + height <= other.y ||
                 other.y + other.height <= y);
    }
};

// World region representing a section of the game world
class WorldRegion {
public:
    WorldRegion(const std::string& name, const RegionBounds& bounds);
    ~WorldRegion();

    // Basic properties
    const std::string& getName() const { return m_name; }
    const RegionBounds& getBounds() const { return m_bounds; }
    void setBounds(const RegionBounds& bounds) { m_bounds = bounds; }

    // Biome
    void setBiome(MapSystem::BiomeType biome) { m_biome = biome; }
    MapSystem::BiomeType getBiome() const { return m_biome; }

    // Level and difficulty
    void setLevel(int level) { m_level = level; }
    int getLevel() const { return m_level; }
    void setDifficulty(float difficulty) { m_difficulty = difficulty; }
    float getDifficulty() const { return m_difficulty; }

    // Map association
    void setMapName(const std::string& mapName) { m_mapName = mapName; }
    const std::string& getMapName() const { return m_mapName; }

    // Sub-regions
    void addSubRegion(std::unique_ptr<WorldRegion> subRegion);
    WorldRegion* getSubRegion(const std::string& name);
    const std::vector<std::unique_ptr<WorldRegion>>& getSubRegions() const {
        return m_subRegions;
    }

    // Properties
    void setProperty(const std::string& key, const std::string& value) {
        m_properties[key] = value;
    }
    std::string getProperty(const std::string& key) const;
    bool hasProperty(const std::string& key) const;

    // Music and ambience
    void setMusic(const std::string& musicTrack) { m_musicTrack = musicTrack; }
    const std::string& getMusic() const { return m_musicTrack; }
    void setAmbientSound(const std::string& sound) { m_ambientSound = sound; }
    const std::string& getAmbientSound() const { return m_ambientSound; }

    // Weather
    enum class Weather {
        Clear,
        Rain,
        Snow,
        Fog,
        Storm,
        Sandstorm
    };
    void setWeather(Weather weather) { m_weather = weather; }
    Weather getWeather() const { return m_weather; }

    // Time of day effects
    void setTimeScale(float scale) { m_timeScale = scale; }
    float getTimeScale() const { return m_timeScale; }

    // Discovery and exploration
    void setDiscovered(bool discovered) { m_discovered = discovered; }
    bool isDiscovered() const { return m_discovered; }
    void setExplorationProgress(float progress) { m_explorationProgress = progress; }
    float getExplorationProgress() const { return m_explorationProgress; }

    // Enemy spawning
    struct SpawnRule {
        std::string enemyType;
        float spawnChance;
        int minLevel;
        int maxLevel;
        int minCount;
        int maxCount;
    };
    void addSpawnRule(const SpawnRule& rule) { m_spawnRules.push_back(rule); }
    const std::vector<SpawnRule>& getSpawnRules() const { return m_spawnRules; }

    // Resources
    struct ResourceNode {
        std::string type; // ore, tree, herb, etc.
        int x, y;
        int quantity;
        float respawnTime;
        bool depleted;
    };
    void addResourceNode(const ResourceNode& node) { m_resources.push_back(node); }
    std::vector<ResourceNode>& getResources() { return m_resources; }

    // Points of interest
    struct PointOfInterest {
        std::string name;
        std::string type; // landmark, dungeon, town, etc.
        int x, y;
        std::string description;
        bool revealed;
    };
    void addPOI(const PointOfInterest& poi) { m_pointsOfInterest.push_back(poi); }
    const std::vector<PointOfInterest>& getPOIs() const { return m_pointsOfInterest; }
    PointOfInterest* getPOI(const std::string& name);

    // Serialization
    bool saveToJSON(const std::string& filename) const;
    bool loadFromJSON(const std::string& filename);

private:
    std::string m_name;
    RegionBounds m_bounds;
    MapSystem::BiomeType m_biome;
    int m_level;
    float m_difficulty;
    std::string m_mapName;

    std::vector<std::unique_ptr<WorldRegion>> m_subRegions;
    std::unordered_map<std::string, std::string> m_properties;

    std::string m_musicTrack;
    std::string m_ambientSound;
    Weather m_weather;
    float m_timeScale;

    bool m_discovered;
    float m_explorationProgress;

    std::vector<SpawnRule> m_spawnRules;
    std::vector<ResourceNode> m_resources;
    std::vector<PointOfInterest> m_pointsOfInterest;
};

// Region manager
class RegionManager {
public:
    RegionManager();
    ~RegionManager();

    // Region management
    void addRegion(std::unique_ptr<WorldRegion> region);
    WorldRegion* getRegion(const std::string& name);
    WorldRegion* getRegionAt(int x, int y);
    void removeRegion(const std::string& name);

    // Active region
    void setActiveRegion(const std::string& name);
    WorldRegion* getActiveRegion() { return m_activeRegion; }
    const WorldRegion* getActiveRegion() const { return m_activeRegion; }

    // Region queries
    std::vector<WorldRegion*> getRegionsInArea(const RegionBounds& area);
    std::vector<WorldRegion*> getNeighboringRegions(const std::string& regionName);

    // World structure
    void setWorldSize(int width, int height) { m_worldWidth = width; m_worldHeight = height; }
    int getWorldWidth() const { return m_worldWidth; }
    int getWorldHeight() const { return m_worldHeight; }

    // Update
    void update(float deltaTime, int playerX, int playerY);

    // Serialization
    bool loadWorldRegions(const std::string& filename);
    bool saveWorldRegions(const std::string& filename) const;

    // Statistics
    struct Stats {
        int totalRegions;
        int discoveredRegions;
        float totalExplorationProgress;
    };
    Stats getStats() const;

private:
    std::unordered_map<std::string, std::unique_ptr<WorldRegion>> m_regions;
    WorldRegion* m_activeRegion;

    int m_worldWidth;
    int m_worldHeight;

    void updateActiveRegion(int playerX, int playerY);
};

} // namespace World
