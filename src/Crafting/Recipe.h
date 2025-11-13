#pragma once

#include "../Inventory/Item.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Crafting {

// Recipe difficulty levels
enum class RecipeDifficulty {
    Trivial,      // Always succeeds
    Easy,         // 95% success rate
    Medium,       // 75% success rate
    Hard,         // 50% success rate
    Expert,       // 25% success rate
    Legendary     // 10% success rate
};

// Recipe categories
enum class RecipeCategory {
    // Blacksmithing
    WeaponSmithing,
    ArmorSmithing,
    ToolSmithing,
    JewelrySmithing,

    // Alchemy
    Potions,
    Elixirs,
    Transmutation,
    Oils,

    // Enchanting
    WeaponEnchants,
    ArmorEnchants,
    RuneCrafting,
    GlyphCrafting,

    // Cooking
    Food,
    Drinks,
    Feast,

    // Tailoring
    LightArmor,
    Robes,
    Bags,

    // Leatherworking
    MediumArmor,
    LeatherGoods,

    // Engineering
    Gadgets,
    Mechanisms,
    Explosives,

    // Woodworking
    Bows,
    Staves,
    Furniture,

    // Jewelry
    Rings,
    Amulets,
    Trinkets,

    // Misc
    Consumables,
    Materials,
    Special
};

// Crafting quality levels
enum class CraftQuality {
    Poor,           // 50% of base stats
    Normal,         // 100% of base stats
    Superior,       // 125% of base stats
    Excellent,      // 150% of base stats
    Masterwork,     // 200% of base stats
    Legendary       // 300% of base stats + special effects
};

// Material requirement
struct MaterialRequirement {
    std::string itemId;
    std::string itemName;
    int quantity;
    bool consumed;              // If true, consumed on use
    float qualityContribution;  // How much this material affects final quality
};

// Tool requirement
struct ToolRequirement {
    std::string toolId;
    std::string toolName;
    int minQuality;            // Minimum tool quality required
    bool consumed;             // If true, tool durability is consumed
    int durabilityUse;         // How much durability is consumed
};

// Station requirement
struct StationRequirement {
    std::string stationId;
    std::string stationName;
    int minLevel;              // Minimum station level
};

// Skill requirement
struct SkillRequirement {
    std::string professionId;
    int skillLevel;
    int experienceGained;      // XP gained on successful craft
};

// Recipe unlock condition
struct UnlockCondition {
    std::string type;          // "level", "quest", "discovery", "reputation", "item"
    std::string id;            // Quest ID, item ID, faction ID, etc.
    int value;                 // Level, reputation amount, etc.
};

// Bonus output (chance to create extra items)
struct BonusOutput {
    std::string itemId;
    int quantity;
    float probability;         // 0.0 to 1.0
    int minSkillLevel;        // Minimum skill needed for bonus
};

/**
 * @brief Represents a crafting recipe
 */
class Recipe {
public:
    Recipe();
    Recipe(const std::string& id, const std::string& name);
    ~Recipe();

    // Getters
    const std::string& GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetDescription() const { return m_description; }
    RecipeCategory GetCategory() const { return m_category; }
    RecipeDifficulty GetDifficulty() const { return m_difficulty; }

    // Output
    const std::string& GetOutputItemId() const { return m_outputItemId; }
    int GetOutputQuantity() const { return m_outputQuantity; }
    const std::vector<BonusOutput>& GetBonusOutputs() const { return m_bonusOutputs; }

    // Requirements
    const std::vector<MaterialRequirement>& GetMaterialRequirements() const { return m_materialRequirements; }
    const std::vector<ToolRequirement>& GetToolRequirements() const { return m_toolRequirements; }
    const std::vector<StationRequirement>& GetStationRequirements() const { return m_stationRequirements; }
    const std::vector<SkillRequirement>& GetSkillRequirements() const { return m_skillRequirements; }

    // Timing
    float GetCraftTime() const { return m_craftTime; }
    float GetCooldown() const { return m_cooldown; }

    // Unlock conditions
    const std::vector<UnlockCondition>& GetUnlockConditions() const { return m_unlockConditions; }
    bool IsDiscoverable() const { return m_isDiscoverable; }
    bool IsLearned() const { return m_isLearned; }
    bool IsOneTime() const { return m_isOneTime; }

    // Quality system
    bool CanHaveQuality() const { return m_canHaveQuality; }
    CraftQuality GetMinQuality() const { return m_minQuality; }
    CraftQuality GetMaxQuality() const { return m_maxQuality; }
    float GetQualityBonus() const { return m_qualityBonus; }

    // Costs
    int GetGoldCost() const { return m_goldCost; }
    int GetManaCost() const { return m_manaCost; }

