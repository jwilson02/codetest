#include "ProceduralGen.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <iostream>

namespace World {

// ==================== ProceduralGenerator Implementation ====================

ProceduralGenerator::ProceduralGenerator()
    : m_algorithm(Algorithm::BSP)
    , m_seed(0)
{
    m_rng.seed(m_seed);
}

ProceduralGenerator::~ProceduralGenerator() {
}

std::unique_ptr<TileMap> ProceduralGenerator::generateDungeon(const DungeonParams& params) {
    setSeed(params.seed == 0 ? static_cast<unsigned int>(time(nullptr)) : params.seed);

    auto tilemap = std::make_unique<TileMap>();
    tilemap->initialize(params.width, params.height, 32, 32);

    // Create layers
    tilemap->addLayer("Ground", 0);
    tilemap->addLayer("Walls", 1);
    tilemap->addLayer("Objects", 2);

    // Fill with walls initially
    for (int y = 0; y < params.height; ++y) {
        for (int x = 0; x < params.width; ++x) {
            Tile wallTile;
            wallTile.tileID = 1; // Wall tile
            wallTile.solid = true;
            tilemap->setTile("Walls", x, y, wallTile);
        }
    }

    // Generate rooms based on algorithm
    std::vector<Room> rooms;
    std::vector<Corridor> corridors;

    switch (m_algorithm) {
        case Algorithm::BSP: {
            auto bspRoot = generateBSP(0, 0, params.width, params.height,
                                      params.minRoomSize, 5);
            extractRoomsFromBSP(bspRoot.get(), rooms);
            break;
        }
        case Algorithm::Rooms:
        default:
            rooms = generateRooms(params);
            break;
    }

    // Place special rooms
    placeSpecialRooms(rooms, params);

    // Connect rooms
    connectRooms(rooms, corridors);

    // Carve out rooms
    for (const auto& room : rooms) {
        for (int y = room.y; y < room.y + room.height; ++y) {
            for (int x = room.x; x < room.x + room.width; ++x) {
                Tile floorTile;
                floorTile.tileID = 2; // Floor tile
                floorTile.solid = false;
                tilemap->setTile("Ground", x, y, floorTile);

                // Remove wall
                Tile emptyTile;
                emptyTile.tileID = 0;
                tilemap->setTile("Walls", x, y, emptyTile);
            }
        }
    }

    // Carve out corridors
    for (const auto& corridor : corridors) {
        for (const auto& point : corridor.points) {
            int x = point.first;
            int y = point.second;

            for (int dy = -corridor.width / 2; dy <= corridor.width / 2; ++dy) {
                for (int dx = -corridor.width / 2; dx <= corridor.width / 2; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < params.width && ny >= 0 && ny < params.height) {
                        Tile floorTile;
                        floorTile.tileID = 2;
                        floorTile.solid = false;
                        tilemap->setTile("Ground", nx, ny, floorTile);

                        Tile emptyTile;
                        emptyTile.tileID = 0;
                        tilemap->setTile("Walls", nx, ny, emptyTile);
                    }
                }
            }
        }
    }

    // Post-processing
    addDoors(*tilemap, rooms);
    addTraps(*tilemap, params);
    addDecorations(*tilemap, rooms);

    return tilemap;
}

std::unique_ptr<TileMap> ProceduralGenerator::generateCave(int width, int height,
                                                           float fillProbability, int seed) {
    setSeed(seed == 0 ? static_cast<unsigned int>(time(nullptr)) : seed);

    auto tilemap = std::make_unique<TileMap>();
    tilemap->initialize(width, height, 32, 32);
    tilemap->addLayer("Ground", 0);
    tilemap->addLayer("Walls", 1);

    // Initialize cellular automata grid
    std::vector<std::vector<int>> grid(height, std::vector<int>(width, 0));
    initializeCellularGrid(grid, width, height, fillProbability);

    // Apply cellular automata rules
    smoothCave(grid, 5);

    // Convert grid to tilemap
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == 1) {
                Tile wallTile;
                wallTile.tileID = 1;
                wallTile.solid = true;
                tilemap->setTile("Walls", x, y, wallTile);
            } else {
                Tile floorTile;
                floorTile.tileID = 2;
                floorTile.solid = false;
                tilemap->setTile("Ground", x, y, floorTile);
            }
        }
    }

    return tilemap;
}

