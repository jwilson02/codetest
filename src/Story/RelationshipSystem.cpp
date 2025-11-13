#include "RelationshipSystem.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>

namespace Story {

// RelationshipSystem::Implementation
struct RelationshipSystem::Implementation {
    std::unordered_map<std::string, NPC> npcs;
    std::unordered_map<std::string, Relationship> relationships;
    std::unordered_map<std::string, Faction> factions;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> npcRelationships;
    std::unordered_map<std::string, BestiaryEntry> bestiary;

    RelationshipCallback relationshipCallback;
    bool debugMode = false;
};

RelationshipSystem::RelationshipSystem() : impl(std::make_unique<Implementation>()) {}

RelationshipSystem::~RelationshipSystem() = default;

bool RelationshipSystem::initialize() {
    impl->npcs.clear();
    impl->relationships.clear();
    impl->factions.clear();
    return true;
}

void RelationshipSystem::shutdown() {
    impl->npcs.clear();
    impl->relationships.clear();
    impl->factions.clear();
}

bool RelationshipSystem::loadNPCs(const std::string& filePath) {
    // Would load from JSON
    return true;
}

void RelationshipSystem::registerNPC(const NPC& npc) {
    impl->npcs[npc.id] = npc;
}

const NPC* RelationshipSystem::getNPC(const std::string& npcId) const {
    auto it = impl->npcs.find(npcId);
    return it != impl->npcs.end() ? &it->second : nullptr;
}

std::vector<const NPC*> RelationshipSystem::getAllNPCs() const {
    std::vector<const NPC*> result;
    for (const auto& [id, npc] : impl->npcs) {
        result.push_back(&npc);
    }
    return result;
}

std::vector<const NPC*> RelationshipSystem::getNPCsByLocation(const std::string& location) const {
    std::vector<const NPC*> result;
    for (const auto& [id, npc] : impl->npcs) {
        if (npc.location == location && npc.isAvailable) {
            result.push_back(&npc);
        }
    }
    return result;
}

std::vector<const NPC*> RelationshipSystem::getNPCsByFaction(const std::string& faction) const {
    std::vector<const NPC*> result;
    for (const auto& [id, npc] : impl->npcs) {
        if (std::find(npc.faction.begin(), npc.faction.end(), faction) != npc.faction.end()) {
            result.push_back(&npc);
        }
    }
    return result;
}

void RelationshipSystem::initializeRelationship(const std::string& npcId, int initialValue) {
    Relationship& rel = impl->relationships[npcId];
    rel.npcId = npcId;
    rel.relationshipValue = initialValue;
    rel.level = calculateLevel(initialValue);
    rel.type = RelationshipType::STRANGER;
    rel.firstMeetTime = static_cast<float>(std::time(nullptr));
    rel.lastInteractionTime = rel.firstMeetTime;
    rel.totalInteractions = 0;
}

Relationship* RelationshipSystem::getRelationship(const std::string& npcId) {
    auto it = impl->relationships.find(npcId);
    if (it == impl->relationships.end()) {
        initializeRelationship(npcId);
        it = impl->relationships.find(npcId);
    }
    return it != impl->relationships.end() ? &it->second : nullptr;
}

const Relationship* RelationshipSystem::getRelationship(const std::string& npcId) const {
    auto it = impl->relationships.find(npcId);
    return it != impl->relationships.end() ? &it->second : nullptr;
}

void RelationshipSystem::modifyRelationship(const std::string& npcId, int amount,
                                           const std::string& reason) {
    Relationship* rel = getRelationship(npcId);
    if (!rel) return;

    int oldValue = rel->relationshipValue;
    RelationshipLevel oldLevel = rel->level;

    rel->relationshipValue = std::clamp(rel->relationshipValue + amount, -100, 100);
    rel->level = calculateLevel(rel->relationshipValue);
    rel->lastInteractionTime = static_cast<float>(std::time(nullptr));

    if (!reason.empty()) {
        rel->interactionHistory.push_back({reason, amount});
    }

    // Notify if level changed
    if (oldLevel != rel->level) {
        RelationshipChangeEvent event;
        event.npcId = npcId;
        event.oldValue = oldValue;
        event.newValue = rel->relationshipValue;
        event.oldLevel = oldLevel;
        event.newLevel = rel->level;
        event.reason = reason;
        event.timestamp = rel->lastInteractionTime;

        notifyRelationshipChange(event);
    }

    // Update faction reputation
    updateFactionReputationFromNPC(npcId, amount);
}

void RelationshipSystem::setRelationship(const std::string& npcId, int value,
                                        const std::string& reason) {
    int current = getRelationshipValue(npcId);
    modifyRelationship(npcId, value - current, reason);
}

int RelationshipSystem::getRelationshipValue(const std::string& npcId) const {
    const Relationship* rel = getRelationship(npcId);
    return rel ? rel->relationshipValue : 0;
}

RelationshipLevel RelationshipSystem::getRelationshipLevel(const std::string& npcId) const {
    const Relationship* rel = getRelationship(npcId);
    return rel ? rel->level : RelationshipLevel::NEUTRAL;
}

std::string RelationshipSystem::getRelationshipLevelName(const std::string& npcId) const {
    RelationshipLevel level = getRelationshipLevel(npcId);

    switch (level) {
        case RelationshipLevel::HATED: return "Hated";
        case RelationshipLevel::HOSTILE: return "Hostile";
        case RelationshipLevel::UNFRIENDLY: return "Unfriendly";
        case RelationshipLevel::NEUTRAL: return "Neutral";
        case RelationshipLevel::ACQUAINTANCE: return "Acquaintance";
        case RelationshipLevel::FRIENDLY: return "Friendly";
        case RelationshipLevel::LIKED: return "Liked";
        case RelationshipLevel::TRUSTED: return "Trusted";
        case RelationshipLevel::CLOSE: return "Close";
        case RelationshipLevel::DEVOTED: return "Devoted";
        case RelationshipLevel::ROMANCED: return "Romanced";
        default: return "Unknown";
    }
}

std::vector<std::string> RelationshipSystem::getFriendlyNPCs(int minValue) const {
    std::vector<std::string> result;
    for (const auto& [id, rel] : impl->relationships) {
        if (rel.relationshipValue >= minValue) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> RelationshipSystem::getHostileNPCs(int maxValue) const {
    std::vector<std::string> result;
    for (const auto& [id, rel] : impl->relationships) {
        if (rel.relationshipValue <= maxValue) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> RelationshipSystem::getRomanticInterests() const {
    std::vector<std::string> result;
    for (const auto& [id, rel] : impl->relationships) {
        if (rel.inRomance) {
            result.push_back(id);
        }
    }
    return result;
}

bool RelationshipSystem::isInRomance(const std::string& npcId) const {
    const Relationship* rel = getRelationship(npcId);
    return rel && rel->inRomance;
}

void RelationshipSystem::recordInteraction(const std::string& npcId,
                                          const std::string& description,
                                          int valueChange) {
    Relationship* rel = getRelationship(npcId);
    if (!rel) return;

    rel->totalInteractions++;
    rel->lastInteractionTime = static_cast<float>(std::time(nullptr));

    if (valueChange != 0) {
        modifyRelationship(npcId, valueChange, description);
    } else if (!description.empty()) {
        rel->interactionHistory.push_back({description, 0});
    }
}

void RelationshipSystem::recordTopicDiscussed(const std::string& npcId, const std::string& topic) {
    Relationship* rel = getRelationship(npcId);
    if (rel) {
        rel->topicsDiscussed[topic] = true;
    }
}

bool RelationshipSystem::hasDiscussedTopic(const std::string& npcId, const std::string& topic) const {
    const Relationship* rel = getRelationship(npcId);
    if (!rel) return false;

    auto it = rel->topicsDiscussed.find(topic);
    return it != rel->topicsDiscussed.end() && it->second;
}

void RelationshipSystem::recordSharedExperience(const std::string& npcId,
                                               const std::string& experience) {
    Relationship* rel = getRelationship(npcId);
    if (rel) {
        rel->sharedExperiences.push_back(experience);
    }
}

void RelationshipSystem::giveGift(const std::string& npcId, const std::string& itemId) {
    Relationship* rel = getRelationship(npcId);
    if (!rel) return;

    rel->giftsGiven.push_back(itemId);

    int value = calculateGiftValue(npcId, itemId);
    if (value != 0) {
        modifyRelationship(npcId, value, "gift:" + itemId);
    }
}

int RelationshipSystem::calculateGiftValue(const std::string& npcId,
                                          const std::string& itemId) const {
    const NPC* npc = getNPC(npcId);
    if (!npc) return 0;

    // Check if NPC likes this item
    auto it = npc->likes.find(itemId);
    if (it != npc->likes.end()) {
        return it->second; // Return like value
    }

    // Check if NPC dislikes this item
    auto dislikeIt = npc->dislikes.find(itemId);
    if (dislikeIt != npc->dislikes.end()) {
        return -dislikeIt->second; // Return negative dislike value
    }

    // Neutral gift
    return 5;
}

std::vector<std::string> RelationshipSystem::getGiftsGiven(const std::string& npcId) const {
    const Relationship* rel = getRelationship(npcId);
    return rel ? rel->giftsGiven : std::vector<std::string>();
}

bool RelationshipSystem::canStartRomance(const std::string& npcId) const {
    const NPC* npc = getNPC(npcId);
    if (!npc || !npc->romanceable) return false;

    const Relationship* rel = getRelationship(npcId);
    if (!rel || rel->inRomance) return false;

    // Check relationship value requirement
    if (rel->relationshipValue < 60) return false;

    // Check romance requirements
    // Would check actual requirements here

    return true;
}

void RelationshipSystem::startRomance(const std::string& npcId) {
    if (!canStartRomance(npcId)) return;

    Relationship* rel = getRelationship(npcId);
    if (rel) {
        rel->inRomance = true;
        rel->romanceStage = 1;
        rel->type = RelationshipType::ROMANTIC_INTEREST;
        rel->level = RelationshipLevel::ROMANCED;
    }
}

void RelationshipSystem::endRomance(const std::string& npcId) {
    Relationship* rel = getRelationship(npcId);
    if (rel && rel->inRomance) {
        rel->inRomance = false;
        rel->romanceStage = 0;
        rel->type = RelationshipType::FRIEND;
        rel->level = calculateLevel(rel->relationshipValue);
    }
}

void RelationshipSystem::advanceRomanceStage(const std::string& npcId) {
    Relationship* rel = getRelationship(npcId);
    if (rel && rel->inRomance) {
        rel->romanceStage++;
    }
}

int RelationshipSystem::getRomanceStage(const std::string& npcId) const {
    const Relationship* rel = getRelationship(npcId);
    return rel ? rel->romanceStage : 0;
}

std::string RelationshipSystem::getCurrentRomancePartner() const {
    for (const auto& [id, rel] : impl->relationships) {
        if (rel.inRomance) {
            return id;
        }
    }
    return "";
}

void RelationshipSystem::registerFaction(const Faction& faction) {
    impl->factions[faction.id] = faction;
}

const Faction* RelationshipSystem::getFaction(const std::string& factionId) const {
    auto it = impl->factions.find(factionId);
    return it != impl->factions.end() ? &it->second : nullptr;
}

void RelationshipSystem::modifyFactionReputation(const std::string& factionId, int amount) {
    const Faction* faction = getFaction(factionId);
    if (!faction) return;

    // Modify reputation with all NPCs in faction
    for (const auto& npcId : faction->memberNpcIds) {
        modifyRelationship(npcId, amount / 2, "faction_reputation");
    }
}

int RelationshipSystem::getFactionReputation(const std::string& factionId) const {
    const Faction* faction = getFaction(factionId);
    if (!faction || faction->memberNpcIds.empty()) return 0;

    // Average reputation with faction members
    int total = 0;
    int count = 0;
    for (const auto& npcId : faction->memberNpcIds) {
        total += getRelationshipValue(npcId);
        count++;
    }

    return count > 0 ? total / count : 0;
}

std::string RelationshipSystem::getFactionReputationTier(const std::string& factionId) const {
    int reputation = getFactionReputation(factionId);
    const Faction* faction = getFaction(factionId);

    if (!faction) return "Unknown";

    // Find appropriate tier
    std::string tierName = "Neutral";
    for (const auto& tier : faction->reputationTiers) {
        if (reputation >= tier.minReputation) {
            tierName = tier.name;
        }
    }

    return tierName;
}

void RelationshipSystem::setNPCRelationship(const std::string& npc1Id, const std::string& npc2Id,
                                           int value) {
    impl->npcRelationships[npc1Id][npc2Id] = value;
}

int RelationshipSystem::getNPCRelationship(const std::string& npc1Id,
                                          const std::string& npc2Id) const {
    auto it1 = impl->npcRelationships.find(npc1Id);
    if (it1 == impl->npcRelationships.end()) return 0;

    auto it2 = it1->second.find(npc2Id);
    return it2 != it1->second.end() ? it2->second : 0;
}

std::string RelationshipSystem::getAppropriateDialogue(const std::string& npcId,
                                                       const std::string& context) const {
    const NPC* npc = getNPC(npcId);
    if (!npc) return "";

    // Check contextual dialogues first
    if (!context.empty()) {
        auto it = npc->contextualDialogues.find(context);
        if (it != npc->contextualDialogues.end()) {
            return it->second;
        }
    }

    // Check relationship-based dialogues
    RelationshipLevel level = getRelationshipLevel(npcId);
    std::string levelContext = "relationship_" + std::to_string(static_cast<int>(level));
    auto it = npc->contextualDialogues.find(levelContext);
    if (it != npc->contextualDialogues.end()) {
        return it->second;
    }

    // Return default dialogue
    return npc->defaultDialogueId;
}

bool RelationshipSystem::isDialogueAvailable(const std::string& npcId,
                                            const std::string& dialogueId) const {
    // Would check dialogue requirements
    return true;
}

void RelationshipSystem::onRelationshipChanged(RelationshipCallback callback) {
    impl->relationshipCallback = callback;
}

void RelationshipSystem::triggerRelationshipEvent(const std::string& eventId,
                                                  const std::string& npcId) {
    // Trigger special relationship events
}

bool RelationshipSystem::npcHasTrait(const std::string& npcId, PersonalityTrait trait) const {
    const NPC* npc = getNPC(npcId);
    if (!npc) return false;

    return std::find(npc->traits.begin(), npc->traits.end(), trait) != npc->traits.end();
}

int RelationshipSystem::calculatePersonalityCompatibility(const std::string& npc1Id,
                                                         const std::string& npc2Id) const {
    const NPC* npc1 = getNPC(npc1Id);
    const NPC* npc2 = getNPC(npc2Id);

    if (!npc1 || !npc2) return 0;

    // Simple compatibility calculation based on shared traits
    int compatibility = 50; // Base

    // Count shared traits
    for (const auto& trait : npc1->traits) {
        if (std::find(npc2->traits.begin(), npc2->traits.end(), trait) != npc2->traits.end()) {
            compatibility += 10;
        }
    }

    return std::clamp(compatibility, 0, 100);
}

RelationshipSystem::Reaction RelationshipSystem::getReactionToAction(const std::string& npcId,
                                                                     const std::string& action) const {
    Reaction reaction;
    reaction.animation = "neutral";
    reaction.dialogue = "";
    reaction.relationshipChange = 0;

    const NPC* npc = getNPC(npcId);
    if (!npc) return reaction;

    // Check likes/dislikes
    auto likeIt = npc->likes.find(action);
    if (likeIt != npc->likes.end()) {
        reaction.animation = "approve";
        reaction.relationshipChange = likeIt->second;
    }

    auto dislikeIt = npc->dislikes.find(action);
    if (dislikeIt != npc->dislikes.end()) {
        reaction.animation = "disapprove";
        reaction.relationshipChange = -dislikeIt->second;
    }

    return reaction;
}

std::vector<std::string> RelationshipSystem::getMutualFriends(const std::string& npc1Id,
                                                              const std::string& npc2Id) const {
    std::vector<std::string> mutual;

    // Get NPCs that both NPCs have good relationships with
    for (const auto& [npcId, rel1] : impl->npcRelationships) {
        if (npcId == npc1Id || npcId == npc2Id) continue;

        int rel1Value = getNPCRelationship(npc1Id, npcId);
        int rel2Value = getNPCRelationship(npc2Id, npcId);

        if (rel1Value > 20 && rel2Value > 20) {
            mutual.push_back(npcId);
        }
    }

    return mutual;
}

std::vector<std::string> RelationshipSystem::getInfluentialNPCs() const {
    std::vector<std::pair<std::string, int>> npcConnections;

    for (const auto& [npcId, npc] : impl->npcs) {
        int connections = 0;

        // Count faction members
        for (const auto& faction : npc.faction) {
            const Faction* f = getFaction(faction);
            if (f) connections += static_cast<int>(f->memberNpcIds.size());
        }

        // Count relationships
        auto it = impl->npcRelationships.find(npcId);
        if (it != impl->npcRelationships.end()) {
            connections += static_cast<int>(it->second.size());
        }

        npcConnections.push_back({npcId, connections});
    }

    // Sort by connections
    std::sort(npcConnections.begin(), npcConnections.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<std::string> result;
    for (const auto& [npcId, connections] : npcConnections) {
        result.push_back(npcId);
    }

    return result;
}

std::string RelationshipSystem::serializeState() const {
    // Would serialize to JSON
    return "{}";
}

bool RelationshipSystem::deserializeState(const std::string& data) {
    // Would deserialize from JSON
    return true;
}

RelationshipSystem::RelationshipStats RelationshipSystem::getStatistics() const {
    RelationshipStats stats;
    stats.totalNPCs = static_cast<int>(impl->npcs.size());
    stats.metNPCs = static_cast<int>(impl->relationships.size());
    stats.friendlyNPCs = static_cast<int>(getFriendlyNPCs(20).size());
    stats.hostileNPCs = static_cast<int>(getHostileNPCs(-20).size());
    stats.romancedNPCs = static_cast<int>(getRomanticInterests().size());
    stats.totalInteractions = 0;

    // Calculate average relationship
    int total = 0;
    for (const auto& [id, rel] : impl->relationships) {
        total += rel.relationshipValue;
        stats.totalInteractions += rel.totalInteractions;
    }
    stats.averageRelationship = impl->relationships.empty() ?
        0.0f : static_cast<float>(total) / impl->relationships.size();

    // Find most liked/hated
    int maxValue = -101;
    int minValue = 101;
    for (const auto& [id, rel] : impl->relationships) {
        if (rel.relationshipValue > maxValue) {
            maxValue = rel.relationshipValue;
            stats.mostLikedNPC = id;
        }
        if (rel.relationshipValue < minValue) {
            minValue = rel.relationshipValue;
            stats.mostHatedNPC = id;
        }
    }

    return stats;
}

void RelationshipSystem::setDebugMode(bool enable) {
    impl->debugMode = enable;
}

void RelationshipSystem::dumpRelationships(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return;

    file << "=== Relationship System State ===\n\n";
    for (const auto& [id, rel] : impl->relationships) {
        const NPC* npc = getNPC(id);
        file << (npc ? npc->name : id) << ": " << rel.relationshipValue
             << " (" << getRelationshipLevelName(id) << ")\n";
    }
}

RelationshipLevel RelationshipSystem::calculateLevel(int value) const {
    if (value <= -80) return RelationshipLevel::HATED;
    if (value <= -60) return RelationshipLevel::HOSTILE;
    if (value <= -40) return RelationshipLevel::UNFRIENDLY;
    if (value <= -20) return RelationshipLevel::NEUTRAL;
    if (value <= 0) return RelationshipLevel::ACQUAINTANCE;
    if (value <= 20) return RelationshipLevel::FRIENDLY;
    if (value <= 40) return RelationshipLevel::LIKED;
    if (value <= 60) return RelationshipLevel::TRUSTED;
    if (value <= 80) return RelationshipLevel::CLOSE;
    return RelationshipLevel::DEVOTED;
}

void RelationshipSystem::notifyRelationshipChange(const RelationshipChangeEvent& event) {
    if (impl->relationshipCallback) {
        impl->relationshipCallback(event);
    }
}

void RelationshipSystem::updateFactionReputationFromNPC(const std::string& npcId, int change) {
    const NPC* npc = getNPC(npcId);
    if (!npc) return;

    // Small reputation change with NPC's factions
    for (const auto& factionId : npc->faction) {
        const Faction* faction = getFaction(factionId);
        if (faction) {
            for (const auto& memberNpcId : faction->memberNpcIds) {
                if (memberNpcId != npcId) {
                    modifyRelationship(memberNpcId, change / 4, "faction_member");
                }
            }
        }
    }
}

} // namespace Story
