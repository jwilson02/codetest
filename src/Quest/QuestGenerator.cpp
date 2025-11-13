#include "QuestGenerator.h"
#include <sstream>
#include <ctime>
#include <algorithm>

namespace QuestSystem {

QuestGenerator::QuestGenerator()
    : m_randomEngine(static_cast<unsigned int>(std::time(nullptr)))
    , m_minObjectives(1)
    , m_maxObjectives(5)
    , m_rewardMultiplier(1.0f)
    , m_idCounter(0)
{
    initializeDefaultPools();

    // Initialize difficulty multipliers
    m_difficultyMultipliers[QuestDifficulty::TRIVIAL] = 0.5f;
    m_difficultyMultipliers[QuestDifficulty::EASY] = 0.75f;
    m_difficultyMultipliers[QuestDifficulty::NORMAL] = 1.0f;
    m_difficultyMultipliers[QuestDifficulty::HARD] = 1.5f;
    m_difficultyMultipliers[QuestDifficulty::VERY_HARD] = 2.0f;
    m_difficultyMultipliers[QuestDifficulty::LEGENDARY] = 3.0f;
    m_difficultyMultipliers[QuestDifficulty::MYTHIC] = 5.0f;
}

std::shared_ptr<Quest> QuestGenerator::generateQuest(const QuestGenerationParams& params) {
    // Select appropriate template
    std::vector<QuestTemplate> availableTemplates;
    for (const auto& templ : m_templates) {
        if (templ.minLevel <= params.playerLevel && templ.maxLevel >= params.playerLevel) {
            if (templ.faction == Faction::NONE || templ.faction == params.playerFaction) {
                availableTemplates.push_back(templ);
            }
        }
    }

    if (availableTemplates.empty()) {
        // Generate a generic quest if no templates available
        return generateKillQuest(params);
    }

    int index = getRandomInt(0, static_cast<int>(availableTemplates.size()) - 1);
    return generateQuestFromTemplate(availableTemplates[index], params);
}

std::shared_ptr<Quest> QuestGenerator::generateQuestFromTemplate(
    const QuestTemplate& templ,
    const QuestGenerationParams& params) {

    std::string questId = generateUniqueId();
    std::string questName = templ.namePrefix + " " + templ.nameSuffix;

    auto quest = std::make_shared<Quest>(questId, questName, templ.type);
    quest->setDifficulty(templ.difficulty);
    quest->setRecommendedLevel(params.playerLevel);

    // Generate objectives
    int objectiveCount = getRandomInt(m_minObjectives, m_maxObjectives);
    auto objectives = generateObjectiveChain(params, objectiveCount);

    for (const auto& obj : objectives) {
        quest->addObjective(obj);
    }

    // Generate description
    std::string description = generateQuestDescription(templ, objectives);
    quest->setDescription(description);

    // Generate rewards
    QuestReward reward = generateReward(params.playerLevel, templ.difficulty);

    // Add faction reputation if applicable
    if (templ.faction != Faction::NONE) {
        reward.reputationGains[templ.faction] = params.playerLevel * 10;
        quest->setRequiredFaction(templ.faction, 0);
    }

    quest->setReward(reward);
    quest->setStatus(QuestStatus::AVAILABLE);

    return quest;
}

std::vector<std::shared_ptr<Quest>> QuestGenerator::generateQuestBatch(
    int count,
    const QuestGenerationParams& params) {

    std::vector<std::shared_ptr<Quest>> quests;
    for (int i = 0; i < count; ++i) {
        auto quest = generateQuest(params);
        if (quest) {
            quests.push_back(quest);
        }
    }
    return quests;
}

void QuestGenerator::registerTemplate(const QuestTemplate& templ) {
    m_templates.push_back(templ);
}

std::vector<QuestTemplate> QuestGenerator::getTemplatesByType(QuestType type) const {
    std::vector<QuestTemplate> templates;
    for (const auto& templ : m_templates) {
        if (templ.type == type) {
            templates.push_back(templ);
        }
    }
    return templates;
}

std::vector<QuestTemplate> QuestGenerator::getTemplatesByFaction(Faction faction) const {
    std::vector<QuestTemplate> templates;
    for (const auto& templ : m_templates) {
        if (templ.faction == faction) {
            templates.push_back(templ);
        }
    }
    return templates;
}

std::shared_ptr<Quest> QuestGenerator::generateKillQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string enemyType = selectRandomElement(m_enemyPool);
    int killCount = scaleToPlayerLevel(getRandomInt(5, 15), params.playerLevel);