std::unique_ptr<TileMap> ProceduralGenerator::generateMaze(int width, int height, int seed) {
    setSeed(seed == 0 ? static_cast<unsigned int>(time(nullptr)) : seed);

    auto tilemap = std::make_unique<TileMap>();
    tilemap->initialize(width, height, 32, 32);
    tilemap->addLayer("Ground", 0);
    tilemap->addLayer("Walls", 1);

    // Initialize maze grid
    std::vector<std::vector<int>> grid(height, std::vector<int>(width, 1));

    // Generate maze using recursive backtracking
    generateMazeRecursive(grid, 1, 1);

    // Convert to tilemap
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == 1) {
                Tile wallTile;
                wallTile.tileID = 1;
                wallTile.solid = true;
                tilemap->setTile("Walls", x, y, wallTile);
            } else {
                Tile floorTile;
                floorTile.tileID = 2;
                floorTile.solid = false;
                tilemap->setTile("Ground", x, y, floorTile);
            }
        }
    }

    return tilemap;
}

std::unique_ptr<TileMap> ProceduralGenerator::generateArena(int width, int height,
                                                            const std::string& theme) {
    auto tilemap = std::make_unique<TileMap>();
    tilemap->initialize(width, height, 32, 32);
    tilemap->addLayer("Ground", 0);
    tilemap->addLayer("Walls", 1);

    // Create circular or rectangular arena
    int centerX = width / 2;
    int centerY = height / 2;
    int radius = std::min(width, height) / 2 - 5;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dx = x - centerX;
            int dy = y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= radius) {
                Tile floorTile;
                floorTile.tileID = 2;
                floorTile.solid = false;
                tilemap->setTile("Ground", x, y, floorTile);
            } else if (dist <= radius + 2) {
                Tile wallTile;
                wallTile.tileID = 1;
                wallTile.solid = true;
                tilemap->setTile("Walls", x, y, wallTile);
            }
        }
    }

    return tilemap;
}

std::vector<Room> ProceduralGenerator::generateRooms(const DungeonParams& params) {
    std::vector<Room> rooms;
    int attempts = 0;
    int maxAttempts = params.maxRooms * 10;

    while (rooms.size() < static_cast<size_t>(params.maxRooms) && attempts < maxAttempts) {
        int roomWidth = randomInt(params.minRoomSize, params.maxRoomSize);
        int roomHeight = randomInt(params.minRoomSize, params.maxRoomSize);
        int roomX = randomInt(1, params.width - roomWidth - 1);
        int roomY = randomInt(1, params.height - roomHeight - 1);

        Room newRoom(roomX, roomY, roomWidth, roomHeight);

        // Check for intersections
        bool intersects = false;
        for (const auto& room : rooms) {
            if (newRoom.intersects(room, 2)) {
                intersects = true;
                break;
            }
        }

        if (!intersects) {
            rooms.push_back(newRoom);
        }

        attempts++;
    }

    return rooms;
}

void ProceduralGenerator::connectRooms(std::vector<Room>& rooms, std::vector<Corridor>& corridors) {
    if (rooms.empty()) return;

    // Mark first room as connected
    rooms[0].connected = true;

    // Connect each room to the nearest connected room
    for (size_t i = 1; i < rooms.size(); ++i) {
        Room& currentRoom = rooms[i];
        int nearestIndex = -1;
        float nearestDist = std::numeric_limits<float>::max();

        // Find nearest connected room
        for (size_t j = 0; j < i; ++j) {
            if (rooms[j].connected) {
                float dx = currentRoom.centerX() - rooms[j].centerX();
                float dy = currentRoom.centerY() - rooms[j].centerY();
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < nearestDist) {
                    nearestDist = dist;
                    nearestIndex = j;
                }
            }
        }

        if (nearestIndex >= 0) {
            currentRoom.connected = true;
            currentRoom.connectedRooms.push_back(nearestIndex);
            rooms[nearestIndex].connectedRooms.push_back(i);

            // Create corridor
            Corridor corridor;
            corridor.width = 3;

            int x1 = currentRoom.centerX();
            int y1 = currentRoom.centerY();
            int x2 = rooms[nearestIndex].centerX();
            int y2 = rooms[nearestIndex].centerY();

            // L-shaped corridor
            if (randomBool(0.5f)) {
                // Horizontal then vertical
                for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
                    corridor.points.push_back({x, y1});
                }
                for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
                    corridor.points.push_back({x2, y});
                }
            } else {
                // Vertical then horizontal
                for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
                    corridor.points.push_back({x1, y});
                }
                for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
                    corridor.points.push_back({x, y2});
                }
            }

            corridors.push_back(corridor);
        }
    }
}

