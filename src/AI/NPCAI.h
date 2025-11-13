#pragma once

#include "StateMachine.h"
#include "Pathfinding.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace AI {

// Forward declarations
class AISystem;

// NPC types
enum class NPCType {
    VILLAGER,
    MERCHANT,
    GUARD,
    QUEST_GIVER,
    COMPANION,
    AMBIENT, // Background NPCs
    TRAINER
};

// Activity types for NPC schedules
enum class ActivityType {
    SLEEP,
    EAT,
    WORK,
    SOCIALIZE,
    PATROL,
    IDLE,
    SHOP,
    PRAY,
    EXERCISE,
    ENTERTAIN,
    CUSTOM
};

// Scheduled activity
struct ScheduledActivity {
    ActivityType type;
    std::string name;
    float startTime;    // Hour of day (0-24)
    float endTime;      // Hour of day (0-24)
    WorldPos location;  // Where to perform this activity
    std::string customData; // Additional data for custom activities

    ScheduledActivity()
        : type(ActivityType::IDLE)
        , startTime(0.0f)
        , endTime(24.0f)
    {}

    ScheduledActivity(ActivityType t, float start, float end, const WorldPos& loc)
        : type(t)
        , startTime(start)
        , endTime(end)
        , location(loc)
    {}

    bool IsActiveAt(float timeOfDay) const {
        if (startTime < endTime) {
            return timeOfDay >= startTime && timeOfDay < endTime;
        } else {
            // Wraps around midnight
            return timeOfDay >= startTime || timeOfDay < endTime;
        }
    }
};

// Social interaction data
struct SocialInteraction {
    std::string text;
    float duration;
    int targetNPCId;

    SocialInteraction()
        : duration(2.0f)
        , targetNPCId(-1)
    {}
};

// Dialogue option
struct DialogueOption {
    std::string text;
    std::string response;
    std::function<void()> callback; // Triggered when chosen

    DialogueOption(const std::string& t = "", const std::string& r = "")
        : text(t), response(r)
    {}
};

// Dialogue tree node
struct DialogueNode {
    std::string npcText;
    std::vector<DialogueOption> options;
    bool isQuestRelated;
    std::string questId;

    DialogueNode()
        : isQuestRelated(false)
    {}
};

// NPC personality traits
struct PersonalityTraits {
    float friendliness;     // 0.0 = hostile, 1.0 = very friendly
    float chattiness;       // 0.0 = quiet, 1.0 = very chatty
    float bravery;          // 0.0 = coward, 1.0 = fearless
    float curiosity;        // 0.0 = indifferent, 1.0 = very curious
    float helpfulness;      // 0.0 = unhelpful, 1.0 = very helpful

    PersonalityTraits()
        : friendliness(0.5f)
        , chattiness(0.5f)
        , bravery(0.5f)
        , curiosity(0.5f)
        , helpfulness(0.5f)
    {}
};

// NPC relationship
struct NPCRelationship {
    int npcId;
    float affinity;         // -1.0 = hostile, 0.0 = neutral, 1.0 = best friend
    std::string relationship; // "friend", "family", "rival", etc.

    NPCRelationship(int id = -1, float aff = 0.0f, const std::string& rel = "stranger")
        : npcId(id)
        , affinity(aff)
        , relationship(rel)
    {}
};

/**
 * NPCAI - Manages NPC behaviors, schedules, and social interactions
 */
class NPCAI {
public:
    NPCAI(int aiId, int entityId, const std::string& npcTypeName);
    ~NPCAI();

    // Initialization
    void Initialize();
    void SetAISystem(AISystem* aiSystem) { m_aiSystem = aiSystem; }

    // Update
    void Update(float deltaTime);

    // NPC type
    void SetNPCType(NPCType type);
    NPCType GetNPCType() const { return m_npcType; }

    // Schedule management
    void AddScheduledActivity(const ScheduledActivity& activity);
    void RemoveScheduledActivity(ActivityType type);
    void ClearSchedule();
    ScheduledActivity* GetCurrentActivity(float timeOfDay);
    const std::vector<ScheduledActivity>& GetSchedule() const { return m_schedule; }

    // Time of day (0-24 hours)
    void SetTimeOfDay(float hours) { m_timeOfDay = hours; }
    float GetTimeOfDay() const { return m_timeOfDay; }

    // Position and movement
    void SetPosition(const WorldPos& pos) { m_position = pos; }
    WorldPos GetPosition() const { return m_position; }
    void SetHomePosition(const WorldPos& pos) { m_homePosition = pos; }
    WorldPos GetHomePosition() const { return m_homePosition; }

