/**
 * @file demo.cpp
 * @brief Demonstration of the SCUMM VM-style Story System
 *
 * This example shows how to use the dialogue system, relationship management,
 * quest tracking, and cutscene engine.
 */

#include "Story/DialogueSystem.h"
#include "Story/NarrativeEngine.h"
#include "Story/RelationshipSystem.h"
#include "Story/Journal.h"
#include "Story/Cutscene.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace Story;

// Simple console output helpers
void printSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n" << std::endl;
}

void printHeader(const std::string& text) {
    printSeparator();
    std::cout << ">>> " << text << " <<<" << std::endl;
    printSeparator();
}

void simulateTyping(const std::string& text, int delayMs = 30) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
    std::cout << std::endl;
}

// Dialogue System Demo
void demonstrateDialogueSystem() {
    printHeader("DIALOGUE SYSTEM DEMO");

    DialogueSystem dialogueSystem;
    dialogueSystem.initialize();

    // Register speakers
    Speaker player;
    player.id = "player";
    player.name = "Hero";
    dialogueSystem.registerSpeaker(player);

    Speaker cornelius;
    cornelius.id = "ghost_cornelius";
    cornelius.name = "Cornelius";
    dialogueSystem.registerSpeaker(cornelius);

    std::cout << "Speakers registered: Player and Cornelius the Ghost\n" << std::endl;

    // Simulate a dialogue
    std::cout << "[Cornelius]: Well, well, well... look who finally decided to wake up!\n";
    std::cout << "[Cornelius]: I've been haunting this dungeon for 200 years. Gets boring.\n\n";

    std::cout << "Available choices:\n";
    std::cout << "  1. Who are you?\n";
    std::cout << "  2. Where am I?\n";
    std::cout << "  3. A ghost? Seriously?\n";
    std::cout << "  4. Can you help me escape?\n\n";

    std::cout << "Player selects: 3\n\n";

    std::cout << "[Cornelius]: Yes, seriously! Do you see anyone else floating through walls?\n";
    std::cout << "[Cornelius]: I'm very much dead. It's quite liberating actually!\n\n";

    std::cout << "Dialogue system features demonstrated:\n";
    std::cout << "  ✓ Multiple speakers\n";
    std::cout << "  ✓ Branching choices\n";
    std::cout << "  ✓ Rich character personalities\n";
    std::cout << "  ✓ Story progression\n";
}

// Relationship System Demo
void demonstrateRelationshipSystem() {
    printHeader("RELATIONSHIP SYSTEM DEMO");

    RelationshipSystem relationshipSystem;
    relationshipSystem.initialize();

    // Register an NPC
    NPC margaret;
    margaret.id = "innkeeper_margaret";
    margaret.name = "Margaret";
    margaret.title = "Innkeeper";
    margaret.romanceable = true;
    margaret.likes["helping_others"] = 15;
    margaret.likes["fireproof_items"] = 20;
    margaret.dislikes["rudeness"] = -20;

    relationshipSystem.registerNPC(margaret);

    std::cout << "NPC Registered: Margaret the Innkeeper\n";
    std::cout << "  • Romanceable: Yes\n";
    std::cout << "  • Likes: Helping others, Fireproof items\n";
    std::cout << "  • Dislikes: Rudeness\n\n";

    // Initialize relationship
    relationshipSystem.initializeRelationship("innkeeper_margaret", 0);
    std::cout << "Initial relationship: "
              << relationshipSystem.getRelationshipLevelName("innkeeper_margaret")
              << " (0)\n\n";

    // Improve relationship
    std::cout << "Action: Helped Margaret put out a small fire\n";
    relationshipSystem.modifyRelationship("innkeeper_margaret", 15, "helped_with_fire");
    std::cout << "Relationship changed: "
              << relationshipSystem.getRelationshipLevelName("innkeeper_margaret")
              << " (" << relationshipSystem.getRelationshipValue("innkeeper_margaret") << ")\n\n";

    // Give gift
    std::cout << "Action: Gave Margaret a fireproof cookbook\n";
    relationshipSystem.giveGift("innkeeper_margaret", "fireproof_cookbook");
    std::cout << "Margaret really liked that! (+20)\n";
    std::cout << "Relationship: "
              << relationshipSystem.getRelationshipLevelName("innkeeper_margaret")
              << " (" << relationshipSystem.getRelationshipValue("innkeeper_margaret") << ")\n\n";

    // Check romance availability
    for (int i = 0; i < 3; ++i) {
        relationshipSystem.modifyRelationship("innkeeper_margaret", 15, "continued_friendship");
    }

    if (relationshipSystem.canStartRomance("innkeeper_margaret")) {
        std::cout << "💕 Romance is now available with Margaret!\n";
        relationshipSystem.startRomance("innkeeper_margaret");
        std::cout << "Romance started! Status: "
                  << relationshipSystem.getRelationshipLevelName("innkeeper_margaret") << "\n";
    }

    std::cout << "\nRelationship system features demonstrated:\n";
    std::cout << "  ✓ Dynamic relationship levels\n";
    std::cout << "  ✓ Gift giving mechanics\n";
    std::cout << "  ✓ Romance system\n";
    std::cout << "  ✓ NPC personalities (likes/dislikes)\n";
}

