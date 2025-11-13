#include "NPCAI.h"
#include "AISystem.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace AI {

NPCAI::NPCAI(int aiId, int entityId, const std::string& npcTypeName)
    : m_aiId(aiId)
    , m_entityId(entityId)
    , m_aiSystem(nullptr)
    , m_npcType(NPCType::VILLAGER)
    , m_npcTypeName(npcTypeName)
    , m_name("NPC")
    , m_occupation("Villager")
    , m_currentActivity(nullptr)
    , m_timeOfDay(8.0f)
    , m_currentWaypointIndex(0)
    , m_wandering(false)
    , m_wanderRadius(10.0f)
    , m_wanderTimer(0.0f)
    , m_interactionTimer(0.0f)
    , m_animationState("idle")
{
}

NPCAI::~NPCAI() {
}

void NPCAI::Initialize() {
    // Determine NPC type from name
    if (m_npcTypeName == "villager" || m_npcTypeName == "Villager") {
        SetNPCType(NPCType::VILLAGER);
    } else if (m_npcTypeName == "merchant" || m_npcTypeName == "Merchant") {
        SetNPCType(NPCType::MERCHANT);
    } else if (m_npcTypeName == "guard" || m_npcTypeName == "Guard") {
        SetNPCType(NPCType::GUARD);
    } else if (m_npcTypeName == "quest_giver" || m_npcTypeName == "QuestGiver") {
        SetNPCType(NPCType::QUEST_GIVER);
    } else if (m_npcTypeName == "companion" || m_npcTypeName == "Companion") {
        SetNPCType(NPCType::COMPANION);
    } else if (m_npcTypeName == "ambient" || m_npcTypeName == "Ambient") {
        SetNPCType(NPCType::AMBIENT);
    } else if (m_npcTypeName == "trainer" || m_npcTypeName == "Trainer") {
        SetNPCType(NPCType::TRAINER);
    } else {
        SetNPCType(NPCType::VILLAGER); // Default
    }

    // Build state machine
    BuildStateMachine();
}

void NPCAI::Update(float deltaTime) {
    // Update schedule
    UpdateSchedule(deltaTime);

    // Update state machine
    if (m_stateMachine) {
        m_stateMachine->Update(deltaTime);
    }

    // Update current activity
    UpdateActivity(deltaTime);

    // Update pathfinding
    UpdatePathing(deltaTime);

    // Update wandering
    if (m_wandering) {
        UpdateWandering(deltaTime);
    }

    // Update social interaction
    UpdateInteraction(deltaTime);
}

void NPCAI::SetNPCType(NPCType type) {
    m_npcType = type;

    // Set default personality based on type
    switch (type) {
        case NPCType::VILLAGER:
            m_occupation = "Villager";
            m_personality.friendliness = 0.7f;
            m_personality.chattiness = 0.6f;
            m_personality.bravery = 0.4f;
            m_personality.helpfulness = 0.7f;
            break;

        case NPCType::MERCHANT:
            m_occupation = "Merchant";
            m_personality.friendliness = 0.8f;
            m_personality.chattiness = 0.8f;
            m_personality.bravery = 0.3f;
            m_personality.helpfulness = 0.6f;
            break;

        case NPCType::GUARD:
            m_occupation = "Guard";
            m_personality.friendliness = 0.5f;
            m_personality.chattiness = 0.3f;
            m_personality.bravery = 0.9f;
            m_personality.helpfulness = 0.7f;
            break;

        case NPCType::QUEST_GIVER:
            m_occupation = "Quest Giver";
            m_personality.friendliness = 0.7f;
            m_personality.chattiness = 0.7f;
            m_personality.bravery = 0.5f;
            m_personality.helpfulness = 0.9f;
            break;

        case NPCType::COMPANION:
            m_occupation = "Companion";
            m_personality.friendliness = 0.9f;
            m_personality.chattiness = 0.6f;
            m_personality.bravery = 0.8f;
            m_personality.helpfulness = 1.0f;
            break;

        case NPCType::AMBIENT:
            m_occupation = "Civilian";
            m_personality.friendliness = 0.5f;
            m_personality.chattiness = 0.4f;
            m_personality.bravery = 0.3f;
            m_personality.helpfulness = 0.4f;
            m_wandering = true;
            break;

        case NPCType::TRAINER:
            m_occupation = "Trainer";
            m_personality.friendliness = 0.6f;
            m_personality.chattiness = 0.5f;
            m_personality.bravery = 0.7f;
            m_personality.helpfulness = 0.8f;
            break;
    }
}