    std::string questName = "Slay the " + enemyType;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::KILL;
    objective.description = "Defeat " + std::to_string(killCount) + " " + enemyType;
    objective.targetId = enemyType;
    objective.requiredCount = killCount;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("The local area is being terrorized by " + enemyType +
                         ". Defeat them to restore peace.");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateCollectionQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string itemType = selectRandomElement(m_itemPool);
    int collectCount = scaleToPlayerLevel(getRandomInt(3, 10), params.playerLevel);

    std::string questName = "Gather " + itemType;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::COLLECT;
    objective.description = "Collect " + std::to_string(collectCount) + " " + itemType;
    objective.targetId = itemType;
    objective.requiredCount = collectCount;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("We need " + itemType + " for our supplies. Please gather them.");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateEscortQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string npcName = selectRandomElement(m_npcPool);
    std::string destination = selectRandomElement(m_locationPool);

    std::string questName = "Escort " + npcName;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::ESCORT_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::ESCORT;
    objective.description = "Safely escort " + npcName + " to " + destination;
    objective.targetId = npcName;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription(npcName + " needs protection while traveling to " + destination + ".");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateDeliveryQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string item = selectRandomElement(m_itemPool);
    std::string npcName = selectRandomElement(m_npcPool);

    std::string questName = "Deliver to " + npcName;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::DELIVER;
    objective.description = "Deliver " + item + " to " + npcName;
    objective.targetId = npcName;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("Please deliver this " + item + " to " + npcName + ".");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(QuestDifficulty::EASY);
    quest->setReward(generateReward(params.playerLevel, QuestDifficulty::EASY));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateExplorationQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string location = selectRandomElement(m_locationPool);

    std::string questName = "Explore " + location;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::DISCOVER;
    objective.description = "Discover the location: " + location;
    objective.targetId = location;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("Explore and discover the mysterious location known as " + location + ".");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateBossQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string bossName = selectRandomElement(m_bossPool);

    std::string questName = "Defeat " + bossName;
    auto bossQuest = std::make_shared<BossQuest>(questId, questName, bossName);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::BOSS_KILL;
    objective.description = "Defeat the powerful " + bossName;
    objective.targetId = bossName;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    bossQuest->addObjective(objective);
    bossQuest->setDescription("A powerful enemy known as " + bossName +
                             " threatens the realm. Gather your strength and defeat this menace!");
    bossQuest->setRecommendedLevel(params.playerLevel + 2);
    bossQuest->setDifficulty(QuestDifficulty::HARD);
    bossQuest->setMinPlayers(1);
    bossQuest->setMaxPlayers(5);

    // Boss quests have better rewards
    auto reward = generateReward(params.playerLevel + 2, QuestDifficulty::HARD);
    reward.experience *= 2;
    reward.gold *= 2;
    bossQuest->setReward(reward);

    return bossQuest;
}

std::shared_ptr<Quest> QuestGenerator::generatePuzzleQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();

    std::string questName = "The Ancient Puzzle";
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::PUZZLE;
    objective.description = "Solve the ancient puzzle";
    objective.targetId = "ancient_puzzle";
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("An ancient puzzle blocks your path. Use your wits to solve it.");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateDefenseQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string location = selectRandomElement(m_locationPool);

    std::string questName = "Defend " + location;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::DEFEND;
    objective.description = "Defend " + location + " from attackers";
    objective.targetId = location;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription(location + " is under attack! Defend it at all costs!");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(QuestDifficulty::HARD);
    quest->setTimeLimit(600);  // 10 minutes
    quest->setReward(generateReward(params.playerLevel, QuestDifficulty::HARD));

    return quest;
}

std::shared_ptr<Quest> QuestGenerator::generateCraftingQuest(const QuestGenerationParams& params) {
    std::string questId = generateUniqueId();
    std::string item = selectRandomElement(m_itemPool);

    std::string questName = "Craft " + item;
    auto quest = std::make_shared<Quest>(questId, questName, QuestType::SIDE_QUEST);

    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::CRAFT;
    objective.description = "Craft " + item;
    objective.targetId = item;
    objective.requiredCount = 1;
    objective.currentCount = 0;

    quest->addObjective(objective);
    quest->setDescription("We need a skilled craftsman to create " + item + ".");
    quest->setRecommendedLevel(params.playerLevel);
    quest->setDifficulty(params.targetDifficulty);
    quest->setReward(generateReward(params.playerLevel, params.targetDifficulty));

    return quest;
}

