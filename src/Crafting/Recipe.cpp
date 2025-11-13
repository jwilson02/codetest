#include "Recipe.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Crafting {

Recipe::Recipe()
    : m_category(RecipeCategory::Materials)
    , m_difficulty(RecipeDifficulty::Easy)
    , m_outputQuantity(1)
    , m_craftTime(1.0f)
    , m_cooldown(0.0f)
    , m_isDiscoverable(false)
    , m_isLearned(false)
    , m_isOneTime(false)
    , m_canHaveQuality(true)
    , m_minQuality(CraftQuality::Poor)
    , m_maxQuality(CraftQuality::Legendary)
    , m_qualityBonus(0.0f)
    , m_goldCost(0)
    , m_manaCost(0)
    , m_isSeasonal(false)
    , m_isLegendary(false)
{
}

Recipe::Recipe(const std::string& id, const std::string& name)
    : Recipe()
{
    m_id = id;
    m_name = name;
}

Recipe::~Recipe() {
}

void Recipe::AddMaterialRequirement(const MaterialRequirement& req) {
    m_materialRequirements.push_back(req);
}

void Recipe::AddToolRequirement(const ToolRequirement& req) {
    m_toolRequirements.push_back(req);
}

void Recipe::AddStationRequirement(const StationRequirement& req) {
    m_stationRequirements.push_back(req);
}

void Recipe::AddSkillRequirement(const SkillRequirement& req) {
    m_skillRequirements.push_back(req);
}

void Recipe::AddUnlockCondition(const UnlockCondition& condition) {
    m_unlockConditions.push_back(condition);
}

void Recipe::AddBonusOutput(const BonusOutput& bonus) {
    m_bonusOutputs.push_back(bonus);
}

