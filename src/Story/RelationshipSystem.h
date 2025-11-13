#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Story {

// Relationship level
enum class RelationshipLevel {
    HATED = -5,      // -100 to -80
    HOSTILE = -4,    // -79 to -60
    UNFRIENDLY = -3, // -59 to -40
    NEUTRAL = -2,    // -39 to -20
    ACQUAINTANCE = -1, // -19 to 0
    FRIENDLY = 0,    // 1 to 20
    LIKED = 1,       // 21 to 40
    TRUSTED = 2,     // 41 to 60
    CLOSE = 3,       // 61 to 80
    DEVOTED = 4,     // 81 to 100
    ROMANCED = 5     // Special romance state
};

// NPC personality traits
enum class PersonalityTrait {
    HONEST,
    DECEPTIVE,
    BRAVE,
    COWARDLY,
    KIND,
    CRUEL,
    LOYAL,
    TREACHEROUS,
    WISE,
    FOOLISH,
    PROUD,
    HUMBLE,
    GENEROUS,
    GREEDY
};

// Relationship type
enum class RelationshipType {
    STRANGER,
    ACQUAINTANCE,
    FRIEND,
    RIVAL,
    ENEMY,
    ALLY,
    MENTOR,
    STUDENT,
    FAMILY,
    ROMANTIC_INTEREST,
    LOVER,
    SPOUSE,
    BUSINESS_PARTNER
};

// NPC information
struct NPC {
    std::string id;
    std::string name;
    std::string title;
    std::string description;

    // Visual
    std::string portraitPath;
    std::string modelId;

    // Personality
    std::vector<PersonalityTrait> traits;
    std::unordered_map<std::string, int> likes;      // What they like (topic -> weight)
    std::unordered_map<std::string, int> dislikes;   // What they dislike

    // Relationships
    std::unordered_map<std::string, int> npcRelationships; // How they feel about other NPCs
    std::vector<std::string> faction;

    // Dialogue
    std::string defaultDialogueId;
    std::unordered_map<std::string, std::string> contextualDialogues; // context -> dialogue ID

    // Quest involvement
    std::vector<std::string> questsGiven;
    std::vector<std::string> questsInvolved;

    // Romance options (if applicable)
    bool romanceable = false;
    std::vector<std::string> romanceRequirements;
    std::vector<std::string> romanceQuests;

    // Availability
    std::string location;
    std::vector<std::string> availabilityConditions;
    bool isAvailable = true;

    // Voice
    std::string voiceId;

    NPC() = default;
};

// Relationship state between player and NPC
struct Relationship {
    std::string npcId;
    int relationshipValue;  // -100 (hated) to 100 (devoted)
    RelationshipLevel level;
    RelationshipType type;

    // History
    std::vector<std::pair<std::string, int>> interactionHistory; // interaction description, value change
    float firstMeetTime;
    float lastInteractionTime;
    int totalInteractions;

    // Special states
    bool inRomance = false;
    int romanceStage = 0;
    std::vector<std::string> sharedExperiences;
    std::vector<std::string> giftsGiven;

    // Conversation topics discussed
    std::unordered_map<std::string, bool> topicsDiscussed;

    Relationship() : relationshipValue(0), level(RelationshipLevel::NEUTRAL),
                    type(RelationshipType::STRANGER), firstMeetTime(0.0f),
                    lastInteractionTime(0.0f), totalInteractions(0) {}
};

// Relationship change event
struct RelationshipChangeEvent {
    std::string npcId;
    int oldValue;
    int newValue;
    RelationshipLevel oldLevel;
    RelationshipLevel newLevel;
    std::string reason;
    float timestamp;
};

// Faction information
struct Faction {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> memberNpcIds;
    std::vector<std::string> allyFactionIds;
    std::vector<std::string> enemyFactionIds;

    // Reputation levels
    struct ReputationTier {
        std::string name;
        int minReputation;
        std::vector<std::string> benefits;
        std::vector<std::string> dialogueUnlocks;
    };
    std::vector<ReputationTier> reputationTiers;
};

// Main relationship system
class RelationshipSystem {
public:
    RelationshipSystem();
    ~RelationshipSystem();

    // Initialization
    bool initialize();
    void shutdown();

    // NPC management
    bool loadNPCs(const std::string& filePath);
    void registerNPC(const NPC& npc);
    const NPC* getNPC(const std::string& npcId) const;
    std::vector<const NPC*> getAllNPCs() const;
    std::vector<const NPC*> getNPCsByLocation(const std::string& location) const;
    std::vector<const NPC*> getNPCsByFaction(const std::string& faction) const;