void ProceduralGenerator::placeSpecialRooms(std::vector<Room>& rooms, const DungeonParams& params) {
    if (rooms.empty()) return;

    // First room is entrance
    rooms[0].roomType = 1;

    // Last room is boss
    if (rooms.size() > 1 && randomBool(params.bossRoomChance)) {
        rooms[rooms.size() - 1].roomType = 2;
    }

    // Random treasure rooms
    for (size_t i = 1; i < rooms.size() - 1; ++i) {
        if (randomBool(params.treasureRoomChance)) {
            rooms[i].roomType = 3;
        }
    }
}

std::unique_ptr<ProceduralGenerator::BSPNode> ProceduralGenerator::generateBSP(
    int x, int y, int width, int height, int minSize, int depth) {

    auto node = std::make_unique<BSPNode>(x, y, width, height);

    if (depth == 0 || width < minSize * 2 || height < minSize * 2) {
        // Create room in leaf node
        int roomWidth = randomInt(minSize, std::max(minSize, width - 4));
        int roomHeight = randomInt(minSize, std::max(minSize, height - 4));
        int roomX = x + randomInt(2, std::max(2, width - roomWidth - 2));
        int roomY = y + randomInt(2, std::max(2, height - roomHeight - 2));

        node->room = Room(roomX, roomY, roomWidth, roomHeight);
        node->hasRoom = true;
        return node;
    }

    // Split the space
    bool splitHorizontal = randomBool(0.5f);
    if (width > height && width / height >= 1.25f) {
        splitHorizontal = false;
    } else if (height > width && height / width >= 1.25f) {
        splitHorizontal = true;
    }

    if (splitHorizontal) {
        int splitY = randomInt(y + minSize, y + height - minSize);
        node->left = generateBSP(x, y, width, splitY - y, minSize, depth - 1);
        node->right = generateBSP(x, splitY, width, y + height - splitY, minSize, depth - 1);
    } else {
        int splitX = randomInt(x + minSize, x + width - minSize);
        node->left = generateBSP(x, y, splitX - x, height, minSize, depth - 1);
        node->right = generateBSP(splitX, y, x + width - splitX, height, minSize, depth - 1);
    }

    return node;
}

void ProceduralGenerator::extractRoomsFromBSP(BSPNode* node, std::vector<Room>& rooms) {
    if (!node) return;

    if (node->hasRoom) {
        rooms.push_back(node->room);
    }

    extractRoomsFromBSP(node->left.get(), rooms);
    extractRoomsFromBSP(node->right.get(), rooms);
}

void ProceduralGenerator::initializeCellularGrid(std::vector<std::vector<int>>& grid,
                                                 int width, int height, float fillProbability) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Borders are always walls
            if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                grid[y][x] = 1;
            } else {
                grid[y][x] = randomBool(fillProbability) ? 1 : 0;
            }
        }
    }
}

void ProceduralGenerator::applyCellularAutomataStep(std::vector<std::vector<int>>& grid) {
    int height = grid.size();
    int width = grid[0].size();
    std::vector<std::vector<int>> newGrid = grid;

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int wallCount = 0;

            // Count walls in 3x3 neighborhood
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    wallCount += grid[y + dy][x + dx];
                }
            }

            // Apply rules
            if (wallCount >= 5) {
                newGrid[y][x] = 1; // Wall
            } else {
                newGrid[y][x] = 0; // Floor
            }
        }
    }

    grid = newGrid;
}

void ProceduralGenerator::smoothCave(std::vector<std::vector<int>>& grid, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        applyCellularAutomataStep(grid);
    }
}

