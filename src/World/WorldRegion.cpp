#include "WorldRegion.h"
#include <fstream>
#include <algorithm>

namespace World {

// ==================== WorldRegion Implementation ====================

WorldRegion::WorldRegion(const std::string& name, const RegionBounds& bounds)
    : m_name(name)
    , m_bounds(bounds)
    , m_biome(MapSystem::BiomeType::Forest)
    , m_level(1)
    , m_difficulty(1.0f)
    , m_weather(Weather::Clear)
    , m_timeScale(1.0f)
    , m_discovered(false)
    , m_explorationProgress(0.0f)
{
}

WorldRegion::~WorldRegion() {
}

void WorldRegion::addSubRegion(std::unique_ptr<WorldRegion> subRegion) {
    if (subRegion) {
        m_subRegions.push_back(std::move(subRegion));
    }
}

WorldRegion* WorldRegion::getSubRegion(const std::string& name) {
    for (auto& subRegion : m_subRegions) {
        if (subRegion->getName() == name) {
            return subRegion.get();
        }
    }
    return nullptr;
}

std::string WorldRegion::getProperty(const std::string& key) const {
    auto it = m_properties.find(key);
    if (it != m_properties.end()) {
        return it->second;
    }
    return "";
}

bool WorldRegion::hasProperty(const std::string& key) const {
    return m_properties.find(key) != m_properties.end();
}

WorldRegion::PointOfInterest* WorldRegion::getPOI(const std::string& name) {
    for (auto& poi : m_pointsOfInterest) {
        if (poi.name == name) {
            return &poi;
        }
    }
    return nullptr;
}

bool WorldRegion::saveToJSON(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Write JSON structure
    file << "{\n";
    file << "  \"name\": \"" << m_name << "\",\n";
    file << "  \"bounds\": {\n";
    file << "    \"x\": " << m_bounds.x << ",\n";
    file << "    \"y\": " << m_bounds.y << ",\n";
    file << "    \"width\": " << m_bounds.width << ",\n";
    file << "    \"height\": " << m_bounds.height << "\n";
    file << "  },\n";
    file << "  \"level\": " << m_level << ",\n";
    file << "  \"difficulty\": " << m_difficulty << ",\n";
    file << "  \"discovered\": " << (m_discovered ? "true" : "false") << ",\n";
    file << "  \"explorationProgress\": " << m_explorationProgress << "\n";
    file << "}\n";

    file.close();
    return true;
}

bool WorldRegion::loadFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TODO: Implement JSON parsing
    file.close();
    return true;
}

// ==================== RegionManager Implementation ====================

RegionManager::RegionManager()
    : m_activeRegion(nullptr)
    , m_worldWidth(10000)
    , m_worldHeight(10000)
{
}

RegionManager::~RegionManager() {
}

void RegionManager::addRegion(std::unique_ptr<WorldRegion> region) {
    if (region) {
        std::string name = region->getName();
        m_regions[name] = std::move(region);
    }
}

WorldRegion* RegionManager::getRegion(const std::string& name) {
    auto it = m_regions.find(name);
    if (it != m_regions.end()) {
        return it->second.get();
    }
    return nullptr;
}

WorldRegion* RegionManager::getRegionAt(int x, int y) {
    for (auto& pair : m_regions) {
        if (pair.second->getBounds().contains(x, y)) {
            return pair.second.get();
        }
    }
    return nullptr;
}

void RegionManager::removeRegion(const std::string& name) {
    m_regions.erase(name);
    if (m_activeRegion && m_activeRegion->getName() == name) {
        m_activeRegion = nullptr;
    }
}

void RegionManager::setActiveRegion(const std::string& name) {
    WorldRegion* region = getRegion(name);
    if (region) {
        m_activeRegion = region;
        m_activeRegion->setDiscovered(true);
    }
}

std::vector<WorldRegion*> RegionManager::getRegionsInArea(const RegionBounds& area) {
    std::vector<WorldRegion*> results;

    for (auto& pair : m_regions) {
        if (pair.second->getBounds().intersects(area)) {
            results.push_back(pair.second.get());
        }
    }

    return results;
}

std::vector<WorldRegion*> RegionManager::getNeighboringRegions(const std::string& regionName) {
    WorldRegion* region = getRegion(regionName);
    if (!region) {
        return std::vector<WorldRegion*>();
    }

    RegionBounds bounds = region->getBounds();
    RegionBounds expandedBounds(
        bounds.x - 100, bounds.y - 100,
        bounds.width + 200, bounds.height + 200
    );

    return getRegionsInArea(expandedBounds);
}

void RegionManager::update(float deltaTime, int playerX, int playerY) {
    updateActiveRegion(playerX, playerY);

    // Update active region exploration
    if (m_activeRegion) {
        float progress = m_activeRegion->getExplorationProgress();
        if (progress < 100.0f) {
            // Slowly increase exploration as player moves through region
            progress += deltaTime * 0.1f;
            m_activeRegion->setExplorationProgress(std::min(100.0f, progress));
        }
    }

    // Update resource respawn timers
    if (m_activeRegion) {
        auto& resources = m_activeRegion->getResources();
        for (auto& resource : resources) {
            if (resource.depleted && resource.respawnTime > 0) {
                resource.respawnTime -= deltaTime;
                if (resource.respawnTime <= 0) {
                    resource.depleted = false;
                    resource.respawnTime = 300.0f; // 5 minutes default
                }
            }
        }
    }
}

bool RegionManager::loadWorldRegions(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TODO: Implement JSON parsing to load regions
    file.close();
    return true;
}

bool RegionManager::saveWorldRegions(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "{\n";
    file << "  \"worldWidth\": " << m_worldWidth << ",\n";
    file << "  \"worldHeight\": " << m_worldHeight << ",\n";
    file << "  \"regions\": [\n";

    bool first = true;
    for (const auto& pair : m_regions) {
        if (!first) file << ",\n";
        file << "    {\n";
        file << "      \"name\": \"" << pair.second->getName() << "\"\n";
        file << "    }";
        first = false;
    }

    file << "\n  ]\n";
    file << "}\n";

    file.close();
    return true;
}

RegionManager::Stats RegionManager::getStats() const {
    Stats stats;
    stats.totalRegions = m_regions.size();
    stats.discoveredRegions = 0;
    stats.totalExplorationProgress = 0.0f;

    for (const auto& pair : m_regions) {
        if (pair.second->isDiscovered()) {
            stats.discoveredRegions++;
        }
        stats.totalExplorationProgress += pair.second->getExplorationProgress();
    }

    if (stats.totalRegions > 0) {
        stats.totalExplorationProgress /= stats.totalRegions;
    }

    return stats;
}

void RegionManager::updateActiveRegion(int playerX, int playerY) {
    WorldRegion* newActiveRegion = getRegionAt(playerX, playerY);

    if (newActiveRegion != m_activeRegion) {
        m_activeRegion = newActiveRegion;
        if (m_activeRegion) {
            m_activeRegion->setDiscovered(true);
        }
    }
}

} // namespace World
