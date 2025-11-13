#include "Enchanting.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace Crafting {

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

// Enchantment implementation
Enchantment::Enchantment()
    : m_type(EnchantmentType::WeaponDamage)
    , m_slot(EnchantmentSlot::Any)
    , m_level(1)
    , m_maxLevel(10)
    , m_requiredLevel(1)
    , m_requiredEnchantingSkill(1)
    , m_goldCost(0)
    , m_manaCost(0)
    , m_procChance(0.0f)
{
}

Enchantment::Enchantment(const std::string& id, const std::string& name, EnchantmentType type)
    : Enchantment()
{
    m_id = id;
    m_name = name;
    m_type = type;
}

Enchantment::~Enchantment() {
}

bool Enchantment::CanEnchantItem(const Inventory::Item& item) const {
    // Check item level requirement
    if (item.GetLevel() < m_requiredLevel) {
        return false;
    }

    // Check slot compatibility
    if (m_slot != EnchantmentSlot::Any) {
        Inventory::ItemType itemType = item.GetType();
        switch (m_slot) {
            case EnchantmentSlot::Weapon:
                if (itemType != Inventory::ItemType::Weapon) return false;
                break;
            case EnchantmentSlot::Helmet:
                if (itemType != Inventory::ItemType::Helmet) return false;
                break;
            case EnchantmentSlot::Chest:
                if (itemType != Inventory::ItemType::Chest) return false;
                break;
            case EnchantmentSlot::Legs:
                if (itemType != Inventory::ItemType::Legs) return false;
                break;
            case EnchantmentSlot::Boots:
                if (itemType != Inventory::ItemType::Boots) return false;
                break;
            case EnchantmentSlot::Ring:
                if (itemType != Inventory::ItemType::Ring) return false;
                break;
            case EnchantmentSlot::Amulet:
                if (itemType != Inventory::ItemType::Amulet) return false;
                break;
            default:
                break;
        }
    }

    return true;
}

bool Enchantment::ConflictsWith(const Enchantment& other) const {
    // Check if this enchantment conflicts with another
    return std::find(m_conflictingEnchants.begin(), m_conflictingEnchants.end(), other.GetId())
           != m_conflictingEnchants.end();
}

void Enchantment::AddMaterialCost(const std::string& itemId, int quantity) {
    m_materialCost.push_back({itemId, quantity});
}

void Enchantment::AddConflictingEnchant(const std::string& enchantId) {
    m_conflictingEnchants.push_back(enchantId);
}

// Rune implementation
Rune::Rune()
    : m_type(RuneType::Fire)
    , m_quality(RuneQuality::Normal)
    , m_level(1)
{
}

Rune::Rune(RuneType type, RuneQuality quality)
    : m_type(type)
    , m_quality(quality)
    , m_level(1)
{
}

Rune::~Rune() {
}

const std::string& Rune::GetName() const {
    static std::string name;
    name = RuneQualityToString(m_quality) + " " + RuneTypeToString(m_type) + " Rune";
    return name;
}

std::shared_ptr<Rune> Rune::CombineRunes(const Rune& rune1, const Rune& rune2) {
    if (!CanCombine(rune1, rune2)) {
        return nullptr;
    }

    auto result = std::make_shared<Rune>();
    result->SetType(rune1.GetType());

    // Quality improves when combining
    int qualityLevel = static_cast<int>(rune1.GetQuality()) + 1;
    qualityLevel = std::min(qualityLevel, static_cast<int>(RuneQuality::Radiant));
    result->SetQuality(static_cast<RuneQuality>(qualityLevel));

    result->SetLevel(rune1.GetLevel());

    // Combine stats
    auto stats = rune1.GetStats();
    stats += rune2.GetStats();
    result->SetStats(stats);

    return result;
}

bool Rune::CanCombine(const Rune& rune1, const Rune& rune2) {
    // Must be same type and quality
    return rune1.GetType() == rune2.GetType() &&
           rune1.GetQuality() == rune2.GetQuality() &&
           rune1.GetQuality() != RuneQuality::Radiant;  // Can't upgrade radiant
}

// Gem implementation
Gem::Gem()
    : m_type(GemType::Ruby)
    , m_tier(1)
{
}

Gem::Gem(GemType type, int tier)
    : m_type(type)
    , m_tier(tier)
{
}

Gem::~Gem() {
}

const std::string& Gem::GetName() const {
    static std::string name;
    name = GemTypeToString(m_type) + " (Tier " + std::to_string(m_tier) + ")";
    return name;
}

