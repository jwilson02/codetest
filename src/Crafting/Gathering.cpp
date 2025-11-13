#include "Gathering.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Crafting {

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

// ResourceNode implementation
ResourceNode::ResourceNode()
    : m_type(ResourceNodeType::OreVein)
    , m_requiredSkillLevel(1)
    , m_gatherTime(3.0f)
    , m_remainingUses(1)
    , m_maxUses(1)
    , m_isAvailable(true)
    , m_isRare(false)
    , m_x(0.0f), m_y(0.0f), m_z(0.0f)
    , m_respawnTime(300.0f)
    , m_timeUntilRespawn(0.0f)
{
}

ResourceNode::ResourceNode(const std::string& id, ResourceNodeType type)
    : ResourceNode()
{
    m_id = id;
    m_type = type;
}

ResourceNode::~ResourceNode() {
}

void ResourceNode::SetPosition(float x, float y, float z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

void ResourceNode::AddLoot(const ResourceLoot& loot) {
    m_lootTable.push_back(loot);
}

bool ResourceNode::CanGather(int skillLevel) const {
    return m_isAvailable && m_remainingUses > 0 && skillLevel >= m_requiredSkillLevel;
}

std::vector<std::pair<std::string, int>> ResourceNode::Gather(int skillLevel, float bonusChance, float rareChance) {
    std::vector<std::pair<std::string, int>> loot;

    if (!CanGather(skillLevel)) {
        return loot;
    }

    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);

    for (const auto& item : m_lootTable) {
        // Check skill requirement
        if (skillLevel < item.minSkillLevel) {
            continue;
        }

        // Calculate drop chance with bonuses
        float effectiveChance = item.dropChance;
        if (item.quality >= ResourceQuality::Fine) {
            effectiveChance *= (1.0f + rareChance);
        }
        effectiveChance = std::min(effectiveChance, 1.0f);

        // Roll for drop
        if (chanceDist(gen) <= effectiveChance) {
            // Determine quantity
            std::uniform_int_distribution<int> quantDist(item.minQuantity, item.maxQuantity);
            int quantity = quantDist(gen);

            // Bonus quantity chance
            if (chanceDist(gen) <= bonusChance) {
                quantity += std::max(1, quantity / 2);
            }

            loot.push_back({item.itemId, quantity});
        }
    }

    m_remainingUses--;
    if (m_remainingUses <= 0) {
        Deplete();
    }

    return loot;
}

void ResourceNode::Deplete() {
    m_isAvailable = false;
    m_timeUntilRespawn = m_respawnTime;
}

void ResourceNode::Respawn() {
    m_isAvailable = true;
    m_remainingUses = m_maxUses;
    m_timeUntilRespawn = 0.0f;
}

void ResourceNode::UpdateRespawnTimer(float deltaTime) {
    if (!m_isAvailable && m_timeUntilRespawn > 0.0f) {
        m_timeUntilRespawn -= deltaTime;
        if (m_timeUntilRespawn <= 0.0f) {
            Respawn();
        }
    }
}

// GatheringSystem implementation
GatheringSystem::GatheringSystem() {
}

GatheringSystem::~GatheringSystem() {
}

void GatheringSystem::RegisterNode(std::shared_ptr<ResourceNode> node) {
    if (node) {
        m_nodes[node->GetId()] = node;
    }
}

void GatheringSystem::UnregisterNode(const std::string& nodeId) {
    m_nodes.erase(nodeId);
}

