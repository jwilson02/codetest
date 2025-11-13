#include "Profession.h"
#include <algorithm>
#include <cmath>

namespace Crafting {

Profession::Profession()
    : m_type(ProfessionType::Mining)
    , m_skillLevel(1)
    , m_skillXP(0)
    , m_maxSkillLevel(600)
    , m_craftSpeedBonus(0.0f)
    , m_qualityBonus(0.0f)
    , m_costReduction(0.0f)
    , m_bonusOutputChance(0.0f)
    , m_critCraftChance(0.0f)
    , m_gatheringSpeed(1.0f)
    , m_bonusResourceChance(0.0f)
    , m_rareResourceChance(0.0f)
    , m_totalCrafts(0)
    , m_successfulCrafts(0)
    , m_masterworkCrafts(0)
    , m_legendaryCrafts(0)
{
}

Profession::Profession(ProfessionType type, const std::string& name)
    : Profession()
{
    m_type = type;
    m_name = name;
}

Profession::~Profession() {
}

SkillTier Profession::GetCurrentTier() const {
    return GetSkillTierForLevel(m_skillLevel);
}

int Profession::GetXPForNextLevel() const {
    if (m_skillLevel >= m_maxSkillLevel) {
        return 0;
    }
    return CalculateXPForLevel(m_skillLevel + 1);
}

float Profession::GetProgressToNextLevel() const {
    if (m_skillLevel >= m_maxSkillLevel) {
        return 1.0f;
    }
    int xpNeeded = GetXPForNextLevel();
    if (xpNeeded == 0) return 1.0f;
    return static_cast<float>(m_skillXP) / static_cast<float>(xpNeeded);
}

bool Profession::CanSpecialize() const {
    return m_specializationId.empty() && m_skillLevel >= 200 && !m_specializations.empty();
}

bool Profession::SetSpecialization(const std::string& specId) {
    if (!CanSpecialize()) {
        return false;
    }

    // Find specialization
    for (const auto& spec : m_specializations) {
        if (spec.id == specId && !spec.isLocked) {
            m_specializationId = specId;

            // Learn bonus recipes
            for (const auto& recipeId : spec.bonusRecipes) {
                LearnRecipe(recipeId);
            }

            RecalculateBonuses();
            return true;
        }
    }

    return false;
}

std::vector<SkillPerk> Profession::GetUnlockedPerks() const {
    std::vector<SkillPerk> unlocked;
    for (const auto& perk : m_perks) {
        if (perk.isUnlocked) {
            unlocked.push_back(perk);
        }
    }
    return unlocked;
}

float Profession::GetPerkBonus(const std::string& type) const {
    float total = 0.0f;
    for (const auto& perk : m_perks) {
        if (perk.isUnlocked && perk.type == type) {
            total += perk.value;
        }
    }
    return total;
}

bool Profession::UnlockPerk(const std::string& perkId) {
    for (auto& perk : m_perks) {
        if (perk.id == perkId && !perk.isUnlocked) {
            if (m_skillLevel >= perk.unlockLevel) {
                perk.isUnlocked = true;
                RecalculateBonuses();
                return true;
            }
        }
    }
    return false;
}

bool Profession::KnowsRecipe(const std::string& recipeId) const {
    return std::find(m_knownRecipes.begin(), m_knownRecipes.end(), recipeId) != m_knownRecipes.end();
}

bool Profession::LearnRecipe(const std::string& recipeId) {
    if (KnowsRecipe(recipeId)) {
        return false;
    }
    m_knownRecipes.push_back(recipeId);
    return true;
}

void Profession::ForgetRecipe(const std::string& recipeId) {
    auto it = std::find(m_knownRecipes.begin(), m_knownRecipes.end(), recipeId);
    if (it != m_knownRecipes.end()) {
        m_knownRecipes.erase(it);
    }
}

std::vector<ProfessionQuest> Profession::GetAvailableQuests() const {
    std::vector<ProfessionQuest> available;
    for (const auto& quest : m_quests) {
        if (!quest.isCompleted && m_skillLevel >= quest.requiredLevel) {
            available.push_back(quest);
        }
    }
    return available;
}

bool Profession::CompleteQuest(const std::string& questId) {
    for (auto& quest : m_quests) {
        if (quest.questId == questId && !quest.isCompleted) {
            quest.isCompleted = true;

            // Grant rewards
            for (const auto& reward : quest.rewards) {
                // Rewards could be recipes, perks, etc.
                // The actual implementation would depend on the reward system
            }

            return true;
        }
    }
    return false;
}

void Profession::GainXP(int amount) {
    if (m_skillLevel >= m_maxSkillLevel) {
        return;
    }

    m_skillXP += amount;

    // Trigger callback
    if (m_xpGainCallback) {
        m_xpGainCallback(amount, m_skillXP);
    }

    // Check for level up
    while (m_skillXP >= GetXPForNextLevel() && m_skillLevel < m_maxSkillLevel) {
        LevelUp();
    }
}

bool Profession::LevelUp() {
    if (m_skillLevel >= m_maxSkillLevel) {
        return false;
    }

    int xpNeeded = GetXPForNextLevel();
    m_skillXP -= xpNeeded;
    m_skillLevel++;

    // Trigger callback
    if (m_levelUpCallback) {
        m_levelUpCallback(m_skillLevel);
    }

    RecalculateBonuses();
    return true;
}

void Profession::SetSkillLevel(int level) {
    m_skillLevel = std::min(level, m_maxSkillLevel);
    m_skillXP = 0;
    RecalculateBonuses();
}

void Profession::AddSpecialization(const Specialization& spec) {
    m_specializations.push_back(spec);
}

void Profession::AddPerk(const SkillPerk& perk) {
    m_perks.push_back(perk);
}

void Profession::AddQuest(const ProfessionQuest& quest) {
    m_quests.push_back(quest);
}

float Profession::GetSuccessRate() const {
    if (m_totalCrafts == 0) return 0.0f;
    return static_cast<float>(m_successfulCrafts) / static_cast<float>(m_totalCrafts);
}

void Profession::IncrementCrafts() {
    m_totalCrafts++;
}

void Profession::IncrementSuccessfulCrafts() {
    m_successfulCrafts++;
}

void Profession::IncrementMasterworkCrafts() {
    m_masterworkCrafts++;
}

void Profession::IncrementLegendaryCrafts() {
    m_legendaryCrafts++;
}

void Profession::RecalculateBonuses() {
    // Reset bonuses
    m_craftSpeedBonus = 0.0f;
    m_qualityBonus = 0.0f;
    m_costReduction = 0.0f;
    m_bonusOutputChance = 0.0f;
    m_critCraftChance = 0.0f;
    m_gatheringSpeed = 1.0f;
    m_bonusResourceChance = 0.0f;
    m_rareResourceChance = 0.0f;

    // Add perk bonuses
    m_craftSpeedBonus += GetPerkBonus("craft_speed");
    m_qualityBonus += GetPerkBonus("quality");
    m_costReduction += GetPerkBonus("cost_reduction");
    m_bonusOutputChance += GetPerkBonus("bonus_output");
    m_critCraftChance += GetPerkBonus("crit_craft");
    m_gatheringSpeed += GetPerkBonus("gathering_speed");
    m_bonusResourceChance += GetPerkBonus("bonus_resource");
    m_rareResourceChance += GetPerkBonus("rare_resource");

    // Add specialization bonuses
    if (!m_specializationId.empty()) {
        for (const auto& spec : m_specializations) {
            if (spec.id == m_specializationId) {
                for (const auto& bonus : spec.bonuses) {
                    if (bonus.first == "craftSpeed") m_craftSpeedBonus += bonus.second;
                    else if (bonus.first == "quality") m_qualityBonus += bonus.second;
                    else if (bonus.first == "costReduction") m_costReduction += bonus.second;
                    else if (bonus.first == "bonusOutput") m_bonusOutputChance += bonus.second;
                    else if (bonus.first == "critCraft") m_critCraftChance += bonus.second;
                    else if (bonus.first == "gatheringSpeed") m_gatheringSpeed += bonus.second;
                    else if (bonus.first == "bonusResource") m_bonusResourceChance += bonus.second;
                    else if (bonus.first == "rareResource") m_rareResourceChance += bonus.second;
                }
                break;
            }
        }
    }

    // Skill level bonuses (small passive bonuses)
    m_qualityBonus += m_skillLevel * 0.001f;  // 0.1% per level
}

int Profession::CalculateXPForLevel(int level) const {
    // Exponential XP curve
    return static_cast<int>(100 * std::pow(1.1, level - 1));
}

// ProfessionManager implementation
ProfessionManager::ProfessionManager()
    : m_maxProfessions(2)
    , m_maxGatheringProfessions(2)
{
}

ProfessionManager::~ProfessionManager() {
}

Profession* ProfessionManager::GetProfession(ProfessionType type) {
    auto it = m_professions.find(type);
    if (it != m_professions.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Profession* ProfessionManager::GetProfession(ProfessionType type) const {
    auto it = m_professions.find(type);
    if (it != m_professions.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ProfessionManager::HasProfession(ProfessionType type) const {
    return m_professions.find(type) != m_professions.end();
}

bool ProfessionManager::LearnProfession(ProfessionType type) {
    if (HasProfession(type)) {
        return false;
    }

    if (!CanLearnProfession(type)) {
        return false;
    }

    auto profession = std::make_unique<Profession>(type, ProfessionTypeToString(type));
    m_professions[type] = std::move(profession);
    return true;
}

bool ProfessionManager::ForgetProfession(ProfessionType type) {
    auto it = m_professions.find(type);
    if (it != m_professions.end()) {
        m_professions.erase(it);
        return true;
    }
    return false;
}

std::vector<Profession*> ProfessionManager::GetAllProfessions() {
    std::vector<Profession*> result;
    for (auto& pair : m_professions) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::vector<const Profession*> ProfessionManager::GetAllProfessions() const {
    std::vector<const Profession*> result;
    for (const auto& pair : m_professions) {
        result.push_back(pair.second.get());
    }
    return result;
}

int ProfessionManager::GetProfessionCount() const {
    return static_cast<int>(m_professions.size());
}

bool ProfessionManager::CanLearnProfession(ProfessionType type) const {
    if (HasProfession(type)) {
        return false;
    }

    int currentCount = GetProfessionCount();
    if (currentCount >= m_maxProfessions) {
        return false;
    }

    if (IsGatheringProfession(type)) {
        int gatheringCount = 0;
        for (const auto& pair : m_professions) {
            if (IsGatheringProfession(pair.first)) {
                gatheringCount++;
            }
        }
        if (gatheringCount >= m_maxGatheringProfessions) {
            return false;
        }
    }

    return true;
}

bool ProfessionManager::KnowsRecipe(const std::string& recipeId) const {
    for (const auto& pair : m_professions) {
        if (pair.second->KnowsRecipe(recipeId)) {
            return true;
        }
    }
    return false;
}

Profession* ProfessionManager::GetProfessionForRecipe(const std::string& recipeId) {
    for (auto& pair : m_professions) {
        if (pair.second->KnowsRecipe(recipeId)) {
            return pair.second.get();
        }
    }
    return nullptr;
}

float ProfessionManager::GetGlobalCraftSpeedBonus() const {
    float total = 0.0f;
    for (const auto& pair : m_professions) {
        total += pair.second->GetCraftSpeedBonus();
    }
    return total;
}

float ProfessionManager::GetGlobalQualityBonus() const {
    float total = 0.0f;
    for (const auto& pair : m_professions) {
        total += pair.second->GetQualityBonus();
    }
    return total;
}

void ProfessionManager::Initialize() {
    // Initialize with default professions if needed
}

void ProfessionManager::LoadFromData(const std::string& filePath) {
    // Load profession data from JSON file
    // This would be implemented based on your JSON loading system
}

// Helper functions
std::string ProfessionTypeToString(ProfessionType type) {
    switch (type) {
        case ProfessionType::Mining:        return "Mining";
        case ProfessionType::Herbalism:     return "Herbalism";
        case ProfessionType::Logging:       return "Logging";
        case ProfessionType::Skinning:      return "Skinning";
        case ProfessionType::Fishing:       return "Fishing";
        case ProfessionType::Blacksmithing: return "Blacksmithing";
        case ProfessionType::Alchemy:       return "Alchemy";
        case ProfessionType::Enchanting:    return "Enchanting";
        case ProfessionType::Cooking:       return "Cooking";
        case ProfessionType::Tailoring:     return "Tailoring";
        case ProfessionType::Leatherworking:return "Leatherworking";
        case ProfessionType::Engineering:   return "Engineering";
        case ProfessionType::Woodworking:   return "Woodworking";
        case ProfessionType::Jewelcrafting: return "Jewelcrafting";
        case ProfessionType::Inscription:   return "Inscription";
        default: return "Unknown";
    }
}

ProfessionType StringToProfessionType(const std::string& str) {
    if (str == "Mining") return ProfessionType::Mining;
    if (str == "Herbalism") return ProfessionType::Herbalism;
    if (str == "Logging") return ProfessionType::Logging;
    if (str == "Skinning") return ProfessionType::Skinning;
    if (str == "Fishing") return ProfessionType::Fishing;
    if (str == "Blacksmithing") return ProfessionType::Blacksmithing;
    if (str == "Alchemy") return ProfessionType::Alchemy;
    if (str == "Enchanting") return ProfessionType::Enchanting;
    if (str == "Cooking") return ProfessionType::Cooking;
    if (str == "Tailoring") return ProfessionType::Tailoring;
    if (str == "Leatherworking") return ProfessionType::Leatherworking;
    if (str == "Engineering") return ProfessionType::Engineering;
    if (str == "Woodworking") return ProfessionType::Woodworking;
    if (str == "Jewelcrafting") return ProfessionType::Jewelcrafting;
    if (str == "Inscription") return ProfessionType::Inscription;
    return ProfessionType::Mining;
}

std::string SkillTierToString(SkillTier tier) {
    switch (tier) {
        case SkillTier::Apprentice:    return "Apprentice";
        case SkillTier::Journeyman:    return "Journeyman";
        case SkillTier::Expert:        return "Expert";
        case SkillTier::Artisan:       return "Artisan";
        case SkillTier::Master:        return "Master";
        case SkillTier::GrandMaster:   return "Grand Master";
        case SkillTier::Illustrious:   return "Illustrious";
        case SkillTier::Legendary:     return "Legendary";
        default: return "Unknown";
    }
}

SkillTier GetSkillTierForLevel(int level) {
    if (level >= 526) return SkillTier::Legendary;
    if (level >= 451) return SkillTier::Illustrious;
    if (level >= 376) return SkillTier::GrandMaster;
    if (level >= 301) return SkillTier::Master;
    if (level >= 226) return SkillTier::Artisan;
    if (level >= 151) return SkillTier::Expert;
    if (level >= 76) return SkillTier::Journeyman;
    return SkillTier::Apprentice;
}

bool IsGatheringProfession(ProfessionType type) {
    switch (type) {
        case ProfessionType::Mining:
        case ProfessionType::Herbalism:
        case ProfessionType::Logging:
        case ProfessionType::Skinning:
        case ProfessionType::Fishing:
            return true;
        default:
            return false;
    }
}

bool IsCraftingProfession(ProfessionType type) {
    return !IsGatheringProfession(type);
}

} // namespace Crafting
