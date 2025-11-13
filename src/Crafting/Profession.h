#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace Crafting {

// Profession types
enum class ProfessionType {
    // Gathering professions
    Mining,
    Herbalism,
    Logging,
    Skinning,
    Fishing,

    // Crafting professions
    Blacksmithing,
    Alchemy,
    Enchanting,
    Cooking,
    Tailoring,
    Leatherworking,
    Engineering,
    Woodworking,
    Jewelcrafting,
    Inscription
};

// Skill tier
enum class SkillTier {
    Apprentice,      // 1-75
    Journeyman,      // 76-150
    Expert,          // 151-225
    Artisan,         // 226-300
    Master,          // 301-375
    GrandMaster,     // 376-450
    Illustrious,     // 451-525
    Legendary        // 526-600
};

// Specialization
struct Specialization {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> bonusRecipes;  // Exclusive recipes
    std::map<std::string, float> bonuses;    // Stat bonuses (e.g., "craftSpeed": 0.2)
    int requiredSkillLevel;
    bool isLocked;
};

// Skill perk (passive bonuses)
struct SkillPerk {
    std::string id;
    std::string name;
    std::string description;
    int unlockLevel;
    std::string type;           // "quality", "speed", "bonus_output", "cost_reduction"
    float value;
    bool isUnlocked;
};

// Profession quest
struct ProfessionQuest {
    std::string questId;
    std::string name;
    std::string description;
    int requiredLevel;
    std::vector<std::string> rewards;  // Recipe IDs, perk IDs, etc.
    bool isCompleted;
};

/**
 * @brief Represents a crafting or gathering profession
 */
class Profession {
public:
    Profession();
    Profession(ProfessionType type, const std::string& name);
    ~Profession();

    // Getters
    ProfessionType GetType() const { return m_type; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetDescription() const { return m_description; }
    const std::string& GetIconPath() const { return m_iconPath; }

    // Skill level
    int GetSkillLevel() const { return m_skillLevel; }
    int GetSkillXP() const { return m_skillXP; }
    int GetMaxSkillLevel() const { return m_maxSkillLevel; }
    SkillTier GetCurrentTier() const;
    int GetXPForNextLevel() const;
    float GetProgressToNextLevel() const;

    // Specialization
    const std::string& GetSpecializationId() const { return m_specializationId; }
    bool HasSpecialization() const { return !m_specializationId.empty(); }
    const std::vector<Specialization>& GetAvailableSpecializations() const { return m_specializations; }
    bool CanSpecialize() const;
    bool SetSpecialization(const std::string& specId);

    // Perks
    const std::vector<SkillPerk>& GetPerks() const { return m_perks; }
    std::vector<SkillPerk> GetUnlockedPerks() const;
    float GetPerkBonus(const std::string& type) const;
    bool UnlockPerk(const std::string& perkId);

    // Known recipes
    const std::vector<std::string>& GetKnownRecipes() const { return m_knownRecipes; }
    bool KnowsRecipe(const std::string& recipeId) const;
    bool LearnRecipe(const std::string& recipeId);
    void ForgetRecipe(const std::string& recipeId);

    // Quests
    const std::vector<ProfessionQuest>& GetQuests() const { return m_quests; }
    std::vector<ProfessionQuest> GetAvailableQuests() const;
    bool CompleteQuest(const std::string& questId);

    // XP and leveling
    void GainXP(int amount);
    bool LevelUp();
    void SetSkillLevel(int level);

    // Crafting bonuses
    float GetCraftSpeedBonus() const { return m_craftSpeedBonus; }
    float GetQualityBonus() const { return m_qualityBonus; }
    float GetCostReduction() const { return m_costReduction; }
    float GetBonusOutputChance() const { return m_bonusOutputChance; }
    float GetCritCraftChance() const { return m_critCraftChance; }

    // Gathering bonuses (for gathering professions)
    float GetGatheringSpeed() const { return m_gatheringSpeed; }
    float GetBonusResourceChance() const { return m_bonusResourceChance; }
    float GetRareResourceChance() const { return m_rareResourceChance; }

    // Setters
    void SetName(const std::string& name) { m_name = name; }
    void SetDescription(const std::string& desc) { m_description = desc; }
    void SetIconPath(const std::string& path) { m_iconPath = path; }
    void SetMaxSkillLevel(int level) { m_maxSkillLevel = level; }

    void SetCraftSpeedBonus(float bonus) { m_craftSpeedBonus = bonus; }
    void SetQualityBonus(float bonus) { m_qualityBonus = bonus; }
    void SetCostReduction(float reduction) { m_costReduction = reduction; }
    void SetBonusOutputChance(float chance) { m_bonusOutputChance = chance; }
    void SetCritCraftChance(float chance) { m_critCraftChance = chance; }

    void SetGatheringSpeed(float speed) { m_gatheringSpeed = speed; }
    void SetBonusResourceChance(float chance) { m_bonusResourceChance = chance; }
    void SetRareResourceChance(float chance) { m_rareResourceChance = chance; }

    // Add content
    void AddSpecialization(const Specialization& spec);
    void AddPerk(const SkillPerk& perk);
    void AddQuest(const ProfessionQuest& quest);

    // Statistics
    int GetTotalCrafts() const { return m_totalCrafts; }
    int GetSuccessfulCrafts() const { return m_successfulCrafts; }
    int GetMasterworkCrafts() const { return m_masterworkCrafts; }
    int GetLegendaryCrafts() const { return m_legendaryCrafts; }
    float GetSuccessRate() const;

    void IncrementCrafts();
    void IncrementSuccessfulCrafts();
    void IncrementMasterworkCrafts();
    void IncrementLegendaryCrafts();

    // Callbacks
    void SetLevelUpCallback(std::function<void(int)> callback) { m_levelUpCallback = callback; }
    void SetXPGainCallback(std::function<void(int, int)> callback) { m_xpGainCallback = callback; }

private:
    void RecalculateBonuses();
    int CalculateXPForLevel(int level) const;

private:
    // Basic info
    ProfessionType m_type;
    std::string m_name;
    std::string m_description;
    std::string m_iconPath;

    // Skill progression
    int m_skillLevel;
    int m_skillXP;
    int m_maxSkillLevel;

    // Specialization
    std::string m_specializationId;
    std::vector<Specialization> m_specializations;

    // Perks
    std::vector<SkillPerk> m_perks;

    // Known recipes
    std::vector<std::string> m_knownRecipes;

    // Quests
    std::vector<ProfessionQuest> m_quests;

    // Bonuses (calculated from perks, specialization, etc.)
    float m_craftSpeedBonus;
    float m_qualityBonus;
    float m_costReduction;
    float m_bonusOutputChance;
    float m_critCraftChance;

    // Gathering bonuses
    float m_gatheringSpeed;
    float m_bonusResourceChance;
    float m_rareResourceChance;

    // Statistics
    int m_totalCrafts;
    int m_successfulCrafts;
    int m_masterworkCrafts;
    int m_legendaryCrafts;

    // Callbacks
    std::function<void(int)> m_levelUpCallback;
    std::function<void(int, int)> m_xpGainCallback;  // (xp gained, new total xp)
};

/**
 * @brief Manages all professions for a player
 */
class ProfessionManager {
public:
    ProfessionManager();
    ~ProfessionManager();

