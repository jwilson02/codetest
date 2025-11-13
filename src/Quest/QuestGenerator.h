#ifndef QUEST_GENERATOR_H
#define QUEST_GENERATOR_H

#include "Quest.h"
#include <random>
#include <unordered_set>

namespace QuestSystem {

// Quest generation parameters
struct QuestGenerationParams {
    int playerLevel;
    Faction playerFaction;
    std::vector<std::string> completedQuestIds;
    std::string currentLocation;
    std::map<Faction, int> reputations;
    QuestDifficulty targetDifficulty;
    bool allowRepeatable;

    QuestGenerationParams()
        : playerLevel(1)
        , playerFaction(Faction::NONE)
        , targetDifficulty(QuestDifficulty::NORMAL)
        , allowRepeatable(true) {}
};

// Quest template for generation
struct QuestTemplate {
    std::string namePrefix;
    std::string nameSuffix;
    QuestType type;
    ObjectiveType primaryObjective;
    int minLevel;
    int maxLevel;
    std::vector<std::string> possibleTargets;
    std::vector<std::string> possibleLocations;
    std::vector<std::string> descriptionTemplates;
    QuestDifficulty difficulty;
    Faction faction;

    QuestTemplate()
        : type(QuestType::SIDE_QUEST)
        , primaryObjective(ObjectiveType::KILL)
        , minLevel(1)
        , maxLevel(99)
        , difficulty(QuestDifficulty::NORMAL)
        , faction(Faction::NONE) {}
};

// Quest generator class
class QuestGenerator {
public:
    QuestGenerator();
    ~QuestGenerator() = default;

    // Quest generation
    std::shared_ptr<Quest> generateQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateQuestFromTemplate(const QuestTemplate& templ,
                                                      const QuestGenerationParams& params);
    std::vector<std::shared_ptr<Quest>> generateQuestBatch(int count,
                                                            const QuestGenerationParams& params);

    // Template management
    void registerTemplate(const QuestTemplate& templ);
    void loadTemplatesFromJson(const std::string& jsonPath);
    std::vector<QuestTemplate> getTemplatesByType(QuestType type) const;
    std::vector<QuestTemplate> getTemplatesByFaction(Faction faction) const;

    // Dynamic quest types
    std::shared_ptr<Quest> generateKillQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateCollectionQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateEscortQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateDeliveryQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateExplorationQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateBossQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generatePuzzleQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateDefenseQuest(const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateCraftingQuest(const QuestGenerationParams& params);

    // Reward generation
    QuestReward generateReward(int questLevel, QuestDifficulty difficulty);
    int calculateExperienceReward(int questLevel, QuestDifficulty difficulty);
    int calculateGoldReward(int questLevel, QuestDifficulty difficulty);
    std::vector<std::string> generateItemRewards(int questLevel, QuestDifficulty difficulty);

    // Objective generation
    QuestObjective generateObjective(ObjectiveType type,
                                     const std::string& targetId,
                                     int playerLevel);
    std::vector<QuestObjective> generateObjectiveChain(const QuestGenerationParams& params,
                                                        int objectiveCount);

    // Quest naming
    std::string generateQuestName(ObjectiveType type, const std::string& targetId);
    std::string generateQuestDescription(const QuestTemplate& templ,
                                         const std::vector<QuestObjective>& objectives);

    // Balancing
    void setDifficultyMultiplier(QuestDifficulty difficulty, float multiplier);
    int scaleToPlayerLevel(int baseValue, int playerLevel);
    QuestDifficulty determineDifficulty(int playerLevel, int questLevel);

    // Configuration
    void setSeed(unsigned int seed);
    void setMinObjectives(int count) { m_minObjectives = count; }
    void setMaxObjectives(int count) { m_maxObjectives = count; }
    void setRewardMultiplier(float multiplier) { m_rewardMultiplier = multiplier; }

private:
    // Helper methods
    std::string selectRandomElement(const std::vector<std::string>& elements);
    int getRandomInt(int min, int max);
    float getRandomFloat(float min, float max);
    std::string generateUniqueId();

    // Quest content pools
    void initializeDefaultPools();
    void loadContentPools();

    // Data members
    std::vector<QuestTemplate> m_templates;
    std::mt19937 m_randomEngine;
    std::unordered_set<std::string> m_generatedIds;

    // Content pools
    std::vector<std::string> m_enemyPool;
    std::vector<std::string> m_itemPool;
    std::vector<std::string> m_npcPool;
    std::vector<std::string> m_locationPool;
    std::vector<std::string> m_bossPool;

    // Name pools
    std::map<ObjectiveType, std::vector<std::string>> m_namePrefixes;
    std::map<ObjectiveType, std::vector<std::string>> m_nameSuffixes;
    std::map<ObjectiveType, std::vector<std::string>> m_descriptionTemplates;

    // Balancing parameters
    std::map<QuestDifficulty, float> m_difficultyMultipliers;
    int m_minObjectives;
    int m_maxObjectives;
    float m_rewardMultiplier;
    int m_idCounter;
};

// Quest chain generator
class QuestChainGenerator {
public:
    QuestChainGenerator();

