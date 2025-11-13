#include "MapSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace World {

// ==================== MapSystem Implementation ====================

MapSystem::MapSystem()
    : m_currentBiome(BiomeType::Forest)
    , m_streamingEnabled(false)
    , m_streamingDistance(2000.0f)
    , m_chunkSize(64)
    , m_debugMode(false)
{
}

MapSystem::~MapSystem() {
    shutdown();
}

bool MapSystem::initialize() {
    // Initialize default biomes
    BiomeData forest;
    forest.type = BiomeType::Forest;
    forest.name = "Forest";
    forest.hazardChance = 0.05f;
    forest.ambientSound = "forest_ambience";
    forest.r = 0.2f; forest.g = 0.8f; forest.b = 0.2f; forest.a = 1.0f;
    registerBiome(forest);

    BiomeData desert;
    desert.type = BiomeType::Desert;
    desert.name = "Desert";
    desert.hazardChance = 0.15f;
    desert.ambientSound = "desert_wind";
    desert.r = 0.9f; desert.g = 0.8f; desert.b = 0.4f; desert.a = 1.0f;
    registerBiome(desert);

    BiomeData snow;
    snow.type = BiomeType::Snow;
    snow.name = "Snow";
    snow.hazardChance = 0.1f;
    snow.ambientSound = "blizzard";
    snow.r = 0.9f; snow.g = 0.9f; snow.b = 1.0f; snow.a = 1.0f;
    registerBiome(snow);

    BiomeData volcanic;
    volcanic.type = BiomeType::Volcanic;
    volcanic.name = "Volcanic";
    volcanic.hazardChance = 0.3f;
    volcanic.ambientSound = "lava_bubbling";
    volcanic.r = 1.0f; volcanic.g = 0.3f; volcanic.b = 0.0f; volcanic.a = 1.0f;
    registerBiome(volcanic);

    BiomeData swamp;
    swamp.type = BiomeType::Swamp;
    swamp.name = "Swamp";
    swamp.hazardChance = 0.2f;
    swamp.ambientSound = "swamp_ambience";
    swamp.r = 0.4f; swamp.g = 0.6f; swamp.b = 0.4f; swamp.a = 1.0f;
    registerBiome(swamp);

    BiomeData cave;
    cave.type = BiomeType::Cave;
    cave.name = "Cave";
    cave.hazardChance = 0.15f;
    cave.ambientSound = "cave_drip";
    cave.r = 0.3f; cave.g = 0.3f; cave.b = 0.4f; cave.a = 1.0f;
    registerBiome(cave);

    BiomeData beach;
    beach.type = BiomeType::Beach;
    beach.name = "Beach";
    beach.hazardChance = 0.02f;
    beach.ambientSound = "ocean_waves";
    beach.r = 0.9f; beach.g = 0.9f; beach.b = 0.7f; beach.a = 1.0f;
    registerBiome(beach);

    BiomeData mountains;
    mountains.type = BiomeType::Mountains;
    mountains.name = "Mountains";
    mountains.hazardChance = 0.12f;
    mountains.ambientSound = "wind_howling";
    mountains.r = 0.6f; mountains.g = 0.6f; mountains.b = 0.65f; mountains.a = 1.0f;
    registerBiome(mountains);

    BiomeData plains;
    plains.type = BiomeType::Plains;
    plains.name = "Plains";
    plains.hazardChance = 0.03f;
    plains.ambientSound = "grassland";
    plains.r = 0.5f; plains.g = 0.8f; plains.b = 0.3f; plains.a = 1.0f;
    registerBiome(plains);

    BiomeData jungle;
    jungle.type = BiomeType::Jungle;
    jungle.name = "Jungle";
    jungle.hazardChance = 0.25f;
    jungle.ambientSound = "jungle_ambience";
    jungle.r = 0.1f; jungle.g = 0.6f; jungle.b = 0.2f; jungle.a = 1.0f;
    registerBiome(jungle);

    BiomeData tundra;
    tundra.type = BiomeType::Tundra;
    tundra.name = "Tundra";
    tundra.hazardChance = 0.08f;
    tundra.ambientSound = "arctic_wind";
    tundra.r = 0.7f; tundra.g = 0.8f; tundra.b = 0.9f; tundra.a = 1.0f;
    registerBiome(tundra);

    BiomeData corruption;
    corruption.type = BiomeType::Corruption;
    corruption.name = "Corruption";
    corruption.hazardChance = 0.4f;
    corruption.ambientSound = "corruption_whispers";
    corruption.r = 0.5f; corruption.g = 0.1f; corruption.b = 0.5f; corruption.a = 1.0f;
    registerBiome(corruption);

    BiomeData crystal;
    crystal.type = BiomeType::Crystal;
    crystal.name = "Crystal";
    crystal.hazardChance = 0.1f;
    crystal.ambientSound = "crystal_hum";
    crystal.r = 0.5f; crystal.g = 0.8f; crystal.b = 1.0f; crystal.a = 1.0f;
    registerBiome(crystal);

    BiomeData ruins;
    ruins.type = BiomeType::Ruins;
    ruins.name = "Ruins";
    ruins.hazardChance = 0.18f;
    ruins.ambientSound = "ancient_ruins";
    ruins.r = 0.6f; ruins.g = 0.55f; ruins.b = 0.5f; ruins.a = 1.0f;
    registerBiome(ruins);

    return true;
}

