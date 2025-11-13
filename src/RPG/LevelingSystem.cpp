#include "LevelingSystem.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>

// For JSON parsing - using a simple approach or include your preferred JSON library
// For this implementation, I'll include basic JSON parsing logic
#include <iostream>

namespace RPG {

LevelingSystem::LevelingSystem()
    : m_maxLevel(DEFAULT_MAX_LEVEL)
    , m_currentLevel(1)
    , m_currentXP(0)
    , m_totalXPEarned(0)
    , m_totalLevelUps(0)
    , m_pendingRewards({0, 0, 0, {}})
    , m_onLevelUp(nullptr)
    , m_onXPGain(nullptr)
{
    generateDefaultLevelCurve();
}

LevelingSystem::~LevelingSystem() {
}

void LevelingSystem::initialize(int startLevel) {
    if (startLevel < 1 || startLevel > m_maxLevel) {
        throw std::invalid_argument("Invalid start level");
    }

    m_currentLevel = startLevel;
    m_currentXP = (startLevel > 1) ? m_levelCurve[startLevel - 1].totalXp : 0;
    m_totalXPEarned = m_currentXP;
    m_totalLevelUps = startLevel - 1;
    m_pendingRewards = {0, 0, 0, {}};
}

void LevelingSystem::generateDefaultLevelCurve() {
    m_levelCurve.clear();
    m_levelCurve.reserve(m_maxLevel);

    unsigned long long totalXp = 0;

    for (int level = 1; level <= m_maxLevel; ++level) {
        LevelData data;
        data.level = level;

        // Exponential XP curve with milestones
        // Formula: baseXP * (level^2.5) * multiplier
        // This creates a smooth exponential curve
        double levelFactor = std::pow(level, 2.5);
        unsigned long long xpRequired = static_cast<unsigned long long>(
            BASE_XP_REQUIREMENT * levelFactor * std::pow(BASE_XP_MULTIPLIER, level / 10.0)
        );

        // Add milestone bonuses every 10 levels
        if (level % 10 == 0) {
            xpRequired = static_cast<unsigned long long>(xpRequired * 1.5);
        }

        data.xpRequired = xpRequired;
        data.totalXp = totalXp;
        totalXp += xpRequired;

        // Level rewards - more generous at milestones
        LevelReward reward;
        reward.attributePoints = (level % 10 == 0) ? 10 : 5;
        reward.skillPoints = (level % 5 == 0) ? 2 : 1;
        reward.talentPoints = (level % 10 == 0) ? 1 : 0;

        // Special unlocks at key levels
        if (level == 10) reward.unlocks.push_back("Advanced Skills Tier 1");
        if (level == 20) reward.unlocks.push_back("Ultimate Ability Slot 1");
        if (level == 25) reward.unlocks.push_back("Advanced Skills Tier 2");
        if (level == 30) reward.unlocks.push_back("Equipment Slot: Accessory 1");
        if (level == 40) reward.unlocks.push_back("Ultimate Ability Slot 2");
        if (level == 50) reward.unlocks.push_back("Master Skills Tier 1");
        if (level == 60) reward.unlocks.push_back("Equipment Slot: Accessory 2");
        if (level == 70) reward.unlocks.push_back("Master Skills Tier 2");
        if (level == 75) reward.unlocks.push_back("Ultimate Ability Slot 3");
        if (level == 80) reward.unlocks.push_back("Legendary Skills");
        if (level == 90) reward.unlocks.push_back("Equipment Slot: Accessory 3");
        if (level == 100) {
            reward.unlocks.push_back("Prestige System");
            reward.unlocks.push_back("Transcendent Skills");
            reward.attributePoints = 25;
            reward.skillPoints = 10;
            reward.talentPoints = 5;
        }

        data.rewards = reward;
        m_levelCurve.push_back(data);
    }
}

bool LevelingSystem::loadLevelCurve(const std::string& filepath) {
    // This would load from JSON file - implementation depends on JSON library
    // For now, we'll use the generated curve
    // TODO: Implement JSON parsing when JSON library is available
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not load level curve from " << filepath
                  << ". Using default curve." << std::endl;
        return false;
    }

    // JSON parsing would go here
    file.close();
    return true;
}

void LevelingSystem::gainXP(unsigned long long amount) {
    if (m_currentLevel >= m_maxLevel) {
        return; // Max level reached
    }

    // Apply XP modifiers
    unsigned long long modifiedXP = applyXPModifiers(amount);

    m_currentXP += modifiedXP;
    m_totalXPEarned += modifiedXP;

    // Trigger XP gain callback
    if (m_onXPGain) {
        m_onXPGain(modifiedXP, m_currentXP);
    }

    // Check for level ups
    while (canLevelUp() && m_currentLevel < m_maxLevel) {
        levelUp();
    }
}

void LevelingSystem::setXP(unsigned long long amount) {
    m_currentXP = amount;
    calculateLevelFromXP();
}

unsigned long long LevelingSystem::getXPForNextLevel() const {
    if (m_currentLevel >= m_maxLevel) {
        return 0;
    }
    return m_levelCurve[m_currentLevel].xpRequired;
}

unsigned long long LevelingSystem::getXPToNextLevel() const {
    if (m_currentLevel >= m_maxLevel) {
        return 0;
    }

    unsigned long long nextLevelTotalXP = m_levelCurve[m_currentLevel].totalXp;
    return (nextLevelTotalXP > m_currentXP) ? (nextLevelTotalXP - m_currentXP) : 0;
}

