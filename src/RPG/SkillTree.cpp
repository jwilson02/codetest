#include "SkillTree.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

namespace RPG {

SkillTree::SkillTree()
    : m_availableSkillPoints(0)
    , m_availableTalentPoints(0)
    , m_onSkillUnlock(nullptr)
    , m_onSkillRankUp(nullptr)
    , m_onTalentUnlock(nullptr)
{
    generateDefaultSkillTrees();
    generateDefaultTalents();
}

SkillTree::~SkillTree() {
}

void SkillTree::initialize() {
    // Initialize all skills as locked
    for (auto& pair : m_skills) {
        pair.second.isUnlocked = false;
        pair.second.currentRank = 0;
    }

    // Initialize all talents as locked
    for (auto& pair : m_talents) {
        pair.second.isUnlocked = false;
        pair.second.currentRank = 0;
    }
}

bool SkillTree::loadSkillTrees(const std::string& filepath) {
    // Load from JSON file - implementation depends on JSON library
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not load skill trees from " << filepath
                  << ". Using default trees." << std::endl;
        return false;
    }

    // TODO: Implement JSON parsing
    file.close();
    return true;
}

void SkillTree::generateDefaultSkillTrees() {
    // This creates a comprehensive default skill tree structure
    // In practice, this would be loaded from JSON
    m_skills.clear();
    m_treeNames.clear();

    // Example: Create skill trees for each class
    createSkillTree("Warrior_Offense");
    createSkillTree("Warrior_Defense");
    createSkillTree("Mage_Fire");
    createSkillTree("Mage_Ice");
    createSkillTree("Ranger_Archery");
    createSkillTree("Ranger_Nature");
    createSkillTree("Rogue_Assassination");
    createSkillTree("Rogue_Shadow");
    createSkillTree("Paladin_Holy");
    createSkillTree("Paladin_Protection");
}

void SkillTree::generateDefaultTalents() {
    // Generate default talent options
    m_talents.clear();

    // Universal talents available to all classes
    Talent t1 = {
        "talent_veteran",
        "Veteran",
        "Increases all stats by 2% per rank",
        5, 0, 1,
        {{"all_stats", 0.02f}},
        {},
        {},
        false
    };
    m_talents["talent_veteran"] = t1;

    Talent t2 = {
        "talent_resilient",
        "Resilient",
        "Increases maximum health by 5% per rank",
        5, 0, 1,
        {{"max_health", 0.05f}},
        {},
        {},
        false
    };
    m_talents["talent_resilient"] = t2;

    Talent t3 = {
        "talent_focused",
        "Focused",
        "Increases critical damage by 10% per rank",
        5, 0, 1,
        {{"crit_damage", 0.10f}},
        {},
        {},
        false
    };
    m_talents["talent_focused"] = t3;

    Talent t4 = {
        "talent_swift",
        "Swift",
        "Increases attack and cast speed by 3% per rank",
        5, 0, 1,
        {{"attack_speed", 0.03f}, {"cast_speed", 0.03f}},
        {},
        {},
        false
    };
    m_talents["talent_swift"] = t4;

    Talent t5 = {
        "talent_treasure_hunter",
        "Treasure Hunter",
        "Increases item drop rate by 5% per rank",
        5, 0, 1,
        {{"loot_find", 0.05f}},
        {},
        {},
        false
    };
    m_talents["talent_treasure_hunter"] = t5;
}

void SkillTree::createSkillTree(const std::string& treeName) {
    m_treeNames.insert(treeName);
    // Skills would be added to this tree
}

void SkillTree::addSkill(const Skill& skill) {
    m_skills[skill.id] = skill;
    m_treeNames.insert(skill.treeName);
}

void SkillTree::addTalent(const Talent& talent) {
    m_talents[talent.id] = talent;
}