void NPCAI::AddScheduledActivity(const ScheduledActivity& activity) {
    m_schedule.push_back(activity);

    // Sort by start time
    std::sort(m_schedule.begin(), m_schedule.end(),
              [](const ScheduledActivity& a, const ScheduledActivity& b) {
                  return a.startTime < b.startTime;
              });
}

void NPCAI::RemoveScheduledActivity(ActivityType type) {
    m_schedule.erase(
        std::remove_if(m_schedule.begin(), m_schedule.end(),
                      [type](const ScheduledActivity& activity) {
                          return activity.type == type;
                      }),
        m_schedule.end()
    );
}

void NPCAI::ClearSchedule() {
    m_schedule.clear();
    m_currentActivity = nullptr;
}

ScheduledActivity* NPCAI::GetCurrentActivity(float timeOfDay) {
    for (auto& activity : m_schedule) {
        if (activity.IsActiveAt(timeOfDay)) {
            return &activity;
        }
    }
    return nullptr;
}

void NPCAI::SetPath(const PathResult& path) {
    m_currentPath = path;
    m_currentWaypointIndex = 0;
}

void NPCAI::ClearPath() {
    m_currentPath = PathResult();
    m_currentWaypointIndex = 0;
}

WorldPos NPCAI::GetNextWaypoint() {
    if (m_currentWaypointIndex < m_currentPath.worldPath.size()) {
        return m_currentPath.worldPath[m_currentWaypointIndex];
    }
    return m_position;
}

void NPCAI::UpdatePathing(float deltaTime) {
    if (!HasPath()) {
        return;
    }

    WorldPos target = GetNextWaypoint();
    float distanceToWaypoint = m_position.DistanceTo(target);

    if (distanceToWaypoint < 0.5f) {
        // Reached waypoint, move to next
        m_currentWaypointIndex++;
        if (m_currentWaypointIndex >= m_currentPath.worldPath.size()) {
            ClearPath();
            m_animationState = "idle";
        }
    } else {
        // Move towards waypoint
        float speed = 2.0f; // NPC walking speed
        float dx = target.x - m_position.x;
        float dy = target.y - m_position.y;
        float length = std::sqrt(dx * dx + dy * dy);

        if (length > 0.0f) {
            m_velocity.x = (dx / length) * speed;
            m_velocity.y = (dy / length) * speed;

            m_position.x += m_velocity.x * deltaTime;
            m_position.y += m_velocity.y * deltaTime;

            m_animationState = "walking";
        }
    }
}

void NPCAI::AddDialogueNode(const std::string& nodeId, const DialogueNode& node) {
    m_dialogueTree[nodeId] = node;
}

const DialogueNode* NPCAI::GetDialogueNode(const std::string& nodeId) const {
    auto it = m_dialogueTree.find(nodeId);
    return (it != m_dialogueTree.end()) ? &it->second : nullptr;
}

void NPCAI::StartInteraction(int targetNPCId, const std::string& text, float duration) {
    m_currentInteraction.targetNPCId = targetNPCId;
    m_currentInteraction.text = text;
    m_currentInteraction.duration = duration;
    m_interactionTimer = 0.0f;
}

void NPCAI::EndInteraction() {
    m_currentInteraction.targetNPCId = -1;
    m_currentInteraction.text.clear();
    m_interactionTimer = 0.0f;
}

void NPCAI::AddRelationship(int npcId, float affinity, const std::string& relationship) {
    NPCRelationship rel(npcId, affinity, relationship);
    m_relationships.push_back(rel);
}

