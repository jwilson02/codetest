#include "AISystem.h"
#include "Pathfinding.h"
#include "EnemyAI.h"
#include "NPCAI.h"
#include "BossAI.h"
#include <algorithm>
#include <cmath>

namespace AI {

AISystem::AISystem()
    : m_nextAIId(1)
    , m_currentDifficultyLevel(1.0f)
    , m_debugDraw(false)
{
}

AISystem::~AISystem() {
    Shutdown();
}

void AISystem::Initialize(int worldWidth, int worldHeight) {
    // Initialize pathfinding system
    m_pathfinding = std::make_shared<Pathfinding>();
    m_pathfinding->Initialize(worldWidth, worldHeight);

    // Set default difficulty
    SetDifficulty(1.0f);

    m_aiEntities.clear();
    m_threatTables.clear();
    m_groups.clear();
    m_eventCallbacks.clear();

    m_nextAIId = 1;
}

void AISystem::Shutdown() {
    // Clean up all AI entities
    m_aiEntities.clear();
    m_threatTables.clear();
    m_groups.clear();
    m_eventCallbacks.clear();

    if (m_pathfinding) {
        m_pathfinding->Shutdown();
        m_pathfinding.reset();
    }
}

int AISystem::RegisterEnemy(int entityId, const std::string& archetypeName) {
    int aiId = m_nextAIId++;

    auto aiEntity = std::make_unique<AIEntity>(aiId, AIEntityType::ENEMY);

    // Create enemy AI component
    auto enemyAI = new EnemyAI(aiId, entityId, archetypeName);
    enemyAI->SetAISystem(this);
    enemyAI->Initialize();

    aiEntity->aiComponent = enemyAI;
    aiEntity->updateInterval = 0.1f; // Update enemies 10 times per second

    m_aiEntities[aiId] = std::move(aiEntity);

    // Apply current difficulty scaling
    ApplyDifficultyScaling(aiId);

    return aiId;
}

int AISystem::RegisterNPC(int entityId, const std::string& npcType) {
    int aiId = m_nextAIId++;

    auto aiEntity = std::make_unique<AIEntity>(aiId, AIEntityType::NPC);

    // Create NPC AI component
    auto npcAI = new NPCAI(aiId, entityId, npcType);
    npcAI->SetAISystem(this);
    npcAI->Initialize();

    aiEntity->aiComponent = npcAI;
    aiEntity->updateInterval = 0.2f; // Update NPCs 5 times per second (less frequent)

    m_aiEntities[aiId] = std::move(aiEntity);

    return aiId;
}

int AISystem::RegisterBoss(int entityId, const std::string& bossName) {
    int aiId = m_nextAIId++;

    auto aiEntity = std::make_unique<AIEntity>(aiId, AIEntityType::BOSS);

    // Create boss AI component
    auto bossAI = new BossAI(aiId, entityId, bossName);
    bossAI->SetAISystem(this);
    bossAI->Initialize();

    aiEntity->aiComponent = bossAI;
    aiEntity->updateInterval = 0.05f; // Update bosses 20 times per second (most frequent)

    m_aiEntities[aiId] = std::move(aiEntity);

    // Apply current difficulty scaling
    ApplyDifficultyScaling(aiId);

    return aiId;
}

void AISystem::UnregisterEntity(int aiId) {
    auto it = m_aiEntities.find(aiId);
    if (it != m_aiEntities.end()) {
        AIEntity* entity = it->second.get();

        // Clean up the AI component
        if (entity->aiComponent) {
            switch (entity->type) {
                case AIEntityType::ENEMY:
                    delete static_cast<EnemyAI*>(entity->aiComponent);
                    break;
                case AIEntityType::NPC:
                    delete static_cast<NPCAI*>(entity->aiComponent);
                    break;
                case AIEntityType::BOSS:
                    delete static_cast<BossAI*>(entity->aiComponent);
                    break;
            }
            entity->aiComponent = nullptr;
        }

        // Remove from groups
        for (auto& [groupName, members] : m_groups) {
            members.erase(std::remove(members.begin(), members.end(), aiId), members.end());
        }

        // Clear threat table
        ClearAllThreat(aiId);

        // Remove the entity
        m_aiEntities.erase(it);
    }
}

void AISystem::Update(float deltaTime) {
    // Update all active AI entities
    for (auto& [aiId, entity] : m_aiEntities) {
        if (!entity->isActive) continue;

        entity->timeSinceLastUpdate += deltaTime;

        // Check if it's time to update this entity
        if (entity->timeSinceLastUpdate >= entity->updateInterval) {
            UpdateEntity(aiId, entity->timeSinceLastUpdate);
            entity->timeSinceLastUpdate = 0.0f;
        }
    }

    // Periodic cleanup
    static float cleanupTimer = 0.0f;
    cleanupTimer += deltaTime;
    if (cleanupTimer >= 5.0f) {
        CleanupInactiveEntities();
        cleanupTimer = 0.0f;
    }
}

void AISystem::UpdateEntity(int aiId, float deltaTime) {
    AIEntity* entity = GetAIEntity(aiId);
    if (!entity || !entity->isActive || !entity->aiComponent) return;

    switch (entity->type) {
        case AIEntityType::ENEMY: {
            EnemyAI* enemyAI = static_cast<EnemyAI*>(entity->aiComponent);
            enemyAI->Update(deltaTime);
            break;
        }
        case AIEntityType::NPC: {
            NPCAI* npcAI = static_cast<NPCAI*>(entity->aiComponent);
            npcAI->Update(deltaTime);
            break;
        }
        case AIEntityType::BOSS: {
            BossAI* bossAI = static_cast<BossAI*>(entity->aiComponent);
            bossAI->Update(deltaTime);
            break;
        }
    }
}

void AISystem::SetDifficulty(float difficultyLevel) {
    m_currentDifficultyLevel = difficultyLevel;
    UpdateDifficultySettings();

    // Reapply difficulty to all existing entities
    for (auto& [aiId, entity] : m_aiEntities) {
        if (entity->type == AIEntityType::ENEMY || entity->type == AIEntityType::BOSS) {
            ApplyDifficultyScaling(aiId);
        }
    }
}

void AISystem::UpdateDifficultySettings() {
    float level = m_currentDifficultyLevel;

    if (level <= 0.5f) {
        // Easy mode
        m_difficultySettings.damageMultiplier = 0.7f;
        m_difficultySettings.healthMultiplier = 0.8f;
        m_difficultySettings.speedMultiplier = 0.9f;
        m_difficultySettings.aggroRangeMultiplier = 0.8f;
        m_difficultySettings.xpMultiplier = 0.8f;
        m_difficultySettings.additionalAbilities = 0;
        m_difficultySettings.enableAdvancedTactics = false;
    }
    else if (level <= 1.5f) {
        // Normal mode
        m_difficultySettings.damageMultiplier = 1.0f;
        m_difficultySettings.healthMultiplier = 1.0f;
        m_difficultySettings.speedMultiplier = 1.0f;
        m_difficultySettings.aggroRangeMultiplier = 1.0f;
        m_difficultySettings.xpMultiplier = 1.0f;
        m_difficultySettings.additionalAbilities = 0;
        m_difficultySettings.enableAdvancedTactics = false;
    }
    else if (level <= 2.5f) {
        // Hard mode
        m_difficultySettings.damageMultiplier = 1.3f;
        m_difficultySettings.healthMultiplier = 1.5f;
        m_difficultySettings.speedMultiplier = 1.1f;
        m_difficultySettings.aggroRangeMultiplier = 1.2f;
        m_difficultySettings.xpMultiplier = 1.5f;
        m_difficultySettings.additionalAbilities = 1;
        m_difficultySettings.enableAdvancedTactics = true;
    }
    else {
        // Extreme mode
        float extremeScale = 1.0f + (level - 2.5f) * 0.3f;
        m_difficultySettings.damageMultiplier = 1.5f * extremeScale;
        m_difficultySettings.healthMultiplier = 2.0f * extremeScale;
        m_difficultySettings.speedMultiplier = 1.2f;
        m_difficultySettings.aggroRangeMultiplier = 1.5f;
        m_difficultySettings.xpMultiplier = 2.0f * extremeScale;
        m_difficultySettings.additionalAbilities = 2;
        m_difficultySettings.enableAdvancedTactics = true;
    }
}

void AISystem::ApplyDifficultyScaling(int aiId) {
    AIEntity* entity = GetAIEntity(aiId);
    if (!entity || !entity->aiComponent) return;

    if (entity->type == AIEntityType::ENEMY) {
        EnemyAI* enemyAI = static_cast<EnemyAI*>(entity->aiComponent);
        enemyAI->ApplyDifficultyScaling(m_difficultySettings);
    }
    else if (entity->type == AIEntityType::BOSS) {
        BossAI* bossAI = static_cast<BossAI*>(entity->aiComponent);
        bossAI->ApplyDifficultyScaling(m_difficultySettings);
    }
}

void AISystem::AddThreat(int enemyId, int targetId, float threatValue) {
    m_threatTables[enemyId][targetId] += threatValue;
}

void AISystem::RemoveThreat(int enemyId, int targetId) {
    auto it = m_threatTables.find(enemyId);
    if (it != m_threatTables.end()) {
        it->second.erase(targetId);
    }
}

void AISystem::ClearAllThreat(int enemyId) {
    m_threatTables.erase(enemyId);
}

int AISystem::GetHighestThreatTarget(int enemyId) {
    auto it = m_threatTables.find(enemyId);
    if (it == m_threatTables.end() || it->second.empty()) {
        return -1;
    }

    int highestThreatTarget = -1;
    float highestThreat = 0.0f;

    for (const auto& [targetId, threat] : it->second) {
        if (threat > highestThreat) {
            highestThreat = threat;
            highestThreatTarget = targetId;
        }
    }

    return highestThreatTarget;
}

void AISystem::RegisterToGroup(int aiId, const std::string& groupName) {
    auto& members = m_groups[groupName];
    if (std::find(members.begin(), members.end(), aiId) == members.end()) {
        members.push_back(aiId);
    }
}

void AISystem::UnregisterFromGroup(int aiId, const std::string& groupName) {
    auto it = m_groups.find(groupName);
    if (it != m_groups.end()) {
        auto& members = it->second;
        members.erase(std::remove(members.begin(), members.end(), aiId), members.end());
    }
}

std::vector<int> AISystem::GetGroupMembers(const std::string& groupName) {
    auto it = m_groups.find(groupName);
    if (it != m_groups.end()) {
        return it->second;
    }
    return std::vector<int>();
}

bool AISystem::IsEntityActive(int aiId) const {
    const AIEntity* entity = GetAIEntity(aiId);
    return entity ? entity->isActive : false;
}

void AISystem::SetEntityActive(int aiId, bool active) {
    AIEntity* entity = GetAIEntity(aiId);
    if (entity) {
        entity->isActive = active;
    }
}

AIEntityType AISystem::GetEntityType(int aiId) const {
    const AIEntity* entity = GetAIEntity(aiId);
    return entity ? entity->type : AIEntityType::ENEMY;
}

void AISystem::RegisterEventCallback(const std::string& eventName, AIEventCallback callback) {
    m_eventCallbacks[eventName].push_back(callback);
}

void AISystem::TriggerEvent(int aiId, const std::string& eventName, void* eventData) {
    auto it = m_eventCallbacks.find(eventName);
    if (it != m_eventCallbacks.end()) {
        for (auto& callback : it->second) {
            callback(aiId, eventName, eventData);
        }
    }
}

int AISystem::GetActiveAICount() const {
    int count = 0;
    for (const auto& [aiId, entity] : m_aiEntities) {
        if (entity->isActive) {
            count++;
        }
    }
    return count;
}

int AISystem::GetAICountByType(AIEntityType type) const {
    int count = 0;
    for (const auto& [aiId, entity] : m_aiEntities) {
        if (entity->type == type && entity->isActive) {
            count++;
        }
    }
    return count;
}

void AISystem::CleanupInactiveEntities() {
    // Remove entities that have been inactive for a while
    // (In a real implementation, you might want to track inactivity time)
    // For now, we just clean up groups
    for (auto& [groupName, members] : m_groups) {
        members.erase(
            std::remove_if(members.begin(), members.end(),
                [this](int aiId) {
                    return m_aiEntities.find(aiId) == m_aiEntities.end();
                }),
            members.end()
        );
    }
}

AIEntity* AISystem::GetAIEntity(int aiId) {
    auto it = m_aiEntities.find(aiId);
    return (it != m_aiEntities.end()) ? it->second.get() : nullptr;
}

const AIEntity* AISystem::GetAIEntity(int aiId) const {
    auto it = m_aiEntities.find(aiId);
    return (it != m_aiEntities.end()) ? it->second.get() : nullptr;
}

} // namespace AI