std::shared_ptr<Gem> Gem::UpgradeGem(const Gem& gem) {
    if (gem.GetTier() >= GetMaxTier()) {
        return nullptr;
    }

    auto upgraded = std::make_shared<Gem>();
    upgraded->SetType(gem.GetType());
    upgraded->SetTier(gem.GetTier() + 1);

    // Stats scale with tier
    auto stats = gem.GetStats();
    // Increase stats by 20% per tier
    // This is a simplified version; you'd calculate based on gem type
    upgraded->SetStats(stats);

    return upgraded;
}

// EnchantingSystem implementation
EnchantingSystem::EnchantingSystem() {
}

EnchantingSystem::~EnchantingSystem() {
}

void EnchantingSystem::RegisterEnchantment(std::shared_ptr<Enchantment> enchant) {
    if (enchant) {
        m_enchantments[enchant->GetId()] = enchant;
    }
}

std::shared_ptr<Enchantment> EnchantingSystem::GetEnchantment(const std::string& enchantId) {
    auto it = m_enchantments.find(enchantId);
    if (it != m_enchantments.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Enchantment>> EnchantingSystem::GetEnchantmentsForSlot(EnchantmentSlot slot) {
    std::vector<std::shared_ptr<Enchantment>> result;
    for (auto& pair : m_enchantments) {
        if (pair.second->GetSlot() == slot || pair.second->GetSlot() == EnchantmentSlot::Any) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Enchantment>> EnchantingSystem::GetAvailableEnchantments(int skillLevel) {
    std::vector<std::shared_ptr<Enchantment>> result;
    for (auto& pair : m_enchantments) {
        if (pair.second->GetRequiredEnchantingSkill() <= skillLevel) {
            result.push_back(pair.second);
        }
    }
    return result;
}

EnchantingSystem::EnchantResult EnchantingSystem::EnchantItem(std::shared_ptr<Inventory::Item> item,
                                                              const std::string& enchantId,
                                                              int enchantingSkill) {
    EnchantResult result;
    result.success = false;
    result.xpGained = 0;
    result.enchantedItem = item;

    auto enchant = GetEnchantment(enchantId);
    if (!enchant) {
        result.message = "Enchantment not found";
        return result;
    }

    if (!item) {
        result.message = "Invalid item";
        return result;
    }

    // Check skill requirement
    if (enchantingSkill < enchant->GetRequiredEnchantingSkill()) {
        result.message = "Enchanting skill too low";
        return result;
    }

    // Check if enchantment can be applied
    if (!enchant->CanEnchantItem(*item)) {
        result.message = "Cannot enchant this item type";
        return result;
    }

    // Calculate success chance
    float successChance = CalculateEnchantSuccessChance(*enchant, enchantingSkill);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    bool success = dist(gen) <= successChance;

    m_stats.totalEnchants++;

    if (success) {
        // Apply enchantment (in a real implementation, you'd add enchantment data to item)
        auto bonusStats = item->GetBonusStats();
        bonusStats += enchant->GetBonusStats();
        item->SetBonusStats(bonusStats);

        result.success = true;
        result.message = "Item successfully enchanted!";
        result.xpGained = CalculateEnchantingXP(*enchant, true);
        m_stats.successfulEnchants++;
    } else {
        result.message = "Enchanting failed";
        result.xpGained = CalculateEnchantingXP(*enchant, false) / 2;
        m_stats.failedEnchants++;
    }

    if (m_enchantCompleteCallback) {
        m_enchantCompleteCallback(result);
    }

    return result;
}

bool EnchantingSystem::CanEnchantItem(const Inventory::Item& item, const std::string& enchantId) const {
    auto it = m_enchantments.find(enchantId);
    if (it != m_enchantments.end()) {
        return it->second->CanEnchantItem(item);
    }
    return false;
}

int EnchantingSystem::GetEnchantmentCount(const Inventory::Item& item) const {
    // In a real implementation, this would count actual enchantments on the item
    return 0;
}

int EnchantingSystem::GetMaxEnchantments(const Inventory::Item& item) const {
    // Max enchantments based on item rarity
    switch (item.GetRarity()) {
        case Inventory::ItemRarity::Common:     return 1;
        case Inventory::ItemRarity::Uncommon:   return 2;
        case Inventory::ItemRarity::Rare:       return 3;
        case Inventory::ItemRarity::Epic:       return 4;
        case Inventory::ItemRarity::Legendary:  return 5;
        case Inventory::ItemRarity::Mythic:     return 6;
        default: return 1;
    }
}

EnchantingSystem::DisenchantResult EnchantingSystem::DisenchantItem(std::shared_ptr<Inventory::Item> item,
                                                                     int enchantingSkill) {
    DisenchantResult result;
    result.success = false;
    result.xpGained = 0;

    if (!item) {
        return result;
    }

    // Calculate materials based on item rarity and level
    int materialAmount = item->GetLevel() / 10 + 1;

    switch (item->GetRarity()) {
        case Inventory::ItemRarity::Uncommon:
            result.materials.push_back({"essence_minor", materialAmount});
            break;
        case Inventory::ItemRarity::Rare:
            result.materials.push_back({"essence_lesser", materialAmount});
            break;
        case Inventory::ItemRarity::Epic:
            result.materials.push_back({"essence_greater", materialAmount});
            break;
        case Inventory::ItemRarity::Legendary:
            result.materials.push_back({"essence_superior", materialAmount});
            break;
        case Inventory::ItemRarity::Mythic:
            result.materials.push_back({"essence_supreme", materialAmount});
            break;
        default:
            break;
    }

    if (!result.materials.empty()) {
        result.success = true;
        result.xpGained = item->GetLevel() * 5;
        m_stats.itemsDisenchanted++;
    }

    return result;
}

bool EnchantingSystem::SocketGem(std::shared_ptr<Inventory::Item> item, std::shared_ptr<Gem> gem, int socketIndex) {
    if (!item || !gem) {
        return false;
    }

    // Check if item has enough sockets
    if (socketIndex >= item->GetSocketCount()) {
        return false;
    }

    // In a real implementation, you would add the gem to the item
    // For now, just add the gem's stats to the item
    auto bonusStats = item->GetBonusStats();
    bonusStats += gem->GetStats();
    item->SetBonusStats(bonusStats);

    m_stats.gemsSocketed++;
    return true;
}

bool EnchantingSystem::RemoveGem(std::shared_ptr<Inventory::Item> item, int socketIndex) {
    if (!item || socketIndex >= item->GetSocketCount()) {
        return false;
    }

    // Remove gem from item (implementation depends on your system)
    return true;
}

bool EnchantingSystem::AddSocketToItem(std::shared_ptr<Inventory::Item> item, int enchantingSkill) {
    if (!item) {
        return false;
    }

    if (item->GetSocketCount() >= item->GetMaxSockets()) {
        return false;
    }

    // Success chance decreases with each socket added
    float baseChance = 0.8f;
    float penalty = item->GetSocketCount() * 0.15f;
    float skillBonus = enchantingSkill * 0.001f;
    float successChance = baseChance - penalty + skillBonus;

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(gen) <= successChance) {
        item->AddSocket();
        m_stats.socketsAdded++;
        return true;
    }

    return false;
}

std::shared_ptr<Rune> EnchantingSystem::CombineRunes(std::shared_ptr<Rune> rune1, std::shared_ptr<Rune> rune2) {
    if (!rune1 || !rune2) {
        return nullptr;
    }

    return Rune::CombineRunes(*rune1, *rune2);
}

std::shared_ptr<Enchantment> EnchantingSystem::RuneToEnchantment(std::shared_ptr<Rune> rune) {
    if (!rune) {
        return nullptr;
    }

    // Convert rune to enchantment
    auto enchant = std::make_shared<Enchantment>();
    enchant->SetName(rune->GetName() + " Enchantment");
    enchant->SetBonusStats(rune->GetStats());
    enchant->SetLevel(rune->GetLevel());

    return enchant;
}

EnchantingSystem::UpgradeResult EnchantingSystem::UpgradeItem(std::shared_ptr<Inventory::Item> item,
                                                              const std::vector<std::pair<std::string, int>>& materials,
                                                              int enchantingSkill) {
    UpgradeResult result;
    result.success = false;
    result.upgradedItem = item;
    result.itemDestroyed = false;
    result.successChance = 0.0f;

    if (!item) {
        result.message = "Invalid item";
        return result;
    }

    // Calculate success chance
    result.successChance = CalculateUpgradeSuccessChance(*item, enchantingSkill);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(gen);

    m_stats.itemsUpgraded++;

    if (roll <= result.successChance) {
        // Success - increase item level
        item->SetLevel(item->GetLevel() + 1);

        // Boost stats by 10%
        auto stats = item->GetBaseStats();
        // Apply 10% increase (simplified)
        item->SetBaseStats(stats);

        result.success = true;
        result.message = "Item successfully upgraded!";
    } else if (roll > result.successChance + 0.3f) {
        // Critical failure - item destroyed
        result.itemDestroyed = true;
        result.message = "Upgrade failed catastrophically! Item destroyed!";
        result.upgradedItem = nullptr;
        m_stats.itemsDestroyed++;
    } else {
        // Normal failure - no change
        result.message = "Upgrade failed, but item is intact.";
    }

    if (m_upgradeCompleteCallback) {
        m_upgradeCompleteCallback(result);
    }

    return result;
}

EnchantingSystem::UpgradeResult EnchantingSystem::ImproveQuality(std::shared_ptr<Inventory::Item> item,
                                                                 const std::vector<std::pair<std::string, int>>& materials,
                                                                 int enchantingSkill) {
    UpgradeResult result;
    result.success = false;
    result.upgradedItem = item;
    result.itemDestroyed = false;

    if (!item) {
        result.message = "Invalid item";
        return result;
    }

    // Can't improve beyond Mythic
    if (item->GetRarity() >= Inventory::ItemRarity::Mythic) {
        result.message = "Item is already maximum quality";
        return result;
    }

    // Success chance decreases with higher rarity
    float baseChance = 0.5f;
    float rarityPenalty = static_cast<int>(item->GetRarity()) * 0.1f;
    float skillBonus = enchantingSkill * 0.002f;
    result.successChance = baseChance - rarityPenalty + skillBonus;

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(gen) <= result.successChance) {
        // Increase rarity
        int newRarity = static_cast<int>(item->GetRarity()) + 1;
        item->SetRarity(static_cast<Inventory::ItemRarity>(newRarity));

        result.success = true;
        result.message = "Item quality improved!";
    } else {
        result.message = "Quality improvement failed.";
    }

    return result;
}

EnchantingSystem::ReforgeResult EnchantingSystem::ReforgeItem(std::shared_ptr<Inventory::Item> item,
                                                              const std::vector<std::pair<std::string, int>>& materials) {
    ReforgeResult result;
    result.success = false;
    result.reforgedItem = item;

    if (!item) {
        return result;
    }

    // Reroll affixes (in a real implementation, you'd regenerate affixes)
    // This is a placeholder
    result.success = true;

    return result;
}

void EnchantingSystem::LoadEnchantmentsFromFile(const std::string& filePath) {
    // Load enchantment data from JSON file
    // Implementation depends on your JSON loading system
}

void EnchantingSystem::ResetStats() {
    m_stats = EnchantingStats();
}

float EnchantingSystem::CalculateEnchantSuccessChance(const Enchantment& enchant, int skillLevel) const {
    float baseChance = 0.7f;

    // Skill difference
    int skillDiff = skillLevel - enchant.GetRequiredEnchantingSkill();
    float skillBonus = skillDiff * 0.01f;

    // Level penalty
    float levelPenalty = enchant.GetLevel() * 0.02f;

    float chance = baseChance + skillBonus - levelPenalty;
    return std::max(0.1f, std::min(0.95f, chance));
}

float EnchantingSystem::CalculateUpgradeSuccessChance(const Inventory::Item& item, int skillLevel) const {
    float baseChance = 0.6f;

    // Item level penalty
    float levelPenalty = item.GetLevel() * 0.005f;

    // Rarity penalty
    float rarityPenalty = static_cast<int>(item.GetRarity()) * 0.05f;

    // Skill bonus
    float skillBonus = skillLevel * 0.001f;

    float chance = baseChance - levelPenalty - rarityPenalty + skillBonus;
    return std::max(0.05f, std::min(0.9f, chance));
}

int EnchantingSystem::CalculateEnchantingXP(const Enchantment& enchant, bool success) const {
    int baseXP = 20 + enchant.GetLevel() * 5 + enchant.GetRequiredEnchantingSkill();
    return success ? baseXP : baseXP / 2;
}

// Helper functions
std::string EnchantmentTypeToString(EnchantmentType type) {
    switch (type) {
        case EnchantmentType::WeaponDamage:     return "Weapon Damage";
        case EnchantmentType::ElementalDamage:  return "Elemental Damage";
        case EnchantmentType::LifeSteal:        return "Life Steal";
        case EnchantmentType::ManaSteal:        return "Mana Steal";
        case EnchantmentType::CriticalStrike:   return "Critical Strike";
        case EnchantmentType::AttackSpeed:      return "Attack Speed";
        case EnchantmentType::ArmorPenetration: return "Armor Penetration";
        case EnchantmentType::Defense:          return "Defense";
        case EnchantmentType::Resistance:       return "Resistance";
        case EnchantmentType::Health:           return "Health";
        case EnchantmentType::Mana:             return "Mana";
        case EnchantmentType::Regeneration:     return "Regeneration";
        case EnchantmentType::MovementSpeed:    return "Movement Speed";
        case EnchantmentType::GoldFind:         return "Gold Find";
        case EnchantmentType::MagicFind:        return "Magic Find";
        case EnchantmentType::ExperienceGain:   return "Experience Gain";
        case EnchantmentType::SkillBonus:       return "Skill Bonus";
        case EnchantmentType::Thorns:           return "Thorns";
        case EnchantmentType::Reflect:          return "Reflect";
        case EnchantmentType::Immunity:         return "Immunity";
        case EnchantmentType::Proc:             return "Proc";
        case EnchantmentType::GemSlot:          return "Gem Slot";
        default: return "Unknown";
    }
}

std::string EnchantmentSlotToString(EnchantmentSlot slot) {
    switch (slot) {
        case EnchantmentSlot::Weapon:  return "Weapon";
        case EnchantmentSlot::Helmet:  return "Helmet";
        case EnchantmentSlot::Chest:   return "Chest";
        case EnchantmentSlot::Legs:    return "Legs";
        case EnchantmentSlot::Boots:   return "Boots";
        case EnchantmentSlot::Ring:    return "Ring";
        case EnchantmentSlot::Amulet:  return "Amulet";
        case EnchantmentSlot::Any:     return "Any";
        default: return "Unknown";
    }
}

std::string RuneTypeToString(RuneType type) {
    switch (type) {
        case RuneType::Fire:      return "Fire";
        case RuneType::Ice:       return "Ice";
        case RuneType::Lightning: return "Lightning";
        case RuneType::Earth:     return "Earth";
        case RuneType::Wind:      return "Wind";
        case RuneType::Holy:      return "Holy";
        case RuneType::Dark:      return "Dark";
        case RuneType::Storm:     return "Storm";
        case RuneType::Magma:     return "Magma";
        case RuneType::Frost:     return "Frost";
        case RuneType::Power:     return "Power";
        case RuneType::Defense:   return "Defense";
        case RuneType::Life:      return "Life";
        case RuneType::Energy:    return "Energy";
        case RuneType::Chaos:     return "Chaos";
        case RuneType::Order:     return "Order";
        case RuneType::Ancient:   return "Ancient";
        case RuneType::Legendary: return "Legendary";
        case RuneType::Mythic:    return "Mythic";
        default: return "Unknown";
    }
}

std::string RuneQualityToString(RuneQuality quality) {
    switch (quality) {
        case RuneQuality::Cracked:  return "Cracked";
        case RuneQuality::Flawed:   return "Flawed";
        case RuneQuality::Normal:   return "Normal";
        case RuneQuality::Flawless: return "Flawless";
        case RuneQuality::Perfect:  return "Perfect";
        case RuneQuality::Radiant:  return "Radiant";
        default: return "Unknown";
    }
}

std::string GemTypeToString(GemType type) {
    switch (type) {
        case GemType::Ruby:      return "Ruby";
        case GemType::Sapphire:  return "Sapphire";
        case GemType::Emerald:   return "Emerald";
        case GemType::Diamond:   return "Diamond";
        case GemType::Topaz:     return "Topaz";
        case GemType::Amethyst:  return "Amethyst";
        case GemType::Opal:      return "Opal";
        case GemType::Onyx:      return "Onyx";
        case GemType::Pearl:     return "Pearl";
        case GemType::Obsidian:  return "Obsidian";
        case GemType::Citrine:   return "Citrine";
        case GemType::Jade:      return "Jade";
        default: return "Unknown";
    }
}

EnchantmentType StringToEnchantmentType(const std::string& str) {
    // Implementation for string to enum conversion
    return EnchantmentType::WeaponDamage;
}

EnchantmentSlot StringToEnchantmentSlot(const std::string& str) {
    return EnchantmentSlot::Any;
}

RuneType StringToRuneType(const std::string& str) {
    return RuneType::Fire;
}

RuneQuality StringToRuneQuality(const std::string& str) {
    return RuneQuality::Normal;
}

GemType StringToGemType(const std::string& str) {
    return GemType::Ruby;
}

float GetRuneQualityMultiplier(RuneQuality quality) {
    switch (quality) {
        case RuneQuality::Cracked:  return 0.5f;
        case RuneQuality::Flawed:   return 0.75f;
        case RuneQuality::Normal:   return 1.0f;
        case RuneQuality::Flawless: return 1.5f;
        case RuneQuality::Perfect:  return 2.0f;
        case RuneQuality::Radiant:  return 3.0f;
        default: return 1.0f;
    }
}

int GetGemTierValue(int tier) {
    return tier * 10;
}

} // namespace Crafting