void NPCAI::ModifyAffinity(int npcId, float delta) {
    for (auto& rel : m_relationships) {
        if (rel.npcId == npcId) {
            rel.affinity += delta;
            rel.affinity = std::max(-1.0f, std::min(1.0f, rel.affinity));
            return;
        }
    }

    // Relationship doesn't exist, create it
    AddRelationship(npcId, delta, "acquaintance");
}

float NPCAI::GetAffinity(int npcId) const {
    for (const auto& rel : m_relationships) {
        if (rel.npcId == npcId) {
            return rel.affinity;
        }
    }
    return 0.0f; // Neutral
}

NPCRelationship* NPCAI::GetRelationship(int npcId) {
    for (auto& rel : m_relationships) {
        if (rel.npcId == npcId) {
            return &rel;
        }
    }
    return nullptr;
}

void NPCAI::OnPlayerApproach(int playerId) {
    // React based on personality
    if (m_personality.friendliness > 0.7f) {
        // Wave or greet
        m_animationState = "wave";
    } else if (m_personality.friendliness < 0.3f) {
        // Look away or frown
        m_animationState = "frown";
    }

    // High chattiness NPCs might initiate conversation
    if (m_personality.chattiness > 0.7f) {
        // Could trigger a dialogue event
    }
}

void NPCAI::OnPlayerLeave(int playerId) {
    m_animationState = "idle";
}

void NPCAI::ReactToEvent(const std::string& eventType, void* eventData) {
    if (eventType == "combat_nearby") {
        // React based on bravery
        if (m_personality.bravery < 0.3f) {
            // Run away
            m_stateMachine->SetValue("scared", true);
        } else if (m_personality.bravery > 0.7f && m_npcType == NPCType::GUARD) {
            // Investigate or help
            m_stateMachine->SetValue("investigate", true);
        }
    } else if (eventType == "fire") {
        // Everyone reacts to fire
        m_stateMachine->SetValue("panic", true);
    } else if (eventType == "celebration") {
        // Join in based on personality
        if (m_personality.friendliness > 0.5f) {
            m_stateMachine->SetValue("celebrate", true);
        }
    }
}

void NPCAI::SetCustomBehavior(const std::string& behaviorName,
                              std::function<void(float)> behaviorFunc) {
    m_customBehaviors[behaviorName] = behaviorFunc;
}

void NPCAI::ExecuteCustomBehavior(const std::string& behaviorName, float deltaTime) {
    auto it = m_customBehaviors.find(behaviorName);
    if (it != m_customBehaviors.end() && it->second) {
        it->second(deltaTime);
    }
}

void NPCAI::BuildStateMachine() {
    m_stateMachine = CreateNPCRoutineStateMachine();
    m_stateMachine->SetDebugName(m_name + "_StateMachine");
}

void NPCAI::UpdateSchedule(float deltaTime) {
    if (m_schedule.empty()) {
        return;
    }

    ScheduledActivity* newActivity = GetCurrentActivity(m_timeOfDay);

    // Check if we need to transition to a new activity
    if (newActivity != m_currentActivity) {
        if (newActivity) {
            TransitionToActivity(newActivity);
        }
        m_currentActivity = newActivity;
    }
}

void NPCAI::UpdateActivity(float deltaTime) {
    if (!m_currentActivity) {
        return;
    }

    // Update state machine with current activity
    m_stateMachine->SetValue("currentActivity", static_cast<int>(m_currentActivity->type));
    m_stateMachine->SetValue("timeOfDay", m_timeOfDay);

    // Set target location for pathfinding
    if (!HasPath() && m_position.DistanceTo(m_currentActivity->location) > 1.0f) {
        // In real implementation, use pathfinding system
        // For now, we'll just set a simple path
        PathResult path;
        path.worldPath.push_back(m_position);
        path.worldPath.push_back(m_currentActivity->location);
        path.success = true;
        SetPath(path);
    }
}