QuestReward QuestGenerator::generateReward(int questLevel, QuestDifficulty difficulty) {
    QuestReward reward;

    reward.experience = calculateExperienceReward(questLevel, difficulty);
    reward.gold = calculateGoldReward(questLevel, difficulty);
    reward.items = generateItemRewards(questLevel, difficulty);

    return reward;
}

int QuestGenerator::calculateExperienceReward(int questLevel, QuestDifficulty difficulty) {
    int baseXP = questLevel * 100;
    float multiplier = m_difficultyMultipliers[difficulty];
    return static_cast<int>(baseXP * multiplier * m_rewardMultiplier);
}

int QuestGenerator::calculateGoldReward(int questLevel, QuestDifficulty difficulty) {
    int baseGold = questLevel * 50;
    float multiplier = m_difficultyMultipliers[difficulty];
    return static_cast<int>(baseGold * multiplier * m_rewardMultiplier);
}

std::vector<std::string> QuestGenerator::generateItemRewards(int questLevel,
                                                              QuestDifficulty difficulty) {
    std::vector<std::string> items;

    int itemCount = 0;
    if (difficulty >= QuestDifficulty::NORMAL) itemCount = 1;
    if (difficulty >= QuestDifficulty::HARD) itemCount = 2;
    if (difficulty >= QuestDifficulty::LEGENDARY) itemCount = 3;

    for (int i = 0; i < itemCount; ++i) {
        items.push_back(selectRandomElement(m_itemPool));
    }

    return items;
}

QuestObjective QuestGenerator::generateObjective(ObjectiveType type,
                                                  const std::string& targetId,
                                                  int playerLevel) {
    QuestObjective objective;
    objective.id = generateUniqueId();
    objective.type = type;
    objective.targetId = targetId;
    objective.requiredCount = scaleToPlayerLevel(getRandomInt(1, 5), playerLevel);
    objective.currentCount = 0;
    objective.optional = false;
    objective.hidden = false;

    // Generate description based on type
    switch (type) {
        case ObjectiveType::KILL:
            objective.description = "Defeat " + std::to_string(objective.requiredCount) + " " + targetId;
            break;
        case ObjectiveType::COLLECT:
            objective.description = "Collect " + std::to_string(objective.requiredCount) + " " + targetId;
            break;
        case ObjectiveType::INTERACT:
            objective.description = "Interact with " + targetId;
            break;
        case ObjectiveType::REACH_LOCATION:
            objective.description = "Travel to " + targetId;
            break;
        default:
            objective.description = "Complete objective: " + targetId;
            break;
    }

    return objective;
}

std::vector<QuestObjective> QuestGenerator::generateObjectiveChain(
    const QuestGenerationParams& params,
    int objectiveCount) {

    std::vector<QuestObjective> objectives;

    for (int i = 0; i < objectiveCount; ++i) {
        ObjectiveType type = static_cast<ObjectiveType>(getRandomInt(0, 5));
        std::string targetId;

        switch (type) {
            case ObjectiveType::KILL:
                targetId = selectRandomElement(m_enemyPool);
                break;
            case ObjectiveType::COLLECT:
                targetId = selectRandomElement(m_itemPool);
                break;
            case ObjectiveType::INTERACT:
                targetId = selectRandomElement(m_npcPool);
                break;
            case ObjectiveType::REACH_LOCATION:
                targetId = selectRandomElement(m_locationPool);
                break;
            default:
                targetId = "unknown";
                break;
        }

        objectives.push_back(generateObjective(type, targetId, params.playerLevel));
    }

    return objectives;
}

std::string QuestGenerator::generateQuestName(ObjectiveType type, const std::string& targetId) {
    auto it = m_namePrefixes.find(type);
    if (it == m_namePrefixes.end() || it->second.empty()) {
        return "Quest: " + targetId;
    }

    std::string prefix = selectRandomElement(it->second);
    return prefix + " " + targetId;
}

std::string QuestGenerator::generateQuestDescription(
    const QuestTemplate& templ,
    const std::vector<QuestObjective>& objectives) {

    if (templ.descriptionTemplates.empty()) {
        return "Complete the quest objectives.";
    }

    std::string description = selectRandomElement(templ.descriptionTemplates);

    // Replace placeholders if any
    // This is a simple implementation - could be made more sophisticated
    return description;
}