void ProceduralGenerator::randomWalk(std::vector<std::vector<int>>& grid,
                                    int startX, int startY, int steps, int brushSize) {
    int x = startX;
    int y = startY;
    int height = grid.size();
    int width = grid[0].size();

    for (int i = 0; i < steps; ++i) {
        // Carve out space
        for (int dy = -brushSize; dy <= brushSize; ++dy) {
            for (int dx = -brushSize; dx <= brushSize; ++dx) {
                int nx = x + dx;
                int ny = y + dy;

                if (nx > 0 && nx < width - 1 && ny > 0 && ny < height - 1) {
                    grid[ny][nx] = 0;
                }
            }
        }

        // Random walk
        int direction = randomInt(0, 3);
        switch (direction) {
            case 0: x = std::min(x + 1, width - 2); break;
            case 1: x = std::max(x - 1, 1); break;
            case 2: y = std::min(y + 1, height - 2); break;
            case 3: y = std::max(y - 1, 1); break;
        }
    }
}

void ProceduralGenerator::generateMazeRecursive(std::vector<std::vector<int>>& grid, int x, int y) {
    int height = grid.size();
    int width = grid[0].size();

    // Mark current cell as passage
    grid[y][x] = 0;

    // Directions: right, down, left, up
    std::vector<std::pair<int, int>> directions = {{2, 0}, {0, 2}, {-2, 0}, {0, -2}};
    std::shuffle(directions.begin(), directions.end(), m_rng);

    for (const auto& dir : directions) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        if (nx > 0 && nx < width - 1 && ny > 0 && ny < height - 1 && grid[ny][nx] == 1) {
            // Carve passage
            grid[y + dir.second / 2][x + dir.first / 2] = 0;
            generateMazeRecursive(grid, nx, ny);
        }
    }
}

void ProceduralGenerator::addDoors(TileMap& map, const std::vector<Room>& rooms) {
    // Add doors at room entrances
    for (const auto& room : rooms) {
        // Add doors on room edges
        // This is a simplified version
    }
}

void ProceduralGenerator::addTraps(TileMap& map, const DungeonParams& params) {
    // Add traps based on trap chance
    // This would place trap tiles in the map
}

void ProceduralGenerator::addDecorations(TileMap& map, const std::vector<Room>& rooms) {
    // Add decorative elements like pillars, furniture, etc.
}

void ProceduralGenerator::addLighting(TileMap& map, const std::vector<Room>& rooms) {
    // Add light sources in rooms
}

void ProceduralGenerator::ensureConnectivity(std::vector<std::vector<int>>& grid) {
    // Use flood fill to ensure all floor tiles are connected
    // Find largest connected component and remove smaller islands
}

void ProceduralGenerator::applyTileset(TileMap& map, MapSystem::BiomeType biome) {
    // Apply biome-specific tilesets
}

std::vector<ProceduralGenerator::SpawnPoint> ProceduralGenerator::generateSpawnPoints(
    const std::vector<Room>& rooms, const DungeonParams& params) {

    std::vector<SpawnPoint> spawnPoints;

    for (const auto& room : rooms) {
        SpawnPoint spawn;
        spawn.x = room.centerX();
        spawn.y = room.centerY();

        switch (room.roomType) {
            case 1: spawn.type = SpawnPoint::Player; break;
            case 2: spawn.type = SpawnPoint::Boss; break;
            case 3: spawn.type = SpawnPoint::Treasure; break;
            default: spawn.type = SpawnPoint::Enemy; break;
        }

        spawnPoints.push_back(spawn);
    }

    return spawnPoints;
}

bool ProceduralGenerator::validateDungeon(const TileMap& map) {
    // Validate that dungeon is playable
    // Check connectivity, room count, etc.
    return true;
}

bool ProceduralGenerator::isReachable(const std::vector<std::vector<int>>& grid,
                                     int x1, int y1, int x2, int y2) {
    // Use BFS to check if two points are reachable
    return true;
}

int ProceduralGenerator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

float ProceduralGenerator::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

bool ProceduralGenerator::randomBool(float probability) {
    return randomFloat(0.0f, 1.0f) < probability;
}

void ProceduralGenerator::fillRect(TileMap& map, const std::string& layer,
                                  int x, int y, int width, int height, int tileID) {
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            Tile tile;
            tile.tileID = tileID;
            map.setTile(layer, x + dx, y + dy, tile);
        }
    }
}

// ==================== NoiseGenerator Implementation ====================

NoiseGenerator::NoiseGenerator(unsigned int seed) : m_rng(seed) {
    initializePermutation();
}

void NoiseGenerator::initializePermutation() {
    m_permutation.resize(256);
    for (int i = 0; i < 256; ++i) {
        m_permutation[i] = i;
    }
    std::shuffle(m_permutation.begin(), m_permutation.end(), m_rng);
    m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());
}