void NPCAI::UpdateWandering(float deltaTime) {
    if (!m_wandering) {
        return;
    }

    m_wanderTimer += deltaTime;

    // Pick a new wander target every few seconds
    if (m_wanderTimer >= 5.0f || m_position.DistanceTo(m_wanderTarget) < 0.5f) {
        m_wanderTarget = GetRandomWanderTarget();
        m_wanderTimer = 0.0f;

        // Create path to wander target
        PathResult path;
        path.worldPath.push_back(m_position);
        path.worldPath.push_back(m_wanderTarget);
        path.success = true;
        SetPath(path);
    }
}

void NPCAI::UpdateInteraction(float deltaTime) {
    if (!IsInteracting()) {
        return;
    }

    m_interactionTimer += deltaTime;

    if (m_interactionTimer >= m_currentInteraction.duration) {
        EndInteraction();
    }
}

void NPCAI::TransitionToActivity(ScheduledActivity* activity) {
    if (!activity) {
        return;
    }

    // Update animation based on activity
    switch (activity->type) {
        case ActivityType::SLEEP:
            m_animationState = "sleeping";
            break;
        case ActivityType::EAT:
            m_animationState = "eating";
            break;
        case ActivityType::WORK:
            m_animationState = "working";
            break;
        case ActivityType::SOCIALIZE:
            m_animationState = "talking";
            break;
        case ActivityType::PATROL:
            m_animationState = "walking";
            break;
        case ActivityType::IDLE:
            m_animationState = "idle";
            break;
        case ActivityType::SHOP:
            m_animationState = "shopping";
            break;
        case ActivityType::PRAY:
            m_animationState = "praying";
            break;
        case ActivityType::EXERCISE:
            m_animationState = "exercising";
            break;
        case ActivityType::ENTERTAIN:
            m_animationState = "entertaining";
            break;
        case ActivityType::CUSTOM:
            // Execute custom behavior
            if (!activity->customData.empty()) {
                ExecuteCustomBehavior(activity->customData, 0.0f);
            }
            break;
    }
}

WorldPos NPCAI::GetRandomWanderTarget() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> radiusDist(0.0f, m_wanderRadius);

    float angle = angleDist(gen);
    float radius = radiusDist(gen);

    WorldPos target;
    target.x = m_homePosition.x + radius * std::cos(angle);
    target.y = m_homePosition.y + radius * std::sin(angle);

    return target;
}

// Factory function
std::shared_ptr<NPCAI> CreateNPCByType(int aiId, int entityId, const std::string& npcTypeName) {
    auto npc = std::make_shared<NPCAI>(aiId, entityId, npcTypeName);
    npc->Initialize();
    return npc;
}

// Predefined schedules
std::vector<ScheduledActivity> CreateVillagerSchedule(const WorldPos& home) {
    std::vector<ScheduledActivity> schedule;

    // Sleep: 10 PM - 6 AM
    ScheduledActivity sleep(ActivityType::SLEEP, 22.0f, 6.0f, home);
    sleep.name = "Sleep";
    schedule.push_back(sleep);

    // Breakfast: 6 AM - 7 AM
    WorldPos kitchen = home; // In real implementation, different location
    ScheduledActivity breakfast(ActivityType::EAT, 6.0f, 7.0f, kitchen);
    breakfast.name = "Breakfast";
    schedule.push_back(breakfast);

    // Work: 8 AM - 12 PM
    WorldPos workplace(home.x + 50.0f, home.y); // Example location
    ScheduledActivity morningWork(ActivityType::WORK, 8.0f, 12.0f, workplace);
    morningWork.name = "Morning Work";
    schedule.push_back(morningWork);

    // Lunch: 12 PM - 1 PM
    ScheduledActivity lunch(ActivityType::EAT, 12.0f, 13.0f, home);
    lunch.name = "Lunch";
    schedule.push_back(lunch);

    // Work: 1 PM - 5 PM
    ScheduledActivity afternoonWork(ActivityType::WORK, 13.0f, 17.0f, workplace);
    afternoonWork.name = "Afternoon Work";
    schedule.push_back(afternoonWork);

    // Socialize: 5 PM - 7 PM
    WorldPos tavern(home.x + 30.0f, home.y + 20.0f);
    ScheduledActivity socialize(ActivityType::SOCIALIZE, 17.0f, 19.0f, tavern);
    socialize.name = "Evening Socializing";
    schedule.push_back(socialize);

    // Dinner: 7 PM - 8 PM
    ScheduledActivity dinner(ActivityType::EAT, 19.0f, 20.0f, home);
    dinner.name = "Dinner";
    schedule.push_back(dinner);

    // Evening leisure: 8 PM - 10 PM
    ScheduledActivity leisure(ActivityType::IDLE, 20.0f, 22.0f, home);
    leisure.name = "Evening Leisure";
    schedule.push_back(leisure);

    return schedule;
}