void QuestGenerator::setDifficultyMultiplier(QuestDifficulty difficulty, float multiplier) {
    m_difficultyMultipliers[difficulty] = multiplier;
}

int QuestGenerator::scaleToPlayerLevel(int baseValue, int playerLevel) {
    return static_cast<int>(baseValue * (1.0f + playerLevel * 0.1f));
}

QuestDifficulty QuestGenerator::determineDifficulty(int playerLevel, int questLevel) {
    int levelDiff = questLevel - playerLevel;

    if (levelDiff <= -5) return QuestDifficulty::TRIVIAL;
    if (levelDiff <= -2) return QuestDifficulty::EASY;
    if (levelDiff <= 2) return QuestDifficulty::NORMAL;
    if (levelDiff <= 5) return QuestDifficulty::HARD;
    if (levelDiff <= 10) return QuestDifficulty::VERY_HARD;
    if (levelDiff <= 15) return QuestDifficulty::LEGENDARY;
    return QuestDifficulty::MYTHIC;
}

void QuestGenerator::setSeed(unsigned int seed) {
    m_randomEngine.seed(seed);
}

std::string QuestGenerator::selectRandomElement(const std::vector<std::string>& elements) {
    if (elements.empty()) {
        return "unknown";
    }
    int index = getRandomInt(0, static_cast<int>(elements.size()) - 1);
    return elements[index];
}

int QuestGenerator::getRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_randomEngine);
}

float QuestGenerator::getRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_randomEngine);
}

std::string QuestGenerator::generateUniqueId() {
    std::stringstream ss;
    ss << "quest_gen_" << m_idCounter++;
    return ss.str();
}

void QuestGenerator::initializeDefaultPools() {
    // Enemy pool
    m_enemyPool = {
        "Goblins", "Bandits", "Wolves", "Skeletons", "Zombies",
        "Orcs", "Trolls", "Spiders", "Rats", "Bats",
        "Dark Knights", "Cultists", "Demons", "Dragons", "Undead"
    };

    // Item pool
    m_itemPool = {
        "Herbs", "Crystals", "Ores", "Gems", "Scrolls",
        "Potions", "Artifacts", "Relics", "Ingredients", "Materials",
        "Ancient Coins", "Magic Stones", "Dragon Scales", "Phoenix Feathers"
    };

    // NPC pool
    m_npcPool = {
        "Merchant Gareth", "Elder Thane", "Priestess Elara", "Guard Captain Marcus",
        "Blacksmith Durin", "Wizard Aldric", "Ranger Sylvia", "Innkeeper Thomas",
        "Noble Lady Catherine", "Hermit Sage"
    };

    // Location pool
    m_locationPool = {
        "Ancient Ruins", "Dark Forest", "Mountain Peak", "Coastal Village",
        "Desert Oasis", "Frozen Wasteland", "Haunted Castle", "Sacred Temple",
        "Underground Caverns", "Mystic Grove", "Abandoned Mine", "Volcano Crater"
    };

    // Boss pool
    m_bossPool = {
        "Shadow Lord Malachar", "Dragon Queen Pyraxis", "Lich King Mortis",
        "Demon Prince Infernus", "Frost Giant Ymir", "Corrupted Treant",
        "Void Stalker", "Ancient Hydra", "Death Knight Commander", "Dark Sorcerer"
    };

    // Name prefixes
    m_namePrefixes[ObjectiveType::KILL] = {"Slay", "Defeat", "Eliminate", "Destroy", "Hunt"};
    m_namePrefixes[ObjectiveType::COLLECT] = {"Gather", "Collect", "Retrieve", "Acquire", "Obtain"};
    m_namePrefixes[ObjectiveType::ESCORT] = {"Escort", "Protect", "Guard", "Accompany"};
    m_namePrefixes[ObjectiveType::DELIVER] = {"Deliver", "Transport", "Bring", "Carry"};
    m_namePrefixes[ObjectiveType::EXPLORE] = {"Explore", "Discover", "Find", "Locate"};
}

// QuestChainGenerator implementation
QuestChainGenerator::QuestChainGenerator()
    : m_progressive(true)
{
}

