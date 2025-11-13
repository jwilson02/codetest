#include "CraftingSystem.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Crafting {

// Static instance
CraftingSystem* CraftingSystem::s_instance = nullptr;

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

// CraftingStation implementation
CraftingStation::CraftingStation()
    : m_type(CraftingStationType::Forge)
    , m_level(1)
    , m_isAvailable(true)
    , m_craftSpeedBonus(0.0f)
    , m_qualityBonus(0.0f)
    , m_successBonus(0.0f)
{
}

CraftingStation::CraftingStation(const std::string& id, CraftingStationType type)
    : CraftingStation()
{
    m_id = id;
    m_type = type;
    m_name = CraftingStationTypeToString(type);
}

CraftingStation::~CraftingStation() {
}

bool CraftingStation::SupportsProfession(ProfessionType profession) const {
    return std::find(m_supportedProfessions.begin(), m_supportedProfessions.end(), profession)
           != m_supportedProfessions.end();
}

void CraftingStation::UpgradeStation() {
    m_level++;
    m_craftSpeedBonus += 0.05f;
    m_qualityBonus += 0.02f;
    m_successBonus += 0.01f;
}

int CraftingStation::GetUpgradeCost() const {
    return m_level * 1000;
}

void CraftingStation::AddSupportedProfession(ProfessionType profession) {
    if (!SupportsProfession(profession)) {
        m_supportedProfessions.push_back(profession);
    }
}

// CraftingSystem implementation
CraftingSystem::CraftingSystem()
    : m_initialized(false)
    , m_batchCraftingCount(0)
    , m_currentCraftingProfession(nullptr)
{
    m_craftingProgress.isActive = false;
    m_craftingProgress.timeRemaining = 0.0f;
    m_craftingProgress.totalTime = 0.0f;
}

CraftingSystem::~CraftingSystem() {
}

CraftingSystem& CraftingSystem::GetInstance() {
    if (!s_instance) {
        s_instance = new CraftingSystem();
    }
    return *s_instance;
}

bool CraftingSystem::Initialize() {
    if (m_initialized) {
        return true;
    }

    m_professionManager.Initialize();
    m_initialized = true;
    return true;
}

void CraftingSystem::Shutdown() {
    m_recipes.clear();
    m_stations.clear();
    m_discoveredRecipes.clear();
    m_craftingQueue.clear();
    m_initialized = false;
}

void CraftingSystem::Update(float deltaTime) {
    if (!m_initialized) {
        return;
    }

    // Update gathering system
    m_gatheringSystem.Update(deltaTime);

    // Update crafting progress
    if (m_craftingProgress.isActive) {
        m_craftingProgress.timeRemaining -= deltaTime;

        if (m_craftingProgress.timeRemaining <= 0.0f) {
            // Crafting complete
            auto result = CompleteCrafting();

            // Handle batch crafting
            if (m_batchCraftingCount > 0) {
                m_batchCraftingCount--;
                if (m_batchCraftingCount > 0 && m_currentCraftingProfession) {
                    StartCrafting(m_craftingProgress.recipeId, m_currentCraftingProfession);
                }
            }
        }
    }

    // Process crafting queue
    ProcessCraftingQueue(deltaTime);
}

void CraftingSystem::RegisterRecipe(std::shared_ptr<Recipe> recipe) {
    if (recipe) {
        m_recipes[recipe->GetId()] = recipe;
    }
}