    // Profession access
    Profession* GetProfession(ProfessionType type);
    const Profession* GetProfession(ProfessionType type) const;
    bool HasProfession(ProfessionType type) const;
    bool LearnProfession(ProfessionType type);
    bool ForgetProfession(ProfessionType type);

    // All professions
    std::vector<Profession*> GetAllProfessions();
    std::vector<const Profession*> GetAllProfessions() const;
    int GetProfessionCount() const;

    // Limits
    int GetMaxProfessions() const { return m_maxProfessions; }
    void SetMaxProfessions(int max) { m_maxProfessions = max; }
    int GetMaxGatheringProfessions() const { return m_maxGatheringProfessions; }
    void SetMaxGatheringProfessions(int max) { m_maxGatheringProfessions = max; }
    bool CanLearnProfession(ProfessionType type) const;

    // Recipe access (searches all professions)
    bool KnowsRecipe(const std::string& recipeId) const;
    Profession* GetProfessionForRecipe(const std::string& recipeId);

    // Global bonuses
    float GetGlobalCraftSpeedBonus() const;
    float GetGlobalQualityBonus() const;

    // Initialization
    void Initialize();
    void LoadFromData(const std::string& filePath);

private:
    std::map<ProfessionType, std::unique_ptr<Profession>> m_professions;
    int m_maxProfessions;
    int m_maxGatheringProfessions;
};

// Helper functions
std::string ProfessionTypeToString(ProfessionType type);
ProfessionType StringToProfessionType(const std::string& str);
std::string SkillTierToString(SkillTier tier);
SkillTier GetSkillTierForLevel(int level);
bool IsGatheringProfession(ProfessionType type);
bool IsCraftingProfession(ProfessionType type);

} // namespace Crafting