std::shared_ptr<ResourceNode> GatheringSystem::GetNode(const std::string& nodeId) {
    auto it = m_nodes.find(nodeId);
    if (it != m_nodes.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ResourceNode>> GatheringSystem::GetNearbyNodes(float x, float y, float z, float radius) {
    std::vector<std::shared_ptr<ResourceNode>> nearby;

    float radiusSquared = radius * radius;
    for (auto& pair : m_nodes) {
        auto& node = pair.second;
        float dx = node->GetX() - x;
        float dy = node->GetY() - y;
        float dz = node->GetZ() - z;
        float distSquared = dx * dx + dy * dy + dz * dz;

        if (distSquared <= radiusSquared && node->IsAvailable()) {
            nearby.push_back(node);
        }
    }

    return nearby;
}

std::vector<std::shared_ptr<ResourceNode>> GatheringSystem::GetNodesByType(ResourceNodeType type) {
    std::vector<std::shared_ptr<ResourceNode>> nodes;

    for (auto& pair : m_nodes) {
        if (pair.second->GetType() == type) {
            nodes.push_back(pair.second);
        }
    }

    return nodes;
}

GatheringSystem::GatheringResult GatheringSystem::StartGathering(const std::string& nodeId, Profession* profession) {
    GatheringResult result;
    result.success = false;
    result.xpGained = 0;
    result.criticalGather = false;
    result.quality = ResourceQuality::Normal;

    if (!profession) {
        result.message = "No profession provided";
        return result;
    }

    auto node = GetNode(nodeId);
    if (!node) {
        result.message = "Node not found";
        return result;
    }

    int skillLevel = profession->GetSkillLevel();
    if (!node->CanGather(skillLevel)) {
        if (skillLevel < node->GetRequiredSkillLevel()) {
            result.message = "Skill level too low";
        } else {
            result.message = "Node depleted or unavailable";
        }
        return result;
    }

    // Gather resources
    float bonusChance = profession->GetBonusResourceChance();
    float rareChance = profession->GetRareResourceChance();
    result.items = node->Gather(skillLevel, bonusChance, rareChance);

    if (result.items.empty()) {
        result.message = "No resources gathered";
        return result;
    }

    // Determine quality
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float luck = dist(gen);
    result.quality = DetermineResourceQuality(skillLevel, node->GetRequiredSkillLevel(), luck);

    // Check for critical gather
    float critChance = 0.05f + (skillLevel - node->GetRequiredSkillLevel()) * 0.001f;
    if (dist(gen) <= critChance) {
        result.criticalGather = true;
        // Double the loot on critical
        for (auto& item : result.items) {
            item.second *= 2;
        }
    }

    // Calculate XP
    result.xpGained = CalculateGatheringXP(node->GetType(), node->GetRequiredSkillLevel(), result.criticalGather);
    profession->GainXP(result.xpGained);

    // Update stats
    m_stats.totalGathers++;
    m_stats.successfulGathers++;
    if (result.criticalGather) {
        m_stats.criticalGathers++;
    }
    if (node->IsRareNode()) {
        m_stats.rareResourcesFound++;
    }
    m_stats.gathersByType[node->GetType()]++;

    // Update profession stats
    profession->IncrementCrafts();
    profession->IncrementSuccessfulCrafts();

    result.success = true;
    result.message = "Successfully gathered resources";

    // Trigger callback
    if (m_gatherCompleteCallback) {
        m_gatherCompleteCallback(result);
    }

    // Check if node depleted
    if (!node->IsAvailable() && m_nodeDepletedCallback) {
        m_nodeDepletedCallback(nodeId);
    }

    // Auto-discover node
    DiscoverNode(nodeId);

    return result;
}

bool GatheringSystem::CanGatherNode(const std::string& nodeId, int skillLevel) const {
    auto it = m_nodes.find(nodeId);
    if (it != m_nodes.end()) {
        return it->second->CanGather(skillLevel);
    }
    return false;
}

void GatheringSystem::Update(float deltaTime) {
    // Update respawn timers
    for (auto& pair : m_nodes) {
        pair.second->UpdateRespawnTimer(deltaTime);

        // Check if node just respawned
        if (pair.second->IsAvailable() && pair.second->GetTimeUntilRespawn() == 0.0f) {
            if (m_nodeRespawnedCallback) {
                m_nodeRespawnedCallback(pair.first);
            }
        }
    }
}

void GatheringSystem::DiscoverNode(const std::string& nodeId) {
    if (!IsNodeDiscovered(nodeId)) {
        m_discoveredNodes.push_back(nodeId);
    }
}

bool GatheringSystem::IsNodeDiscovered(const std::string& nodeId) const {
    return std::find(m_discoveredNodes.begin(), m_discoveredNodes.end(), nodeId) != m_discoveredNodes.end();
}

void GatheringSystem::ResetStats() {
    m_stats = GatheringStats();
}

ResourceQuality GatheringSystem::DetermineResourceQuality(int skillLevel, int requiredLevel, float luck) const {
    float qualityScore = 0.0f;

    // Skill contribution
    float skillRatio = static_cast<float>(skillLevel) / static_cast<float>(std::max(1, requiredLevel));
    qualityScore += std::min(skillRatio, 2.0f) * 0.5f;

    // Luck contribution
    qualityScore += luck * 0.5f;

    if (qualityScore >= 0.95f) return ResourceQuality::Pristine;
    if (qualityScore >= 0.80f) return ResourceQuality::Exceptional;
    if (qualityScore >= 0.60f) return ResourceQuality::Fine;
    if (qualityScore >= 0.30f) return ResourceQuality::Normal;
    return ResourceQuality::Poor;
}

int GatheringSystem::CalculateGatheringXP(ResourceNodeType type, int nodeLevel, bool critical) const {
    int baseXP = 10 + nodeLevel * 2;
    if (critical) {
        baseXP *= 2;
    }
    return baseXP;
}

// GatheringMiniGame implementation
GatheringMiniGame::GatheringMiniGame()
    : m_type(MiniGameType::None)
    , m_difficulty(0.5f)
    , m_progress(0.0f)
    , m_isActive(false)
    , m_success(false)
    , m_successBonus(1.0f)
    , m_targetTime(0.0f)
    , m_currentTime(0.0f)
    , m_perfectWindow(0.1f)
    , m_goodWindow(0.3f)
{
}

GatheringMiniGame::~GatheringMiniGame() {
}

void GatheringMiniGame::Start(float difficulty) {
    m_difficulty = difficulty;
    m_isActive = true;
    m_success = false;
    m_progress = 0.0f;
    m_successBonus = 1.0f;

    if (m_type == MiniGameType::TimingChallenge) {
        std::uniform_real_distribution<float> dist(1.0f, 3.0f);
        m_targetTime = dist(gen);
        m_currentTime = 0.0f;
    }
}

void GatheringMiniGame::Update(float deltaTime) {
    if (!m_isActive) return;

    if (m_type == MiniGameType::TimingChallenge) {
        m_currentTime += deltaTime;
    }
}

bool GatheringMiniGame::CheckSuccess() const {
    if (m_type == MiniGameType::TimingChallenge) {
        float diff = std::abs(m_currentTime - m_targetTime);
        return diff <= m_goodWindow;
    }
    return false;
}

float GatheringMiniGame::GetSuccessBonus() const {
    if (!m_isActive || !m_success) {
        return 1.0f;
    }

    if (m_type == MiniGameType::TimingChallenge) {
        float diff = std::abs(m_currentTime - m_targetTime);
        if (diff <= m_perfectWindow) {
            return 2.0f;  // Perfect timing: 2x bonus
        } else if (diff <= m_goodWindow) {
            return 1.5f;  // Good timing: 1.5x bonus
        }
    }

    return 1.0f;
}

void GatheringMiniGame::OnPlayerInput(float value) {
    // Handle player input for mini-game
}

void GatheringMiniGame::OnPlayerAction() {
    if (!m_isActive) return;

    if (m_type == MiniGameType::TimingChallenge) {
        m_success = CheckSuccess();
        m_successBonus = GetSuccessBonus();
        m_isActive = false;
    }
}

// Helper functions
std::string ResourceNodeTypeToString(ResourceNodeType type) {
    switch (type) {
        case ResourceNodeType::OreVein:       return "Ore Vein";
        case ResourceNodeType::HerbNode:      return "Herb Node";
        case ResourceNodeType::TreeNode:      return "Tree";
        case ResourceNodeType::FishingSpot:   return "Fishing Spot";
        case ResourceNodeType::AnimalCorpse:  return "Animal Corpse";
        case ResourceNodeType::TreasureChest: return "Treasure Chest";
        case ResourceNodeType::AncientRelic:  return "Ancient Relic";
        default: return "Unknown";
    }
}

ResourceNodeType StringToResourceNodeType(const std::string& str) {
    if (str == "OreVein") return ResourceNodeType::OreVein;
    if (str == "HerbNode") return ResourceNodeType::HerbNode;
    if (str == "TreeNode") return ResourceNodeType::TreeNode;
    if (str == "FishingSpot") return ResourceNodeType::FishingSpot;
    if (str == "AnimalCorpse") return ResourceNodeType::AnimalCorpse;
    if (str == "TreasureChest") return ResourceNodeType::TreasureChest;
    if (str == "AncientRelic") return ResourceNodeType::AncientRelic;
    return ResourceNodeType::OreVein;
}

std::string ResourceQualityToString(ResourceQuality quality) {
    switch (quality) {
        case ResourceQuality::Poor:         return "Poor";
        case ResourceQuality::Normal:       return "Normal";
        case ResourceQuality::Fine:         return "Fine";
        case ResourceQuality::Exceptional:  return "Exceptional";
        case ResourceQuality::Pristine:     return "Pristine";
        default: return "Unknown";
    }
}

ResourceQuality StringToResourceQuality(const std::string& str) {
    if (str == "Poor") return ResourceQuality::Poor;
    if (str == "Normal") return ResourceQuality::Normal;
    if (str == "Fine") return ResourceQuality::Fine;
    if (str == "Exceptional") return ResourceQuality::Exceptional;
    if (str == "Pristine") return ResourceQuality::Pristine;
    return ResourceQuality::Normal;
}

ProfessionType GetProfessionForNodeType(ResourceNodeType type) {
    switch (type) {
        case ResourceNodeType::OreVein:
            return ProfessionType::Mining;
        case ResourceNodeType::HerbNode:
            return ProfessionType::Herbalism;
        case ResourceNodeType::TreeNode:
            return ProfessionType::Logging;
        case ResourceNodeType::FishingSpot:
            return ProfessionType::Fishing;
        case ResourceNodeType::AnimalCorpse:
            return ProfessionType::Skinning;
        default:
            return ProfessionType::Mining;
    }
}

float GetQualityMultiplier(ResourceQuality quality) {
    switch (quality) {
        case ResourceQuality::Poor:        return 0.5f;
        case ResourceQuality::Normal:      return 1.0f;
        case ResourceQuality::Fine:        return 1.5f;
        case ResourceQuality::Exceptional: return 2.0f;
        case ResourceQuality::Pristine:    return 3.0f;
        default: return 1.0f;
    }
}

} // namespace Crafting
