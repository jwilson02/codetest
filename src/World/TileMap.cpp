#include "TileMap.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace World {

// ==================== TileLayer Implementation ====================

TileLayer::TileLayer(const std::string& name, int width, int height, int layerIndex)
    : m_name(name)
    , m_width(width)
    , m_height(height)
    , m_layerIndex(layerIndex)
    , m_visible(true)
    , m_opacity(1.0f)
    , m_parallaxX(1.0f)
    , m_parallaxY(1.0f)
{
    m_tiles.resize(width * height);
}

TileLayer::~TileLayer() {
}

void TileLayer::setTile(int x, int y, const Tile& tile) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }

    int index = getTileIndex(x, y);
    m_tiles[index] = tile;
    m_tiles[index].x = x;
    m_tiles[index].y = y;
    m_tiles[index].layer = m_layerIndex;
}

Tile* TileLayer::getTile(int x, int y) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return nullptr;
    }

    int index = getTileIndex(x, y);
    return &m_tiles[index];
}

const Tile* TileLayer::getTile(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return nullptr;
    }

    int index = getTileIndex(x, y);
    return &m_tiles[index];
}

void TileLayer::removeTile(int x, int y) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }

    int index = getTileIndex(x, y);
    m_tiles[index] = Tile(); // Reset to default
}

std::vector<const Tile*> TileLayer::getVisibleTiles(int viewX, int viewY,
                                                     int viewWidth, int viewHeight) const {
    std::vector<const Tile*> visibleTiles;

    // Calculate tile bounds with parallax
    int startX = std::max(0, static_cast<int>(viewX * m_parallaxX) / 32);
    int startY = std::max(0, static_cast<int>(viewY * m_parallaxY) / 32);
    int endX = std::min(m_width, static_cast<int>((viewX + viewWidth) * m_parallaxX) / 32 + 1);
    int endY = std::min(m_height, static_cast<int>((viewY + viewHeight) * m_parallaxY) / 32 + 1);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile* tile = getTile(x, y);
            if (tile && tile->tileID > 0) {
                visibleTiles.push_back(tile);
            }
        }
    }

    return visibleTiles;
}

int TileLayer::getTileIndex(int x, int y) const {
    return y * m_width + x;
}

// ==================== TileMap Implementation ====================

TileMap::TileMap()
    : m_width(0)
    , m_height(0)
    , m_tileWidth(32)
    , m_tileHeight(32)
    , m_debugDraw(false)
{
}

TileMap::~TileMap() {
    clear();
}

bool TileMap::initialize(int width, int height, int tileWidth, int tileHeight) {
    m_width = width;
    m_height = height;
    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;

    // Create default background layer
    addLayer("Background", 0);

    return true;
}

void TileMap::clear() {
    m_layers.clear();
    m_layerNameToIndex.clear();
    m_tilesets.clear();
}

int TileMap::addTileset(const Tileset& tileset) {
    m_tilesets.push_back(tileset);
    return m_tilesets.size() - 1;
}

const Tileset* TileMap::getTileset(int index) const {
    if (index < 0 || index >= static_cast<int>(m_tilesets.size())) {
        return nullptr;
    }
    return &m_tilesets[index];
}

TileLayer* TileMap::addLayer(const std::string& name, int layerIndex) {
    // Auto-assign layer index if not specified
    if (layerIndex < 0) {
        layerIndex = m_layers.size();
    }

    // Create new layer
    auto layer = std::make_unique<TileLayer>(name, m_width, m_height, layerIndex);
    TileLayer* layerPtr = layer.get();

    // Store layer
    m_layerNameToIndex[name] = m_layers.size();
    m_layers.push_back(std::move(layer));

    return layerPtr;
}

TileLayer* TileMap::getLayer(const std::string& name) {
    auto it = m_layerNameToIndex.find(name);
    if (it == m_layerNameToIndex.end()) {
        return nullptr;
    }
    return m_layers[it->second].get();
}