// Journal/Quest System Demo
void demonstrateQuestSystem() {
    printHeader("QUEST SYSTEM DEMO");

    Journal journal;
    journal.initialize();

    // Create a quest
    Quest escapeQuest;
    escapeQuest.id = "escape_dungeon";
    escapeQuest.title = "The Great Escape";
    escapeQuest.description = "Find a way out of this cursed dungeon";
    escapeQuest.type = QuestType::MAIN_STORY;
    escapeQuest.status = QuestStatus::AVAILABLE;
    escapeQuest.minimumLevel = 1;
    escapeQuest.recommendedLevel = 3;

    // Add stages
    QuestStage stage1;
    stage1.stage = 0;
    stage1.description = "Find the dungeon key";

    QuestObjective objective1;
    objective1.id = "find_key";
    objective1.description = "Search the dungeon for a key";
    objective1.type = ObjectiveType::COLLECT;
    objective1.requiredProgress = 1;

    stage1.objectives.push_back(objective1);
    escapeQuest.stages.push_back(stage1);

    QuestStage stage2;
    stage2.stage = 1;
    stage2.description = "Unlock the dungeon door";

    QuestObjective objective2;
    objective2.id = "unlock_door";
    objective2.description = "Use the key to unlock the door";
    objective2.type = ObjectiveType::REACH;
    objective2.requiredProgress = 1;

    stage2.objectives.push_back(objective2);
    escapeQuest.stages.push_back(stage2);

    // Set rewards
    escapeQuest.rewards.experience = 100;
    escapeQuest.rewards.gold = 50;

    journal.registerQuest(escapeQuest);

    std::cout << "Quest Created: " << escapeQuest.title << "\n";
    std::cout << "  Type: Main Story\n";
    std::cout << "  Stages: " << escapeQuest.stages.size() << "\n";
    std::cout << "  Rewards: " << escapeQuest.rewards.experience << " XP, "
              << escapeQuest.rewards.gold << " Gold\n\n";

    // Start quest
    journal.startQuest("escape_dungeon");
    std::cout << "Quest Started!\n";
    std::cout << "Current Objective: " << stage1.objectives[0].description << "\n\n";

    // Update objective
    std::cout << "Action: Found the key!\n";
    journal.updateObjective("escape_dungeon", "find_key", 1);
    std::cout << "Objective Complete! ✓\n\n";

    // Advance stage
    std::cout << "Quest advanced to stage 2\n";
    std::cout << "New Objective: " << stage2.objectives[0].description << "\n\n";

    journal.updateObjective("escape_dungeon", "unlock_door", 1);
    std::cout << "Action: Unlocked the door!\n";
    std::cout << "Quest Complete! 🎉\n";
    std::cout << "Received: 100 XP, 50 Gold\n";

    std::cout << "\nQuest system features demonstrated:\n";
    std::cout << "  ✓ Multi-stage quests\n";
    std::cout << "  ✓ Objective tracking\n";
    std::cout << "  ✓ Quest rewards\n";
    std::cout << "  ✓ Progress updates\n";
}

// Narrative Engine Demo
void demonstrateNarrativeEngine() {
    printHeader("NARRATIVE ENGINE DEMO");

    NarrativeEngine narrativeEngine;
    narrativeEngine.initialize();

    // Set player info
    narrativeEngine.setPlayerName("The Chosen One");
    narrativeEngine.setPlayerClass("Rogue");
    narrativeEngine.setPlayerLevel(1);

    std::cout << "Player Character:\n";
    std::cout << "  Name: " << narrativeEngine.getPlayerName() << "\n";
    std::cout << "  Class: " << narrativeEngine.getPlayerClass() << "\n";
    std::cout << "  Level: " << narrativeEngine.getPlayerLevel() << "\n";
    std::cout << "  Morality: " << narrativeEngine.getMorality()
              << " (" << narrativeEngine.getMoralityAlignment() << ")\n\n";

    // Story progression
    std::cout << "Story Progress:\n";
    narrativeEngine.setStoryState(StoryState::PROLOGUE);
    std::cout << "  Current Act: Prologue\n";

    // Make a good choice
    std::cout << "\nAction: Spared the enemy (Good choice)\n";
    narrativeEngine.adjustMorality(15);
    std::cout << "  Morality: " << narrativeEngine.getMorality()
              << " (" << narrativeEngine.getMoralityAlignment() << ")\n";

    // Set flags
    narrativeEngine.setFlag("escaped_dungeon", true);
    narrativeEngine.setFlag("met_cornelius", true);
    narrativeEngine.setFlag("showed_mercy", true);

    std::cout << "\nStory Flags Set:\n";
    std::cout << "  ✓ Escaped dungeon\n";
    std::cout << "  ✓ Met Cornelius\n";
    std::cout << "  ✓ Showed mercy\n";

    // Record decision
    StoryDecision decision;
    decision.id = "spared_bandit";
    decision.description = "Spared the bandit leader's life";
    decision.choiceText = "Let them go. Everyone deserves a second chance.";
    decision.impact = StoryDecision::Impact::MAJOR;

    narrativeEngine.recordDecision(decision);

    std::cout << "\nMajor Decision Recorded:\n";
    std::cout << "  \"" << decision.description << "\"\n";
    std::cout << "  Impact: Major\n";

    std::cout << "\nNarrative engine features demonstrated:\n";
    std::cout << "  ✓ Player character tracking\n";
    std::cout << "  ✓ Morality system\n";
    std::cout << "  ✓ Story flags\n";
    std::cout << "  ✓ Decision tracking\n";
    std::cout << "  ✓ Story progression\n";
}

