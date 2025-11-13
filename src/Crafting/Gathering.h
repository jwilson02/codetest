#pragma once

#include "Profession.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Crafting {

// Resource node types
enum class ResourceNodeType {
    OreVein,         // Mining
    HerbNode,        // Herbalism
    TreeNode,        // Logging
    FishingSpot,     // Fishing
    AnimalCorpse,    // Skinning
    TreasureChest,   // Special
    AncientRelic     // Special/Archaeology
};

// Resource quality
enum class ResourceQuality {
    Poor,
    Normal,
    Fine,
    Exceptional,
    Pristine
};

// Loot table entry
struct ResourceLoot {
    std::string itemId;
    int minQuantity;
    int maxQuantity;
    float dropChance;      // 0.0 to 1.0
    int minSkillLevel;     // Minimum skill to get this
    ResourceQuality quality;
};

/**
 * @brief Represents a gatherable resource node in the world
 */
class ResourceNode {
public:
    ResourceNode();
    ResourceNode(const std::string& id, ResourceNodeType type);
    ~ResourceNode();

    // Getters
    const std::string& GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    ResourceNodeType GetType() const { return m_type; }
    int GetRequiredSkillLevel() const { return m_requiredSkillLevel; }
    float GetGatherTime() const { return m_gatherTime; }
    int GetRemainingUses() const { return m_remainingUses; }
    int GetMaxUses() const { return m_maxUses; }
    bool IsAvailable() const { return m_isAvailable && m_remainingUses > 0; }
    bool IsRareNode() const { return m_isRare; }

    // Position
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetZ() const { return m_z; }
    void SetPosition(float x, float y, float z);

    // Loot table
    const std::vector<ResourceLoot>& GetLootTable() const { return m_lootTable; }
    void AddLoot(const ResourceLoot& loot);

    // Gathering
    bool CanGather(int skillLevel) const;
    std::vector<std::pair<std::string, int>> Gather(int skillLevel, float bonusChance, float rareChance);
    void Deplete();
    void Respawn();

    // Setters
    void SetId(const std::string& id) { m_id = id; }
    void SetName(const std::string& name) { m_name = name; }
    void SetType(ResourceNodeType type) { m_type = type; }
    void SetRequiredSkillLevel(int level) { m_requiredSkillLevel = level; }
    void SetGatherTime(float time) { m_gatherTime = time; }
    void SetMaxUses(int uses) { m_maxUses = uses; m_remainingUses = uses; }
    void SetRare(bool rare) { m_isRare = rare; }
    void SetRespawnTime(float time) { m_respawnTime = time; }

    // Respawn management
    float GetRespawnTime() const { return m_respawnTime; }
    float GetTimeUntilRespawn() const { return m_timeUntilRespawn; }
    void UpdateRespawnTimer(float deltaTime);

private:
    std::string m_id;
    std::string m_name;
    ResourceNodeType m_type;
    int m_requiredSkillLevel;
    float m_gatherTime;
    int m_remainingUses;
    int m_maxUses;
    bool m_isAvailable;
    bool m_isRare;

    // Position
    float m_x, m_y, m_z;

    // Loot
    std::vector<ResourceLoot> m_lootTable;

    // Respawn
    float m_respawnTime;
    float m_timeUntilRespawn;
};

/**
 * @brief Manages the gathering process
 */
class GatheringSystem {
public:
    GatheringSystem();
    ~GatheringSystem();

    // Node management
    void RegisterNode(std::shared_ptr<ResourceNode> node);
    void UnregisterNode(const std::string& nodeId);
    std::shared_ptr<ResourceNode> GetNode(const std::string& nodeId);
    std::vector<std::shared_ptr<ResourceNode>> GetNearbyNodes(float x, float y, float z, float radius);
    std::vector<std::shared_ptr<ResourceNode>> GetNodesByType(ResourceNodeType type);

    // Gathering
    struct GatheringResult {
        bool success;
        std::vector<std::pair<std::string, int>> items;  // itemId, quantity
        int xpGained;
        bool criticalGather;
        ResourceQuality quality;
        std::string message;
    };

    GatheringResult StartGathering(const std::string& nodeId, Profession* profession);
    bool CanGatherNode(const std::string& nodeId, int skillLevel) const;

    // Update
    void Update(float deltaTime);

    // Discovery system
    void DiscoverNode(const std::string& nodeId);
    bool IsNodeDiscovered(const std::string& nodeId) const;
    const std::vector<std::string>& GetDiscoveredNodes() const { return m_discoveredNodes; }

    // Statistics
    struct GatheringStats {
        int totalGathers = 0;
        int successfulGathers = 0;
        int criticalGathers = 0;
        int rareResourcesFound = 0;
        std::map<ResourceNodeType, int> gathersByType;
    };

    const GatheringStats& GetStats() const { return m_stats; }
    void ResetStats();

    // Callbacks
    void SetGatherCompleteCallback(std::function<void(const GatheringResult&)> callback) {
        m_gatherCompleteCallback = callback;
    }

    void SetNodeDepletedCallback(std::function<void(const std::string&)> callback) {
        m_nodeDepletedCallback = callback;
    }

    void SetNodeRespawnedCallback(std::function<void(const std::string&)> callback) {
        m_nodeRespawnedCallback = callback;
    }

private:
    ResourceQuality DetermineResourceQuality(int skillLevel, int requiredLevel, float luck) const;
    int CalculateGatheringXP(ResourceNodeType type, int nodeLevel, bool critical) const;

private:
    std::map<std::string, std::shared_ptr<ResourceNode>> m_nodes;
    std::vector<std::string> m_discoveredNodes;
    GatheringStats m_stats;

    // Callbacks
    std::function<void(const GatheringResult&)> m_gatherCompleteCallback;
    std::function<void(const std::string&)> m_nodeDepletedCallback;
    std::function<void(const std::string&)> m_nodeRespawnedCallback;
};

/**
 * @brief Gathering mini-game system (optional skill-based gathering)
 */
class GatheringMiniGame {
public:
    enum class MiniGameType {
        None,
        TimingChallenge,     // Hit the right timing
        SequenceMatch,       // Match a sequence
        StabilityGame,       // Keep a meter stable
        SpotTheResource      // Find the right spot
    };

    GatheringMiniGame();
    ~GatheringMiniGame();

    void SetType(MiniGameType type) { m_type = type; }
    MiniGameType GetType() const { return m_type; }

    void Start(float difficulty);
    void Update(float deltaTime);
    bool CheckSuccess() const;
    float GetSuccessBonus() const;  // Multiplier for rewards

    // Player input
    void OnPlayerInput(float value);
    void OnPlayerAction();

private:
    MiniGameType m_type;
    float m_difficulty;
    float m_progress;
    bool m_isActive;
    bool m_success;
    float m_successBonus;

    // Timing challenge
    float m_targetTime;
    float m_currentTime;
    float m_perfectWindow;
    float m_goodWindow;
};

// Helper functions
std::string ResourceNodeTypeToString(ResourceNodeType type);
ResourceNodeType StringToResourceNodeType(const std::string& str);
std::string ResourceQualityToString(ResourceQuality quality);
ResourceQuality StringToResourceQuality(const std::string& str);

ProfessionType GetProfessionForNodeType(ResourceNodeType type);
float GetQualityMultiplier(ResourceQuality quality);

} // namespace Crafting