bool Recipe::CanCraft(const std::map<std::string, int>& availableMaterials,
                      const std::map<std::string, int>& playerSkills,
                      const std::vector<std::string>& availableStations) const {
    // Check materials
    for (const auto& req : m_materialRequirements) {
        auto it = availableMaterials.find(req.itemId);
        if (it == availableMaterials.end() || it->second < req.quantity) {
            return false;
        }
    }

    // Check skills
    for (const auto& req : m_skillRequirements) {
        auto it = playerSkills.find(req.professionId);
        if (it == playerSkills.end() || it->second < req.skillLevel) {
            return false;
        }
    }

    // Check stations
    for (const auto& req : m_stationRequirements) {
        bool found = false;
        for (const auto& station : availableStations) {
            if (station == req.stationId) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

float Recipe::CalculateSuccessChance(int skillLevel) const {
    float baseChance = 0.0f;

    switch (m_difficulty) {
        case RecipeDifficulty::Trivial:
            return 1.0f;
        case RecipeDifficulty::Easy:
            baseChance = 0.95f;
            break;
        case RecipeDifficulty::Medium:
            baseChance = 0.75f;
            break;
        case RecipeDifficulty::Hard:
            baseChance = 0.50f;
            break;
        case RecipeDifficulty::Expert:
            baseChance = 0.25f;
            break;
        case RecipeDifficulty::Legendary:
            baseChance = 0.10f;
            break;
    }

    // Skill level bonus: +0.5% per skill level above requirement
    if (!m_skillRequirements.empty()) {
        int requiredLevel = m_skillRequirements[0].skillLevel;
        int skillDiff = skillLevel - requiredLevel;
        if (skillDiff > 0) {
            baseChance += skillDiff * 0.005f;
        }
    }

    return std::min(baseChance, 1.0f);
}

CraftQuality Recipe::DetermineQuality(const std::map<std::string, float>& materialQualities,
                                       int skillLevel, float luck) const {
    if (!m_canHaveQuality) {
        return CraftQuality::Normal;
    }

    // Calculate base quality score (0.0 to 1.0)
    float qualityScore = 0.0f;

    // Material quality contribution (40%)
    float materialQuality = 0.0f;
    float totalContribution = 0.0f;
    for (const auto& req : m_materialRequirements) {
        auto it = materialQualities.find(req.itemId);
        if (it != materialQualities.end()) {
            materialQuality += it->second * req.qualityContribution;
            totalContribution += req.qualityContribution;
        }
    }
    if (totalContribution > 0.0f) {
        materialQuality /= totalContribution;
    }
    qualityScore += materialQuality * 0.4f;

    // Skill level contribution (40%)
    if (!m_skillRequirements.empty()) {
        int requiredLevel = m_skillRequirements[0].skillLevel;
        float skillRatio = static_cast<float>(skillLevel) / static_cast<float>(requiredLevel);
        skillRatio = std::min(skillRatio, 2.0f);  // Cap at 2x
        qualityScore += (skillRatio / 2.0f) * 0.4f;
    } else {
        qualityScore += 0.4f;
    }

    // Luck contribution (20%)
    qualityScore += luck * 0.2f;

    // Apply recipe quality bonus
    qualityScore += m_qualityBonus;
    qualityScore = std::min(qualityScore, 1.0f);

    // Determine quality tier
    if (qualityScore >= 0.95f) return CraftQuality::Legendary;
    if (qualityScore >= 0.80f) return CraftQuality::Masterwork;
    if (qualityScore >= 0.65f) return CraftQuality::Excellent;
    if (qualityScore >= 0.50f) return CraftQuality::Superior;
    if (qualityScore >= 0.30f) return CraftQuality::Normal;
    return CraftQuality::Poor;
}

std::shared_ptr<Recipe> Recipe::Clone() const {
    auto clone = std::make_shared<Recipe>();

    // Copy basic info
    clone->m_id = m_id;
    clone->m_name = m_name;
    clone->m_description = m_description;
    clone->m_category = m_category;
    clone->m_difficulty = m_difficulty;

    // Copy output
    clone->m_outputItemId = m_outputItemId;
    clone->m_outputQuantity = m_outputQuantity;
    clone->m_bonusOutputs = m_bonusOutputs;

    // Copy requirements
    clone->m_materialRequirements = m_materialRequirements;
    clone->m_toolRequirements = m_toolRequirements;
    clone->m_stationRequirements = m_stationRequirements;
    clone->m_skillRequirements = m_skillRequirements;

    // Copy timing
    clone->m_craftTime = m_craftTime;
    clone->m_cooldown = m_cooldown;

    // Copy unlock
    clone->m_unlockConditions = m_unlockConditions;
    clone->m_isDiscoverable = m_isDiscoverable;
    clone->m_isLearned = m_isLearned;
    clone->m_isOneTime = m_isOneTime;

    // Copy quality
    clone->m_canHaveQuality = m_canHaveQuality;
    clone->m_minQuality = m_minQuality;
    clone->m_maxQuality = m_maxQuality;
    clone->m_qualityBonus = m_qualityBonus;

    // Copy costs
    clone->m_goldCost = m_goldCost;
    clone->m_manaCost = m_manaCost;

    // Copy special
    clone->m_isSeasonal = m_isSeasonal;
    clone->m_seasonId = m_seasonId;
    clone->m_isLegendary = m_isLegendary;
    clone->m_recipeScrollId = m_recipeScrollId;

    return clone;
}

// Helper functions
std::string RecipeDifficultyToString(RecipeDifficulty difficulty) {
    switch (difficulty) {
        case RecipeDifficulty::Trivial:   return "Trivial";
        case RecipeDifficulty::Easy:      return "Easy";
        case RecipeDifficulty::Medium:    return "Medium";
        case RecipeDifficulty::Hard:      return "Hard";
        case RecipeDifficulty::Expert:    return "Expert";
        case RecipeDifficulty::Legendary: return "Legendary";
        default: return "Unknown";
    }
}

std::string RecipeCategoryToString(RecipeCategory category) {
    switch (category) {
        case RecipeCategory::WeaponSmithing:   return "Weapon Smithing";
        case RecipeCategory::ArmorSmithing:    return "Armor Smithing";
        case RecipeCategory::ToolSmithing:     return "Tool Smithing";
        case RecipeCategory::JewelrySmithing:  return "Jewelry Smithing";
        case RecipeCategory::Potions:          return "Potions";
        case RecipeCategory::Elixirs:          return "Elixirs";
        case RecipeCategory::Transmutation:    return "Transmutation";
        case RecipeCategory::Oils:             return "Oils";
        case RecipeCategory::WeaponEnchants:   return "Weapon Enchants";
        case RecipeCategory::ArmorEnchants:    return "Armor Enchants";
        case RecipeCategory::RuneCrafting:     return "Rune Crafting";
        case RecipeCategory::GlyphCrafting:    return "Glyph Crafting";
        case RecipeCategory::Food:             return "Food";
        case RecipeCategory::Drinks:           return "Drinks";
        case RecipeCategory::Feast:            return "Feast";
        case RecipeCategory::LightArmor:       return "Light Armor";
        case RecipeCategory::Robes:            return "Robes";
        case RecipeCategory::Bags:             return "Bags";
        case RecipeCategory::MediumArmor:      return "Medium Armor";
        case RecipeCategory::LeatherGoods:     return "Leather Goods";
        case RecipeCategory::Gadgets:          return "Gadgets";
        case RecipeCategory::Mechanisms:       return "Mechanisms";
        case RecipeCategory::Explosives:       return "Explosives";
        case RecipeCategory::Bows:             return "Bows";
        case RecipeCategory::Staves:           return "Staves";
        case RecipeCategory::Furniture:        return "Furniture";
        case RecipeCategory::Rings:            return "Rings";
        case RecipeCategory::Amulets:          return "Amulets";
        case RecipeCategory::Trinkets:         return "Trinkets";
        case RecipeCategory::Consumables:      return "Consumables";
        case RecipeCategory::Materials:        return "Materials";
        case RecipeCategory::Special:          return "Special";
        default: return "Unknown";
    }
}

std::string CraftQualityToString(CraftQuality quality) {
    switch (quality) {
        case CraftQuality::Poor:        return "Poor";
        case CraftQuality::Normal:      return "Normal";
        case CraftQuality::Superior:    return "Superior";
        case CraftQuality::Excellent:   return "Excellent";
        case CraftQuality::Masterwork:  return "Masterwork";
        case CraftQuality::Legendary:   return "Legendary";
        default: return "Unknown";
    }
}

RecipeDifficulty StringToRecipeDifficulty(const std::string& str) {
    if (str == "Trivial") return RecipeDifficulty::Trivial;
    if (str == "Easy") return RecipeDifficulty::Easy;
    if (str == "Medium") return RecipeDifficulty::Medium;
    if (str == "Hard") return RecipeDifficulty::Hard;
    if (str == "Expert") return RecipeDifficulty::Expert;
    if (str == "Legendary") return RecipeDifficulty::Legendary;
    return RecipeDifficulty::Easy;
}

RecipeCategory StringToRecipeCategory(const std::string& str) {
    if (str == "WeaponSmithing") return RecipeCategory::WeaponSmithing;
    if (str == "ArmorSmithing") return RecipeCategory::ArmorSmithing;
    if (str == "ToolSmithing") return RecipeCategory::ToolSmithing;
    if (str == "JewelrySmithing") return RecipeCategory::JewelrySmithing;
    if (str == "Potions") return RecipeCategory::Potions;
    if (str == "Elixirs") return RecipeCategory::Elixirs;
    if (str == "Transmutation") return RecipeCategory::Transmutation;
    if (str == "Oils") return RecipeCategory::Oils;
    if (str == "WeaponEnchants") return RecipeCategory::WeaponEnchants;
    if (str == "ArmorEnchants") return RecipeCategory::ArmorEnchants;
    if (str == "RuneCrafting") return RecipeCategory::RuneCrafting;
    if (str == "GlyphCrafting") return RecipeCategory::GlyphCrafting;
    if (str == "Food") return RecipeCategory::Food;
    if (str == "Drinks") return RecipeCategory::Drinks;
    if (str == "Feast") return RecipeCategory::Feast;
    if (str == "LightArmor") return RecipeCategory::LightArmor;
    if (str == "Robes") return RecipeCategory::Robes;
    if (str == "Bags") return RecipeCategory::Bags;
    if (str == "MediumArmor") return RecipeCategory::MediumArmor;
    if (str == "LeatherGoods") return RecipeCategory::LeatherGoods;
    if (str == "Gadgets") return RecipeCategory::Gadgets;
    if (str == "Mechanisms") return RecipeCategory::Mechanisms;
    if (str == "Explosives") return RecipeCategory::Explosives;
    if (str == "Bows") return RecipeCategory::Bows;
    if (str == "Staves") return RecipeCategory::Staves;
    if (str == "Furniture") return RecipeCategory::Furniture;
    if (str == "Rings") return RecipeCategory::Rings;
    if (str == "Amulets") return RecipeCategory::Amulets;
    if (str == "Trinkets") return RecipeCategory::Trinkets;
    if (str == "Consumables") return RecipeCategory::Consumables;
    if (str == "Special") return RecipeCategory::Special;
    return RecipeCategory::Materials;
}

CraftQuality StringToCraftQuality(const std::string& str) {
    if (str == "Poor") return CraftQuality::Poor;
    if (str == "Normal") return CraftQuality::Normal;
    if (str == "Superior") return CraftQuality::Superior;
    if (str == "Excellent") return CraftQuality::Excellent;
    if (str == "Masterwork") return CraftQuality::Masterwork;
    if (str == "Legendary") return CraftQuality::Legendary;
    return CraftQuality::Normal;
}

float GetQualityMultiplier(CraftQuality quality) {
    switch (quality) {
        case CraftQuality::Poor:        return 0.5f;
        case CraftQuality::Normal:      return 1.0f;
        case CraftQuality::Superior:    return 1.25f;
        case CraftQuality::Excellent:   return 1.5f;
        case CraftQuality::Masterwork:  return 2.0f;
        case CraftQuality::Legendary:   return 3.0f;
        default: return 1.0f;
    }
}

std::string GetQualityColor(CraftQuality quality) {
    switch (quality) {
        case CraftQuality::Poor:        return "#808080";  // Gray
        case CraftQuality::Normal:      return "#FFFFFF";  // White
        case CraftQuality::Superior:    return "#1EFF00";  // Green
        case CraftQuality::Excellent:   return "#0070DD";  // Blue
        case CraftQuality::Masterwork:  return "#A335EE";  // Purple
        case CraftQuality::Legendary:   return "#FF8000";  // Orange
        default: return "#FFFFFF";
    }
}

} // namespace Crafting