    // Relationship management
    void initializeRelationship(const std::string& npcId, int initialValue = 0);
    Relationship* getRelationship(const std::string& npcId);
    const Relationship* getRelationship(const std::string& npcId) const;

    // Relationship modification
    void modifyRelationship(const std::string& npcId, int amount, const std::string& reason = "");
    void setRelationship(const std::string& npcId, int value, const std::string& reason = "");
    int getRelationshipValue(const std::string& npcId) const;
    RelationshipLevel getRelationshipLevel(const std::string& npcId) const;
    std::string getRelationshipLevelName(const std::string& npcId) const;

    // Relationship queries
    std::vector<std::string> getFriendlyNPCs(int minValue = 20) const;
    std::vector<std::string> getHostileNPCs(int maxValue = -20) const;
    std::vector<std::string> getRomanticInterests() const;
    bool isInRomance(const std::string& npcId) const;

    // Interaction tracking
    void recordInteraction(const std::string& npcId, const std::string& description, int valueChange = 0);
    void recordTopicDiscussed(const std::string& npcId, const std::string& topic);
    bool hasDiscussedTopic(const std::string& npcId, const std::string& topic) const;
    void recordSharedExperience(const std::string& npcId, const std::string& experience);

    // Gift system
    void giveGift(const std::string& npcId, const std::string& itemId);
    int calculateGiftValue(const std::string& npcId, const std::string& itemId) const;
    std::vector<std::string> getGiftsGiven(const std::string& npcId) const;

    // Romance system
    bool canStartRomance(const std::string& npcId) const;
    void startRomance(const std::string& npcId);
    void endRomance(const std::string& npcId);
    void advanceRomanceStage(const std::string& npcId);
    int getRomanceStage(const std::string& npcId) const;
    std::string getCurrentRomancePartner() const; // Returns first romanced NPC

    // Faction system
    void registerFaction(const Faction& faction);
    const Faction* getFaction(const std::string& factionId) const;
    void modifyFactionReputation(const std::string& factionId, int amount);
    int getFactionReputation(const std::string& factionId) const;
    std::string getFactionReputationTier(const std::string& factionId) const;

    // NPC-to-NPC relationships
    void setNPCRelationship(const std::string& npc1Id, const std::string& npc2Id, int value);
    int getNPCRelationship(const std::string& npc1Id, const std::string& npc2Id) const;

    // Contextual dialogue selection
    std::string getAppropriateDialogue(const std::string& npcId, const std::string& context = "") const;
    bool isDialogueAvailable(const std::string& npcId, const std::string& dialogueId) const;

    // Callbacks
    using RelationshipCallback = std::function<void(const RelationshipChangeEvent&)>;
    void onRelationshipChanged(RelationshipCallback callback);

    // Special events
    void triggerRelationshipEvent(const std::string& eventId, const std::string& npcId);

    // Personality system
    bool npcHasTrait(const std::string& npcId, PersonalityTrait trait) const;
    int calculatePersonalityCompatibility(const std::string& npc1Id, const std::string& npc2Id) const;

    // Reaction system (how NPC reacts to player actions)
    struct Reaction {
        std::string animation;
        std::string dialogue;
        int relationshipChange;
    };
    Reaction getReactionToAction(const std::string& npcId, const std::string& action) const;

    // Social network
    std::vector<std::string> getMutualFriends(const std::string& npc1Id, const std::string& npc2Id) const;
    std::vector<std::string> getInfluentialNPCs() const; // NPCs with many connections

    // Save/load
    std::string serializeState() const;
    bool deserializeState(const std::string& data);

    // Statistics
    struct RelationshipStats {
        int totalNPCs;
        int metNPCs;
        int friendlyNPCs;
        int hostileNPCs;
        int romancedNPCs;
        float averageRelationship;
        std::string mostLikedNPC;
        std::string mostHatedNPC;
        int totalInteractions;
    };
    RelationshipStats getStatistics() const;

    // Debug
    void setDebugMode(bool enable);
    void dumpRelationships(const std::string& filePath) const;

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal helpers
    RelationshipLevel calculateLevel(int value) const;
    void notifyRelationshipChange(const RelationshipChangeEvent& event);
    void updateFactionReputationFromNPC(const std::string& npcId, int change);
};

} // namespace Story
