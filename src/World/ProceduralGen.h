#pragma once

#include "TileMap.h"
#include "MapSystem.h"
#include <vector>
#include <random>
#include <memory>

namespace World {

// Room structure for dungeon generation
struct Room {
    int x, y;
    int width, height;
    bool connected;
    int roomType; // 0=normal, 1=entrance, 2=boss, 3=treasure, 4=shop
    std::vector<int> connectedRooms;

    Room() : x(0), y(0), width(0), height(0), connected(false), roomType(0) {}
    Room(int _x, int _y, int _w, int _h)
        : x(_x), y(_y), width(_w), height(_h), connected(false), roomType(0) {}

    int centerX() const { return x + width / 2; }
    int centerY() const { return y + height / 2; }

    bool intersects(const Room& other, int padding = 0) const {
        return !(x + width + padding <= other.x ||
                 other.x + other.width + padding <= x ||
                 y + height + padding <= other.y ||
                 other.y + other.height + padding <= y);
    }
};

// Corridor/hallway structure
struct Corridor {
    std::vector<std::pair<int, int>> points;
    int width;

    Corridor() : width(1) {}
};

// Dungeon generation parameters
struct DungeonParams {
    int width;
    int height;
    int minRooms;
    int maxRooms;
    int minRoomSize;
    int maxRoomSize;
    int corridorWidth;
    float treasureRoomChance;
    float bossRoomChance;
    float trapChance;
    float enemyDensity;
    bool allowCycles; // Allow loops in dungeon
    int seed;

    DungeonParams()
        : width(100), height(100)
        , minRooms(5), maxRooms(15)
        , minRoomSize(5), maxRoomSize(12)
        , corridorWidth(3)
        , treasureRoomChance(0.15f)
        , bossRoomChance(0.1f)
        , trapChance(0.1f)
        , enemyDensity(0.2f)
        , allowCycles(false)
        , seed(0)
    {}
};

// Procedural generator algorithms
class ProceduralGenerator {
public:
    ProceduralGenerator();
    ~ProceduralGenerator();

    // Main generation methods
    std::unique_ptr<TileMap> generateDungeon(const DungeonParams& params);
    std::unique_ptr<TileMap> generateCave(int width, int height, float fillProbability, int seed);
    std::unique_ptr<TileMap> generateMaze(int width, int height, int seed);
    std::unique_ptr<TileMap> generateArena(int width, int height, const std::string& theme);

    // Dungeon algorithms
    enum class Algorithm {
        BSP,              // Binary Space Partitioning
        RandomWalk,       // Drunkard's walk
        CellularAutomata, // For caves
        Maze,             // Recursive maze generation
        Rooms,            // Simple room placement
        Mixed             // Combination of algorithms
    };

    void setAlgorithm(Algorithm algo) { m_algorithm = algo; }
    Algorithm getAlgorithm() const { return m_algorithm; }

    // Room-based generation
    std::vector<Room> generateRooms(const DungeonParams& params);
    void connectRooms(std::vector<Room>& rooms, std::vector<Corridor>& corridors);
    void placeSpecialRooms(std::vector<Room>& rooms, const DungeonParams& params);

    // BSP tree generation
    struct BSPNode {
        int x, y, width, height;
        std::unique_ptr<BSPNode> left;
        std::unique_ptr<BSPNode> right;
        Room room;
        bool hasRoom;

        BSPNode(int _x, int _y, int _w, int _h)
            : x(_x), y(_y), width(_w), height(_h), hasRoom(false) {}
    };
    std::unique_ptr<BSPNode> generateBSP(int x, int y, int width, int height,
                                         int minSize, int depth);
    void extractRoomsFromBSP(BSPNode* node, std::vector<Room>& rooms);

    // Cellular automata for caves
    void initializeCellularGrid(std::vector<std::vector<int>>& grid,
                               int width, int height, float fillProbability);
    void applyCellularAutomataStep(std::vector<std::vector<int>>& grid);
    void smoothCave(std::vector<std::vector<int>>& grid, int iterations);

    // Random walk
    void randomWalk(std::vector<std::vector<int>>& grid, int startX, int startY,
                   int steps, int brushSize);

    // Maze generation
    void generateMazeRecursive(std::vector<std::vector<int>>& grid, int x, int y);

    // Post-processing
    void addDoors(TileMap& map, const std::vector<Room>& rooms);
    void addTraps(TileMap& map, const DungeonParams& params);
    void addDecorations(TileMap& map, const std::vector<Room>& rooms);
    void addLighting(TileMap& map, const std::vector<Room>& rooms);
    void ensureConnectivity(std::vector<std::vector<int>>& grid);