void MapSystem::shutdown() {
    unloadAllMaps();
    m_warpPoints.clear();
    m_biomes.clear();
    m_hazards.clear();
}

bool MapSystem::loadMap(const std::string& mapName) {
    // Check if already loaded
    auto it = m_loadedMaps.find(mapName);
    if (it != m_loadedMaps.end()) {
        m_currentMap = std::move(it->second);
        m_currentMapName = mapName;
        m_loadedMaps.erase(it);
        return true;
    }

    // Get metadata
    const MapMetadata* metadata = getMapMetadata(mapName);
    if (!metadata) {
        std::cerr << "Map not found: " << mapName << std::endl;
        return false;
    }

    // Load from file
    if (!metadata->filename.empty()) {
        return loadMapFromFile(metadata->filename);
    }

    return false;
}

bool MapSystem::loadMapFromFile(const std::string& filename) {
    // Determine file type by extension
    if (filename.find(".tmx") != std::string::npos) {
        return loadTMXMap(filename);
    } else if (filename.find(".json") != std::string::npos) {
        auto tilemap = std::make_unique<TileMap>();
        if (MapLoader::loadJSON(filename, *tilemap)) {
            m_currentMap = std::move(tilemap);
            return true;
        }
    }

    return false;
}

bool MapSystem::loadTMXMap(const std::string& filename) {
    auto tilemap = std::make_unique<TileMap>();
    if (MapLoader::loadTMX(filename, *tilemap)) {
        m_currentMap = std::move(tilemap);
        return true;
    }
    return false;
}

void MapSystem::unloadMap(const std::string& mapName) {
    // Move current map to loaded maps if it matches
    if (m_currentMapName == mapName && m_currentMap) {
        m_loadedMaps[mapName] = std::move(m_currentMap);
        m_currentMapName.clear();
    }

    // Remove from loaded maps
    m_loadedMaps.erase(mapName);
}

void MapSystem::unloadAllMaps() {
    m_currentMap.reset();
    m_currentMapName.clear();
    m_loadedMaps.clear();
    m_loadedChunks.clear();
}

void MapSystem::setCurrentMap(const std::string& mapName) {
    loadMap(mapName);
}

void MapSystem::registerMap(const std::string& name, const MapMetadata& metadata) {
    m_mapRegistry[name] = metadata;
}

const MapMetadata* MapSystem::getMapMetadata(const std::string& name) const {
    auto it = m_mapRegistry.find(name);
    if (it != m_mapRegistry.end()) {
        return &it->second;
    }
    return nullptr;
}