TileLayer* TileMap::getLayer(int index) {
    if (index < 0 || index >= static_cast<int>(m_layers.size())) {
        return nullptr;
    }
    return m_layers[index].get();
}

const TileLayer* TileMap::getLayer(int index) const {
    if (index < 0 || index >= static_cast<int>(m_layers.size())) {
        return nullptr;
    }
    return m_layers[index].get();
}

void TileMap::removeLayer(const std::string& name) {
    auto it = m_layerNameToIndex.find(name);
    if (it == m_layerNameToIndex.end()) {
        return;
    }

    int index = it->second;
    m_layers.erase(m_layers.begin() + index);
    m_layerNameToIndex.erase(it);

    // Rebuild index map
    m_layerNameToIndex.clear();
    for (size_t i = 0; i < m_layers.size(); ++i) {
        m_layerNameToIndex[m_layers[i]->getName()] = i;
    }
}

void TileMap::setLayerOrder(const std::vector<std::string>& layerNames) {
    std::vector<std::unique_ptr<TileLayer>> newOrder;

    for (const auto& name : layerNames) {
        auto it = m_layerNameToIndex.find(name);
        if (it != m_layerNameToIndex.end()) {
            newOrder.push_back(std::move(m_layers[it->second]));
        }
    }

    m_layers = std::move(newOrder);

    // Rebuild index map
    m_layerNameToIndex.clear();
    for (size_t i = 0; i < m_layers.size(); ++i) {
        m_layerNameToIndex[m_layers[i]->getName()] = i;
    }
}

void TileMap::setTile(const std::string& layerName, int x, int y, const Tile& tile) {
    TileLayer* layer = getLayer(layerName);
    if (layer) {
        layer->setTile(x, y, tile);
    }
}

Tile* TileMap::getTile(const std::string& layerName, int x, int y) {
    TileLayer* layer = getLayer(layerName);
    if (layer) {
        return layer->getTile(x, y);
    }
    return nullptr;
}

bool TileMap::isSolid(int x, int y) const {
    // Check all layers for solid tiles
    for (const auto& layer : m_layers) {
        if (!layer->isVisible()) continue;

        const Tile* tile = layer->getTile(x, y);
        if (tile && tile->tileID > 0 && tile->solid) {
            return true;
        }
    }
    return false;
}

bool TileMap::isDestructible(int x, int y) const {
    for (const auto& layer : m_layers) {
        if (!layer->isVisible()) continue;

        const Tile* tile = layer->getTile(x, y);
        if (tile && tile->tileID > 0 && tile->destructible) {
            return true;
        }
    }
    return false;
}

bool TileMap::damageTile(int x, int y, int damage) {
    for (auto& layer : m_layers) {
        if (!layer->isVisible()) continue;

        Tile* tile = layer->getTile(x, y);
        if (tile && tile->tileID > 0 && tile->destructible) {
            tile->health -= damage;
            if (tile->health <= 0) {
                layer->removeTile(x, y);
                return true; // Tile destroyed
            }
            return false; // Tile damaged but not destroyed
        }
    }
    return false;
}

void TileMap::render(int cameraX, int cameraY, int screenWidth, int screenHeight) {
    // Render all layers in order
    for (size_t i = 0; i < m_layers.size(); ++i) {
        renderLayer(i, cameraX, cameraY, screenWidth, screenHeight);
    }

    if (m_debugDraw) {
        renderDebug();
    }
}

void TileMap::renderLayer(int layerIndex, int cameraX, int cameraY,
                         int screenWidth, int screenHeight) {
    const TileLayer* layer = getLayer(layerIndex);
    if (!layer || !layer->isVisible()) {
        return;
    }

    // Get visible tiles for this layer
    auto visibleTiles = layer->getVisibleTiles(cameraX, cameraY, screenWidth, screenHeight);

    // Render each visible tile
    for (const Tile* tile : visibleTiles) {
        int screenX = tile->x * m_tileWidth - cameraX;
        int screenY = tile->y * m_tileHeight - cameraY;
        renderTile(*tile, screenX, screenY, layer->getOpacity());
    }
}