    // Generate quest chains
    std::vector<std::shared_ptr<Quest>> generateLinearChain(int chainLength,
                                                             const QuestGenerationParams& params);
    std::vector<std::shared_ptr<Quest>> generateBranchingChain(int branchCount,
                                                                int questsPerBranch,
                                                                const QuestGenerationParams& params);
    std::vector<std::shared_ptr<Quest>> generateStoryArc(const std::string& arcName,
                                                          int chapterCount,
                                                          const QuestGenerationParams& params);

    // Chain configuration
    void setChainTheme(const std::string& theme) { m_chainTheme = theme; }
    void setProgression(bool progressive) { m_progressive = progressive; }

private:
    void linkQuests(std::shared_ptr<Quest> from, std::shared_ptr<Quest> to);
    std::shared_ptr<Quest> generateChainQuest(int questIndex,
                                               int totalQuests,
                                               const QuestGenerationParams& params);

    QuestGenerator m_generator;
    std::string m_chainTheme;
    bool m_progressive;  // If true, difficulty increases through chain
};

// Special event quest generator
class EventQuestGenerator {
public:
    EventQuestGenerator();

    // Seasonal events
    std::shared_ptr<Quest> generateHolidayQuest(const std::string& holiday,
                                                 const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateSeasonalQuest(const std::string& season,
                                                  const QuestGenerationParams& params);

    // Time-limited events
    std::shared_ptr<Quest> generateTimedEvent(int durationSeconds,
                                               const QuestGenerationParams& params);
    std::shared_ptr<Quest> generateRaidEvent(int minPlayers,
                                              int maxPlayers,
                                              const QuestGenerationParams& params);

    // Community events
    std::shared_ptr<Quest> generateCommunityGoalQuest(int totalPlayersNeeded,
                                                       const QuestGenerationParams& params);

private:
    QuestGenerator m_generator;
    std::map<std::string, QuestTemplate> m_eventTemplates;
};

// Dynamic difficulty adjuster
class DynamicDifficultyAdjuster {
public:
    DynamicDifficultyAdjuster();

    // Adjust quest difficulty based on player performance
    void recordQuestCompletion(const std::string& questId, bool succeeded, float completionTime);
    QuestDifficulty getRecommendedDifficulty() const;
    float getSuccessRate() const;

    // Adaptive scaling
    void adjustQuestObjectives(std::shared_ptr<Quest> quest);
    void adjustQuestRewards(QuestReward& reward);

    void reset();

private:
    struct PerformanceData {
        int totalAttempts;
        int successes;
        int failures;
        float averageCompletionTime;
        std::chrono::system_clock::time_point lastUpdate;
    };

    PerformanceData m_performance;
    float m_difficultyScaling;
};

// Procedural quest content generator
class ProceduralContentGenerator {
public:
    ProceduralContentGenerator();

    // Generate quest elements
    std::string generateEnemyEncounter(int playerLevel, int count);
    std::string generateLootTable(int playerLevel, QuestDifficulty difficulty);
    std::string generateDialogue(const std::string& npcId, const std::string& questContext);
    std::string generatePuzzle(QuestDifficulty difficulty);
    std::string generateLocation(const std::string& biome);

    // Story generation
    std::string generateQuestStory(const QuestTemplate& templ,
                                   const std::vector<QuestObjective>& objectives);
    std::vector<std::string> generateDialogueTree(const std::string& questId);

private:
    std::mt19937 m_randomEngine;

    // Content libraries
    std::map<std::string, std::vector<std::string>> m_dialogueLibrary;
    std::map<std::string, std::vector<std::string>> m_storyFragments;
    std::vector<std::string> m_puzzleTypes;
};

} // namespace QuestSystem

#endif // QUEST_GENERATOR_H