void MapSystem::updateStreaming(float playerX, float playerY) {
    if (!m_streamingEnabled || !m_currentMap) {
        return;
    }

    // Calculate player chunk
    int playerChunkX = static_cast<int>(playerX / m_chunkSize);
    int playerChunkY = static_cast<int>(playerY / m_chunkSize);

    // Load nearby chunks
    int loadRadius = static_cast<int>(m_streamingDistance / m_chunkSize) + 1;
    for (int dy = -loadRadius; dy <= loadRadius; ++dy) {
        for (int dx = -loadRadius; dx <= loadRadius; ++dx) {
            int chunkX = playerChunkX + dx;
            int chunkY = playerChunkY + dy;

            float distance = std::sqrt(dx * dx + dy * dy) * m_chunkSize;
            if (distance <= m_streamingDistance) {
                std::string key = getChunkKey(chunkX, chunkY);
                if (m_loadedChunks.find(key) == m_loadedChunks.end()) {
                    loadChunk(chunkX, chunkY);
                }
            }
        }
    }

    // Unload distant chunks
    std::vector<std::string> chunksToUnload;
    for (auto& pair : m_loadedChunks) {
        MapChunk* chunk = pair.second.get();
        float dx = chunk->chunkX - playerChunkX;
        float dy = chunk->chunkY - playerChunkY;
        float distance = std::sqrt(dx * dx + dy * dy) * m_chunkSize;

        if (distance > m_streamingDistance * 1.5f) {
            chunksToUnload.push_back(pair.first);
        }
    }

    for (const auto& key : chunksToUnload) {
        m_loadedChunks.erase(key);
    }
}

void MapSystem::loadChunk(int chunkX, int chunkY) {
    std::string key = getChunkKey(chunkX, chunkY);
    auto chunk = std::make_unique<MapChunk>();
    chunk->chunkX = chunkX;
    chunk->chunkY = chunkY;
    chunk->chunkSize = m_chunkSize;
    chunk->loaded = true;

    // TODO: Load actual chunk data
    chunk->tilemap = std::make_unique<TileMap>();
    chunk->tilemap->initialize(m_chunkSize, m_chunkSize, 32, 32);

    m_loadedChunks[key] = std::move(chunk);
}

void MapSystem::unloadChunk(int chunkX, int chunkY) {
    std::string key = getChunkKey(chunkX, chunkY);
    m_loadedChunks.erase(key);
}

MapSystem::MinimapData MapSystem::generateMinimap(const std::string& mapName,
                                                   int minimapWidth, int minimapHeight) {
    MinimapData data;
    data.width = minimapWidth;
    data.height = minimapHeight;
    data.scale = 1.0f;

    // Load map if needed
    const MapMetadata* metadata = getMapMetadata(mapName);
    if (!metadata) {
        return data;
    }

    // Generate minimap pixels
    data.pixels.resize(minimapWidth * minimapHeight * 4);

    // TODO: Render map to minimap
    // For now, fill with placeholder color
    for (int i = 0; i < minimapWidth * minimapHeight * 4; i += 4) {
        data.pixels[i + 0] = 100; // R
        data.pixels[i + 1] = 100; // G
        data.pixels[i + 2] = 100; // B
        data.pixels[i + 3] = 255; // A
    }

    return data;
}

MapSystem::MinimapData MapSystem::generateMinimapForCurrentMap(int width, int height) {
    if (m_currentMapName.empty()) {
        return MinimapData();
    }
    return generateMinimap(m_currentMapName, width, height);
}

bool MapSystem::loadWorldStructure(const std::string& filename) {
    // TODO: Load world structure from JSON file
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Placeholder implementation
    file.close();
    return true;
}

void MapSystem::registerWarpPoint(const WarpPoint& warp) {
    m_warpPoints[warp.name] = warp;
}

bool MapSystem::activateWarp(const std::string& warpName, float playerX, float playerY) {
    const WarpPoint* warp = getWarpPoint(warpName);
    if (!warp || !warp->enabled) {
        return false;
    }

    // Check if player is within range
    float dx = playerX - warp->x;
    float dy = playerY - warp->y;
    float distSq = dx * dx + dy * dy;

    if (distSq > 100.0f * 100.0f) { // 100 pixel range
        return false;
    }

    // Load target map
    loadMap(warp->targetMap);

    // TODO: Move player to target position
    // This would be handled by the game logic

    return true;
}

