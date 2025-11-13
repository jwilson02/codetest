#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace World {

// Tile structure representing a single tile in the map
struct Tile {
    int tileID;                  // Tileset index
    int x, y;                    // Position in tile coordinates
    int layer;                   // Layer index (0 = background, higher = foreground)
    bool solid;                  // Collision flag
    bool destructible;           // Can be destroyed
    int health;                  // Health for destructible tiles
    std::string properties;      // Custom properties (JSON string)

    Tile() : tileID(0), x(0), y(0), layer(0), solid(false),
             destructible(false), health(100), properties("{}") {}
};

// Tileset data
struct Tileset {
    std::string name;
    std::string texturePath;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int columns;
    int tileCount;
    std::unordered_map<int, std::string> tileProperties; // Per-tile properties

    Tileset() : tileWidth(32), tileHeight(32), spacing(0),
                margin(0), columns(0), tileCount(0) {}
};

// Layer for organizing tiles
class TileLayer {
public:
    TileLayer(const std::string& name, int width, int height, int layerIndex);
    ~TileLayer();

    // Layer management
    void setTile(int x, int y, const Tile& tile);
    Tile* getTile(int x, int y);
    const Tile* getTile(int x, int y) const;
    void removeTile(int x, int y);

    // Layer properties
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    void setOpacity(float opacity) { m_opacity = opacity; }
    float getOpacity() const { return m_opacity; }
    void setParallaxFactor(float x, float y) { m_parallaxX = x; m_parallaxY = y; }

    // Getters
    const std::string& getName() const { return m_name; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getLayerIndex() const { return m_layerIndex; }
    const std::vector<Tile>& getTiles() const { return m_tiles; }

    // Culling
    std::vector<const Tile*> getVisibleTiles(int viewX, int viewY,
                                             int viewWidth, int viewHeight) const;

private:
    std::string m_name;
    int m_width;
    int m_height;
    int m_layerIndex;
    bool m_visible;
    float m_opacity;
    float m_parallaxX;
    float m_parallaxY;
    std::vector<Tile> m_tiles;

    int getTileIndex(int x, int y) const;
};

// Main TileMap class
class TileMap {
public:
    TileMap();
    ~TileMap();

    // Initialization
    bool initialize(int width, int height, int tileWidth, int tileHeight);
    void clear();

    // Tileset management
    int addTileset(const Tileset& tileset);
    const Tileset* getTileset(int index) const;
    const std::vector<Tileset>& getTilesets() const { return m_tilesets; }

    // Layer management
    TileLayer* addLayer(const std::string& name, int layerIndex = -1);
    TileLayer* getLayer(const std::string& name);
    TileLayer* getLayer(int index);
    const TileLayer* getLayer(int index) const;
    int getLayerCount() const { return m_layers.size(); }
    void removeLayer(const std::string& name);
    void setLayerOrder(const std::vector<std::string>& layerNames);

    // Tile operations
    void setTile(const std::string& layerName, int x, int y, const Tile& tile);
    Tile* getTile(const std::string& layerName, int x, int y);

    // Collision queries
    bool isSolid(int x, int y) const;
    bool isDestructible(int x, int y) const;
    bool damageTile(int x, int y, int damage);

    // Map properties
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileWidth() const { return m_tileWidth; }
    int getTileHeight() const { return m_tileHeight; }
    int getPixelWidth() const { return m_width * m_tileWidth; }
    int getPixelHeight() const { return m_height * m_tileHeight; }

    // Rendering
    void render(int cameraX, int cameraY, int screenWidth, int screenHeight);
    void renderLayer(int layerIndex, int cameraX, int cameraY,
                    int screenWidth, int screenHeight);

    // Culling optimization
    struct ViewBounds {
        int minTileX, minTileY;
        int maxTileX, maxTileY;
    };
    ViewBounds calculateViewBounds(int cameraX, int cameraY,
                                  int screenWidth, int screenHeight) const;

    // Debug
    void enableDebugDraw(bool enable) { m_debugDraw = enable; }
    void renderDebug();

    // Serialization
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

private:
    int m_width;
    int m_height;
    int m_tileWidth;
    int m_tileHeight;

    std::vector<Tileset> m_tilesets;
    std::vector<std::unique_ptr<TileLayer>> m_layers;
    std::unordered_map<std::string, int> m_layerNameToIndex;

    bool m_debugDraw;

    // Helper methods
    void renderTile(const Tile& tile, int screenX, int screenY, float opacity);
    bool isInBounds(int x, int y) const;
};

// Tile animation support
class TileAnimation {
public:
    struct Frame {
        int tileID;
        float duration; // seconds
    };

    TileAnimation(int baseTileID);

    void addFrame(int tileID, float duration);
    int getCurrentTileID(float elapsedTime) const;
    void reset();

private:
    int m_baseTileID;
    std::vector<Frame> m_frames;
    mutable float m_currentTime;
    mutable int m_currentFrame;
};

// Tile animation manager
class TileAnimationManager {
public:
    void registerAnimation(int baseTileID, const TileAnimation& animation);
    const TileAnimation* getAnimation(int tileID) const;
    void update(float deltaTime);
    int getAnimatedTileID(int baseTileID) const;

private:
    std::unordered_map<int, TileAnimation> m_animations;
    float m_elapsedTime;
};

} // namespace World