std::shared_ptr<Recipe> CraftingSystem::GetRecipe(const std::string& recipeId) {
    auto it = m_recipes.find(recipeId);
    if (it != m_recipes.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Recipe>> CraftingSystem::GetRecipesByCategory(RecipeCategory category) {
    std::vector<std::shared_ptr<Recipe>> result;
    for (auto& pair : m_recipes) {
        if (pair.second->GetCategory() == category) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Recipe>> CraftingSystem::GetRecipesByProfession(ProfessionType profession) {
    auto categories = GetCategoriesForProfession(profession);
    std::vector<std::shared_ptr<Recipe>> result;

    for (auto& pair : m_recipes) {
        for (auto& category : categories) {
            if (pair.second->GetCategory() == category) {
                result.push_back(pair.second);
                break;
            }
        }
    }

    return result;
}

std::vector<std::shared_ptr<Recipe>> CraftingSystem::GetCraftableRecipes(ProfessionType profession, int skillLevel) {
    auto recipes = GetRecipesByProfession(profession);
    std::vector<std::shared_ptr<Recipe>> craftable;

    for (auto& recipe : recipes) {
        auto& skillReqs = recipe->GetSkillRequirements();
        if (!skillReqs.empty() && skillReqs[0].skillLevel <= skillLevel) {
            craftable.push_back(recipe);
        }
    }

    return craftable;
}

std::vector<std::shared_ptr<Recipe>> CraftingSystem::GetAllRecipes() const {
    std::vector<std::shared_ptr<Recipe>> result;
    for (const auto& pair : m_recipes) {
        result.push_back(pair.second);
    }
    return result;
}

bool CraftingSystem::DiscoverRecipe(const std::string& recipeId) {
    if (IsRecipeDiscovered(recipeId)) {
        return false;
    }

    m_discoveredRecipes.push_back(recipeId);

    if (m_recipeDiscoveredCallback) {
        m_recipeDiscoveredCallback(recipeId);
    }

    return true;
}

bool CraftingSystem::IsRecipeDiscovered(const std::string& recipeId) const {
    return std::find(m_discoveredRecipes.begin(), m_discoveredRecipes.end(), recipeId)
           != m_discoveredRecipes.end();
}

void CraftingSystem::RegisterStation(std::shared_ptr<CraftingStation> station) {
    if (station) {
        m_stations[station->GetId()] = station;
    }
}

std::shared_ptr<CraftingStation> CraftingSystem::GetStation(const std::string& stationId) {
    auto it = m_stations.find(stationId);
    if (it != m_stations.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<CraftingStation>> CraftingSystem::GetNearbyStations(float x, float y, float z, float radius) {
    // In a real implementation, this would check distance
    // For now, return all available stations
    std::vector<std::shared_ptr<CraftingStation>> result;
    for (auto& pair : m_stations) {
        if (pair.second->IsAvailable()) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void CraftingSystem::SetCurrentStation(std::shared_ptr<CraftingStation> station) {
    m_currentStation = station;
}

bool CraftingSystem::StartCrafting(const std::string& recipeId, Profession* profession) {
    if (m_craftingProgress.isActive) {
        return false;
    }

    auto recipe = GetRecipe(recipeId);
    if (!recipe || !profession) {
        return false;
    }

    // Check if profession knows this recipe
    if (!profession->KnowsRecipe(recipeId)) {
        return false;
    }

    // Calculate craft time with bonuses
    float craftTime = recipe->GetCraftTime();

    // Apply profession speed bonus
    float speedBonus = profession->GetCraftSpeedBonus();
    craftTime *= (1.0f - speedBonus);

    // Apply station bonus
    if (m_currentStation) {
        craftTime *= (1.0f - m_currentStation->GetCraftSpeedBonus());
    }

    m_craftingProgress.recipeId = recipeId;
    m_craftingProgress.recipe = recipe;
    m_craftingProgress.totalTime = craftTime;
    m_craftingProgress.timeRemaining = craftTime;
    m_craftingProgress.isActive = true;
    m_currentCraftingProfession = profession;

    if (m_craftingStartCallback) {
        m_craftingStartCallback(recipeId);
    }

    return true;
}

void CraftingSystem::CancelCrafting() {
    m_craftingProgress.isActive = false;
    m_craftingProgress.timeRemaining = 0.0f;
    m_batchCraftingCount = 0;
    m_currentCraftingProfession = nullptr;
}

CraftingSystem::CraftingResult CraftingSystem::CompleteCrafting() {
    CraftingResult result;
    result.success = false;

    if (!m_craftingProgress.isActive || !m_currentCraftingProfession) {
        result.message = "No active crafting";
        return result;
    }

    auto recipe = m_craftingProgress.recipe;
    result = ExecuteCraft(recipe, m_currentCraftingProfession);

    // Clear crafting progress
    m_craftingProgress.isActive = false;
    m_craftingProgress.timeRemaining = 0.0f;

    if (m_batchCraftingCount <= 0) {
        m_currentCraftingProfession = nullptr;
    }

    return result;
}

CraftingSystem::CraftingResult CraftingSystem::CraftItem(const std::string& recipeId, Profession* profession) {
    CraftingResult result;
    result.success = false;

    auto recipe = GetRecipe(recipeId);
    if (!recipe || !profession) {
        result.message = "Invalid recipe or profession";
        return result;
    }

    if (!profession->KnowsRecipe(recipeId)) {
        result.message = "Recipe not learned";
        return result;
    }

    return ExecuteCraft(recipe, profession);
}

void CraftingSystem::StartBatchCrafting(const std::string& recipeId, int count, Profession* profession) {
    m_batchCraftingCount = count;
    StartCrafting(recipeId, profession);
}

std::vector<CraftingSystem::MaterialAvailability> CraftingSystem::CheckMaterials(const std::string& recipeId) {
    std::vector<MaterialAvailability> result;

    auto recipe = GetRecipe(recipeId);
    if (!recipe) {
        return result;
    }

    // In a real implementation, you'd check against player inventory
    for (const auto& req : recipe->GetMaterialRequirements()) {
        MaterialAvailability mat;
        mat.itemId = req.itemId;
        mat.itemName = req.itemName;
        mat.required = req.quantity;
        mat.available = 0;  // Would check inventory
        mat.sufficient = mat.available >= mat.required;
        result.push_back(mat);
    }

    return result;
}

bool CraftingSystem::HasRequiredMaterials(const std::string& recipeId) const {
    // In a real implementation, check against inventory
    return true;
}

void CraftingSystem::LoadRecipesFromFile(const std::string& filePath) {
    // Load recipe data from JSON file
    // Implementation depends on your JSON loading system
}

void CraftingSystem::LoadProfessionsFromFile(const std::string& filePath) {
    m_professionManager.LoadFromData(filePath);
}

void CraftingSystem::LoadMaterialsFromFile(const std::string& filePath) {
    // Load material data from JSON file
}

void CraftingSystem::LoadEnchantmentsFromFile(const std::string& filePath) {
    m_enchantingSystem.LoadEnchantmentsFromFile(filePath);
}

void CraftingSystem::ResetStatistics() {
    m_statistics = CraftingStatistics();
}

bool CraftingSystem::LearnRecipe(const std::string& recipeId, Profession* profession) {
    if (!profession) {
        return false;
    }

    auto recipe = GetRecipe(recipeId);
    if (!recipe) {
        return false;
    }

    if (!CanLearnRecipe(recipeId, profession)) {
        return false;
    }

    return profession->LearnRecipe(recipeId);
}

bool CraftingSystem::CanLearnRecipe(const std::string& recipeId, const Profession* profession) const {
    if (!profession) {
        return false;
    }

    auto it = m_recipes.find(recipeId);
    if (it == m_recipes.end()) {
        return false;
    }

    auto& recipe = it->second;
    auto& skillReqs = recipe->GetSkillRequirements();

    if (!skillReqs.empty()) {
        return profession->GetSkillLevel() >= skillReqs[0].skillLevel;
    }

    return true;
}

void CraftingSystem::QueueCraft(const std::string& recipeId, int count) {
    m_craftingQueue.push_back({recipeId, count});
}

void CraftingSystem::ClearCraftingQueue() {
    m_craftingQueue.clear();
}

void CraftingSystem::ProcessCraftingQueue(float deltaTime) {
    // Auto-process crafting queue
    if (!m_craftingProgress.isActive && !m_craftingQueue.empty() && m_currentCraftingProfession) {
        auto& front = m_craftingQueue.front();
        StartCrafting(front.first, m_currentCraftingProfession);
        front.second--;

        if (front.second <= 0) {
            m_craftingQueue.erase(m_craftingQueue.begin());
        }
    }
}

CraftingSystem::CraftingResult CraftingSystem::ExecuteCraft(std::shared_ptr<Recipe> recipe, Profession* profession) {
    CraftingResult result;
    result.success = false;
    result.xpGained = 0;
    result.criticalCraft = false;
    result.quality = CraftQuality::Normal;

    if (!recipe || !profession) {
        result.message = "Invalid parameters";
        return result;
    }

    // Check success chance
    float successChance = recipe->CalculateSuccessChance(profession->GetSkillLevel());

    // Apply station bonus
    if (m_currentStation) {
        successChance += m_currentStation->GetSuccessBonus();
    }

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(gen);

    m_statistics.totalCrafts++;
    profession->IncrementCrafts();

    if (roll <= successChance) {
        // Success!
        result.success = true;

        // Determine quality
        std::map<std::string, float> materialQualities;  // Would be populated from actual materials
        float luck = dist(gen);
        result.quality = recipe->DetermineQuality(materialQualities, profession->GetSkillLevel(), luck);

        // Apply profession quality bonus
        float qualityBonus = profession->GetQualityBonus();
        if (m_currentStation) {
            qualityBonus += m_currentStation->GetQualityBonus();
        }

        // Check for critical craft
        float critChance = profession->GetCritCraftChance();
        if (dist(gen) <= critChance) {
            result.criticalCraft = true;
            // Upgrade quality on critical
            int qualityLevel = std::min(static_cast<int>(result.quality) + 1,
                                       static_cast<int>(CraftQuality::Legendary));
            result.quality = static_cast<CraftQuality>(qualityLevel);
        }

        // Create output item(s)
        for (int i = 0; i < recipe->GetOutputQuantity(); i++) {
            auto item = CreateItemFromRecipe(recipe, result.quality);
            if (item) {
                result.items.push_back(item);
            }
        }

        // Check for bonus outputs
        GrantBonusOutputs(recipe, profession, result.items);

        // Calculate XP
        auto& skillReqs = recipe->GetSkillRequirements();
        if (!skillReqs.empty()) {
            result.xpGained = skillReqs[0].experienceGained;
            if (result.criticalCraft) {
                result.xpGained *= 2;
            }
            profession->GainXP(result.xpGained);
        }

        // Update statistics
        m_statistics.successfulCrafts++;
        profession->IncrementSuccessfulCrafts();

        if (result.criticalCraft) {
            m_statistics.criticalCrafts++;
        }

        if (result.quality == CraftQuality::Masterwork) {
            profession->IncrementMasterworkCrafts();
        } else if (result.quality == CraftQuality::Legendary) {
            profession->IncrementLegendaryCrafts();
            m_statistics.legendaryQualityCrafts++;
        }

        m_statistics.craftsByCategory[recipe->GetCategory()]++;
        m_statistics.mostCraftedRecipes[recipe->GetId()]++;

        result.message = "Crafting successful!";

        // Consume materials
        ConsumeMaterials(recipe);
    } else {
        // Failure
        result.success = false;
        result.message = "Crafting failed";
        m_statistics.failedCrafts++;

        // May still consume some materials on failure
    }

    if (m_craftingCompleteCallback) {
        m_craftingCompleteCallback(result);
    }

    return result;
}

std::shared_ptr<Inventory::Item> CraftingSystem::CreateItemFromRecipe(std::shared_ptr<Recipe> recipe, CraftQuality quality) {
    // In a real implementation, this would create an actual item based on the recipe output
    // This is a placeholder
    auto item = std::make_shared<Inventory::Item>();
    item->SetId(recipe->GetOutputItemId());
    item->SetName(recipe->GetName());

    // Apply quality multiplier to stats
    float multiplier = GetQualityMultiplier(quality);
    // Apply to item stats...

    return item;
}

bool CraftingSystem::ConsumeMaterials(std::shared_ptr<Recipe> recipe) {
    // In a real implementation, remove materials from inventory
    return true;
}

void CraftingSystem::GrantBonusOutputs(std::shared_ptr<Recipe> recipe, Profession* profession,
                                      std::vector<std::shared_ptr<Inventory::Item>>& items) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float bonusChance = profession->GetBonusOutputChance();

    for (const auto& bonus : recipe->GetBonusOutputs()) {
        if (profession->GetSkillLevel() >= bonus.minSkillLevel) {
            float effectiveChance = bonus.probability + bonusChance;
            if (dist(gen) <= effectiveChance) {
                // Grant bonus output
                for (int i = 0; i < bonus.quantity; i++) {
                    auto item = std::make_shared<Inventory::Item>();
                    item->SetId(bonus.itemId);
                    items.push_back(item);
                }
            }
        }
    }
}

// Helper functions
std::string CraftingStationTypeToString(CraftingStationType type) {
    switch (type) {
        case CraftingStationType::Forge:             return "Forge";
        case CraftingStationType::AnvilStation:      return "Anvil Station";
        case CraftingStationType::AlchemyLab:        return "Alchemy Lab";
        case CraftingStationType::EnchantingTable:   return "Enchanting Table";
        case CraftingStationType::CookingStation:    return "Cooking Station";
        case CraftingStationType::TailoringBench:    return "Tailoring Bench";
        case CraftingStationType::LeatherworkBench:  return "Leatherwork Bench";
        case CraftingStationType::EngineeringBench:  return "Engineering Bench";
        case CraftingStationType::Sawmill:           return "Sawmill";
        case CraftingStationType::JewelryBench:      return "Jewelry Bench";
        case CraftingStationType::ScribeDesk:        return "Scribe Desk";
        case CraftingStationType::ArcaneForge:       return "Arcane Forge";
        case CraftingStationType::RuneForge:         return "Rune Forge";
        case CraftingStationType::GemCuttingStation: return "Gem Cutting Station";
        default: return "Unknown";
    }
}

CraftingStationType StringToCraftingStationType(const std::string& str) {
    // Implementation for conversion
    return CraftingStationType::Forge;
}

ProfessionType GetProfessionForStation(CraftingStationType station) {
    switch (station) {
        case CraftingStationType::Forge:
        case CraftingStationType::AnvilStation:
            return ProfessionType::Blacksmithing;
        case CraftingStationType::AlchemyLab:
            return ProfessionType::Alchemy;
        case CraftingStationType::EnchantingTable:
        case CraftingStationType::ArcaneForge:
        case CraftingStationType::RuneForge:
            return ProfessionType::Enchanting;
        case CraftingStationType::CookingStation:
            return ProfessionType::Cooking;
        case CraftingStationType::TailoringBench:
            return ProfessionType::Tailoring;
        case CraftingStationType::LeatherworkBench:
            return ProfessionType::Leatherworking;
        case CraftingStationType::EngineeringBench:
            return ProfessionType::Engineering;
        case CraftingStationType::Sawmill:
            return ProfessionType::Woodworking;
        case CraftingStationType::JewelryBench:
        case CraftingStationType::GemCuttingStation:
            return ProfessionType::Jewelcrafting;
        case CraftingStationType::ScribeDesk:
            return ProfessionType::Inscription;
        default:
            return ProfessionType::Blacksmithing;
    }
}

std::vector<RecipeCategory> GetCategoriesForProfession(ProfessionType profession) {
    std::vector<RecipeCategory> categories;

    switch (profession) {
        case ProfessionType::Blacksmithing:
            categories.push_back(RecipeCategory::WeaponSmithing);
            categories.push_back(RecipeCategory::ArmorSmithing);
            categories.push_back(RecipeCategory::ToolSmithing);
            break;
        case ProfessionType::Alchemy:
            categories.push_back(RecipeCategory::Potions);
            categories.push_back(RecipeCategory::Elixirs);
            categories.push_back(RecipeCategory::Transmutation);
            categories.push_back(RecipeCategory::Oils);
            break;
        case ProfessionType::Enchanting:
            categories.push_back(RecipeCategory::WeaponEnchants);
            categories.push_back(RecipeCategory::ArmorEnchants);
            categories.push_back(RecipeCategory::RuneCrafting);
            categories.push_back(RecipeCategory::GlyphCrafting);
            break;
        case ProfessionType::Cooking:
            categories.push_back(RecipeCategory::Food);
            categories.push_back(RecipeCategory::Drinks);
            categories.push_back(RecipeCategory::Feast);
            break;
        case ProfessionType::Tailoring:
            categories.push_back(RecipeCategory::LightArmor);
            categories.push_back(RecipeCategory::Robes);
            categories.push_back(RecipeCategory::Bags);
            break;
        case ProfessionType::Leatherworking:
            categories.push_back(RecipeCategory::MediumArmor);
            categories.push_back(RecipeCategory::LeatherGoods);
            break;
        case ProfessionType::Engineering:
            categories.push_back(RecipeCategory::Gadgets);
            categories.push_back(RecipeCategory::Mechanisms);
            categories.push_back(RecipeCategory::Explosives);
            break;
        case ProfessionType::Woodworking:
            categories.push_back(RecipeCategory::Bows);
            categories.push_back(RecipeCategory::Staves);
            categories.push_back(RecipeCategory::Furniture);
            break;
        case ProfessionType::Jewelcrafting:
            categories.push_back(RecipeCategory::Rings);
            categories.push_back(RecipeCategory::Amulets);
            categories.push_back(RecipeCategory::Trinkets);
            categories.push_back(RecipeCategory::JewelrySmithing);
            break;
        default:
            break;
    }

    return categories;
}

} // namespace Crafting