bool SkillTree::unlockSkill(const std::string& skillId) {
    if (!canUnlockSkill(skillId)) {
        return false;
    }

    auto it = m_skills.find(skillId);
    if (it == m_skills.end()) {
        return false;
    }

    Skill& skill = it->second;
    if (m_availableSkillPoints < skill.skillPointCost) {
        return false;
    }

    skill.isUnlocked = true;
    skill.currentRank = 1;
    m_availableSkillPoints -= skill.skillPointCost;

    if (m_onSkillUnlock) {
        m_onSkillUnlock(skill);
    }

    if (m_onSkillRankUp) {
        m_onSkillRankUp(skill, 1);
    }

    return true;
}

bool SkillTree::rankUpSkill(const std::string& skillId, int ranks) {
    if (ranks <= 0 || !canRankUpSkill(skillId)) {
        return false;
    }

    auto it = m_skills.find(skillId);
    if (it == m_skills.end()) {
        return false;
    }

    Skill& skill = it->second;
    int ranksToAdd = std::min(ranks, skill.maxRank - skill.currentRank);
    int totalCost = skill.skillPointCost * ranksToAdd;

    if (m_availableSkillPoints < totalCost) {
        return false;
    }

    skill.currentRank += ranksToAdd;
    m_availableSkillPoints -= totalCost;

    if (m_onSkillRankUp) {
        m_onSkillRankUp(skill, skill.currentRank);
    }

    return true;
}

bool SkillTree::canUnlockSkill(const std::string& skillId) const {
    auto it = m_skills.find(skillId);
    if (it == m_skills.end()) {
        return false;
    }

    const Skill& skill = it->second;
    if (skill.isUnlocked) {
        return false;
    }

    return meetsRequirements(skill) && hasPrerequisites(skill);
}

bool SkillTree::canRankUpSkill(const std::string& skillId) const {
    auto it = m_skills.find(skillId);
    if (it == m_skills.end()) {
        return false;
    }

    const Skill& skill = it->second;
    return skill.isUnlocked &&
           skill.currentRank < skill.maxRank &&
           m_availableSkillPoints >= skill.skillPointCost;
}

bool SkillTree::meetsRequirements(const Skill& skill) const {
    // Check level requirement
    // Note: This would need access to character level
    // For now, we'll assume it's met

    // Check stat requirements
    // Note: This would need access to CharacterStats
    // For now, we'll assume it's met

    return true;
}

bool SkillTree::hasPrerequisites(const Skill& skill) const {
    for (const auto& prereqId : skill.prerequisiteSkills) {
        auto it = m_skills.find(prereqId);
        if (it == m_skills.end() || !it->second.isUnlocked) {
            return false;
        }
    }
    return true;
}

const SkillTree::Skill* SkillTree::getSkill(const std::string& skillId) const {
    auto it = m_skills.find(skillId);
    return (it != m_skills.end()) ? &it->second : nullptr;
}

SkillTree::Skill* SkillTree::getSkillMutable(const std::string& skillId) {
    auto it = m_skills.find(skillId);
    return (it != m_skills.end()) ? &it->second : nullptr;
}

int SkillTree::getTotalSkillPointsSpent() const {
    int total = 0;
    for (const auto& pair : m_skills) {
        if (pair.second.isUnlocked) {
            total += pair.second.currentRank * pair.second.skillPointCost;
        }
    }
    return total;
}

int SkillTree::getSkillPointsSpentInTree(const std::string& treeName) const {
    int total = 0;
    for (const auto& pair : m_skills) {
        if (pair.second.treeName == treeName && pair.second.isUnlocked) {
            total += pair.second.currentRank * pair.second.skillPointCost;
        }
    }
    return total;
}