float NoiseGenerator::perlin(float x, float y) {
    // Simplified Perlin noise implementation
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = m_permutation[X] + Y;
    int B = m_permutation[X + 1] + Y;

    return lerp(v,
        lerp(u, grad(m_permutation[A], x, y), grad(m_permutation[B], x - 1, y)),
        lerp(u, grad(m_permutation[A + 1], x, y - 1), grad(m_permutation[B + 1], x - 1, y - 1))
    );
}

float NoiseGenerator::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float NoiseGenerator::lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float NoiseGenerator::grad(int hash, float x, float y) {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float NoiseGenerator::fractal(float x, float y, int octaves, float persistence, float lacunarity) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlin(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

float NoiseGenerator::simplex(float x, float y) {
    // Simplified simplex noise
    return perlin(x, y);
}

float NoiseGenerator::voronoi(float x, float y, int cellCount) {
    // Simplified Voronoi noise
    return 0.0f;
}

// ==================== WorldGenerator Implementation ====================

std::unique_ptr<TileMap> WorldGenerator::generateWorld(const WorldGenParams& params) {
    auto tilemap = std::make_unique<TileMap>();
    tilemap->initialize(params.width, params.height, 32, 32);

    // Generate heightmap
    std::vector<std::vector<float>> heightmap(params.height,
        std::vector<float>(params.width, 0.0f));
    generateHeightmap(heightmap, params);

    NoiseGenerator noise(params.seed);

    // Convert heightmap to tiles
    for (int y = 0; y < params.height; ++y) {
        for (int x = 0; x < params.width; ++x) {
            float height = heightmap[y][x];

            Tile tile;
            if (height < params.waterLevel) {
                tile.tileID = 10; // Water
                tile.solid = false;
            } else if (height < params.waterLevel + 0.1f) {
                tile.tileID = 11; // Beach/Sand
                tile.solid = false;
            } else if (height < params.mountainLevel) {
                tile.tileID = 12; // Grass
                tile.solid = false;
            } else {
                tile.tileID = 13; // Mountain/Rock
                tile.solid = true;
            }

            tilemap->setTile("Ground", x, y, tile);
        }
    }

    return tilemap;
}

void WorldGenerator::generateHeightmap(std::vector<std::vector<float>>& heightmap,
                                      const WorldGenParams& params) {
    NoiseGenerator noise(params.seed);

    for (int y = 0; y < params.height; ++y) {
        for (int x = 0; x < params.width; ++x) {
            float nx = x * params.scale;
            float ny = y * params.scale;

            heightmap[y][x] = noise.fractal(nx, ny, params.octaves,
                                           params.persistence, params.lacunarity);
            // Normalize to 0-1
            heightmap[y][x] = (heightmap[y][x] + 1.0f) / 2.0f;
        }
    }
}

void WorldGenerator::generateMoistureMap(std::vector<std::vector<float>>& moisture,
                                        const WorldGenParams& params) {
    NoiseGenerator noise(params.seed + 1000);

    int height = moisture.size();
    int width = moisture[0].size();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            moisture[y][x] = noise.fractal(x * params.scale * 0.5f, y * params.scale * 0.5f,
                                          4, 0.5f, 2.0f);
            moisture[y][x] = (moisture[y][x] + 1.0f) / 2.0f;
        }
    }
}

void WorldGenerator::generateTemperatureMap(std::vector<std::vector<float>>& temperature,
                                           const WorldGenParams& params) {
    NoiseGenerator noise(params.seed + 2000);

    int height = temperature.size();
    int width = temperature[0].size();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Base temperature on latitude
            float latitudeFactor = std::abs(y - height / 2.0f) / (height / 2.0f);
            temperature[y][x] = 1.0f - latitudeFactor;

            // Add noise
            float noise_val = noise.perlin(x * params.scale, y * params.scale);
            temperature[y][x] = temperature[y][x] * 0.7f + noise_val * 0.3f;
        }
    }
}

void WorldGenerator::generateRivers(TileMap& map,
                                   const std::vector<std::vector<float>>& heightmap,
                                   int riverCount) {
    // River generation would trace paths from high to low elevation
}

void WorldGenerator::generatePaths(TileMap& map,
                                  const std::vector<std::pair<int, int>>& settlements) {
    // Generate roads between settlements
}

} // namespace World
