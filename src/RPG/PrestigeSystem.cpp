#include "PrestigeSystem.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <ctime>

namespace RPG {

PrestigeSystem::PrestigeSystem()
    : m_prestigeLevel(0)
    , m_totalPrestiges(0)
    , m_prestigeRequiredLevel(DEFAULT_PRESTIGE_LEVEL)
    , m_prestigePoints(0)
    , m_totalPrestigePoints(0)
    , m_totalPrestigeTime(0)
    , m_fastestPrestige(0)
    , m_currentRunStartTime(0)
    , m_onPrestige(nullptr)
    , m_onCurrencyGain(nullptr)
{
    initializePrestigeTiers();
    initializePrestigeUpgrades();
    initializePrestigeChallenges();
    initializePrestigeMilestones();
}

PrestigeSystem::~PrestigeSystem() {
}

void PrestigeSystem::initialize() {
    m_currentRunStartTime = std::time(nullptr);
}

void PrestigeSystem::initializePrestigeTiers() {
    m_prestigeTiers.clear();

    // Tier 0: Novice (0-9 prestiges)
    PrestigeTier novice;
    novice.tier = 0;
    novice.name = "Novice";
    novice.description = "Beginning your journey of rebirth";
    novice.prestigeRequired = 0;
    novice.cumulativeRewards = {5, 1.1f, 1.1f, 1.05f, 1, 0, {}, {}, {}};
    novice.colorTheme = "#CCCCCC";
    m_prestigeTiers.push_back(novice);

    // Tier 1: Adept (10-24 prestiges)
    PrestigeTier adept;
    adept.tier = 1;
    adept.name = "Adept";
    adept.description = "Growing stronger with each rebirth";
    adept.prestigeRequired = 10;
    adept.cumulativeRewards = {15, 1.3f, 1.3f, 1.15f, 2, 1, {"auto_loot"}, {}, {}};
    adept.colorTheme = "#00FF00";
    m_prestigeTiers.push_back(adept);

    // Tier 2: Expert (25-49 prestiges)
    PrestigeTier expert;
    expert.tier = 2;
    expert.name = "Expert";
    expert.description = "Mastery over the cycle of rebirth";
    expert.prestigeRequired = 25;
    expert.cumulativeRewards = {30, 1.6f, 1.6f, 1.30f, 3, 2, {"auto_loot", "fast_travel"}, {"prestige_skill_1"}, {}};
    expert.colorTheme = "#0088FF";
    m_prestigeTiers.push_back(expert);

    // Tier 3: Master (50-99 prestiges)
    PrestigeTier master;
    master.tier = 3;
    master.name = "Master";
    master.description = "Transcending mortal limitations";
    master.prestigeRequired = 50;
    master.cumulativeRewards = {50, 2.0f, 2.0f, 1.50f, 4, 3, {"auto_loot", "fast_travel", "multi_class"}, {"prestige_skill_1", "prestige_skill_2"}, {}};
    master.colorTheme = "#AA00FF";
    m_prestigeTiers.push_back(master);

    // Tier 4: Grandmaster (100-249 prestiges)
    PrestigeTier grandmaster;
    grandmaster.tier = 4;
    grandmaster.name = "Grandmaster";
    grandmaster.description = "Approaching divine power";
    grandmaster.prestigeRequired = 100;
    grandmaster.cumulativeRewards = {100, 3.0f, 3.0f, 2.00f, 5, 5, {"auto_loot", "fast_travel", "multi_class", "prestige_zones"}, {"prestige_skill_1", "prestige_skill_2", "prestige_skill_3"}, {"prestige_weapon"}};
    grandmaster.colorTheme = "#FF8800";
    m_prestigeTiers.push_back(grandmaster);

    // Tier 5: Transcendent (250+ prestiges)
    PrestigeTier transcendent;
    transcendent.tier = 5;
    transcendent.name = "Transcendent";
    transcendent.description = "Beyond mortal comprehension";
    transcendent.prestigeRequired = 250;
    transcendent.cumulativeRewards = {200, 5.0f, 5.0f, 3.00f, 10, 10, {"auto_loot", "fast_travel", "multi_class", "prestige_zones", "infinite_potential"}, {"prestige_skill_1", "prestige_skill_2", "prestige_skill_3", "prestige_skill_4", "prestige_ultimate"}, {"prestige_weapon", "prestige_armor", "transcendent_core"}};
    transcendent.colorTheme = "#FF0000";
    m_prestigeTiers.push_back(transcendent);
}

void PrestigeSystem::initializePrestigeUpgrades() {
    m_upgrades.clear();

    // Experience boost upgrades
    PrestigeUpgrade xpBoost1 = {
        "xp_boost_1", "XP Boost I", "Gain 10% more XP permanently", "ascension_tokens",
        100, 10, 0, {{"xp_multiplier", 0.10f}}
    };
    m_upgrades["xp_boost_1"] = xpBoost1;

    PrestigeUpgrade xpBoost2 = {
        "xp_boost_2", "XP Boost II", "Gain 25% more XP permanently", "ascension_tokens",
        500, 5, 0, {{"xp_multiplier", 0.25f}}
    };
    m_upgrades["xp_boost_2"] = xpBoost2;

    // Gold boost upgrades
    PrestigeUpgrade goldBoost1 = {
        "gold_boost_1", "Gold Boost I", "Gain 15% more gold permanently", "ascension_tokens",
        150, 10, 0, {{"gold_multiplier", 0.15f}}
    };
    m_upgrades["gold_boost_1"] = goldBoost1;

    // Stat upgrades
    PrestigeUpgrade statBoost1 = {
        "stat_boost_1", "Stat Boost I", "Gain +5 to all stats permanently", "ascension_tokens",
        200, -1, 0, {{"all_stats", 5.0f}}
    };
    m_upgrades["stat_boost_1"] = statBoost1;

    // Skill point upgrades
    PrestigeUpgrade skillPoints1 = {
        "skill_points_1", "Bonus Skill Points", "Gain +1 skill point per level", "ascension_tokens",
        300, 5, 0, {{"skill_points_per_level", 1.0f}}
    };
    m_upgrades["skill_points_1"] = skillPoints1;

    // Drop rate upgrades
    PrestigeUpgrade dropRate1 = {
        "drop_rate_1", "Lucky Find", "Increase drop rate by 20%", "ascension_tokens",
        250, 10, 0, {{"drop_rate", 0.20f}}
    };
    m_upgrades["drop_rate_1"] = dropRate1;

    // Starting level upgrade
    PrestigeUpgrade startLevel = {
        "start_level", "Head Start", "Start at level 10 after prestige", "ascension_tokens",
        1000, 1, 0, {{"starting_level", 10.0f}}
    };
    m_upgrades["start_level"] = startLevel;

    // Critical damage boost
    PrestigeUpgrade critDamage = {
        "crit_damage", "Critical Master", "Increase critical damage by 50%", "ascension_tokens",
        400, 5, 0, {{"crit_damage", 0.50f}}
    };
    m_upgrades["crit_damage"] = critDamage;

    // Health boost
    PrestigeUpgrade healthBoost = {
        "health_boost", "Vitality Surge", "Increase max health by 25%", "ascension_tokens",
        300, 10, 0, {{"max_health", 0.25f}}
    };
    m_upgrades["health_boost"] = healthBoost;

    // Damage boost
    PrestigeUpgrade damageBoost = {
        "damage_boost", "Power Surge", "Increase all damage by 15%", "ascension_tokens",
        350, 10, 0, {{"damage", 0.15f}}
    };
    m_upgrades["damage_boost"] = damageBoost;
}

void PrestigeSystem::initializePrestigeChallenges() {
    m_challenges.clear();

    // Speed run challenge
    PrestigeChallenge speedRun = {
        "speed_run", "Speed Runner", "Reach level 100 in under 5 hours",
        1, false, false,
        {{"time_limit", 18000}, {"target_level", 100}},
        {{"time_elapsed", 0}, {"current_level", 0}},
        {10, 1.5f, 1.0f, 1.0f, 2, 1, {}, {}, {}}
    };
    m_challenges["speed_run"] = speedRun;

    // No death challenge
    PrestigeChallenge noDeath = {
        "no_death", "Immortal", "Reach level 100 without dying",
        5, false, false,
        {{"deaths", 0}, {"target_level", 100}},
        {{"deaths", 0}, {"current_level", 0}},
        {20, 1.0f, 1.0f, 1.5f, 3, 2, {}, {}, {}}
    };
    m_challenges["no_death"] = noDeath;

    // Solo challenge
    PrestigeChallenge soloPlay = {
        "solo_play", "Lone Wolf", "Complete 10 dungeons solo",
        3, false, false,
        {{"solo_dungeons", 10}},
        {{"solo_dungeons", 0}},
        {15, 1.0f, 1.2f, 1.0f, 2, 1, {}, {}, {}}
    };
    m_challenges["solo_play"] = soloPlay;

    // Boss rush challenge
    PrestigeChallenge bossRush = {
        "boss_rush", "Boss Slayer", "Defeat 50 bosses in one prestige",
        10, false, false,
        {{"bosses_defeated", 50}},
        {{"bosses_defeated", 0}},
        {25, 1.0f, 1.0f, 1.3f, 5, 3, {}, {"boss_slayer_skill"}, {}}
    };
    m_challenges["boss_rush"] = bossRush;
}

void PrestigeSystem::initializePrestigeMilestones() {
    m_milestones.clear();

    // Key milestones
    for (int i : {1, 5, 10, 25, 50, 100, 250, 500, 999}) {
        PrestigeMilestone milestone;
        milestone.prestigeLevel = i;
        milestone.title = "Prestige " + std::to_string(i);
        milestone.description = "Reached prestige level " + std::to_string(i);
        milestone.isUnlocked = false;

        // Scale rewards with prestige level
        milestone.rewards.permanentStatBonus = i * 2;
        milestone.rewards.xpMultiplier = 1.0f + (i * 0.01f);
        milestone.rewards.goldMultiplier = 1.0f + (i * 0.01f);
        milestone.rewards.dropRateMultiplier = 1.0f + (i * 0.005f);
        milestone.rewards.skillPointBonus = i / 10;
        milestone.rewards.talentPointBonus = i / 20;

        m_milestones[i] = milestone;
    }
}

bool PrestigeSystem::canPrestige() const {
    // Must be at required level (typically 100)
    // In a real implementation, this would check the character's current level
    return true; // Simplified for this example
}

bool PrestigeSystem::performPrestige() {
    if (!canPrestige()) {
        return false;
    }

    // Calculate time for this prestige run
    if (m_currentRunStartTime > 0) {
        long long runTime = std::time(nullptr) - m_currentRunStartTime;
        m_totalPrestigeTime += runTime;

        if (m_fastestPrestige == 0 || runTime < m_fastestPrestige) {
            m_fastestPrestige = static_cast<int>(runTime);
        }
    }

    // Increment prestige
    m_prestigeLevel++;
    m_totalPrestiges++;

    // Award prestige rewards
    PrestigeReward rewards = getRewardForPrestigeLevel(m_prestigeLevel);
    applyPrestigeRewards(rewards);

    // Award prestige currency
    int tokensEarned = 100 + (m_prestigeLevel * 10); // More tokens at higher prestige
    addCurrency("ascension_tokens", tokensEarned);

    // Award prestige points
    int pointsEarned = 10 * m_prestigeLevel;
    addPrestigePoints(pointsEarned);

    // Check milestones
    if (m_milestones.find(m_prestigeLevel) != m_milestones.end()) {
        m_milestones[m_prestigeLevel].isUnlocked = true;
        applyPrestigeRewards(m_milestones[m_prestigeLevel].rewards);
    }

    // Reset for new run
    m_currentRunStartTime = std::time(nullptr);

    // Trigger callback
    if (m_onPrestige) {
        m_onPrestige(m_prestigeLevel, rewards);
    }

    return true;
}

PrestigeSystem::PrestigeReward PrestigeSystem::getRewardForPrestigeLevel(int level) const {
    PrestigeReward reward;

    // Base rewards that scale with prestige level
    reward.permanentStatBonus = 5 + (level * 2);
    reward.xpMultiplier = 1.0f + (level * 0.02f);
    reward.goldMultiplier = 1.0f + (level * 0.02f);
    reward.dropRateMultiplier = 1.0f + (level * 0.01f);
    reward.skillPointBonus = 1 + (level / 10);
    reward.talentPointBonus = level / 20;

    // Unlock features at specific prestige levels
    if (level >= 5) reward.unlockedFeatures.push_back("auto_loot");
    if (level >= 10) reward.unlockedFeatures.push_back("fast_travel");
    if (level >= 25) reward.unlockedFeatures.push_back("multi_class");
    if (level >= 50) reward.unlockedFeatures.push_back("prestige_zones");
    if (level >= 100) reward.unlockedFeatures.push_back("infinite_potential");

    return reward;
}

PrestigeSystem::PrestigeReward PrestigeSystem::getCurrentPrestigeReward() const {
    return getRewardForPrestigeLevel(m_prestigeLevel);
}

PrestigeSystem::PrestigeReward PrestigeSystem::getCumulativeRewards() const {
    PrestigeReward cumulative = {0, 1.0f, 1.0f, 1.0f, 0, 0, {}, {}, {}};

    for (int i = 1; i <= m_prestigeLevel; ++i) {
        PrestigeReward reward = getRewardForPrestigeLevel(i);
        cumulative.permanentStatBonus += reward.permanentStatBonus;
        cumulative.xpMultiplier *= reward.xpMultiplier;
        cumulative.goldMultiplier *= reward.goldMultiplier;
        cumulative.dropRateMultiplier *= reward.dropRateMultiplier;
        cumulative.skillPointBonus += reward.skillPointBonus;
        cumulative.talentPointBonus += reward.talentPointBonus;
        cumulative.unlockedFeatures.insert(cumulative.unlockedFeatures.end(),
                                          reward.unlockedFeatures.begin(),
                                          reward.unlockedFeatures.end());
    }

    // Add purchased upgrade bonuses
    for (const auto& pair : m_upgrades) {
        if (pair.second.isPurchased()) {
            for (const auto& effect : pair.second.effects) {
                if (effect.first == "xp_multiplier") {
                    cumulative.xpMultiplier *= (1.0f + effect.second * pair.second.currentPurchases);
                } else if (effect.first == "gold_multiplier") {
                    cumulative.goldMultiplier *= (1.0f + effect.second * pair.second.currentPurchases);
                } else if (effect.first == "all_stats") {
                    cumulative.permanentStatBonus += static_cast<int>(effect.second * pair.second.currentPurchases);
                }
            }
        }
    }

    return cumulative;
}

PrestigeSystem::PrestigeReward PrestigeSystem::getNextPrestigeReward() const {
    return getRewardForPrestigeLevel(m_prestigeLevel + 1);
}

const PrestigeSystem::PrestigeTier* PrestigeSystem::getCurrentTier() const {
    for (auto it = m_prestigeTiers.rbegin(); it != m_prestigeTiers.rend(); ++it) {
        if (m_prestigeLevel >= it->prestigeRequired) {
            return &(*it);
        }
    }
    return &m_prestigeTiers[0];
}

const PrestigeSystem::PrestigeTier* PrestigeSystem::getNextTier() const {
    for (const auto& tier : m_prestigeTiers) {
        if (m_prestigeLevel < tier.prestigeRequired) {
            return &tier;
        }
    }
    return nullptr;
}

void PrestigeSystem::addCurrency(const std::string& currencyType, int amount) {
    m_currencies[currencyType] += amount;

    if (m_onCurrencyGain) {
        m_onCurrencyGain(currencyType, amount);
    }
}

int PrestigeSystem::getCurrency(const std::string& currencyType) const {
    auto it = m_currencies.find(currencyType);
    return (it != m_currencies.end()) ? it->second : 0;
}

bool PrestigeSystem::spendCurrency(const std::string& currencyType, int amount) {
    int current = getCurrency(currencyType);
    if (current < amount) {
        return false;
    }

    m_currencies[currencyType] -= amount;
    return true;
}

bool PrestigeSystem::purchaseUpgrade(const std::string& upgradeId) {
    auto it = m_upgrades.find(upgradeId);
    if (it == m_upgrades.end()) {
        return false;
    }

    PrestigeUpgrade& upgrade = it->second;

    // Check if maxed
    if (upgrade.isMaxed()) {
        return false;
    }

    // Check currency
    if (!spendCurrency(upgrade.currencyType, upgrade.cost)) {
        return false;
    }

    // Purchase
    upgrade.currentPurchases++;
    return true;
}

const PrestigeSystem::PrestigeUpgrade* PrestigeSystem::getUpgrade(const std::string& upgradeId) const {
    auto it = m_upgrades.find(upgradeId);
    return (it != m_upgrades.end()) ? &it->second : nullptr;
}

std::vector<const PrestigeSystem::PrestigeUpgrade*> PrestigeSystem::getAvailableUpgrades() const {
    std::vector<const PrestigeUpgrade*> upgrades;
    for (const auto& pair : m_upgrades) {
        if (!pair.second.isMaxed()) {
            upgrades.push_back(&pair.second);
        }
    }
    return upgrades;
}

std::vector<const PrestigeSystem::PrestigeUpgrade*> PrestigeSystem::getPurchasedUpgrades() const {
    std::vector<const PrestigeUpgrade*> upgrades;
    for (const auto& pair : m_upgrades) {
        if (pair.second.isPurchased()) {
            upgrades.push_back(&pair.second);
        }
    }
    return upgrades;
}

bool PrestigeSystem::activateChallenge(const std::string& challengeId) {
    auto it = m_challenges.find(challengeId);
    if (it == m_challenges.end()) {
        return false;
    }

    PrestigeChallenge& challenge = it->second;

    // Check prestige requirement
    if (m_prestigeLevel < challenge.prestigeRequired) {
        return false;
    }

    // Check if already completed
    if (challenge.isCompleted) {
        return false;
    }

    challenge.isActive = true;
    // Reset progress
    for (auto& prog : challenge.progress) {
        prog.second = 0;
    }

    return true;
}

bool PrestigeSystem::completeChallenge(const std::string& challengeId) {
    auto it = m_challenges.find(challengeId);
    if (it == m_challenges.end()) {
        return false;
    }

    PrestigeChallenge& challenge = it->second;

    // Check if active
    if (!challenge.isActive) {
        return false;
    }

    // Check if objectives met
    for (const auto& obj : challenge.objectives) {
        auto progIt = challenge.progress.find(obj.first);
        if (progIt == challenge.progress.end() || progIt->second < obj.second) {
            return false;
        }
    }

    challenge.isCompleted = true;
    challenge.isActive = false;
    applyPrestigeRewards(challenge.rewards);

    return true;
}

void PrestigeSystem::updateChallengeProgress(const std::string& challengeId, const std::string& objective, int amount) {
    auto it = m_challenges.find(challengeId);
    if (it == m_challenges.end() || !it->second.isActive) {
        return;
    }

    it->second.progress[objective] = amount;

    // Auto-complete if objectives met
    completeChallenge(challengeId);
}

const PrestigeSystem::PrestigeChallenge* PrestigeSystem::getChallenge(const std::string& challengeId) const {
    auto it = m_challenges.find(challengeId);
    return (it != m_challenges.end()) ? &it->second : nullptr;
}

std::vector<const PrestigeSystem::PrestigeChallenge*> PrestigeSystem::getActiveChallenges() const {
    std::vector<const PrestigeChallenge*> challenges;
    for (const auto& pair : m_challenges) {
        if (pair.second.isActive) {
            challenges.push_back(&pair.second);
        }
    }
    return challenges;
}

std::vector<const PrestigeSystem::PrestigeChallenge*> PrestigeSystem::getCompletedChallenges() const {
    std::vector<const PrestigeChallenge*> challenges;
    for (const auto& pair : m_challenges) {
        if (pair.second.isCompleted) {
            challenges.push_back(&pair.second);
        }
    }
    return challenges;
}

const PrestigeSystem::PrestigeMilestone* PrestigeSystem::getMilestone(int prestigeLevel) const {
    auto it = m_milestones.find(prestigeLevel);
    return (it != m_milestones.end()) ? &it->second : nullptr;
}

std::vector<const PrestigeSystem::PrestigeMilestone*> PrestigeSystem::getUnlockedMilestones() const {
    std::vector<const PrestigeMilestone*> milestones;
    for (const auto& pair : m_milestones) {
        if (pair.second.isUnlocked) {
            milestones.push_back(&pair.second);
        }
    }
    return milestones;
}

float PrestigeSystem::getStatMultiplier() const {
    return 1.0f + (m_prestigeLevel * 0.02f); // 2% per prestige
}

float PrestigeSystem::getXPMultiplier() const {
    return getCumulativeRewards().xpMultiplier;
}

float PrestigeSystem::getGoldMultiplier() const {
    return getCumulativeRewards().goldMultiplier;
}

float PrestigeSystem::getDropRateMultiplier() const {
    return getCumulativeRewards().dropRateMultiplier;
}

int PrestigeSystem::getExtraSkillPoints() const {
    return getCumulativeRewards().skillPointBonus;
}

int PrestigeSystem::getExtraTalentPoints() const {
    return getCumulativeRewards().talentPointBonus;
}

PrestigeSystem::CarryoverData PrestigeSystem::getCarryoverData() const {
    CarryoverData data;
    data.currencies = m_currencies;
    data.achievementsCarryOver = true;
    data.skillTreesReset = true;
    data.statsReset = true;

    // Add any prestige-exclusive items that carry over
    const auto& rewards = getCumulativeRewards();
    data.items = rewards.prestigeItems;
    data.unlockedFeatures = rewards.unlockedFeatures;

    return data;
}

void PrestigeSystem::addPrestigePoints(int points) {
    m_prestigePoints += points;
    m_totalPrestigePoints += points;
}

std::vector<PrestigeSystem::LegacyBonus> PrestigeSystem::getLegacyBonuses() const {
    std::vector<LegacyBonus> bonuses;

    // Example legacy bonuses
    bonuses.push_back({"damage", 0.01f * m_prestigeLevel, m_prestigeLevel});
    bonuses.push_back({"health", 0.01f * m_prestigeLevel, m_prestigeLevel});
    bonuses.push_back({"crit_chance", 0.005f * m_prestigeLevel, m_prestigeLevel});

    return bonuses;
}

float PrestigeSystem::getLegacyBonusTotal(const std::string& type) const {
    auto bonuses = getLegacyBonuses();
    for (const auto& bonus : bonuses) {
        if (bonus.type == type) {
            return bonus.value;
        }
    }
    return 0.0f;
}

std::map<std::string, int> PrestigeSystem::getPrestigeStats() const {
    std::map<std::string, int> stats;
    stats["prestige_level"] = m_prestigeLevel;
    stats["total_prestiges"] = m_totalPrestiges;
    stats["prestige_points"] = m_prestigePoints;
    stats["total_prestige_points"] = m_totalPrestigePoints;
    stats["fastest_prestige"] = m_fastestPrestige;
    return stats;
}

void PrestigeSystem::applyPrestigeRewards(const PrestigeReward& rewards) {
    // In a real implementation, this would apply the rewards to the character
    // For now, we just store them in the cumulative rewards
}

std::string PrestigeSystem::serialize() const {
    std::stringstream ss;
    ss << m_prestigeLevel << "," << m_totalPrestiges << "," << m_prestigePoints;
    return ss.str();
}

bool PrestigeSystem::deserialize(const std::string& data) {
    // Implementation for deserialization
    return true;
}

} // namespace RPG