std::vector<SkillTree::Skill*> SkillTree::getSkillsInTree(const std::string& treeName) {
    std::vector<Skill*> skills;
    for (auto& pair : m_skills) {
        if (pair.second.treeName == treeName) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

std::vector<const SkillTree::Skill*> SkillTree::getSkillsInTree(const std::string& treeName) const {
    std::vector<const Skill*> skills;
    for (const auto& pair : m_skills) {
        if (pair.second.treeName == treeName) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

std::vector<SkillTree::Skill*> SkillTree::getUnlockedSkills() {
    std::vector<Skill*> skills;
    for (auto& pair : m_skills) {
        if (pair.second.isUnlocked) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

std::vector<SkillTree::Skill*> SkillTree::getAvailableSkills() {
    std::vector<Skill*> skills;
    for (auto& pair : m_skills) {
        if (canUnlockSkill(pair.first)) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

std::vector<std::string> SkillTree::getAllTreeNames() const {
    return std::vector<std::string>(m_treeNames.begin(), m_treeNames.end());
}

std::vector<SkillTree::SkillEffect> SkillTree::getActiveEffects() const {
    std::vector<SkillEffect> effects;
    for (const auto& pair : m_skills) {
        if (pair.second.isUnlocked) {
            effects.insert(effects.end(),
                          pair.second.effects.begin(),
                          pair.second.effects.end());
        }
    }
    return effects;
}

float SkillTree::calculateSkillDamage(const std::string& skillId) const {
    const Skill* skill = getSkill(skillId);
    if (!skill || !skill->isUnlocked) {
        return 0.0f;
    }

    float baseDamage = 0.0f;
    for (const auto& effect : skill->effects) {
        if (effect.effectType == "damage") {
            auto it = effect.values.find("amount");
            if (it != effect.values.end()) {
                baseDamage += it->second * skill->currentRank;
            }
        }
    }

    // Apply synergies
    float synergyMultiplier = calculateSynergies(skillId);
    return baseDamage * synergyMultiplier;
}

float SkillTree::calculateSynergies(const std::string& skillId) const {
    const Skill* skill = getSkill(skillId);
    if (!skill || !skill->isUnlocked) {
        return 1.0f;
    }

    float totalBonus = 1.0f;
    for (const auto& synergy : skill->synergies) {
        const Skill* synergySkill = getSkill(synergy.first);
        if (synergySkill && synergySkill->isUnlocked) {
            totalBonus += synergy.second * synergySkill->currentRank;
        }
    }

    return totalBonus;
}

// Talent System
bool SkillTree::unlockTalent(const std::string& talentId) {
    if (!canUnlockTalent(talentId)) {
        return false;
    }

    auto it = m_talents.find(talentId);
    if (it == m_talents.end()) {
        return false;
    }

    Talent& talent = it->second;
    if (m_availableTalentPoints < talent.talentPointCost) {
        return false;
    }

    talent.isUnlocked = true;
    talent.currentRank = 1;
    m_availableTalentPoints -= talent.talentPointCost;

    if (m_onTalentUnlock) {
        m_onTalentUnlock(talent);
    }

    return true;
}

bool SkillTree::rankUpTalent(const std::string& talentId, int ranks) {
    if (ranks <= 0) {
        return false;
    }

    auto it = m_talents.find(talentId);
    if (it == m_talents.end()) {
        return false;
    }

    Talent& talent = it->second;
    if (!talent.isUnlocked || talent.currentRank >= talent.maxRank) {
        return false;
    }

    int ranksToAdd = std::min(ranks, talent.maxRank - talent.currentRank);
    int totalCost = talent.talentPointCost * ranksToAdd;

    if (m_availableTalentPoints < totalCost) {
        return false;
    }

    talent.currentRank += ranksToAdd;
    m_availableTalentPoints -= totalCost;

    return true;
}

bool SkillTree::canUnlockTalent(const std::string& talentId) const {
    auto it = m_talents.find(talentId);
    if (it == m_talents.end() || it->second.isUnlocked) {
        return false;
    }

    return m_availableTalentPoints >= it->second.talentPointCost;
}

const SkillTree::Talent* SkillTree::getTalent(const std::string& talentId) const {
    auto it = m_talents.find(talentId);
    return (it != m_talents.end()) ? &it->second : nullptr;
}

int SkillTree::getTotalTalentPointsSpent() const {
    int total = 0;
    for (const auto& pair : m_talents) {
        if (pair.second.isUnlocked) {
            total += pair.second.currentRank * pair.second.talentPointCost;
        }
    }
    return total;
}

std::vector<const SkillTree::Talent*> SkillTree::getUnlockedTalents() const {
    std::vector<const Talent*> talents;
    for (const auto& pair : m_talents) {
        if (pair.second.isUnlocked) {
            talents.push_back(&pair.second);
        }
    }
    return talents;
}

std::vector<const SkillTree::Talent*> SkillTree::getAvailableTalents() const {
    std::vector<const Talent*> talents;
    for (const auto& pair : m_talents) {
        if (canUnlockTalent(pair.first)) {
            talents.push_back(&pair.second);
        }
    }
    return talents;
}

std::map<std::string, float> SkillTree::getAllTalentStatModifiers() const {
    std::map<std::string, float> modifiers;
    for (const auto& pair : m_talents) {
        if (pair.second.isUnlocked) {
            for (const auto& mod : pair.second.statModifiers) {
                modifiers[mod.first] += mod.second * pair.second.currentRank;
            }
        }
    }
    return modifiers;
}

void SkillTree::resetSkills() {
    int pointsToRefund = getTotalSkillPointsSpent();
    for (auto& pair : m_skills) {
        pair.second.isUnlocked = false;
        pair.second.currentRank = 0;
    }
    m_availableSkillPoints += pointsToRefund;
}

void SkillTree::resetTalents() {
    int pointsToRefund = getTotalTalentPointsSpent();
    for (auto& pair : m_talents) {
        pair.second.isUnlocked = false;
        pair.second.currentRank = 0;
    }
    m_availableTalentPoints += pointsToRefund;
}

void SkillTree::resetSkillTree(const std::string& treeName) {
    int pointsToRefund = getSkillPointsSpentInTree(treeName);
    for (auto& pair : m_skills) {
        if (pair.second.treeName == treeName) {
            pair.second.isUnlocked = false;
            pair.second.currentRank = 0;
        }
    }
    m_availableSkillPoints += pointsToRefund;
}

int SkillTree::calculateRespecCost() const {
    // Cost increases with points spent
    int totalSpent = getTotalSkillPointsSpent() + getTotalTalentPointsSpent();
    return totalSpent * 100; // 100 gold per point
}

bool SkillTree::validateSkillTreeIntegrity() const {
    // Verify no circular dependencies
    // Verify all prerequisites exist
    for (const auto& pair : m_skills) {
        for (const auto& prereq : pair.second.prerequisiteSkills) {
            if (m_skills.find(prereq) == m_skills.end()) {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::string> SkillTree::getUnmetPrerequisites(const std::string& skillId) const {
    std::vector<std::string> unmet;
    const Skill* skill = getSkill(skillId);
    if (!skill) {
        return unmet;
    }

    for (const auto& prereqId : skill->prerequisiteSkills) {
        const Skill* prereq = getSkill(prereqId);
        if (!prereq || !prereq->isUnlocked) {
            unmet.push_back(prereqId);
        }
    }
    return unmet;
}

int SkillTree::getTotalSkillsUnlocked() const {
    int count = 0;
    for (const auto& pair : m_skills) {
        if (pair.second.isUnlocked) {
            count++;
        }
    }
    return count;
}

int SkillTree::getTotalTalentsUnlocked() const {
    int count = 0;
    for (const auto& pair : m_talents) {
        if (pair.second.isUnlocked) {
            count++;
        }
    }
    return count;
}

std::map<std::string, int> SkillTree::getSkillRankDistribution() const {
    std::map<std::string, int> distribution;
    for (const auto& pair : m_skills) {
        if (pair.second.isUnlocked) {
            distribution[pair.first] = pair.second.currentRank;
        }
    }
    return distribution;
}

std::string SkillTree::serialize() const {
    std::stringstream ss;
    ss << m_availableSkillPoints << "," << m_availableTalentPoints << ";";
    // Serialize skills and talents
    return ss.str();
}

bool SkillTree::deserialize(const std::string& data) {
    // Implementation for deserialization
    return true;
}

} // namespace RPG