    // Tileset assignment
    void applyTileset(TileMap& map, MapSystem::BiomeType biome);

    // Spawn point generation
    struct SpawnPoint {
        enum Type {
            Player,
            Enemy,
            Boss,
            Treasure,
            NPC,
            Trap
        };

        Type type;
        int x, y;
        std::string entityID;
        int level;
    };
    std::vector<SpawnPoint> generateSpawnPoints(const std::vector<Room>& rooms,
                                               const DungeonParams& params);

    // Seed management
    void setSeed(unsigned int seed) { m_seed = seed; m_rng.seed(seed); }
    unsigned int getSeed() const { return m_seed; }

    // Validation
    bool validateDungeon(const TileMap& map);
    bool isReachable(const std::vector<std::vector<int>>& grid, int x1, int y1,
                    int x2, int y2);

private:
    Algorithm m_algorithm;
    unsigned int m_seed;
    std::mt19937 m_rng;

    // Helper methods
    int randomInt(int min, int max);
    float randomFloat(float min, float max);
    bool randomBool(float probability);

    void fillRect(TileMap& map, const std::string& layer, int x, int y,
                 int width, int height, int tileID);
    void drawCorridor(TileMap& map, const std::string& layer,
                     int x1, int y1, int x2, int y2, int width);
    void floodFill(std::vector<std::vector<int>>& grid, int x, int y,
                  int oldValue, int newValue);

    std::vector<std::pair<int, int>> findPath(const std::vector<std::vector<int>>& grid,
                                             int x1, int y1, int x2, int y2);
};

// Template system for preset dungeon layouts
class DungeonTemplate {
public:
    struct TemplateData {
        std::string name;
        std::string description;
        int width, height;
        std::vector<std::string> layout; // ASCII layout
        std::unordered_map<char, int> tileMapping;
        DungeonParams suggestedParams;
    };

    static bool loadTemplate(const std::string& filename, TemplateData& outTemplate);
    static std::unique_ptr<TileMap> generateFromTemplate(const TemplateData& tmpl);
    static void saveTemplate(const std::string& filename, const TileMap& map);
};

// Noise generation for terrain
class NoiseGenerator {
public:
    NoiseGenerator(unsigned int seed);

    // Perlin noise
    float perlin(float x, float y);
    float perlin(float x, float y, float z);

    // Fractal noise
    float fractal(float x, float y, int octaves, float persistence, float lacunarity);

    // Simplex noise
    float simplex(float x, float y);

    // Voronoi noise
    float voronoi(float x, float y, int cellCount);

private:
    std::mt19937 m_rng;
    std::vector<int> m_permutation;

    void initializePermutation();
    float fade(float t);
    float lerp(float t, float a, float b);
    float grad(int hash, float x, float y);
};

// Biome blending for world generation
class BiomeBlender {
public:
    struct BiomeTile {
        MapSystem::BiomeType biome;
        int tileID;
        float weight;
    };

    static int blendTiles(const std::vector<BiomeTile>& tiles);
    static MapSystem::BiomeType determineBiome(float temperature, float moisture, float elevation);

    // Generate transition zones between biomes
    static void generateTransitions(TileMap& map,
                                   const std::vector<std::pair<int, int>>& biomePositions,
                                   const std::vector<MapSystem::BiomeType>& biomes);
};

// World generation for overworld
class WorldGenerator {
public:
    struct WorldGenParams {
        int width, height;
        int seed;
        float scale;
        int octaves;
        float persistence;
        float lacunarity;
        float waterLevel;
        float mountainLevel;

        WorldGenParams()
            : width(200), height(200), seed(0), scale(0.05f)
            , octaves(6), persistence(0.5f), lacunarity(2.0f)
            , waterLevel(0.3f), mountainLevel(0.7f)
        {}
    };

    static std::unique_ptr<TileMap> generateWorld(const WorldGenParams& params);
    static void generateHeightmap(std::vector<std::vector<float>>& heightmap,
                                 const WorldGenParams& params);
    static void generateMoistureMap(std::vector<std::vector<float>>& moisture,
                                   const WorldGenParams& params);
    static void generateTemperatureMap(std::vector<std::vector<float>>& temperature,
                                      const WorldGenParams& params);

    // River generation
    static void generateRivers(TileMap& map,
                              const std::vector<std::vector<float>>& heightmap,
                              int riverCount);

    // Path generation for roads
    static void generatePaths(TileMap& map,
                             const std::vector<std::pair<int, int>>& settlements);
};

} // namespace World
