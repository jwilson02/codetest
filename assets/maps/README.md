# Map Creation Guide

## Table of Contents
1. [Introduction](#introduction)
2. [System Overview](#system-overview)
3. [Creating Maps](#creating-maps)
4. [Tilemap System](#tilemap-system)
5. [Procedural Generation](#procedural-generation)
6. [Collision System](#collision-system)
7. [Triggers and Interactive Zones](#triggers-and-interactive-zones)
8. [World Regions](#world-regions)
9. [Biomes](#biomes)
10. [Level Streaming](#level-streaming)
11. [TMX Format Support](#tmx-format-support)
12. [Best Practices](#best-practices)
13. [Examples](#examples)

---

## Introduction

This guide covers the comprehensive level and map system built for the ARPG game engine. The system supports:
- Multi-layered tilemaps with culling optimization
- Procedural dungeon generation with multiple algorithms
- Spatial partitioning collision detection
- Trigger zones and interactive objects
- 14+ unique biomes with environmental effects
- Level streaming for large open worlds
- TMX format support for Tiled Map Editor integration
- Destructible objects and environmental hazards

---

## System Overview

### Core Components

**TileMap** (`src/World/TileMap.h/cpp`)
- Multi-layer tile rendering with parallax effects
- Tile animations
- Destructible tiles with health system
- Culling optimization for performance

**MapSystem** (`src/World/MapSystem.h/cpp`)
- Map loading and management
- TMX format parser
- Level streaming
- Minimap generation
- Warp/teleportation system
- Biome management (14 different biomes)

**ProceduralGen** (`src/World/ProceduralGen.h/cpp`)
- BSP tree dungeon generation
- Cellular automata cave generation
- Maze generation
- Random walk algorithms
- Noise-based world generation
- Dungeon templates system

**Collision** (`src/World/Collision.h/cpp`)
- Spatial hash grid (broad phase)
- QuadTree (alternative broad phase)
- Multiple collision shapes (AABB, Circle, Polygon)
- Raycasting
- Tile collision detection

**WorldRegion** (`src/World/WorldRegion.h/cpp`)
- Region-based world organization
- Biome assignment
- Enemy spawn rules
- Resource nodes
- Points of interest

**Trigger** (`src/World/Trigger.h/cpp`)
- Multiple trigger types (Enter, Exit, Stay, Timed, etc.)
- Condition system
- Action system
- Interactive objects

---

## Creating Maps

### Manual Map Creation

#### 1. Using Tiled Map Editor

The system supports the industry-standard Tiled Map Editor (TMX format):

```bash
# Install Tiled from https://www.mapeditor.org/
# Create a new map with these settings:
# - Tile size: 32x32 pixels
# - Map size: Variable (e.g., 100x100 tiles)
# - Orientation: Orthogonal
```

**Basic TMX Structure:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<map version="1.0" orientation="orthogonal" width="100" height="100" tilewidth="32" tileheight="32">
  <tileset firstgid="1" name="dungeon" tilewidth="32" tileheight="32">
    <image source="tilesets/dungeon.png" width="512" height="512"/>
  </tileset>

  <layer name="Ground" width="100" height="100">
    <data encoding="csv">
      1,1,1,1,1,1...
    </data>
  </layer>

  <layer name="Walls" width="100" height="100">
    <data encoding="csv">
      0,0,0,2,2,2...
    </data>
  </layer>

  <objectgroup name="Triggers">
    <object id="1" name="entrance_trigger" type="warp" x="160" y="160" width="64" height="64">
      <properties>
        <property name="targetMap" value="overworld"/>
        <property name="targetX" value="500"/>
        <property name="targetY" value="500"/>
      </properties>
    </object>
  </objectgroup>
</map>
```

#### 2. Loading TMX Maps

```cpp
#include "World/MapSystem.h"

// Initialize map system
World::MapSystem mapSystem;
mapSystem.initialize();

// Load TMX map
if (mapSystem.loadTMXMap("assets/maps/dungeon_01.tmx")) {
    std::cout << "Map loaded successfully!" << std::endl;
}

// Get current map
World::TileMap* currentMap = mapSystem.getCurrentMap();
```

### Programmatic Map Creation

#### Creating a Simple Map

```cpp
#include "World/TileMap.h"

// Create tilemap
World::TileMap tilemap;
tilemap.initialize(50, 50, 32, 32); // 50x50 tiles, 32x32 pixels each

// Add tileset
World::Tileset tileset;
tileset.name = "dungeon";
tileset.texturePath = "assets/tilesets/dungeon.png";
tileset.tileWidth = 32;
tileset.tileHeight = 32;
tileset.columns = 16;
tileset.tileCount = 256;
int tilesetID = tilemap.addTileset(tileset);

// Add layers
tilemap.addLayer("Background", 0);
tilemap.addLayer("Ground", 1);
tilemap.addLayer("Walls", 2);
tilemap.addLayer("Objects", 3);

// Place tiles
for (int y = 0; y < 50; ++y) {
    for (int x = 0; x < 50; ++x) {
        World::Tile tile;

        // Ground layer
        tile.tileID = 2; // Floor tile
        tile.solid = false;
        tilemap.setTile("Ground", x, y, tile);

        // Create walls on edges
        if (x == 0 || y == 0 || x == 49 || y == 49) {
            tile.tileID = 1; // Wall tile
            tile.solid = true;
            tilemap.setTile("Walls", x, y, tile);
        }
    }
}

// Save to file
tilemap.saveToFile("assets/maps/custom_map.json");
```

---

## Tilemap System

### Layer Management

Tilemaps support multiple layers for organizing visual elements:

```cpp
// Create layers with different purposes
tilemap.addLayer("Background", 0);    // Decorative background
tilemap.addLayer("Ground", 1);        // Main walkable floor
tilemap.addLayer("Walls", 2);         // Collision walls
tilemap.addLayer("Foreground", 3);    // Objects in front of player
tilemap.addLayer("Decorations", 4);   // Additional decorations

// Layer properties
auto* layer = tilemap.getLayer("Ground");
layer->setOpacity(0.8f);
layer->setVisible(true);
layer->setParallaxFactor(1.0f, 1.0f); // Parallax scrolling

// Reorder layers
std::vector<std::string> newOrder = {"Background", "Ground", "Walls", "Foreground"};
tilemap.setLayerOrder(newOrder);
```

### Destructible Tiles

```cpp
// Create destructible wall
World::Tile destructibleWall;
destructibleWall.tileID = 5;
destructibleWall.solid = true;
destructibleWall.destructible = true;
destructibleWall.health = 100;
tilemap.setTile("Walls", 10, 10, destructibleWall);

// Damage tile
bool destroyed = tilemap.damageTile(10, 10, 50); // Deal 50 damage
if (destroyed) {
    std::cout << "Wall destroyed!" << std::endl;
}
```

### Tile Animations

```cpp
World::TileAnimationManager animManager;

// Create water animation
World::TileAnimation waterAnim(10); // Base tile ID
waterAnim.addFrame(10, 0.2f); // Frame 1: 0.2 seconds
waterAnim.addFrame(11, 0.2f); // Frame 2: 0.2 seconds
waterAnim.addFrame(12, 0.2f); // Frame 3: 0.2 seconds
waterAnim.addFrame(13, 0.2f); // Frame 4: 0.2 seconds
animManager.registerAnimation(10, waterAnim);

// Update animations
animManager.update(deltaTime);

// Get current animated tile
int currentTile = animManager.getAnimatedTileID(10);
```

### Rendering with Culling

```cpp
// Render only visible tiles
int cameraX = 0, cameraY = 0;
int screenWidth = 1920, screenHeight = 1080;

tilemap.render(cameraX, cameraY, screenWidth, screenHeight);

// Calculate view bounds for custom culling
auto bounds = tilemap.calculateViewBounds(cameraX, cameraY, screenWidth, screenHeight);
std::cout << "Visible tiles: "
          << bounds.minTileX << "," << bounds.minTileY
          << " to "
          << bounds.maxTileX << "," << bounds.maxTileY << std::endl;
```

---

## Procedural Generation

### Dungeon Generation Algorithms

#### 1. BSP (Binary Space Partitioning)

Creates structured dungeons with rectangular rooms:

```cpp
#include "World/ProceduralGen.h"

World::ProceduralGenerator generator;
generator.setAlgorithm(World::ProceduralGenerator::Algorithm::BSP);

// Configure parameters
World::DungeonParams params;
params.width = 80;
params.height = 80;
params.minRooms = 5;
params.maxRooms = 12;
params.minRoomSize = 5;
params.maxRoomSize = 10;
params.corridorWidth = 3;
params.treasureRoomChance = 0.2f;
params.bossRoomChance = 0.8f;
params.trapChance = 0.1f;
params.seed = 12345; // Fixed seed for reproducibility

// Generate dungeon
auto dungeon = generator.generateDungeon(params);

// Apply tileset based on biome
generator.applyTileset(*dungeon, World::MapSystem::BiomeType::Cave);
```

#### 2. Cellular Automata (Caves)

Creates organic, natural-looking caves:

```cpp
generator.setAlgorithm(World::ProceduralGenerator::Algorithm::CellularAutomata);

// Generate cave
auto cave = generator.generateCave(
    100,      // width
    100,      // height
    0.45f,    // fill probability (higher = more walls)
    42        // seed
);

// Smooth the cave
std::vector<std::vector<int>> grid; // Your grid data
generator.smoothCave(grid, 5); // 5 smoothing iterations
```

#### 3. Maze Generation

Creates maze-like dungeons:

```cpp
auto maze = generator.generateMaze(60, 60, 999);
```

#### 4. Arena Generation

Creates combat arenas:

```cpp
auto arena = generator.generateArena(
    40,          // width
    40,          // height
    "colosseum"  // theme
);
```

### Using Dungeon Templates

Templates allow you to define preset dungeon configurations:

```cpp
// Load template from JSON
World::DungeonTemplate::TemplateData tmpl;
if (World::DungeonTemplate::loadTemplate("data/maps/dungeons.json", tmpl)) {
    auto dungeon = World::DungeonTemplate::generateFromTemplate(tmpl);
}
```

**Example Template** (from `data/maps/dungeons.json`):
- forest_dungeon: Easy starter dungeon (Level 1-5)
- desert_dungeon: Medium difficulty pyramid (Level 5-12)
- ice_dungeon: Challenging frozen caves (Level 10-18)
- volcanic_dungeon: Hard lava caverns (Level 18-28)
- corruption_dungeon: Endgame content (Level 30-45)

### Spawn Points

```cpp
// Generate spawn points for rooms
std::vector<World::Room> rooms = generator.generateRooms(params);
auto spawnPoints = generator.generateSpawnPoints(rooms, params);

for (const auto& spawn : spawnPoints) {
    switch (spawn.type) {
        case World::ProceduralGenerator::SpawnPoint::Player:
            // Spawn player at this location
            break;
        case World::ProceduralGenerator::SpawnPoint::Enemy:
            // Spawn enemy
            break;
        case World::ProceduralGenerator::SpawnPoint::Boss:
            // Spawn boss
            break;
        case World::ProceduralGenerator::SpawnPoint::Treasure:
            // Place treasure chest
            break;
    }
}
```

### World Generation

Generate large open worlds with biomes:

```cpp
World::WorldGenerator::WorldGenParams worldParams;
worldParams.width = 200;
worldParams.height = 200;
worldParams.seed = 54321;
worldParams.scale = 0.05f;
worldParams.octaves = 6;
worldParams.persistence = 0.5f;
worldParams.lacunarity = 2.0f;
worldParams.waterLevel = 0.3f;
worldParams.mountainLevel = 0.7f;

auto world = World::WorldGenerator::generateWorld(worldParams);

// Add rivers
std::vector<std::vector<float>> heightmap;
World::WorldGenerator::generateHeightmap(heightmap, worldParams);
World::WorldGenerator::generateRivers(*world, heightmap, 5); // 5 rivers
```

---

## Collision System

### Setting Up Collision

```cpp
#include "World/Collision.h"

// Initialize collision system
World::CollisionSystem collisionSystem;
collisionSystem.initialize(10000.0f, 10000.0f); // World size

// Choose broad phase algorithm
collisionSystem.setBroadPhaseAlgorithm(
    World::CollisionSystem::BroadPhase::SpatialGrid
);
collisionSystem.setCellSize(128.0f);

// Set tilemap for tile collision
collisionSystem.setTileMap(tilemap.get());
```

### Creating Collision Objects

```cpp
// Create rectangle collision
auto* player = collisionSystem.createObject(World::CollisionShape::Rectangle);
player->setRectangle(100.0f, 100.0f, 32.0f, 48.0f); // x, y, width, height
player->setLayer(World::CollisionLayers::LAYER_PLAYER);
player->setCollisionMask(
    World::CollisionLayers::LAYER_WORLD |
    World::CollisionLayers::LAYER_ENEMY
);

// Create circle collision
auto* enemy = collisionSystem.createObject(World::CollisionShape::Circle);
enemy->setCircle(200.0f, 200.0f, 16.0f); // x, y, radius
enemy->setLayer(World::CollisionLayers::LAYER_ENEMY);

// Create polygon collision
std::vector<float> vertices = {
    0, 0,    // point 1
    32, 0,   // point 2
    32, 32,  // point 3
    0, 32    // point 4
};
auto* custom = collisionSystem.createObject(World::CollisionShape::Polygon);
custom->setPolygon(vertices, 150.0f, 150.0f);
```

### Collision Detection

```cpp
// Check collisions for an object
auto collisions = collisionSystem.checkCollisions(player);
for (const auto& result : collisions) {
    std::cout << "Collision detected!" << std::endl;
    std::cout << "Penetration: " << result.penetrationX << ", "
              << result.penetrationY << std::endl;
    std::cout << "Normal: " << result.normalX << ", "
              << result.normalY << std::endl;

    if (result.isTrigger) {
        // Handle trigger
    } else {
        // Resolve collision
        collisionSystem.resolveCollision(player, result);
    }
}

// Check tile collision
bool hitTile = collisionSystem.checkTileCollision(
    playerX, playerY, playerWidth, playerHeight
);
```

### Raycasting

```cpp
// Cast a ray
World::CollisionSystem::RaycastResult result;
bool hit = collisionSystem.raycast(
    startX, startY,  // Ray start
    endX, endY,      // Ray end
    result
);

if (hit) {
    std::cout << "Ray hit at: " << result.hitX << ", " << result.hitY << std::endl;
    std::cout << "Distance: " << result.distance << std::endl;
    std::cout << "Object: " << result.object << std::endl;
}

// Get all hits along ray
auto allHits = collisionSystem.raycastAll(startX, startY, endX, endY);
```

### Collision Callbacks

```cpp
collisionSystem.setCollisionCallback([](World::CollisionObject* a, World::CollisionObject* b) {
    std::cout << "Collision between objects!" << std::endl;
});

collisionSystem.setTriggerCallback([](World::CollisionObject* a, World::CollisionObject* b) {
    std::cout << "Trigger activated!" << std::endl;
});
```

---

## Triggers and Interactive Zones

### Creating Triggers

```cpp
#include "World/Trigger.h"

// Initialize trigger manager
World::TriggerManager triggerManager;
triggerManager.initialize(&collisionSystem);

// Create enter trigger
auto* enterTrigger = triggerManager.createEnterTrigger("door_trigger");
enterTrigger->setPosition(100.0f, 100.0f);
enterTrigger->setSize(64.0f, 64.0f);
enterTrigger->setEnabled(true);

// Add action
World::TriggerAction action;
action.type = World::TriggerAction::LoadMap;
action.parameter = "next_area";
enterTrigger->addAction(action);

// Add condition
World::TriggerCondition condition;
condition.type = World::TriggerCondition::HasItem;
condition.parameter = "golden_key";
enterTrigger->addCondition(condition);

// Set callback
enterTrigger->setOnEnter([](World::Trigger* trigger) {
    std::cout << "Player entered trigger zone!" << std::endl;
});
```

### Trigger Types

#### Enter Trigger
```cpp
auto* trigger = triggerManager.createEnterTrigger("entrance");
trigger->setOneShot(true); // Trigger only once
```

#### Exit Trigger
```cpp
auto* exitTrigger = triggerManager.createExitTrigger("exit_zone");
```

#### Timed Trigger
```cpp
auto* timedTrigger = triggerManager.createTimedTrigger("countdown");
auto* timed = static_cast<World::TimedTrigger*>(timedTrigger);
timed->setDelay(5.0f); // Activate after 5 seconds
```

#### Proximity Trigger
```cpp
auto* proximityTrigger = triggerManager.createProximityTrigger("near_chest");
auto* prox = static_cast<World::ProximityTrigger*>(proximityTrigger);
prox->setTarget(200.0f, 200.0f); // Target position
prox->setCheckRadius(100.0f);
```

#### Script Trigger
```cpp
auto* scriptTrigger = triggerManager.createScriptTrigger("custom_event");
auto* script = static_cast<World::ScriptTrigger*>(scriptTrigger);
script->setScript("onPlayerEnter()");
```

### Interactive Objects

```cpp
World::InteractiveObject chest("treasure_chest", 150.0f, 150.0f);
chest.setSprite("chest_closed");
chest.setInteractionPrompt("Press E to open");
chest.setInteractionDistance(50.0f);

chest.setOnInteract([](World::InteractiveObject* obj, void* player) {
    obj->setSprite("chest_open");
    // Give player loot
    std::cout << "Chest opened!" << std::endl;
});

// Update (in game loop)
triggerManager.update(deltaTime);
chest.update(deltaTime);
```

### Trigger Actions

Available action types:
- **LoadMap**: Load a new map
- **Teleport**: Move player to location
- **SpawnEnemy**: Spawn enemy
- **SpawnItem**: Spawn item
- **PlaySound**: Play sound effect
- **PlayMusic**: Change music track
- **ShowMessage**: Display message
- **StartQuest**: Begin quest
- **CompleteQuest**: Finish quest
- **OpenDoor**: Open door
- **ChangeWeather**: Change weather
- **GiveItem**: Give item to player
- **Script**: Execute custom script
- **Cutscene**: Start cutscene

---

## World Regions

### Creating Regions

```cpp
#include "World/WorldRegion.h"

World::RegionManager regionManager;
regionManager.setWorldSize(10000, 10000);

// Create region
World::RegionBounds bounds(1000, 1000, 2000, 2000);
auto region = std::make_unique<World::WorldRegion>("Forest of Whispers", bounds);

// Configure region
region->setBiome(World::MapSystem::BiomeType::Forest);
region->setLevel(5);
region->setDifficulty(1.5f);
region->setMusic("mysterious_forest");
region->setAmbientSound("forest_ambience");
region->setWeather(World::WorldRegion::Weather::Rain);

// Add spawn rules
World::WorldRegion::SpawnRule spawnRule;
spawnRule.enemyType = "wolf";
spawnRule.spawnChance = 0.3f;
spawnRule.minLevel = 4;
spawnRule.maxLevel = 7;
spawnRule.minCount = 2;
spawnRule.maxCount = 4;
region->addSpawnRule(spawnRule);

// Add point of interest
World::WorldRegion::PointOfInterest poi;
poi.name = "Ancient Tree";
poi.type = "landmark";
poi.x = 2000;
poi.y = 2000;
poi.description = "A massive ancient oak tree";
poi.revealed = false;
region->addPOI(poi);

// Add resource nodes
World::WorldRegion::ResourceNode resource;
resource.type = "tree";
resource.x = 1500;
resource.y = 1500;
resource.quantity = 100;
resource.respawnTime = 300.0f; // 5 minutes
resource.depleted = false;
region->addResourceNode(resource);

// Add to manager
regionManager.addRegion(std::move(region));
```

### Sub-regions

```cpp
// Create main region
auto mainRegion = std::make_unique<World::WorldRegion>(
    "Northern Wilderness",
    World::RegionBounds(0, 0, 5000, 5000)
);

// Add sub-regions
auto subRegion1 = std::make_unique<World::WorldRegion>(
    "Dark Forest",
    World::RegionBounds(1000, 1000, 1000, 1000)
);
mainRegion->addSubRegion(std::move(subRegion1));

auto subRegion2 = std::make_unique<World::WorldRegion>(
    "Mountain Pass",
    World::RegionBounds(3000, 500, 800, 800)
);
mainRegion->addSubRegion(std::move(subRegion2));

regionManager.addRegion(std::move(mainRegion));
```

### Using Regions

```cpp
// Update based on player position
regionManager.update(deltaTime, playerX, playerY);

// Get active region
auto* activeRegion = regionManager.getActiveRegion();
if (activeRegion) {
    std::cout << "Current region: " << activeRegion->getName() << std::endl;
    std::cout << "Biome: " << (int)activeRegion->getBiome() << std::endl;
    std::cout << "Level: " << activeRegion->getLevel() << std::endl;
}

// Query regions
std::vector<World::WorldRegion*> regions = regionManager.getRegionsInArea(
    World::RegionBounds(2000, 2000, 1000, 1000)
);

// Get neighbors
auto neighbors = regionManager.getNeighboringRegions("Forest of Whispers");
```

---

## Biomes

The system includes 14 pre-configured biomes:

### Available Biomes

1. **Forest** - Lush woodlands
2. **Desert** - Scorching sand dunes
3. **Snow** - Frozen tundra
4. **Volcanic** - Lava and ash
5. **Swamp** - Murky wetlands
6. **Cave** - Underground caverns
7. **Beach** - Coastal areas
8. **Mountains** - Towering peaks
9. **Plains** - Open grasslands
10. **Jungle** - Dense rainforest
11. **Tundra** - Frozen wasteland
12. **Corruption** - Twisted dark lands
13. **Crystal** - Magical crystals
14. **Ruins** - Ancient structures

### Biome Configuration

Each biome in `data/maps/world.json` includes:
- Visual color tint
- Hazard chance
- Ambient sound
- Music track
- Spawn rules
- Resource types
- Environmental hazards

```cpp
// Set current biome
mapSystem.setCurrentBiome(World::MapSystem::BiomeType::Volcanic);

// Get biome data
const auto* biomeData = mapSystem.getBiome(World::MapSystem::BiomeType::Volcanic);
if (biomeData) {
    std::cout << "Biome: " << biomeData->name << std::endl;
    std::cout << "Hazard chance: " << biomeData->hazardChance << std::endl;
    std::cout << "Ambient sound: " << biomeData->ambientSound << std::endl;
}
```

### Environmental Hazards

```cpp
// Add hazard to map
World::MapSystem::EnvironmentalHazard hazard;
hazard.type = World::MapSystem::EnvironmentalHazard::Fire;
hazard.x = 100;
hazard.y = 100;
hazard.width = 64;
hazard.height = 64;
hazard.damage = 5.0f;
hazard.tickRate = 1.0f; // Damage per second
hazard.active = true;

mapSystem.addHazard(hazard);

// Check hazards at player position
auto hazards = mapSystem.getHazardsAt(playerX, playerY);
for (auto* h : hazards) {
    // Apply damage to player
}

// Update hazards
mapSystem.updateHazards(deltaTime);
```

---

## Level Streaming

For large open worlds, level streaming loads/unloads map chunks based on player position:

```cpp
// Enable streaming
mapSystem.enableStreaming(true);
mapSystem.setStreamingDistance(2000.0f); // Load chunks within 2000 pixels
mapSystem.setChunkSize(64); // 64x64 tiles per chunk

// Update streaming (call every frame)
mapSystem.updateStreaming(playerX, playerY);

// Manual chunk control
mapSystem.loadChunk(0, 0);   // Load chunk at grid position (0, 0)
mapSystem.unloadChunk(5, 5); // Unload chunk at (5, 5)

// Statistics
auto stats = mapSystem.getStats();
std::cout << "Maps loaded: " << stats.mapsLoaded << std::endl;
std::cout << "Chunks loaded: " << stats.chunksLoaded << std::endl;
std::cout << "Memory usage: " << stats.memoryUsageMB << " MB" << std::endl;
```

---

## TMX Format Support

### Supported TMX Features

- Tile layers with CSV or Base64 encoding
- Multiple tilesets
- Object groups (for triggers, spawn points)
- Custom properties
- Layer opacity and visibility
- Tile animations
- Image layers

### Converting TMX to JSON

```cpp
bool success = World::MapLoader::convertTMXToJSON(
    "assets/maps/dungeon.tmx",
    "assets/maps/dungeon.json"
);
```

---

## Best Practices

### Performance Optimization

1. **Use layer culling**: Only visible layers are rendered
2. **Spatial partitioning**: Choose appropriate broad phase algorithm
   - SpatialGrid: Best for many moving objects
   - QuadTree: Best for mostly static objects
   - BruteForce: Only for small maps (<100 objects)
3. **Chunk size**: Optimize based on map size
   - Small maps: 32x32 tiles
   - Medium maps: 64x64 tiles
   - Large maps: 128x128 tiles
4. **Limit collision checks**: Use collision layers and masks

### Map Design

1. **Layer organization**:
   - Layer 0: Background decorations
   - Layer 1: Ground/floor
   - Layer 2: Walls/obstacles
   - Layer 3: Foreground objects
   - Layer 4+: Effects, lighting

2. **Collision setup**:
   - Use simple shapes where possible
   - Combine small objects into larger collision boxes
   - Mark static objects as static

3. **Procedural generation**:
   - Test with fixed seeds for reproducibility
   - Adjust parameters incrementally
   - Validate generated dungeons

4. **Triggers**:
   - Use one-shot triggers for cutscenes
   - Add cooldowns to prevent spam
   - Combine conditions for complex behavior

### Memory Management

```cpp
// Unload maps when not needed
mapSystem.unloadMap("old_map");

// Clear unused data
tilemap.clear();
collisionSystem.shutdown();
triggerManager.shutdown();
```

---

## Examples

### Complete Dungeon Example

```cpp
#include "World/MapSystem.h"
#include "World/ProceduralGen.h"
#include "World/Collision.h"
#include "World/Trigger.h"

// Initialize systems
World::MapSystem mapSystem;
mapSystem.initialize();

World::CollisionSystem collisionSystem;
collisionSystem.initialize(5000, 5000);

World::TriggerManager triggerManager;
triggerManager.initialize(&collisionSystem);

// Generate dungeon
World::ProceduralGenerator generator;
World::DungeonParams params;
params.width = 80;
params.height = 80;
params.minRooms = 8;
params.maxRooms = 15;
params.seed = 42;

auto dungeon = generator.generateDungeon(params);

// Add to map system
// (You would need to convert the unique_ptr to your map system)

// Setup collision
collisionSystem.setTileMap(dungeon.get());

// Add entrance trigger
auto* entrance = triggerManager.createEnterTrigger("entrance");
entrance->setPosition(40 * 32, 40 * 32); // Center of map
entrance->setSize(64, 64);

World::TriggerAction action;
action.type = World::TriggerAction::ShowMessage;
action.parameter = "Welcome to the dungeon!";
entrance->addAction(action);

// Game loop
float deltaTime = 0.016f; // 60 FPS
while (running) {
    // Update
    collisionSystem.update(deltaTime);
    triggerManager.update(deltaTime);
    mapSystem.update(deltaTime);

    // Render
    mapSystem.render(cameraX, cameraY, screenWidth, screenHeight);
    collisionSystem.renderDebug();
    triggerManager.renderDebug();
}
```

### Warp Point Example

```cpp
// Create warp point
World::MapSystem::WarpPoint warp;
warp.name = "town_to_dungeon";
warp.targetMap = "dungeon_01";
warp.x = 500.0f;
warp.y = 500.0f;
warp.targetX = 100.0f;
warp.targetY = 100.0f;
warp.enabled = true;

mapSystem.registerWarpPoint(warp);

// Activate warp
if (mapSystem.activateWarp("town_to_dungeon", playerX, playerY)) {
    std::cout << "Warped to dungeon!" << std::endl;
}

// Find nearby warps
auto nearbyWarps = mapSystem.getWarpsInRange(playerX, playerY, 100.0f);
```

### Minimap Generation

```cpp
// Generate minimap
auto minimapData = mapSystem.generateMinimapForCurrentMap(256, 256);

std::cout << "Minimap size: " << minimapData.width << "x"
          << minimapData.height << std::endl;
std::cout << "Scale: " << minimapData.scale << std::endl;

// minimapData.pixels contains RGBA pixel data
// You can upload this to a texture for rendering
```

---

## Additional Resources

- **Tiled Map Editor**: https://www.mapeditor.org/
- **World JSON**: `/data/maps/world.json` - Complete world configuration
- **Dungeon Templates**: `/data/maps/dungeons.json` - Procedural templates
- **Source Code**: `/src/World/` - Full implementation

---

## Troubleshooting

### Map won't load
- Check file path is correct
- Verify TMX format version is supported
- Ensure tileset images exist

### Poor performance
- Enable level streaming
- Reduce visible layers
- Use appropriate broad phase algorithm
- Optimize collision layer masks

### Collision not working
- Verify collision layers and masks
- Check if objects are enabled
- Ensure collision object is updated after movement
- Confirm tilemap is set in collision system

### Triggers not firing
- Check trigger is enabled
- Verify conditions are met
- Register entities with trigger manager
- Update trigger manager each frame

---

## Version History

- **1.0** - Initial release with full feature set
  - Multi-layer tilemaps
  - Procedural generation
  - Collision system
  - Triggers and regions
  - 14 biomes
  - Level streaming
  - TMX support

---

For questions or issues, please refer to the source code documentation or contact the development team.