TileMap::ViewBounds TileMap::calculateViewBounds(int cameraX, int cameraY,
                                                 int screenWidth, int screenHeight) const {
    ViewBounds bounds;
    bounds.minTileX = std::max(0, cameraX / m_tileWidth);
    bounds.minTileY = std::max(0, cameraY / m_tileHeight);
    bounds.maxTileX = std::min(m_width, (cameraX + screenWidth) / m_tileWidth + 1);
    bounds.maxTileY = std::min(m_height, (cameraY + screenHeight) / m_tileHeight + 1);
    return bounds;
}

void TileMap::renderTile(const Tile& tile, int screenX, int screenY, float opacity) {
    // This would integrate with your rendering system
    // For now, this is a placeholder that would call your sprite renderer
    // Example:
    // Renderer::drawTile(tile.tileID, screenX, screenY, m_tileWidth, m_tileHeight, opacity);
}

void TileMap::renderDebug() {
    // Render debug information like grid lines, collision boxes, etc.
    // This would integrate with your debug rendering system
}

bool TileMap::saveToFile(const std::string& filename) const {
    // TODO: Implement JSON serialization
    // This would save the tilemap to a JSON file
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Write basic structure
    file << "{\n";
    file << "  \"width\": " << m_width << ",\n";
    file << "  \"height\": " << m_height << ",\n";
    file << "  \"tileWidth\": " << m_tileWidth << ",\n";
    file << "  \"tileHeight\": " << m_tileHeight << ",\n";
    file << "  \"layers\": " << m_layers.size() << "\n";
    file << "}\n";

    file.close();
    return true;
}

bool TileMap::loadFromFile(const std::string& filename) {
    // TODO: Implement JSON deserialization
    // This would load the tilemap from a JSON file
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // For now, just a placeholder
    file.close();
    return true;
}

bool TileMap::isInBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

// ==================== TileAnimation Implementation ====================

TileAnimation::TileAnimation(int baseTileID)
    : m_baseTileID(baseTileID)
    , m_currentTime(0.0f)
    , m_currentFrame(0)
{
}

void TileAnimation::addFrame(int tileID, float duration) {
    Frame frame;
    frame.tileID = tileID;
    frame.duration = duration;
    m_frames.push_back(frame);
}

int TileAnimation::getCurrentTileID(float elapsedTime) const {
    if (m_frames.empty()) {
        return m_baseTileID;
    }

    m_currentTime += elapsedTime;

    // Calculate total animation duration
    float totalDuration = 0.0f;
    for (const auto& frame : m_frames) {
        totalDuration += frame.duration;
    }

    // Loop the animation
    while (m_currentTime >= totalDuration) {
        m_currentTime -= totalDuration;
    }

    // Find current frame
    float accumulatedTime = 0.0f;
    for (size_t i = 0; i < m_frames.size(); ++i) {
        accumulatedTime += m_frames[i].duration;
        if (m_currentTime < accumulatedTime) {
            m_currentFrame = i;
            return m_frames[i].tileID;
        }
    }

    return m_baseTileID;
}

void TileAnimation::reset() {
    m_currentTime = 0.0f;
    m_currentFrame = 0;
}

// ==================== TileAnimationManager Implementation ====================

void TileAnimationManager::registerAnimation(int baseTileID, const TileAnimation& animation) {
    m_animations[baseTileID] = animation;
}

const TileAnimation* TileAnimationManager::getAnimation(int tileID) const {
    auto it = m_animations.find(tileID);
    if (it != m_animations.end()) {
        return &it->second;
    }
    return nullptr;
}

void TileAnimationManager::update(float deltaTime) {
    m_elapsedTime += deltaTime;
}

int TileAnimationManager::getAnimatedTileID(int baseTileID) const {
    auto it = m_animations.find(baseTileID);
    if (it != m_animations.end()) {
        return it->second.getCurrentTileID(m_elapsedTime);
    }
    return baseTileID;
}

} // namespace World
