#pragma once

#include "Recipe.h"
#include "Profession.h"
#include "Gathering.h"
#include "Enchanting.h"
#include "../Inventory/Item.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace Crafting {

// Crafting station types
enum class CraftingStationType {
    Forge,              // Blacksmithing
    AnvilStation,       // Advanced blacksmithing
    AlchemyLab,         // Alchemy
    EnchantingTable,    // Enchanting
    CookingStation,     // Cooking
    TailoringBench,     // Tailoring
    LeatherworkBench,   // Leatherworking
    EngineeringBench,   // Engineering
    Sawmill,            // Woodworking
    JewelryBench,       // Jewelcrafting
    ScribeDesk,         // Inscription
    ArcaneForge,        // Special/Legendary crafting
    RuneForge,          // Rune crafting
    GemCuttingStation   // Gem processing
};

/**
 * @brief Represents a crafting station in the world
 */
class CraftingStation {
public:
    CraftingStation();
    CraftingStation(const std::string& id, CraftingStationType type);
    ~CraftingStation();

    const std::string& GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    CraftingStationType GetType() const { return m_type; }
    int GetLevel() const { return m_level; }
    bool IsAvailable() const { return m_isAvailable; }

    // Supported professions
    const std::vector<ProfessionType>& GetSupportedProfessions() const { return m_supportedProfessions; }
    bool SupportsProfession(ProfessionType profession) const;

    // Bonuses
    float GetCraftSpeedBonus() const { return m_craftSpeedBonus; }
    float GetQualityBonus() const { return m_qualityBonus; }
    float GetSuccessBonus() const { return m_successBonus; }

    // Upgrade
    void UpgradeStation();
    int GetUpgradeCost() const;

    // Setters
    void SetId(const std::string& id) { m_id = id; }
    void SetName(const std::string& name) { m_name = name; }
    void SetType(CraftingStationType type) { m_type = type; }
    void SetLevel(int level) { m_level = level; }
    void SetAvailable(bool available) { m_isAvailable = available; }
    void AddSupportedProfession(ProfessionType profession);

private:
    std::string m_id;
    std::string m_name;
    CraftingStationType m_type;
    int m_level;
    bool m_isAvailable;
    std::vector<ProfessionType> m_supportedProfessions;

    // Station bonuses
    float m_craftSpeedBonus;
    float m_qualityBonus;
    float m_successBonus;
};

/**
 * @brief Main crafting system that coordinates all crafting activities
 */
class CraftingSystem {
public:
    static CraftingSystem& GetInstance();

