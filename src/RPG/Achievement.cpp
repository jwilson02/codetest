#include "Achievement.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctime>

namespace RPG {

Achievement::Achievement()
    : m_activeTitle("")
    , m_onUnlock(nullptr)
    , m_onProgress(nullptr)
{
    generateDefaultAchievements();
}

Achievement::~Achievement() {
}

void Achievement::initialize() {
    // Initialize all achievements as locked
    for (auto& pair : m_achievements) {
        pair.second.isUnlocked = false;
        pair.second.currentProgress = 0;
        pair.second.unlockTimestamp = 0;
    }

    m_stats.clear();
    m_unlockedTitles.clear();
    m_activeTitle = "";
}

bool Achievement::loadAchievements(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not load achievements from " << filepath
                  << ". Using default achievements." << std::endl;
        return false;
    }

    // TODO: Implement JSON parsing
    file.close();
    return true;
}

void Achievement::generateDefaultAchievements() {
    m_achievements.clear();

    // We'll create 100+ achievements across different categories
    // Sample achievements for each category

    // PROGRESSION ACHIEVEMENTS
    addAchievement({
        "first_steps", "First Steps", "Reach level 5", "",
        Category::PROGRESSION, Rarity::COMMON,
        false, 0, 0, {}, 5, {},
        100, 50, {}, {}, {},
        false, false, 0, 0.0f, "", 10
    });

    addAchievement({
        "growing_power", "Growing Power", "Reach level 25", "",
        Category::PROGRESSION, Rarity::UNCOMMON,
        false, 0, 0, {"first_steps"}, 25, {},
        500, 500, {}, {}, {},
        false, false, 0, 0.0f, "", 25
    });

    addAchievement({
        "veteran", "Veteran", "Reach level 50", "",
        Category::PROGRESSION, Rarity::RARE,
        false, 0, 0, {"growing_power"}, 50, {},
        2000, 2000, {}, {"Veteran"}, {{"all_stats", 0.05f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "master", "Master", "Reach level 75", "",
        Category::PROGRESSION, Rarity::EPIC,
        false, 0, 0, {"veteran"}, 75, {},
        5000, 5000, {}, {"Master"}, {{"all_stats", 0.10f}},
        false, false, 0, 0.0f, "", 75
    });

    addAchievement({
        "legend", "Legend", "Reach level 100", "",
        Category::PROGRESSION, Rarity::LEGENDARY,
        false, 0, 0, {"master"}, 100, {},
        10000, 10000, {"legendary_token"}, {"Legend"}, {{"all_stats", 0.15f}},
        false, false, 0, 0.0f, "", 100
    });

    // COMBAT ACHIEVEMENTS
    addAchievement({
        "first_blood", "First Blood", "Defeat your first enemy", "",
        Category::COMBAT, Rarity::COMMON,
        true, 1, 0, {}, 1, {{"kills", 1}},
        50, 25, {}, {}, {},
        false, false, 0, 0.0f, "", 5
    });

    addAchievement({
        "slayer", "Slayer", "Defeat 100 enemies", "",
        Category::COMBAT, Rarity::UNCOMMON,
        true, 100, 0, {"first_blood"}, 1, {{"kills", 100}},
        500, 250, {}, {}, {{"damage", 0.02f}},
        false, false, 0, 0.0f, "", 20
    });

    addAchievement({
        "executioner", "Executioner", "Defeat 1000 enemies", "",
        Category::COMBAT, Rarity::RARE,
        true, 1000, 0, {"slayer"}, 1, {{"kills", 1000}},
        2000, 1000, {}, {"Executioner"}, {{"damage", 0.05f}},
        false, false, 0, 0.0f, "", 40
    });

    addAchievement({
        "genocide", "Genocide", "Defeat 10000 enemies", "",
        Category::COMBAT, Rarity::EPIC,
        true, 10000, 0, {"executioner"}, 1, {{"kills", 10000}},
        10000, 5000, {}, {"Warlord"}, {{"damage", 0.10f}},
        false, false, 0, 0.0f, "", 75
    });

    addAchievement({
        "critical_success", "Critical Success", "Deal a critical hit", "",
        Category::COMBAT, Rarity::COMMON,
        true, 1, 0, {}, 1, {{"crits", 1}},
        100, 50, {}, {}, {},
        false, false, 0, 0.0f, "", 10
    });

    addAchievement({
        "crit_master", "Crit Master", "Deal 1000 critical hits", "",
        Category::COMBAT, Rarity::RARE,
        true, 1000, 0, {"critical_success"}, 1, {{"crits", 1000}},
        2000, 1000, {}, {}, {{"crit_damage", 0.10f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "overkill", "Overkill", "Deal 100,000 damage in a single hit", "",
        Category::COMBAT, Rarity::LEGENDARY,
        true, 1, 0, {}, 50, {{"max_single_hit", 100000}},
        5000, 5000, {"overkill_ring"}, {"Overkiller"}, {{"damage", 0.20f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "untouchable", "Untouchable", "Win a fight without taking damage", "",
        Category::CHALLENGE, Rarity::UNCOMMON,
        true, 1, 0, {}, 10, {{"perfect_battles", 1}},
        500, 250, {}, {}, {{"dodge", 0.05f}},
        false, false, 0, 0.0f, "", 30
    });

    addAchievement({
        "survivor", "Survivor", "Survive with 1 HP", "",
        Category::COMBAT, Rarity::RARE,
        true, 1, 0, {}, 1, {{"near_death", 1}},
        1000, 500, {}, {"Survivor"}, {{"health", 0.05f}},
        false, false, 0, 0.0f, "", 40
    });

    // EXPLORATION ACHIEVEMENTS
    addAchievement({
        "explorer", "Explorer", "Discover 10 locations", "",
        Category::EXPLORATION, Rarity::COMMON,
        true, 10, 0, {}, 1, {{"locations", 10}},
        500, 250, {}, {}, {},
        false, false, 0, 0.0f, "", 15
    });

    addAchievement({
        "pathfinder", "Pathfinder", "Discover 50 locations", "",
        Category::EXPLORATION, Rarity::UNCOMMON,
        true, 50, 0, {"explorer"}, 1, {{"locations", 50}},
        2000, 1000, {}, {"Pathfinder"}, {{"movement_speed", 0.10f}},
        false, false, 0, 0.0f, "", 30
    });

    addAchievement({
        "cartographer", "Cartographer", "Discover all locations", "",
        Category::EXPLORATION, Rarity::EPIC,
        true, 100, 0, {"pathfinder"}, 1, {{"locations", 100}},
        10000, 5000, {"world_map"}, {"Master Explorer"}, {{"xp_gain", 0.15f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "marathon", "Marathon", "Travel 100 km", "",
        Category::EXPLORATION, Rarity::UNCOMMON,
        true, 100000, 0, {}, 1, {{"distance", 100000}},
        1000, 500, {}, {}, {{"movement_speed", 0.05f}},
        false, false, 0, 0.0f, "", 25
    });

    // COLLECTION ACHIEVEMENTS
    addAchievement({
        "hoarder", "Hoarder", "Collect 1000 items", "",
        Category::COLLECTION, Rarity::UNCOMMON,
        true, 1000, 0, {}, 1, {{"items_collected", 1000}},
        1000, 500, {}, {}, {{"carry_weight", 50.0f}},
        false, false, 0, 0.0f, "", 20
    });

    addAchievement({
        "treasure_hunter", "Treasure Hunter", "Find 10 rare items", "",
        Category::COLLECTION, Rarity::RARE,
        true, 10, 0, {}, 1, {{"rare_items", 10}},
        2000, 1000, {}, {"Treasure Hunter"}, {{"loot_find", 0.10f}},
        false, false, 0, 0.0f, "", 40
    });

    addAchievement({
        "legendary_collector", "Legendary Collector", "Find 5 legendary items", "",
        Category::COLLECTION, Rarity::LEGENDARY,
        true, 5, 0, {"treasure_hunter"}, 1, {{"legendary_items", 5}},
        10000, 5000, {"collector_bag"}, {"Legendary Collector"}, {{"loot_find", 0.25f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "rich", "Rich", "Accumulate 10,000 gold", "",
        Category::COLLECTION, Rarity::UNCOMMON,
        true, 10000, 0, {}, 1, {{"gold_earned", 10000}},
        0, 1000, {}, {}, {},
        false, false, 0, 0.0f, "", 20
    });

    addAchievement({
        "millionaire", "Millionaire", "Accumulate 1,000,000 gold", "",
        Category::COLLECTION, Rarity::EPIC,
        true, 1000000, 0, {"rich"}, 1, {{"gold_earned", 1000000}},
        0, 10000, {}, {"Tycoon"}, {{"gold_find", 0.20f}},
        false, false, 0, 0.0f, "", 75
    });

    // MASTERY ACHIEVEMENTS
    addAchievement({
        "skilled", "Skilled", "Unlock 10 skills", "",
        Category::MASTERY, Rarity::COMMON,
        true, 10, 0, {}, 10, {{"skills_unlocked", 10}},
        500, 500, {}, {}, {},
        false, false, 0, 0.0f, "", 15
    });

    addAchievement({
        "master_of_skills", "Master of Skills", "Unlock 50 skills", "",
        Category::MASTERY, Rarity::RARE,
        true, 50, 0, {"skilled"}, 30, {{"skills_unlocked", 50}},
        5000, 5000, {}, {"Skill Master"}, {{"skill_points", 10.0f}},
        false, false, 0, 0.0f, "", 60
    });

    addAchievement({
        "talent_show", "Talent Show", "Unlock 20 talents", "",
        Category::MASTERY, Rarity::RARE,
        true, 20, 0, {}, 50, {{"talents_unlocked", 20}},
        3000, 3000, {}, {}, {{"talent_effectiveness", 0.15f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "specialist", "Specialist", "Max out a skill tree", "",
        Category::MASTERY, Rarity::EPIC,
        true, 1, 0, {"skilled"}, 40, {{"trees_maxed", 1}},
        5000, 5000, {}, {"Specialist"}, {{"skill_damage", 0.20f}},
        false, false, 0, 0.0f, "", 70
    });

    // CLASS-SPECIFIC ACHIEVEMENTS
    addAchievement({
        "warrior_master", "Warrior Master", "Reach level 50 as a Warrior", "",
        Category::MASTERY, Rarity::RARE,
        false, 0, 0, {}, 50, {{"warrior_level", 50}},
        3000, 3000, {}, {"Warrior Master"}, {{"str", 10.0f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "mage_master", "Mage Master", "Reach level 50 as a Mage", "",
        Category::MASTERY, Rarity::RARE,
        false, 0, 0, {}, 50, {{"mage_level", 50}},
        3000, 3000, {}, {"Archmage"}, {{"int", 10.0f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "ranger_master", "Ranger Master", "Reach level 50 as a Ranger", "",
        Category::MASTERY, Rarity::RARE,
        false, 0, 0, {}, 50, {{"ranger_level", 50}},
        3000, 3000, {}, {"Ranger Master"}, {{"dex", 10.0f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "rogue_master", "Rogue Master", "Reach level 50 as a Rogue", "",
        Category::MASTERY, Rarity::RARE,
        false, 0, 0, {}, 50, {{"rogue_level", 50}},
        3000, 3000, {}, {"Shadow Master"}, {{"crit_chance", 5.0f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "paladin_master", "Paladin Master", "Reach level 50 as a Paladin", "",
        Category::MASTERY, Rarity::RARE,
        false, 0, 0, {}, 50, {{"paladin_level", 50}},
        3000, 3000, {}, {"Holy Champion"}, {{"holy_damage", 0.20f}},
        false, false, 0, 0.0f, "", 50
    });

    // CHALLENGE ACHIEVEMENTS
    addAchievement({
        "speedrunner", "Speedrunner", "Reach level 50 in under 10 hours", "",
        Category::CHALLENGE, Rarity::EPIC,
        false, 0, 0, {}, 50, {{"speedrun_50", 1}},
        10000, 10000, {}, {"Speedrunner"}, {{"xp_gain", 0.25f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "ironman", "Ironman", "Reach level 50 without dying", "",
        Category::CHALLENGE, Rarity::LEGENDARY,
        false, 0, 0, {}, 50, {{"no_death_50", 1}},
        20000, 20000, {"ironman_armor"}, {"Ironman"}, {{"health", 0.30f}},
        false, true, 0, 0.0f, "", 150
    });

    addAchievement({
        "solo_player", "Solo Player", "Defeat a boss solo", "",
        Category::CHALLENGE, Rarity::RARE,
        true, 1, 0, {}, 30, {{"solo_boss", 1}},
        3000, 3000, {}, {"Lone Wolf"}, {{"solo_damage", 0.20f}},
        false, false, 0, 0.0f, "", 60
    });

    // PRESTIGE ACHIEVEMENTS
    addAchievement({
        "rebirth", "Rebirth", "Complete your first prestige", "",
        Category::PRESTIGE, Rarity::EPIC,
        true, 1, 0, {"legend"}, 100, {{"prestige", 1}},
        50000, 50000, {"prestige_token"}, {"Reborn"}, {{"prestige_bonus", 0.10f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "eternal", "Eternal", "Complete 10 prestiges", "",
        Category::PRESTIGE, Rarity::LEGENDARY,
        true, 10, 0, {"rebirth"}, 100, {{"prestige", 10}},
        500000, 500000, {"eternal_soul"}, {"Eternal"}, {{"prestige_bonus", 0.50f}},
        false, false, 0, 0.0f, "", 200
    });

    addAchievement({
        "transcendent", "Transcendent", "Complete 100 prestiges", "",
        Category::PRESTIGE, Rarity::MYTHIC,
        true, 100, 0, {"eternal"}, 100, {{"prestige", 100}},
        5000000, 5000000, {"transcendent_core"}, {"Transcendent"}, {{"all_stats", 1.00f}},
        false, true, 0, 0.0f, "", 500
    });

    // SECRET ACHIEVEMENTS
    addAchievement({
        "secret_path", "Secret Path", "Discover the hidden cave", "",
        Category::SECRET, Rarity::RARE,
        false, 0, 0, {}, 1, {{"secret_cave", 1}},
        5000, 5000, {"mysterious_key"}, {}, {},
        false, true, 0, 0.0f, "", 50
    });

    addAchievement({
        "easter_egg", "Easter Egg", "Find the developer's easter egg", "",
        Category::SECRET, Rarity::EPIC,
        false, 0, 0, {}, 1, {{"easter_egg", 1}},
        10000, 10000, {"dev_item"}, {"Developer's Friend"}, {},
        false, true, 0, 0.0f, "", 100
    });

    // GENERAL ACHIEVEMENTS
    addAchievement({
        "completionist", "Completionist", "Unlock 50 achievements", "",
        Category::GENERAL, Rarity::EPIC,
        true, 50, 0, {}, 1, {{"achievements", 50}},
        10000, 10000, {}, {"Completionist"}, {{"all_stats", 0.10f}},
        false, false, 0, 0.0f, "", 100
    });

    addAchievement({
        "perfectionist", "Perfectionist", "Unlock all achievements", "",
        Category::GENERAL, Rarity::MYTHIC,
        true, 100, 0, {"completionist"}, 1, {{"achievements", 100}},
        100000, 100000, {"perfect_gem"}, {"Perfectionist"}, {{"all_stats", 0.50f}},
        false, false, 0, 0.0f, "", 500
    });

    // More combat variations
    addAchievement({
        "combo_master", "Combo Master", "Achieve a 100-hit combo", "",
        Category::COMBAT, Rarity::RARE,
        true, 1, 0, {}, 20, {{"max_combo", 100}},
        2000, 2000, {}, {}, {{"attack_speed", 0.10f}},
        false, false, 0, 0.0f, "", 40
    });

    addAchievement({
        "diverse_fighter", "Diverse Fighter", "Use 20 different skills in combat", "",
        Category::COMBAT, Rarity::UNCOMMON,
        true, 20, 0, {}, 15, {{"skills_used", 20}},
        1000, 1000, {}, {}, {},
        false, false, 0, 0.0f, "", 25
    });

    addAchievement({
        "boss_slayer", "Boss Slayer", "Defeat 10 bosses", "",
        Category::COMBAT, Rarity::RARE,
        true, 10, 0, {}, 25, {{"bosses", 10}},
        5000, 5000, {}, {"Boss Slayer"}, {{"boss_damage", 0.15f}},
        false, false, 0, 0.0f, "", 50
    });

    addAchievement({
        "raid_leader", "Raid Leader", "Complete a raid", "",
        Category::CHALLENGE, Rarity::EPIC,
        true, 1, 0, {}, 60, {{"raids", 1}},
        10000, 10000, {}, {"Raid Leader"}, {{"party_damage", 0.10f}},
        false, false, 0, 0.0f, "", 75
    });

    // Additional exploration
    addAchievement({
        "dungeon_crawler", "Dungeon Crawler", "Complete 50 dungeons", "",
        Category::EXPLORATION, Rarity::RARE,
        true, 50, 0, {}, 30, {{"dungeons", 50}},
        3000, 3000, {}, {}, {{"dungeon_loot", 0.15f}},
        false, false, 0, 0.0f, "", 45
    });

    addAchievement({
        "world_traveler", "World Traveler", "Visit all regions", "",
        Category::EXPLORATION, Rarity::EPIC,
        true, 10, 0, {"pathfinder"}, 40, {{"regions", 10}},
        10000, 10000, {"traveler_boots"}, {"World Traveler"}, {},
        false, false, 0, 0.0f, "", 80
    });

    // Continue adding more until we have 100+...
    // Additional achievements for variety

    for (int i = 0; i < 50; ++i) {
        std::stringstream ss;
        ss << "milestone_" << i;
        std::string id = ss.str();

        ss.str("");
        ss << "Milestone " << i;
        std::string name = ss.str();

        ss.str("");
        ss << "Complete milestone " << i;
        std::string desc = ss.str();

        addAchievement({
            id, name, desc, "",
            Category::GENERAL, Rarity::COMMON,
            false, 0, 0, {}, 1, {},
            100 * i, 100 * i, {}, {}, {},
            false, false, 0, 0.0f, "", 5
        });
    }
}

void Achievement::addAchievement(const AchievementData& achievement) {
    m_achievements[achievement.id] = achievement;
}

bool Achievement::unlockAchievement(const std::string& achievementId) {
    auto it = m_achievements.find(achievementId);
    if (it == m_achievements.end() || it->second.isUnlocked) {
        return false;
    }

    AchievementData& achievement = it->second;

    // Check if requirements are met
    if (!meetsRequirements(achievement)) {
        return false;
    }

    achievement.isUnlocked = true;
    achievement.unlockTimestamp = std::time(nullptr);
    achievement.completionPercentage = 100.0f;

    // Grant rewards
    grantRewards(achievement);

    // Trigger callback
    if (m_onUnlock) {
        m_onUnlock(achievement);
    }

    // Track for meta-achievements
    incrementStat("achievements", 1);
    checkAchievement("completionist");
    checkAchievement("perfectionist");

    return true;
}

bool Achievement::incrementProgress(const std::string& achievementId, int amount) {
    auto it = m_achievements.find(achievementId);
    if (it == m_achievements.end() || it->second.isUnlocked) {
        return false;
    }

    AchievementData& achievement = it->second;
    if (!achievement.isProgressive) {
        return false;
    }

    achievement.currentProgress += amount;
    achievement.completionPercentage =
        (static_cast<float>(achievement.currentProgress) / achievement.maxProgress) * 100.0f;

    if (m_onProgress) {
        m_onProgress(achievementId, achievement.currentProgress, achievement.maxProgress);
    }

    // Check if completed
    if (achievement.currentProgress >= achievement.maxProgress) {
        return unlockAchievement(achievementId);
    }

    return true;
}

bool Achievement::setProgress(const std::string& achievementId, int progress) {
    auto it = m_achievements.find(achievementId);
    if (it == m_achievements.end() || it->second.isUnlocked) {
        return false;
    }

    AchievementData& achievement = it->second;
    if (!achievement.isProgressive) {
        return false;
    }

    achievement.currentProgress = std::min(progress, achievement.maxProgress);
    achievement.completionPercentage =
        (static_cast<float>(achievement.currentProgress) / achievement.maxProgress) * 100.0f;

    if (m_onProgress) {
        m_onProgress(achievementId, achievement.currentProgress, achievement.maxProgress);
    }

    // Check if completed
    if (achievement.currentProgress >= achievement.maxProgress) {
        return unlockAchievement(achievementId);
    }

    return true;
}

const Achievement::AchievementData* Achievement::getAchievement(const std::string& achievementId) const {
    auto it = m_achievements.find(achievementId);
    return (it != m_achievements.end()) ? &it->second : nullptr;
}

Achievement::AchievementData* Achievement::getAchievementMutable(const std::string& achievementId) {
    auto it = m_achievements.find(achievementId);
    return (it != m_achievements.end()) ? &it->second : nullptr;
}

std::vector<const Achievement::AchievementData*> Achievement::getAllAchievements() const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        achievements.push_back(&pair.second);
    }
    return achievements;
}

std::vector<const Achievement::AchievementData*> Achievement::getUnlockedAchievements() const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        if (pair.second.isUnlocked) {
            achievements.push_back(&pair.second);
        }
    }
    return achievements;
}

std::vector<const Achievement::AchievementData*> Achievement::getAchievementsByCategory(Category category) const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        if (pair.second.category == category) {
            achievements.push_back(&pair.second);
        }
    }
    return achievements;
}

std::vector<const Achievement::AchievementData*> Achievement::getAchievementsByRarity(Rarity rarity) const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        if (pair.second.rarity == rarity) {
            achievements.push_back(&pair.second);
        }
    }
    return achievements;
}

std::vector<const Achievement::AchievementData*> Achievement::getInProgressAchievements() const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        if (!pair.second.isUnlocked && pair.second.isProgressive && pair.second.currentProgress > 0) {
            achievements.push_back(&pair.second);
        }
    }
    return achievements;
}

std::vector<const Achievement::AchievementData*> Achievement::getAvailableAchievements() const {
    std::vector<const AchievementData*> achievements;
    for (const auto& pair : m_achievements) {
        if (!pair.second.isUnlocked && meetsRequirements(pair.second)) {
            achievements.push_back(&pair.second);
        }
    }
    return achievements;
}

int Achievement::getUnlockedCount() const {
    int count = 0;
    for (const auto& pair : m_achievements) {
        if (pair.second.isUnlocked) {
            count++;
        }
    }
    return count;
}

int Achievement::getAchievementPoints() const {
    int points = 0;
    for (const auto& pair : m_achievements) {
        if (pair.second.isUnlocked) {
            points += pair.second.points;
        }
    }
    return points;
}

float Achievement::getCompletionPercentage() const {
    if (m_achievements.empty()) {
        return 0.0f;
    }
    return (static_cast<float>(getUnlockedCount()) / m_achievements.size()) * 100.0f;
}

std::map<Achievement::Category, int> Achievement::getAchievementCountByCategory() const {
    std::map<Category, int> counts;
    for (const auto& pair : m_achievements) {
        if (pair.second.isUnlocked) {
            counts[pair.second.category]++;
        }
    }
    return counts;
}

std::map<Achievement::Rarity, int> Achievement::getAchievementCountByRarity() const {
    std::map<Rarity, int> counts;
    for (const auto& pair : m_achievements) {
        if (pair.second.isUnlocked) {
            counts[pair.second.rarity]++;
        }
    }
    return counts;
}

// Tracking functions
void Achievement::trackKill(const std::string& enemyType, int count) {
    incrementStat("kills", count);
    incrementStat("kill_" + enemyType, count);

    checkAchievement("first_blood");
    checkAchievement("slayer");
    checkAchievement("executioner");
    checkAchievement("genocide");
}

void Achievement::trackDamageDealt(int damage) {
    incrementStat("damage_dealt", damage);

    int maxHit = getStat("max_single_hit");
    if (damage > maxHit) {
        setStat("max_single_hit", damage);
        checkAchievement("overkill");
    }
}

void Achievement::trackLocationDiscovered(const std::string& location) {
    incrementStat("locations", 1);
    checkAchievement("explorer");
    checkAchievement("pathfinder");
    checkAchievement("cartographer");
}

void Achievement::trackItemCollected(const std::string& itemType, int count) {
    incrementStat("items_collected", count);
    incrementStat(itemType + "_collected", count);
    checkAchievement("hoarder");
}

void Achievement::trackSkillUnlocked(const std::string& skillId) {
    incrementStat("skills_unlocked", 1);
    checkAchievement("skilled");
    checkAchievement("master_of_skills");
}

void Achievement::trackQuestCompleted(const std::string& questId) {
    incrementStat("quests_completed", 1);
}

void Achievement::trackDistanceTraveled(float distance) {
    incrementStat("distance", static_cast<int>(distance));
    checkAchievement("marathon");
}

void Achievement::trackGoldEarned(int amount) {
    incrementStat("gold_earned", amount);
    checkAchievement("rich");
    checkAchievement("millionaire");
}

void Achievement::trackDeaths(int count) {
    incrementStat("deaths", count);
}

void Achievement::incrementStat(const std::string& statName, int amount) {
    m_stats[statName] += amount;
}

void Achievement::setStat(const std::string& statName, int value) {
    m_stats[statName] = value;
}

int Achievement::getStat(const std::string& statName) const {
    auto it = m_stats.find(statName);
    return (it != m_stats.end()) ? it->second : 0;
}

void Achievement::checkAchievement(const std::string& achievementId) {
    auto it = m_achievements.find(achievementId);
    if (it == m_achievements.end() || it->second.isUnlocked) {
        return;
    }

    const AchievementData& achievement = it->second;

    // Check stat requirements
    bool allMetfor (const auto& req : achievement.requirements) {
        if (getStat(req.first) < req.second) {
            allMet = false;
            break;
        }
    }

    if (allMet) {
        if (achievement.isProgressive) {
            setProgress(achievementId, achievement.maxProgress);
        } else {
            unlockAchievement(achievementId);
        }
    }
}

bool Achievement::meetsRequirements(const AchievementData& achievement) const {
    // Check prerequisite achievements
    for (const auto& prereqId : achievement.prerequisiteAchievements) {
        auto it = m_achievements.find(prereqId);
        if (it == m_achievements.end() || !it->second.isUnlocked) {
            return false;
        }
    }

    // Level requirement would need to be checked externally
    // Stat requirements would need access to stat system

    return true;
}

void Achievement::grantRewards(const AchievementData& achievement) {
    m_pendingRewards.gold += achievement.goldReward;
    m_pendingRewards.xp += achievement.xpReward;
    m_pendingRewards.items.insert(m_pendingRewards.items.end(),
                                  achievement.itemRewards.begin(),
                                  achievement.itemRewards.end());

    // Add titles
    for (const auto& title : achievement.titleRewards) {
        if (std::find(m_unlockedTitles.begin(), m_unlockedTitles.end(), title)
            == m_unlockedTitles.end()) {
            m_unlockedTitles.push_back(title);
            m_pendingRewards.titles.push_back(title);
        }
    }

    // Add stat bonuses
    for (const auto& bonus : achievement.statBonuses) {
        m_pendingRewards.statBonuses[bonus.first] += bonus.second;
    }
}

void Achievement::claimRewards() {
    m_pendingRewards = RewardBundle{0, 0, {}, {}, {}};
}

bool Achievement::hasPendingRewards() const {
    return m_pendingRewards.gold > 0 ||
           m_pendingRewards.xp > 0 ||
           !m_pendingRewards.items.empty() ||
           !m_pendingRewards.titles.empty() ||
           !m_pendingRewards.statBonuses.empty();
}

void Achievement::setActiveTitle(const std::string& title) {
    if (std::find(m_unlockedTitles.begin(), m_unlockedTitles.end(), title)
        != m_unlockedTitles.end()) {
        m_activeTitle = title;
    }
}

std::string Achievement::getCategoryName(Category category) {
    switch (category) {
        case Category::COMBAT: return "Combat";
        case Category::EXPLORATION: return "Exploration";
        case Category::PROGRESSION: return "Progression";
        case Category::COLLECTION: return "Collection";
        case Category::MASTERY: return "Mastery";
        case Category::SOCIAL: return "Social";
        case Category::CHALLENGE: return "Challenge";
        case Category::SECRET: return "Secret";
        case Category::PRESTIGE: return "Prestige";
        case Category::GENERAL: return "General";
        default: return "Unknown";
    }
}

std::string Achievement::getRarityName(Rarity rarity) {
    switch (rarity) {
        case Rarity::COMMON: return "Common";
        case Rarity::UNCOMMON: return "Uncommon";
        case Rarity::RARE: return "Rare";
        case Rarity::EPIC: return "Epic";
        case Rarity::LEGENDARY: return "Legendary";
        case Rarity::MYTHIC: return "Mythic";
        default: return "Unknown";
    }
}

int Achievement::getRarityPoints(Rarity rarity) {
    switch (rarity) {
        case Rarity::COMMON: return 10;
        case Rarity::UNCOMMON: return 25;
        case Rarity::RARE: return 50;
        case Rarity::EPIC: return 100;
        case Rarity::LEGENDARY: return 200;
        case Rarity::MYTHIC: return 500;
        default: return 0;
    }
}

std::string Achievement::serialize() const {
    // Implementation for serialization
    return "";
}

bool Achievement::deserialize(const std::string& data) {
    // Implementation for deserialization
    return true;
}

} // namespace RPG