std::vector<std::shared_ptr<Quest>> QuestChainGenerator::generateLinearChain(
    int chainLength,
    const QuestGenerationParams& params) {

    std::vector<std::shared_ptr<Quest>> chain;

    for (int i = 0; i < chainLength; ++i) {
        auto quest = generateChainQuest(i, chainLength, params);
        chain.push_back(quest);

        if (i > 0) {
            linkQuests(chain[i - 1], quest);
        }
    }

    return chain;
}

std::vector<std::shared_ptr<Quest>> QuestChainGenerator::generateBranchingChain(
    int branchCount,
    int questsPerBranch,
    const QuestGenerationParams& params) {

    std::vector<std::shared_ptr<Quest>> chain;

    // Create starting quest
    auto startQuest = m_generator.generateQuest(params);
    chain.push_back(startQuest);

    // Create branches
    for (int branch = 0; branch < branchCount; ++branch) {
        for (int i = 0; i < questsPerBranch; ++i) {
            auto quest = m_generator.generateQuest(params);
            chain.push_back(quest);

            if (i == 0) {
                linkQuests(startQuest, quest);
            } else {
                linkQuests(chain[chain.size() - 2], quest);
            }
        }
    }

    return chain;
}

std::vector<std::shared_ptr<Quest>> QuestChainGenerator::generateStoryArc(
    const std::string& arcName,
    int chapterCount,
    const QuestGenerationParams& params) {

    std::vector<std::shared_ptr<Quest>> arc;

    for (int chapter = 1; chapter <= chapterCount; ++chapter) {
        std::string questId = "story_" + arcName + "_ch" + std::to_string(chapter);
        std::string questName = arcName + " - Chapter " + std::to_string(chapter);

        auto storyQuest = std::make_shared<StoryQuest>(questId, questName, chapter);
        storyQuest->setStoryArc(arcName);
        storyQuest->setDifficulty(m_generator.determineDifficulty(params.playerLevel,
                                                                   params.playerLevel + chapter));

        // Generate appropriate objectives for story quest
        auto objectives = m_generator.generateObjectiveChain(params, 3);
        for (const auto& obj : objectives) {
            storyQuest->addObjective(obj);
        }

        storyQuest->setReward(m_generator.generateReward(params.playerLevel + chapter,
                                                          storyQuest->getDifficulty()));

        arc.push_back(storyQuest);

        if (chapter > 1) {
            linkQuests(arc[chapter - 2], storyQuest);
        }
    }

    return arc;
}

void QuestChainGenerator::linkQuests(std::shared_ptr<Quest> from, std::shared_ptr<Quest> to) {
    from->addNextQuest(to->getId());
    to->addPrerequisiteQuest(from->getId());
}

std::shared_ptr<Quest> QuestChainGenerator::generateChainQuest(
    int questIndex,
    int totalQuests,
    const QuestGenerationParams& params) {

    QuestGenerationParams adjustedParams = params;

    if (m_progressive) {
        adjustedParams.playerLevel += questIndex;
        adjustedParams.targetDifficulty = m_generator.determineDifficulty(
            params.playerLevel, params.playerLevel + questIndex);
    }

    return m_generator.generateQuest(adjustedParams);
}

// EventQuestGenerator implementation
EventQuestGenerator::EventQuestGenerator() {
}

std::shared_ptr<Quest> EventQuestGenerator::generateHolidayQuest(
    const std::string& holiday,
    const QuestGenerationParams& params) {

    std::string questId = "event_" + holiday + "_" + std::to_string(std::time(nullptr));
    std::string questName = holiday + " Special Event";

    auto quest = std::make_shared<Quest>(questId, questName, QuestType::TIMED_EVENT);
    quest->setDescription("A special quest for " + holiday + "!");
    quest->setTimeLimit(86400);  // 24 hours

    // Generate themed objectives
    auto objectives = m_generator.generateObjectiveChain(params, 3);
    for (const auto& obj : objectives) {
        quest->addObjective(obj);
    }

    // Special rewards
    auto reward = m_generator.generateReward(params.playerLevel, QuestDifficulty::HARD);
    reward.experience *= 2;
    reward.gold *= 2;
    quest->setReward(reward);

    return quest;
}

std::shared_ptr<Quest> EventQuestGenerator::generateTimedEvent(
    int durationSeconds,
    const QuestGenerationParams& params) {

    auto quest = m_generator.generateQuest(params);
    quest->setTimeLimit(durationSeconds);

    // Boost rewards for timed events
    auto reward = quest->getReward();
    reward.experience = static_cast<int>(reward.experience * 1.5f);
    quest->setReward(reward);

    return quest;
}