    // Initialization
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);

    // Recipe management
    void RegisterRecipe(std::shared_ptr<Recipe> recipe);
    std::shared_ptr<Recipe> GetRecipe(const std::string& recipeId);
    std::vector<std::shared_ptr<Recipe>> GetRecipesByCategory(RecipeCategory category);
    std::vector<std::shared_ptr<Recipe>> GetRecipesByProfession(ProfessionType profession);
    std::vector<std::shared_ptr<Recipe>> GetCraftableRecipes(ProfessionType profession, int skillLevel);
    std::vector<std::shared_ptr<Recipe>> GetAllRecipes() const;

    // Recipe discovery
    bool DiscoverRecipe(const std::string& recipeId);
    bool IsRecipeDiscovered(const std::string& recipeId) const;
    std::vector<std::string> GetDiscoveredRecipes() const { return m_discoveredRecipes; }

    // Crafting station management
    void RegisterStation(std::shared_ptr<CraftingStation> station);
    std::shared_ptr<CraftingStation> GetStation(const std::string& stationId);
    std::vector<std::shared_ptr<CraftingStation>> GetNearbyStations(float x, float y, float z, float radius);
    std::shared_ptr<CraftingStation> GetCurrentStation() const { return m_currentStation; }
    void SetCurrentStation(std::shared_ptr<CraftingStation> station);

    // Crafting process
    struct CraftingProgress {
        std::string recipeId;
        float timeRemaining;
        float totalTime;
        bool isActive;
        std::shared_ptr<Recipe> recipe;
    };

    struct CraftingResult {
        bool success;
        std::vector<std::shared_ptr<Inventory::Item>> items;
        int xpGained;
        CraftQuality quality;
        bool criticalCraft;
        std::string message;
    };

    bool StartCrafting(const std::string& recipeId, Profession* profession);
    void CancelCrafting();
    CraftingResult CompleteCrafting();
    const CraftingProgress& GetCraftingProgress() const { return m_craftingProgress; }
    bool IsCrafting() const { return m_craftingProgress.isActive; }

    // Instant crafting (for testing or simple recipes)
    CraftingResult CraftItem(const std::string& recipeId, Profession* profession);

    // Batch crafting
    void StartBatchCrafting(const std::string& recipeId, int count, Profession* profession);
    int GetBatchCraftingRemaining() const { return m_batchCraftingCount; }

    // Material checking
    struct MaterialAvailability {
        std::string itemId;
        std::string itemName;
        int required;
        int available;
        bool sufficient;
    };

    std::vector<MaterialAvailability> CheckMaterials(const std::string& recipeId);
    bool HasRequiredMaterials(const std::string& recipeId) const;

    // Subsystem access
    ProfessionManager& GetProfessionManager() { return m_professionManager; }
    const ProfessionManager& GetProfessionManager() const { return m_professionManager; }

    GatheringSystem& GetGatheringSystem() { return m_gatheringSystem; }
    const GatheringSystem& GetGatheringSystem() const { return m_gatheringSystem; }

    EnchantingSystem& GetEnchantingSystem() { return m_enchantingSystem; }
    const EnchantingSystem& GetEnchantingSystem() const { return m_enchantingSystem; }

    // Data loading
    void LoadRecipesFromFile(const std::string& filePath);
    void LoadProfessionsFromFile(const std::string& filePath);
    void LoadMaterialsFromFile(const std::string& filePath);
    void LoadEnchantmentsFromFile(const std::string& filePath);

    // Statistics
    struct CraftingStatistics {
        int totalCrafts = 0;
        int successfulCrafts = 0;
        int failedCrafts = 0;
        int criticalCrafts = 0;
        int legendaryQualityCrafts = 0;
        std::map<RecipeCategory, int> craftsByCategory;
        std::map<std::string, int> mostCraftedRecipes;
    };

    const CraftingStatistics& GetStatistics() const { return m_statistics; }
    void ResetStatistics();

    // Callbacks
    void SetCraftingStartCallback(std::function<void(const std::string&)> callback) {
        m_craftingStartCallback = callback;
    }

    void SetCraftingCompleteCallback(std::function<void(const CraftingResult&)> callback) {
        m_craftingCompleteCallback = callback;
    }

    void SetRecipeDiscoveredCallback(std::function<void(const std::string&)> callback) {
        m_recipeDiscoveredCallback = callback;
    }

    // Recipe learning
    bool LearnRecipe(const std::string& recipeId, Profession* profession);
    bool CanLearnRecipe(const std::string& recipeId, const Profession* profession) const;

    // Auto-crafting/queuing system
    void QueueCraft(const std::string& recipeId, int count = 1);
    void ClearCraftingQueue();
    const std::vector<std::pair<std::string, int>>& GetCraftingQueue() const { return m_craftingQueue; }

private:
    CraftingSystem();
    ~CraftingSystem();
    CraftingSystem(const CraftingSystem&) = delete;
    CraftingSystem& operator=(const CraftingSystem&) = delete;

    void ProcessCraftingQueue(float deltaTime);
    CraftingResult ExecuteCraft(std::shared_ptr<Recipe> recipe, Profession* profession);
    std::shared_ptr<Inventory::Item> CreateItemFromRecipe(std::shared_ptr<Recipe> recipe, CraftQuality quality);
    bool ConsumeMaterials(std::shared_ptr<Recipe> recipe);
    void GrantBonusOutputs(std::shared_ptr<Recipe> recipe, Profession* profession,
                          std::vector<std::shared_ptr<Inventory::Item>>& items);

private:
    bool m_initialized;

    // Recipe database
    std::map<std::string, std::shared_ptr<Recipe>> m_recipes;
    std::vector<std::string> m_discoveredRecipes;

    // Crafting stations
    std::map<std::string, std::shared_ptr<CraftingStation>> m_stations;
    std::shared_ptr<CraftingStation> m_currentStation;

    // Subsystems
    ProfessionManager m_professionManager;
    GatheringSystem m_gatheringSystem;
    EnchantingSystem m_enchantingSystem;

    // Crafting state
    CraftingProgress m_craftingProgress;
    int m_batchCraftingCount;
    Profession* m_currentCraftingProfession;

    // Crafting queue
    std::vector<std::pair<std::string, int>> m_craftingQueue;  // recipeId, count

    // Statistics
    CraftingStatistics m_statistics;

    // Callbacks
    std::function<void(const std::string&)> m_craftingStartCallback;
    std::function<void(const CraftingResult&)> m_craftingCompleteCallback;
    std::function<void(const std::string&)> m_recipeDiscoveredCallback;

    // Singleton instance
    static CraftingSystem* s_instance;
};

// Helper functions
std::string CraftingStationTypeToString(CraftingStationType type);
CraftingStationType StringToCraftingStationType(const std::string& str);
ProfessionType GetProfessionForStation(CraftingStationType station);
std::vector<RecipeCategory> GetCategoriesForProfession(ProfessionType profession);

} // namespace Crafting