    // Pathfinding
    void SetPath(const PathResult& path);
    void ClearPath();
    bool HasPath() const { return !m_currentPath.worldPath.empty(); }
    WorldPos GetNextWaypoint();
    void UpdatePathing(float deltaTime);

    // Dialogue system
    void AddDialogueNode(const std::string& nodeId, const DialogueNode& node);
    const DialogueNode* GetDialogueNode(const std::string& nodeId) const;
    void SetCurrentDialogueNode(const std::string& nodeId) { m_currentDialogueNode = nodeId; }
    const std::string& GetCurrentDialogueNode() const { return m_currentDialogueNode; }

    // Social interactions
    void StartInteraction(int targetNPCId, const std::string& text, float duration);
    void EndInteraction();
    bool IsInteracting() const { return m_currentInteraction.targetNPCId >= 0; }
    const SocialInteraction& GetCurrentInteraction() const { return m_currentInteraction; }

    // Personality
    PersonalityTraits& GetPersonality() { return m_personality; }
    const PersonalityTraits& GetPersonality() const { return m_personality; }

    // Relationships
    void AddRelationship(int npcId, float affinity, const std::string& relationship);
    void ModifyAffinity(int npcId, float delta);
    float GetAffinity(int npcId) const;
    NPCRelationship* GetRelationship(int npcId);

    // Reactions and awareness
    void OnPlayerApproach(int playerId);
    void OnPlayerLeave(int playerId);
    void ReactToEvent(const std::string& eventType, void* eventData);

    // State machine
    std::shared_ptr<StateMachine> GetStateMachine() { return m_stateMachine; }

    // Custom behavior
    void SetCustomBehavior(const std::string& behaviorName,
                          std::function<void(float)> behaviorFunc);
    void ExecuteCustomBehavior(const std::string& behaviorName, float deltaTime);

    // Names and identity
    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }
    void SetOccupation(const std::string& occupation) { m_occupation = occupation; }
    const std::string& GetOccupation() const { return m_occupation; }

    // Movement patterns
    void SetWanderRadius(float radius) { m_wanderRadius = radius; }
    float GetWanderRadius() const { return m_wanderRadius; }
    void EnableWandering(bool enable) { m_wandering = enable; }
    bool IsWandering() const { return m_wandering; }

    // Animation state (for external systems)
    void SetAnimationState(const std::string& state) { m_animationState = state; }
    const std::string& GetAnimationState() const { return m_animationState; }

    // Debug
    int GetAIId() const { return m_aiId; }
    int GetEntityId() const { return m_entityId; }

private:
    // IDs
    int m_aiId;
    int m_entityId;

    // AI system reference
    AISystem* m_aiSystem;

    // NPC type and identity
    NPCType m_npcType;
    std::string m_npcTypeName;
    std::string m_name;
    std::string m_occupation;

    // State machine
    std::shared_ptr<StateMachine> m_stateMachine;

    // Schedule
    std::vector<ScheduledActivity> m_schedule;
    ScheduledActivity* m_currentActivity;
    float m_timeOfDay; // 0-24 hours

    // Position and movement
    WorldPos m_position;
    WorldPos m_homePosition;
    WorldPos m_velocity;
    PathResult m_currentPath;
    size_t m_currentWaypointIndex;

    // Wandering
    bool m_wandering;
    float m_wanderRadius;
    float m_wanderTimer;
    WorldPos m_wanderTarget;

    // Dialogue
    std::unordered_map<std::string, DialogueNode> m_dialogueTree;
    std::string m_currentDialogueNode;

    // Social
    SocialInteraction m_currentInteraction;
    float m_interactionTimer;

    // Personality and relationships
    PersonalityTraits m_personality;
    std::vector<NPCRelationship> m_relationships;

    // Custom behaviors
    std::unordered_map<std::string, std::function<void(float)>> m_customBehaviors;

    // Animation
    std::string m_animationState;

    // Helper methods
    void BuildStateMachine();
    void UpdateSchedule(float deltaTime);
    void UpdateActivity(float deltaTime);
    void UpdateWandering(float deltaTime);
    void UpdateInteraction(float deltaTime);
    void TransitionToActivity(ScheduledActivity* activity);
    WorldPos GetRandomWanderTarget();
};

/**
 * Factory function to create NPCs by type
 */
std::shared_ptr<NPCAI> CreateNPCByType(int aiId, int entityId, const std::string& npcTypeName);

/**
 * Predefined schedules
 */
std::vector<ScheduledActivity> CreateVillagerSchedule(const WorldPos& home);
std::vector<ScheduledActivity> CreateMerchantSchedule(const WorldPos& shop);
std::vector<ScheduledActivity> CreateGuardSchedule(const WorldPos& post);

} // namespace AI