std::shared_ptr<Quest> EventQuestGenerator::generateRaidEvent(
    int minPlayers,
    int maxPlayers,
    const QuestGenerationParams& params) {

    std::string questId = "raid_" + std::to_string(std::time(nullptr));
    std::string bossName = "Raid Boss";

    auto raidQuest = std::make_shared<BossQuest>(questId, "Raid: " + bossName, bossName);
    raidQuest->setMinPlayers(minPlayers);
    raidQuest->setMaxPlayers(maxPlayers);
    raidQuest->setDifficulty(QuestDifficulty::LEGENDARY);

    // Raid objectives
    QuestObjective objective;
    objective.id = questId + "_obj1";
    objective.type = ObjectiveType::BOSS_KILL;
    objective.description = "Defeat the raid boss";
    objective.targetId = bossName;
    objective.requiredCount = 1;

    raidQuest->addObjective(objective);

    // Massive rewards
    auto reward = m_generator.generateReward(params.playerLevel + 5, QuestDifficulty::LEGENDARY);
    reward.experience *= 5;
    reward.gold *= 5;
    raidQuest->setReward(reward);

    return raidQuest;
}

// DynamicDifficultyAdjuster implementation
DynamicDifficultyAdjuster::DynamicDifficultyAdjuster()
    : m_difficultyScaling(1.0f)
{
    reset();
}

void DynamicDifficultyAdjuster::recordQuestCompletion(
    const std::string& questId,
    bool succeeded,
    float completionTime) {

    m_performance.totalAttempts++;

    if (succeeded) {
        m_performance.successes++;

        float totalTime = m_performance.averageCompletionTime * (m_performance.successes - 1);
        m_performance.averageCompletionTime = (totalTime + completionTime) / m_performance.successes;
    } else {
        m_performance.failures++;
    }

    m_performance.lastUpdate = std::chrono::system_clock::now();

    // Adjust difficulty scaling
    float successRate = getSuccessRate();
    if (successRate > 0.8f) {
        m_difficultyScaling = std::min(2.0f, m_difficultyScaling + 0.1f);
    } else if (successRate < 0.4f) {
        m_difficultyScaling = std::max(0.5f, m_difficultyScaling - 0.1f);
    }
}

QuestDifficulty DynamicDifficultyAdjuster::getRecommendedDifficulty() const {
    float successRate = getSuccessRate();

    if (successRate > 0.9f) return QuestDifficulty::HARD;
    if (successRate > 0.7f) return QuestDifficulty::NORMAL;
    if (successRate > 0.5f) return QuestDifficulty::EASY;
    return QuestDifficulty::TRIVIAL;
}

float DynamicDifficultyAdjuster::getSuccessRate() const {
    if (m_performance.totalAttempts == 0) {
        return 0.5f;  // Default
    }
    return static_cast<float>(m_performance.successes) / m_performance.totalAttempts;
}

void DynamicDifficultyAdjuster::reset() {
    m_performance.totalAttempts = 0;
    m_performance.successes = 0;
    m_performance.failures = 0;
    m_performance.averageCompletionTime = 0.0f;
    m_difficultyScaling = 1.0f;
}

// ProceduralContentGenerator implementation
ProceduralContentGenerator::ProceduralContentGenerator()
    : m_randomEngine(static_cast<unsigned int>(std::time(nullptr)))
{
}

std::string ProceduralContentGenerator::generateEnemyEncounter(int playerLevel, int count) {
    std::stringstream ss;
    ss << "enemy_encounter_" << playerLevel << "_" << count;
    return ss.str();
}

std::string ProceduralContentGenerator::generateLootTable(int playerLevel,
                                                          QuestDifficulty difficulty) {
    std::stringstream ss;
    ss << "loot_table_" << playerLevel << "_" << static_cast<int>(difficulty);
    return ss.str();
}

std::string ProceduralContentGenerator::generateDialogue(const std::string& npcId,
                                                         const std::string& questContext) {
    return "Greetings, adventurer! I have a task for you...";
}

std::string ProceduralContentGenerator::generatePuzzle(QuestDifficulty difficulty) {
    return "puzzle_" + std::to_string(static_cast<int>(difficulty));
}

std::string ProceduralContentGenerator::generateLocation(const std::string& biome) {
    return "location_" + biome;
}

} // namespace QuestSystem