std::vector<ScheduledActivity> CreateMerchantSchedule(const WorldPos& shop) {
    std::vector<ScheduledActivity> schedule;

    // Sleep: 11 PM - 7 AM
    ScheduledActivity sleep(ActivityType::SLEEP, 23.0f, 7.0f, shop);
    sleep.name = "Sleep";
    schedule.push_back(sleep);

    // Breakfast: 7 AM - 8 AM
    ScheduledActivity breakfast(ActivityType::EAT, 7.0f, 8.0f, shop);
    breakfast.name = "Breakfast";
    schedule.push_back(breakfast);

    // Open shop: 8 AM - 12 PM
    ScheduledActivity morningShop(ActivityType::SHOP, 8.0f, 12.0f, shop);
    morningShop.name = "Morning Business";
    schedule.push_back(morningShop);

    // Lunch break: 12 PM - 1 PM
    ScheduledActivity lunch(ActivityType::EAT, 12.0f, 13.0f, shop);
    lunch.name = "Lunch";
    schedule.push_back(lunch);

    // Afternoon business: 1 PM - 6 PM
    ScheduledActivity afternoonShop(ActivityType::SHOP, 13.0f, 18.0f, shop);
    afternoonShop.name = "Afternoon Business";
    schedule.push_back(afternoonShop);

    // Close shop, count money: 6 PM - 7 PM
    ScheduledActivity closing(ActivityType::WORK, 18.0f, 19.0f, shop);
    closing.name = "Closing Shop";
    schedule.push_back(closing);

    // Dinner: 7 PM - 8 PM
    ScheduledActivity dinner(ActivityType::EAT, 19.0f, 20.0f, shop);
    dinner.name = "Dinner";
    schedule.push_back(dinner);

    // Evening relaxation: 8 PM - 11 PM
    ScheduledActivity evening(ActivityType::IDLE, 20.0f, 23.0f, shop);
    evening.name = "Evening Relaxation";
    schedule.push_back(evening);

    return schedule;
}

std::vector<ScheduledActivity> CreateGuardSchedule(const WorldPos& post) {
    std::vector<ScheduledActivity> schedule;

    // Guards work in shifts
    // Morning shift: 6 AM - 2 PM
    ScheduledActivity morningPatrol(ActivityType::PATROL, 6.0f, 14.0f, post);
    morningPatrol.name = "Morning Patrol";
    schedule.push_back(morningPatrol);

    // Break: 2 PM - 3 PM
    ScheduledActivity lunch(ActivityType::EAT, 14.0f, 15.0f, post);
    lunch.name = "Meal Break";
    schedule.push_back(lunch);

    // Afternoon shift: 3 PM - 10 PM
    ScheduledActivity afternoonPatrol(ActivityType::PATROL, 15.0f, 22.0f, post);
    afternoonPatrol.name = "Afternoon Patrol";
    schedule.push_back(afternoonPatrol);

    // Sleep: 10 PM - 6 AM
    WorldPos barracks(post.x + 20.0f, post.y);
    ScheduledActivity sleep(ActivityType::SLEEP, 22.0f, 6.0f, barracks);
    sleep.name = "Rest";
    schedule.push_back(sleep);

    return schedule;
}

} // namespace AI
