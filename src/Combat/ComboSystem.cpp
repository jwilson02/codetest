#include "ComboSystem.h"
#include <algorithm>
#include <cmath>

namespace Combat {

// ===== ComboTracker Implementation =====

ComboTracker::ComboTracker()
    : m_performer(nullptr)
    , m_state(ComboState::NONE)
    , m_comboCount(0)
    , m_comboTime(0.0f)
    , m_timeSinceLastAction(0.0f)
    , m_decayTimer(0.0f)
    , m_totalDamage(0.0f)
    , m_currentRank(ComboRank::D)
    , m_maxTimeBetweenActions(2.0f)
    , m_comboDecayRate(1.0f)
    , m_damageScalingFactor(0.1f)
    , m_maxComboMultiplier(5.0f)
{
}

ComboTracker::~ComboTracker() {
}

void ComboTracker::startCombo(CombatEntity* performer) {
    m_performer = performer;
    m_state = ComboState::ACTIVE;
    m_comboCount = 0;
    m_comboTime = 0.0f;
    m_timeSinceLastAction = 0.0f;
    m_totalDamage = 0.0f;
    m_actions.clear();
    m_currentRank = ComboRank::D;

    if (m_onComboStart) {
        m_onComboStart(m_comboCount);
    }
}

void ComboTracker::addAction(ComboActionType actionType, const std::string& skillId, float damage, bool hitLanded) {
    if (m_state == ComboState::NONE) {
        startCombo(m_performer);
    }

    // Check timing
    if (m_comboCount > 0 && m_timeSinceLastAction > m_maxTimeBetweenActions) {
        breakCombo();
        startCombo(m_performer);
    }

    // Only count hits that landed
    if (hitLanded) {
        m_comboCount++;
        m_totalDamage += damage;

        ComboAction action;
        action.type = actionType;
        action.skillId = skillId;
        action.timestamp = m_comboTime;
        action.damage = damage;
        action.hitLanded = hitLanded;
        m_actions.push_back(action);

        m_timeSinceLastAction = 0.0f;

        // Update rank
        ComboRank oldRank = m_currentRank;
        updateRank();

        // Trigger callbacks
        if (m_onComboIncrement) {
            m_onComboIncrement(m_comboCount);
        }

        if (oldRank != m_currentRank && m_onRankUp) {
            m_onRankUp(m_currentRank);
        }
    }
    else {
        // Missed attacks break combo
        breakCombo();
    }
}

void ComboTracker::endCombo() {
    if (m_state == ComboState::NONE) return;

    m_state = ComboState::FINISHING;

    if (m_onComboEnd) {
        m_onComboEnd(m_comboCount, m_totalDamage);
    }

    // Clear after a short delay
    m_decayTimer = 1.0f;
}

void ComboTracker::breakCombo() {
    if (m_state == ComboState::NONE) return;

    m_state = ComboState::BROKEN;

    if (m_onComboBreak) {
        m_onComboBreak(m_comboCount);
    }

    // Reset
    m_comboCount = 0;
    m_comboTime = 0.0f;
    m_timeSinceLastAction = 0.0f;
    m_totalDamage = 0.0f;
    m_actions.clear();
    m_currentRank = ComboRank::D;
    m_state = ComboState::NONE;
}

void ComboTracker::update(float deltaTime) {
    if (m_state == ComboState::NONE) return;

    m_comboTime += deltaTime;
    m_timeSinceLastAction += deltaTime;

    // Check for timeout
    if (m_state == ComboState::ACTIVE && m_timeSinceLastAction > m_maxTimeBetweenActions) {
        endCombo();
    }

    // Handle finishing state
    if (m_state == ComboState::FINISHING) {
        m_decayTimer -= deltaTime;
        if (m_decayTimer <= 0.0f) {
            m_state = ComboState::NONE;
            m_comboCount = 0;
            m_totalDamage = 0.0f;
            m_actions.clear();
        }
    }
}

ComboRank ComboTracker::getRank() const {
    return m_currentRank;
}

float ComboTracker::getComboMultiplier() const {
    return ComboUtil::calculateDamageMultiplier(m_comboCount, m_damageScalingFactor, m_maxComboMultiplier);
}

bool ComboTracker::checkForComboChain(const ComboChainDefinition& chain) {
    if (m_actions.size() < chain.sequence.size()) {
        return false;
    }

    // Check last N actions match the chain sequence
    size_t startIndex = m_actions.size() - chain.sequence.size();

    for (size_t i = 0; i < chain.sequence.size(); ++i) {
        if (m_actions[startIndex + i].type != chain.sequence[i]) {
            return false;
        }
    }

    // Check timing
    if (chain.sequence.size() > 1) {
        float totalTime = m_actions.back().timestamp - m_actions[startIndex].timestamp;
        if (totalTime > chain.maxTimeBetweenActions) {
            return false;
        }
    }

    return true;
}

std::vector<ComboChainDefinition*> ComboTracker::getActiveChains() const {
    // This would be populated by the ComboSystem
    return std::vector<ComboChainDefinition*>();
}

void ComboTracker::updateRank() {
    m_currentRank = ComboUtil::calculateRank(m_comboCount, m_totalDamage, m_comboTime);
}

void ComboTracker::triggerCallbacks() {
    // Callback triggering handled in individual methods
}

// ===== ComboSystem Implementation =====

ComboSystem::ComboSystem()
    : m_maxTimeBetweenActions(2.0f)
    , m_comboDecayRate(1.0f)
    , m_damageScalingFactor(0.1f)
    , m_maxComboMultiplier(5.0f)
{
}

ComboSystem::~ComboSystem() {
    // Clean up trackers
    for (auto& pair : m_comboTrackers) {
        delete pair.second;
    }
    m_comboTrackers.clear();
}

void ComboSystem::update(float deltaTime) {
    for (auto& pair : m_comboTrackers) {
        pair.second->update(deltaTime);
    }
}

ComboTracker* ComboSystem::getComboTracker(CombatEntity* entity) {
    auto it = m_comboTrackers.find(entity);
    if (it != m_comboTrackers.end()) {
        return it->second;
    }
    return nullptr;
}

ComboTracker* ComboSystem::createComboTracker(CombatEntity* entity) {
    if (!entity) return nullptr;

    // Check if already exists
    auto existing = getComboTracker(entity);
    if (existing) return existing;

    // Create new tracker
    ComboTracker* tracker = new ComboTracker();
    tracker->startCombo(entity);
    m_comboTrackers[entity] = tracker;

    // Initialize stats
    ComboStats stats;
    stats.totalCombos = 0;
    stats.longestCombo = 0;
    stats.highestMultiplier = 1.0f;
    stats.highestRank = ComboRank::D;
    m_stats[entity] = stats;

    return tracker;
}

void ComboSystem::removeComboTracker(CombatEntity* entity) {
    auto it = m_comboTrackers.find(entity);
    if (it != m_comboTrackers.end()) {
        delete it->second;
        m_comboTrackers.erase(it);
    }
}

void ComboSystem::registerComboChain(const ComboChainDefinition& chain) {
    m_comboChains[chain.id] = chain;
}

ComboChainDefinition* ComboSystem::getComboChain(const std::string& chainId) {
    auto it = m_comboChains.find(chainId);
    if (it != m_comboChains.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<ComboChainDefinition*> ComboSystem::getAllComboChains() {
    std::vector<ComboChainDefinition*> chains;
    for (auto& pair : m_comboChains) {
        chains.push_back(&pair.second);
    }
    return chains;
}

std::vector<ComboChainDefinition*> ComboSystem::checkForMatchingChains(const std::vector<ComboActionType>& sequence) {
    std::vector<ComboChainDefinition*> matches;

    for (auto& pair : m_comboChains) {
        if (pair.second.sequence.size() != sequence.size()) {
            continue;
        }

        bool matches_all = true;
        for (size_t i = 0; i < sequence.size(); ++i) {
            if (pair.second.sequence[i] != sequence[i]) {
                matches_all = false;
                break;
            }
        }

        if (matches_all) {
            matches.push_back(&pair.second);
        }
    }

    return matches;
}

ComboSystem::ComboStats ComboSystem::getStats(CombatEntity* entity) {
    auto it = m_stats.find(entity);
    if (it != m_stats.end()) {
        return it->second;
    }
    return ComboStats();
}

void ComboSystem::resetStats(CombatEntity* entity) {
    auto it = m_stats.find(entity);
    if (it != m_stats.end()) {
        it->second = ComboStats();
    }
}

// ===== ComboChainBuilder Implementation =====

ComboChainBuilder::ComboChainBuilder(const std::string& id) {
    m_chain.id = id;
    m_chain.name = id;
    m_chain.maxTimeBetweenActions = 2.0f;
    m_chain.totalExecutionTime = 5.0f;
    m_chain.levelRequired = 1;
    m_chain.damageMultiplier = 1.5f;
}

ComboChainBuilder& ComboChainBuilder::setName(const std::string& name) {
    m_chain.name = name;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setDescription(const std::string& desc) {
    m_chain.description = desc;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::addAction(ComboActionType action) {
    m_chain.sequence.push_back(action);
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setTimingWindow(float maxTime) {
    m_chain.maxTimeBetweenActions = maxTime;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setDamageMultiplier(float multiplier) {
    m_chain.damageMultiplier = multiplier;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setFinisher(const std::string& skillId) {
    m_chain.finisherSkillId = skillId;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::addEffect(const std::string& effectId) {
    m_chain.appliedEffects.push_back(effectId);
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setRequirement(int level, const std::string& weaponType) {
    m_chain.levelRequired = level;
    m_chain.weaponTypeRequired = weaponType;
    return *this;
}

ComboChainBuilder& ComboChainBuilder::setVisuals(const std::string& anim, const std::string& particle) {
    m_chain.animationOverride = anim;
    m_chain.particleEffect = particle;
    return *this;
}

ComboChainDefinition ComboChainBuilder::build() {
    return m_chain;
}

// ===== ComboUtil Implementation =====

namespace ComboUtil {

ComboRank calculateRank(int comboCount, float totalDamage, float comboTime) {
    // Rank based on combo count primarily
    if (comboCount >= 100) return ComboRank::SSS;
    if (comboCount >= 75) return ComboRank::SS;
    if (comboCount >= 50) return ComboRank::S;
    if (comboCount >= 30) return ComboRank::A;
    if (comboCount >= 15) return ComboRank::B;
    if (comboCount >= 8) return ComboRank::C;
    return ComboRank::D;
}

float calculateDamageMultiplier(int comboCount, float scalingFactor, float maxMultiplier) {
    // Logarithmic scaling
    float multiplier = 1.0f + (scalingFactor * std::log(comboCount + 1.0f));
    return std::min(multiplier, maxMultiplier);
}

bool isComboValid(float timeSinceLastAction, float maxTime) {
    return timeSinceLastAction <= maxTime;
}

std::string rankToString(ComboRank rank) {
    switch (rank) {
        case ComboRank::D: return "D";
        case ComboRank::C: return "C";
        case ComboRank::B: return "B";
        case ComboRank::A: return "A";
        case ComboRank::S: return "S";
        case ComboRank::SS: return "SS";
        case ComboRank::SSS: return "SSS";
        default: return "NONE";
    }
}

Color getRankColor(ComboRank rank) {
    switch (rank) {
        case ComboRank::D:
            return Color{0.5f, 0.5f, 0.5f, 1.0f}; // Gray
        case ComboRank::C:
            return Color{1.0f, 1.0f, 1.0f, 1.0f}; // White
        case ComboRank::B:
            return Color{0.0f, 1.0f, 0.0f, 1.0f}; // Green
        case ComboRank::A:
            return Color{0.0f, 0.5f, 1.0f, 1.0f}; // Blue
        case ComboRank::S:
            return Color{1.0f, 1.0f, 0.0f, 1.0f}; // Yellow
        case ComboRank::SS:
            return Color{1.0f, 0.5f, 0.0f, 1.0f}; // Orange
        case ComboRank::SSS:
            return Color{1.0f, 0.0f, 0.0f, 1.0f}; // Red
        default:
            return Color{1.0f, 1.0f, 1.0f, 1.0f};
    }
}

} // namespace ComboUtil

} // namespace Combat