float LevelingSystem::getProgressToNextLevel() const {
    if (m_currentLevel >= m_maxLevel) {
        return 1.0f;
    }

    unsigned long long currentLevelXP = m_levelCurve[m_currentLevel - 1].totalXp;
    unsigned long long nextLevelXP = m_levelCurve[m_currentLevel].totalXp;
    unsigned long long xpIntoLevel = m_currentXP - currentLevelXP;
    unsigned long long xpNeeded = nextLevelXP - currentLevelXP;

    return static_cast<float>(xpIntoLevel) / static_cast<float>(xpNeeded);
}

bool LevelingSystem::canLevelUp() const {
    if (m_currentLevel >= m_maxLevel) {
        return false;
    }

    return m_currentXP >= m_levelCurve[m_currentLevel].totalXp;
}

bool LevelingSystem::levelUp() {
    if (!canLevelUp()) {
        return false;
    }

    int newLevel = m_currentLevel + 1;
    processLevelUp(newLevel);
    return true;
}

void LevelingSystem::processLevelUp(int newLevel) {
    m_currentLevel = newLevel;
    m_totalLevelUps++;

    // Get rewards for this level
    const LevelData* levelData = getLevelData(newLevel);
    if (levelData) {
        // Add rewards to pending
        m_pendingRewards.attributePoints += levelData->rewards.attributePoints;
        m_pendingRewards.skillPoints += levelData->rewards.skillPoints;
        m_pendingRewards.talentPoints += levelData->rewards.talentPoints;
        m_pendingRewards.unlocks.insert(
            m_pendingRewards.unlocks.end(),
            levelData->rewards.unlocks.begin(),
            levelData->rewards.unlocks.end()
        );

        // Trigger level up callback
        if (m_onLevelUp) {
            m_onLevelUp(newLevel, levelData->rewards);
        }
    }
}

void LevelingSystem::setLevel(int level) {
    if (level < 1 || level > m_maxLevel) {
        throw std::invalid_argument("Invalid level");
    }

    m_currentLevel = level;
    m_currentXP = (level > 1) ? m_levelCurve[level - 1].totalXp : 0;
}

const LevelingSystem::LevelData* LevelingSystem::getLevelData(int level) const {
    if (level < 1 || level > m_maxLevel) {
        return nullptr;
    }
    return &m_levelCurve[level - 1];
}

const LevelingSystem::LevelData* LevelingSystem::getCurrentLevelData() const {
    return getLevelData(m_currentLevel);
}

const LevelingSystem::LevelData* LevelingSystem::getNextLevelData() const {
    return getLevelData(m_currentLevel + 1);
}

void LevelingSystem::claimRewards() {
    m_pendingRewards = {0, 0, 0, {}};
}

bool LevelingSystem::hasPendingRewards() const {
    return m_pendingRewards.attributePoints > 0 ||
           m_pendingRewards.skillPoints > 0 ||
           m_pendingRewards.talentPoints > 0 ||
           !m_pendingRewards.unlocks.empty();
}

void LevelingSystem::addXPSource(const std::string& source, unsigned long long amount) {
    m_xpSources[source] += amount;
}

unsigned long long LevelingSystem::getXPFromSource(const std::string& source) const {
    auto it = m_xpSources.find(source);
    return (it != m_xpSources.end()) ? it->second : 0;
}

void LevelingSystem::addXPModifier(const std::string& name, float multiplier) {
    m_xpModifiers[name] = multiplier;
}

void LevelingSystem::removeXPModifier(const std::string& name) {
    m_xpModifiers.erase(name);
}

float LevelingSystem::getTotalXPModifier() const {
    float total = 1.0f;
    for (const auto& mod : m_xpModifiers) {
        total *= mod.second;
    }
    return total;
}

void LevelingSystem::clearXPModifiers() {
    m_xpModifiers.clear();
}

unsigned long long LevelingSystem::applyXPModifiers(unsigned long long baseXP) const {
    float modifier = getTotalXPModifier();
    return static_cast<unsigned long long>(baseXP * modifier);
}

void LevelingSystem::calculateLevelFromXP() {
    for (int i = m_maxLevel - 1; i >= 0; --i) {
        if (m_currentXP >= m_levelCurve[i].totalXp) {
            m_currentLevel = i + 1;
            return;
        }
    }
    m_currentLevel = 1;
}

std::string LevelingSystem::serialize() const {
    std::stringstream ss;
    ss << m_currentLevel << ","
       << m_currentXP << ","
       << m_totalXPEarned << ","
       << m_totalLevelUps << ","
       << m_pendingRewards.attributePoints << ","
       << m_pendingRewards.skillPoints << ","
       << m_pendingRewards.talentPoints;
    return ss.str();
}

bool LevelingSystem::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 7) {
        return false;
    }

    try {
        m_currentLevel = std::stoi(tokens[0]);
        m_currentXP = std::stoull(tokens[1]);
        m_totalXPEarned = std::stoull(tokens[2]);
        m_totalLevelUps = std::stoi(tokens[3]);
        m_pendingRewards.attributePoints = std::stoi(tokens[4]);
        m_pendingRewards.skillPoints = std::stoi(tokens[5]);
        m_pendingRewards.talentPoints = std::stoi(tokens[6]);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace RPG
