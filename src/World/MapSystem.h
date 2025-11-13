#pragma once

#include "TileMap.h"
#include "WorldRegion.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace World {

// Forward declarations
class ProceduralGenerator;
class Collision;

// TMX format structures
namespace TMX {
    struct Property {
        std::string name;
        std::string type;
        std::string value;
    };

    struct Layer {
        std::string name;
        int width;
        int height;
        std::vector<int> data;
        float opacity;
        bool visible;
        std::string encoding;
        std::string compression;
        std::vector<Property> properties;
    };

    struct TilesetRef {
        int firstGID;
        std::string source;
        std::string name;
        int tileWidth;
        int tileHeight;
        int spacing;
        int margin;
        int tileCount;
        int columns;
        std::string image;
        int imageWidth;
        int imageHeight;
    };

    struct ObjectGroup {
        std::string name;
        std::vector<struct Object> objects;
    };

    struct Object {
        int id;
        std::string name;
        std::string type;
        float x, y;
        float width, height;
        std::vector<Property> properties;
    };

    struct Map {
        std::string version;
        std::string tiledVersion;
        std::string orientation;
        std::string renderOrder;
        int width;
        int height;
        int tileWidth;
        int tileHeight;
        bool infinite;
        std::vector<Layer> layers;
        std::vector<TilesetRef> tilesets;
        std::vector<ObjectGroup> objectGroups;
        std::vector<Property> properties;
    };
}

// Map metadata
struct MapMetadata {
    std::string name;
    std::string filename;
    std::string biome;
    int width;
    int height;
    bool loaded;
    bool procedural;

    MapMetadata() : width(0), height(0), loaded(false), procedural(false) {}
};

// Level streaming chunk
struct MapChunk {
    int chunkX;
    int chunkY;
    int chunkSize;
    std::unique_ptr<TileMap> tilemap;
    bool loaded;
    float distanceFromPlayer;

    MapChunk() : chunkX(0), chunkY(0), chunkSize(64), loaded(false), distanceFromPlayer(0.0f) {}
};

// Main map system
class MapSystem {
public:
    MapSystem();
    ~MapSystem();

    // Initialization
    bool initialize();
    void shutdown();

    // Map loading
    bool loadMap(const std::string& mapName);
    bool loadMapFromFile(const std::string& filename);
    bool loadTMXMap(const std::string& filename);
    void unloadMap(const std::string& mapName);
    void unloadAllMaps();

    // Current map management
    TileMap* getCurrentMap() { return m_currentMap.get(); }
    const TileMap* getCurrentMap() const { return m_currentMap.get(); }
    void setCurrentMap(const std::string& mapName);
    const std::string& getCurrentMapName() const { return m_currentMapName; }

    // Map registration
    void registerMap(const std::string& name, const MapMetadata& metadata);
    const MapMetadata* getMapMetadata(const std::string& name) const;

    // Level streaming
    void enableStreaming(bool enable) { m_streamingEnabled = enable; }
    bool isStreamingEnabled() const { return m_streamingEnabled; }
    void setStreamingDistance(float distance) { m_streamingDistance = distance; }
    void setChunkSize(int size) { m_chunkSize = size; }
    void updateStreaming(float playerX, float playerY);
    void loadChunk(int chunkX, int chunkY);
    void unloadChunk(int chunkX, int chunkY);

    // Minimap generation
    struct MinimapData {
        std::vector<unsigned char> pixels; // RGBA format
        int width;
        int height;
        float scale;
    };
    MinimapData generateMinimap(const std::string& mapName, int minimapWidth, int minimapHeight);
    MinimapData generateMinimapForCurrentMap(int width, int height);

    // World structure
    struct WorldData {
        std::string name;
        std::vector<std::string> regions;
        std::unordered_map<std::string, std::string> connections; // mapName -> connectedMapName
    };
    bool loadWorldStructure(const std::string& filename);
    const WorldData& getWorldData() const { return m_worldData; }