const MapSystem::WarpPoint* MapSystem::getWarpPoint(const std::string& name) const {
    auto it = m_warpPoints.find(name);
    if (it != m_warpPoints.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<MapSystem::WarpPoint> MapSystem::getWarpsInRange(float x, float y, float range) const {
    std::vector<WarpPoint> warps;

    for (const auto& pair : m_warpPoints) {
        const WarpPoint& warp = pair.second;
        float dx = x - warp.x;
        float dy = y - warp.y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= range * range) {
            warps.push_back(warp);
        }
    }

    return warps;
}

void MapSystem::registerBiome(const BiomeData& biome) {
    m_biomes[biome.type] = biome;
}

const MapSystem::BiomeData* MapSystem::getBiome(BiomeType type) const {
    auto it = m_biomes.find(type);
    if (it != m_biomes.end()) {
        return &it->second;
    }
    return nullptr;
}

void MapSystem::setCurrentBiome(BiomeType type) {
    m_currentBiome = type;
}

void MapSystem::addHazard(const EnvironmentalHazard& hazard) {
    m_hazards.push_back(hazard);
}

void MapSystem::removeHazard(int x, int y) {
    m_hazards.erase(
        std::remove_if(m_hazards.begin(), m_hazards.end(),
            [x, y](const EnvironmentalHazard& h) {
                return h.x == x && h.y == y;
            }),
        m_hazards.end()
    );
}

std::vector<MapSystem::EnvironmentalHazard*> MapSystem::getHazardsAt(int x, int y) {
    std::vector<EnvironmentalHazard*> hazards;

    for (auto& hazard : m_hazards) {
        if (x >= hazard.x && x < hazard.x + hazard.width &&
            y >= hazard.y && y < hazard.y + hazard.height &&
            hazard.active) {
            hazards.push_back(&hazard);
        }
    }

    return hazards;
}

void MapSystem::updateHazards(float deltaTime) {
    // Update hazard effects
    for (auto& hazard : m_hazards) {
        if (hazard.active) {
            // TODO: Apply damage to entities in hazard area
        }
    }
}

void MapSystem::update(float deltaTime) {
    updateHazards(deltaTime);

    // Update current map
    if (m_currentMap) {
        // TODO: Update tile animations, etc.
    }
}

void MapSystem::render(int cameraX, int cameraY, int screenWidth, int screenHeight) {
    if (m_currentMap) {
        m_currentMap->render(cameraX, cameraY, screenWidth, screenHeight);
    }

    if (m_debugMode) {
        renderDebugInfo();
    }
}

void MapSystem::renderDebugInfo() {
    // Render debug overlays
    // TODO: Implement debug rendering
}

MapSystem::Stats MapSystem::getStats() const {
    Stats stats;
    stats.mapsLoaded = m_loadedMaps.size() + (m_currentMap ? 1 : 0);
    stats.chunksLoaded = m_loadedChunks.size();
    stats.totalTiles = 0;
    stats.memoryUsageMB = 0.0f;

    if (m_currentMap) {
        stats.totalTiles = m_currentMap->getWidth() * m_currentMap->getHeight();
    }

    // Rough memory estimate
    stats.memoryUsageMB = stats.totalTiles * sizeof(Tile) / (1024.0f * 1024.0f);

    return stats;
}

std::string MapSystem::getChunkKey(int chunkX, int chunkY) const {
    return std::to_string(chunkX) + "_" + std::to_string(chunkY);
}

unsigned char MapSystem::getTileColorForMinimap(int tileID, BiomeType biome) {
    // Simple color mapping based on tile ID
    return static_cast<unsigned char>((tileID * 10) % 256);
}

// ==================== MapLoader Implementation ====================

bool MapLoader::loadJSON(const std::string& filename, TileMap& outMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TODO: Implement JSON parsing
    // For now, this is a placeholder

    file.close();
    return true;
}

bool MapLoader::saveJSON(const std::string& filename, const TileMap& map) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TODO: Implement JSON serialization

    file.close();
    return true;
}

bool MapLoader::loadTMX(const std::string& filename, TileMap& outMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TODO: Implement TMX parsing using XML parser
    // This would require an XML parsing library like TinyXML2 or RapidXML

    file.close();
    return true;
}

bool MapLoader::convertTMXToJSON(const std::string& tmxFile, const std::string& jsonFile) {
    TileMap tempMap;
    if (!loadTMX(tmxFile, tempMap)) {
        return false;
    }
    return saveJSON(jsonFile, tempMap);
}

} // namespace World