    // Special properties
    bool IsSeasonalRecipe() const { return m_isSeasonal; }
    const std::string& GetSeasonId() const { return m_seasonId; }
    bool IsLegendaryRecipe() const { return m_isLegendary; }
    const std::string& GetRecipeScrollId() const { return m_recipeScrollId; }

    // Setters
    void SetId(const std::string& id) { m_id = id; }
    void SetName(const std::string& name) { m_name = name; }
    void SetDescription(const std::string& desc) { m_description = desc; }
    void SetCategory(RecipeCategory category) { m_category = category; }
    void SetDifficulty(RecipeDifficulty difficulty) { m_difficulty = difficulty; }

    void SetOutputItemId(const std::string& itemId) { m_outputItemId = itemId; }
    void SetOutputQuantity(int quantity) { m_outputQuantity = quantity; }

    void SetCraftTime(float time) { m_craftTime = time; }
    void SetCooldown(float cooldown) { m_cooldown = cooldown; }

    void SetDiscoverable(bool discoverable) { m_isDiscoverable = discoverable; }
    void SetLearned(bool learned) { m_isLearned = learned; }
    void SetOneTime(bool oneTime) { m_isOneTime = oneTime; }

    void SetCanHaveQuality(bool canHave) { m_canHaveQuality = canHave; }
    void SetMinQuality(CraftQuality quality) { m_minQuality = quality; }
    void SetMaxQuality(CraftQuality quality) { m_maxQuality = quality; }
    void SetQualityBonus(float bonus) { m_qualityBonus = bonus; }

    void SetGoldCost(int cost) { m_goldCost = cost; }
    void SetManaCost(int cost) { m_manaCost = cost; }

    void SetSeasonal(bool seasonal) { m_isSeasonal = seasonal; }
    void SetSeasonId(const std::string& season) { m_seasonId = season; }
    void SetLegendary(bool legendary) { m_isLegendary = legendary; }
    void SetRecipeScrollId(const std::string& scrollId) { m_recipeScrollId = scrollId; }

    // Add requirements
    void AddMaterialRequirement(const MaterialRequirement& req);
    void AddToolRequirement(const ToolRequirement& req);
    void AddStationRequirement(const StationRequirement& req);
    void AddSkillRequirement(const SkillRequirement& req);
    void AddUnlockCondition(const UnlockCondition& condition);
    void AddBonusOutput(const BonusOutput& bonus);

    // Check if requirements are met
    bool CanCraft(const std::map<std::string, int>& availableMaterials,
                  const std::map<std::string, int>& playerSkills,
                  const std::vector<std::string>& availableStations) const;

    // Calculate success chance based on skill
    float CalculateSuccessChance(int skillLevel) const;

    // Calculate quality based on materials and skill
    CraftQuality DetermineQuality(const std::map<std::string, float>& materialQualities,
                                   int skillLevel, float luck) const;

    // Clone
    std::shared_ptr<Recipe> Clone() const;

private:
    // Basic info
    std::string m_id;
    std::string m_name;
    std::string m_description;
    RecipeCategory m_category;
    RecipeDifficulty m_difficulty;

    // Output
    std::string m_outputItemId;
    int m_outputQuantity;
    std::vector<BonusOutput> m_bonusOutputs;

    // Requirements
    std::vector<MaterialRequirement> m_materialRequirements;
    std::vector<ToolRequirement> m_toolRequirements;
    std::vector<StationRequirement> m_stationRequirements;
    std::vector<SkillRequirement> m_skillRequirements;

    // Timing
    float m_craftTime;         // Seconds to complete
    float m_cooldown;          // Cooldown between crafts

    // Unlock
    std::vector<UnlockCondition> m_unlockConditions;
    bool m_isDiscoverable;     // Can be discovered by experimentation
    bool m_isLearned;          // Has been learned by player
    bool m_isOneTime;          // Can only be crafted once

    // Quality system
    bool m_canHaveQuality;
    CraftQuality m_minQuality;
    CraftQuality m_maxQuality;
    float m_qualityBonus;      // Bonus to quality roll

    // Costs
    int m_goldCost;
    int m_manaCost;

    // Special properties
    bool m_isSeasonal;
    std::string m_seasonId;
    bool m_isLegendary;
    std::string m_recipeScrollId;  // Item ID of recipe scroll
};

// Helper functions
std::string RecipeDifficultyToString(RecipeDifficulty difficulty);
std::string RecipeCategoryToString(RecipeCategory category);
std::string CraftQualityToString(CraftQuality quality);

RecipeDifficulty StringToRecipeDifficulty(const std::string& str);
RecipeCategory StringToRecipeCategory(const std::string& str);
CraftQuality StringToCraftQuality(const std::string& str);

float GetQualityMultiplier(CraftQuality quality);
std::string GetQualityColor(CraftQuality quality);

} // namespace Crafting