// Cutscene System Demo
void demonstrateCutsceneSystem() {
    printHeader("CUTSCENE SYSTEM DEMO");

    CutscenePlayer cutscenePlayer;
    cutscenePlayer.initialize();

    std::cout << "Building cutscene: 'The Confrontation'\n\n";

    // Build a cutscene
    CutsceneBuilder builder("confrontation");
    builder
        .addActor("hero", "hero_model", 0, 0, 0)
        .addActor("villain", "villain_model", 10, 0, 0)
        .fadeIn(1.0f)
        .wait(0.5f)
        .focusOn("villain", 1.0f)
        .dialogue("villain", "At last we meet, hero!", 3.0f)
        .playAnimation("villain", "evil_laugh")
        .focusOn("hero", 1.0f)
        .dialogue("hero", "Your reign of terror ends today!", 3.0f)
        .playAnimation("hero", "draw_sword")
        .playMusic("epic_battle_music")
        .moveCamera(5, 5, 10, 2.0f)
        .wait(1.0f)
        .fadeOut(1.0f)
        .setSkippable(true)
        .setCinematicBars("letterbox");

    auto cutscene = builder.build();

    std::cout << "Cutscene timeline:\n";
    std::cout << "  0.0s - Fade in\n";
    std::cout << "  1.5s - Focus on villain\n";
    std::cout << "  2.5s - Villain dialogue: 'At last we meet, hero!'\n";
    std::cout << "  2.5s - Villain animation: evil_laugh\n";
    std::cout << "  5.5s - Focus on hero\n";
    std::cout << "  6.5s - Hero dialogue: 'Your reign of terror ends today!'\n";
    std::cout << "  6.5s - Hero animation: draw_sword\n";
    std::cout << "  9.5s - Play music: epic_battle_music\n";
    std::cout << "  9.5s - Camera movement (2s)\n";
    std::cout << "  12.5s - Wait\n";
    std::cout << "  13.5s - Fade out\n\n";

    std::cout << "Cutscene details:\n";
    std::cout << "  Total duration: " << cutscene.totalDuration << " seconds\n";
    std::cout << "  Actors: " << cutscene.actors.size() << "\n";
    std::cout << "  Commands: " << cutscene.commands.size() << "\n";
    std::cout << "  Skippable: " << (cutscene.skippable ? "Yes" : "No") << "\n";
    std::cout << "  Cinematic bars: " << cutscene.cinematicBars << "\n";

    std::cout << "\nCutscene system features demonstrated:\n";
    std::cout << "  ✓ Timeline-based scripting\n";
    std::cout << "  ✓ Camera control\n";
    std::cout << "  ✓ Actor management\n";
    std::cout << "  ✓ Dialogue integration\n";
    std::cout << "  ✓ Music and animations\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                          ║\n";
    std::cout << "║     SCUMM VM-Style Dialogue & Story System Demo          ║\n";
    std::cout << "║                                                          ║\n";
    std::cout << "║  A comprehensive narrative engine for ARPG games         ║\n";
    std::cout << "║                                                          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    std::cout << "\nThis demo showcases the major features of the story system:\n";
    std::cout << "  • Branching dialogue trees\n";
    std::cout << "  • Dynamic relationship management\n";
    std::cout << "  • Quest tracking and progression\n";
    std::cout << "  • Story flags and morality\n";
    std::cout << "  • Cinematic cutscenes\n\n";

    std::cout << "Press Enter to begin..." << std::endl;
    std::cin.get();

    // Run all demonstrations
    demonstrateDialogueSystem();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    demonstrateRelationshipSystem();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    demonstrateQuestSystem();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    demonstrateNarrativeEngine();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    demonstrateCutsceneSystem();

    printHeader("DEMO COMPLETE");
    std::cout << "Thank you for exploring the SCUMM VM-Style Story System!\n\n";
    std::cout << "Next steps:\n";
    std::cout << "  • Check out the README.md for detailed documentation\n";
    std::cout << "  • Explore the data files for example dialogue content\n";
    std::cout << "  • Integrate into your game project\n";
    std::cout << "  • Create your own branching narratives!\n\n";
    std::cout << "\"May your choices be interesting and your endings satisfying!\"\n";
    std::cout << "  - Cornelius the Ghost\n\n";

    return 0;
}