    // Teleportation/warp system
    struct WarpPoint {
        std::string name;
        std::string targetMap;
        float x, y;
        float targetX, targetY;
        bool enabled;
        std::string condition; // Optional condition for warp activation
    };
    void registerWarpPoint(const WarpPoint& warp);
    bool activateWarp(const std::string& warpName, float playerX, float playerY);
    const WarpPoint* getWarpPoint(const std::string& name) const;
    std::vector<WarpPoint> getWarpsInRange(float x, float y, float range) const;

    // Biome system
    enum class BiomeType {
        Forest,
        Desert,
        Snow,
        Volcanic,
        Swamp,
        Cave,
        Beach,
        Mountains,
        Plains,
        Jungle,
        Tundra,
        Corruption,
        Crystal,
        Ruins
    };

    struct BiomeData {
        BiomeType type;
        std::string name;
        std::vector<int> allowedTiles;
        float hazardChance;
        std::string ambientSound;
        float r, g, b, a; // Color tint
    };

    void registerBiome(const BiomeData& biome);
    const BiomeData* getBiome(BiomeType type) const;
    BiomeType getCurrentBiome() const { return m_currentBiome; }
    void setCurrentBiome(BiomeType type);

    // Environmental effects
    struct EnvironmentalHazard {
        enum Type {
            Fire,
            Poison,
            Ice,
            Lightning,
            Water,
            Acid
        };

        Type type;
        int x, y;
        int width, height;
        float damage;
        float tickRate;
        bool active;
    };

    void addHazard(const EnvironmentalHazard& hazard);
    void removeHazard(int x, int y);
    std::vector<EnvironmentalHazard*> getHazardsAt(int x, int y);
    void updateHazards(float deltaTime);

    // Update and render
    void update(float deltaTime);
    void render(int cameraX, int cameraY, int screenWidth, int screenHeight);

    // Debug
    void setDebugMode(bool enable) { m_debugMode = enable; }
    bool isDebugMode() const { return m_debugMode; }
    void renderDebugInfo();

    // Statistics
    struct Stats {
        int mapsLoaded;
        int chunksLoaded;
        int totalTiles;
        float memoryUsageMB;
    };
    Stats getStats() const;

private:
    // Current map
    std::unique_ptr<TileMap> m_currentMap;
    std::string m_currentMapName;
    BiomeType m_currentBiome;

    // Map registry
    std::unordered_map<std::string, MapMetadata> m_mapRegistry;
    std::unordered_map<std::string, std::unique_ptr<TileMap>> m_loadedMaps;

    // Level streaming
    bool m_streamingEnabled;
    float m_streamingDistance;
    int m_chunkSize;
    std::unordered_map<std::string, std::unique_ptr<MapChunk>> m_loadedChunks;

    // World data
    WorldData m_worldData;

    // Warp system
    std::unordered_map<std::string, WarpPoint> m_warpPoints;

    // Biome system
    std::unordered_map<BiomeType, BiomeData> m_biomes;

    // Environmental hazards
    std::vector<EnvironmentalHazard> m_hazards;

    // Debug
    bool m_debugMode;

    // Helper methods
    bool parseTMX(const std::string& xmlContent, TMX::Map& outMap);
    bool parseLayer(void* layerNode, TMX::Layer& outLayer);
    bool parseTileset(void* tilesetNode, TMX::TilesetRef& outTileset);
    bool parseObjectGroup(void* objectGroupNode, TMX::ObjectGroup& outGroup);

    void buildTileMapFromTMX(const TMX::Map& tmxMap, TileMap& tileMap);
    std::string getChunkKey(int chunkX, int chunkY) const;

    unsigned char getTileColorForMinimap(int tileID, BiomeType biome);
};

// Map loader utility
class MapLoader {
public:
    static bool loadJSON(const std::string& filename, TileMap& outMap);
    static bool saveJSON(const std::string& filename, const TileMap& map);
    static bool loadTMX(const std::string& filename, TileMap& outMap);

    // Convert between formats
    static bool convertTMXToJSON(const std::string& tmxFile, const std::string& jsonFile);
};

} // namespace World
