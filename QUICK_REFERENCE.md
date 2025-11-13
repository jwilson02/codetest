# SCUMM VM Story System - Quick Reference

## 🚀 Quick Start (30 seconds)

```bash
# Build the system
./build_story_system.sh

# Run demo
cd build_story && ./bin/StorySystemDemo
```

## 📋 File Locations

### C++ Source
```
src/Story/
├── DialogueSystem.h/cpp    # Main dialogue engine
├── DialogueTree.h/cpp      # Branching narrative
├── NarrativeEngine.h/cpp   # Story progression
├── RelationshipSystem.h/cpp # NPC relationships
├── Journal.h/cpp           # Quest tracking
└── Cutscene.h/cpp          # Cutscene player
```

### Story Content
```
data/
├── dialogues/
│   ├── main_story.json     # Main narrative
│   └── npcs.json           # Character dialogues
├── story/
│   └── chapters.json       # Story structure
└── localization/
    ├── en_US.json          # English
    ├── es_ES.json          # Spanish
    └── ja_JP.json          # Japanese
```

## 💻 Basic Usage

### Initialize Systems
```cpp
#include "Story/DialogueSystem.h"
#include "Story/NarrativeEngine.h"
#include "Story/RelationshipSystem.h"
#include "Story/Journal.h"

Story::DialogueSystem dialogue;
Story::NarrativeEngine narrative;
Story::RelationshipSystem relationships;
Story::Journal journal;

dialogue.initialize();
narrative.initialize();
relationships.initialize();
journal.initialize();
```

### Load Data
```cpp
dialogue.loadDialogue("data/dialogues/main_story.json");
narrative.loadChapters("data/story/chapters.json");
relationships.loadNPCs("data/dialogues/npcs.json");
```

### Start Dialogue
```cpp
dialogue.startDialogue("prologue_awakening");

while (dialogue.isDialogueActive()) {
    auto* node = dialogue.getCurrentNode();
    auto choices = dialogue.getAvailableChoices();
    // Display and handle choices
    dialogue.selectChoice(playerChoice);
}
```

## 🎮 Common Operations

### Dialogue
```cpp
// Check if dialogue is active
bool active = dialogue.isDialogueActive();

// Get current dialogue node
const auto* node = dialogue.getCurrentNode();

// Get available choices
auto choices = dialogue.getAvailableChoices();

// Select choice
dialogue.selectChoice(choiceIndex);

// Advance dialogue
dialogue.advanceDialogue();

// End dialogue
dialogue.endDialogue();
```

### Relationships
```cpp
// Modify relationship
relationships.modifyRelationship("npc_id", 15, "helped_them");

// Get relationship level
auto level = relationships.getRelationshipLevel("npc_id");

// Check romance availability
if (relationships.canStartRomance("npc_id")) {
    relationships.startRomance("npc_id");
}

// Give gift
relationships.giveGift("npc_id", "item_id");
```

### Quests
```cpp
// Start quest
journal.startQuest("quest_id");

// Update objective
journal.updateObjective("quest_id", "objective_id", progress);

// Complete objective
journal.completeObjective("quest_id", "objective_id");

// Get active quests
auto quests = journal.getActiveQuests();

// Track quest
journal.setTrackedQuest("quest_id");
```

### Story Progression
```cpp
// Set story flag
narrative.setFlag("flag_id", true);

// Check flag
bool hasFlag = narrative.getFlag("flag_id");

// Adjust morality
narrative.adjustMorality(15); // Good choice

// Record decision
Story::StoryDecision decision;
decision.id = "spared_enemy";
decision.impact = Story::StoryDecision::Impact::MAJOR;
narrative.recordDecision(decision);

// Get available endings
auto endings = narrative.getAvailableEndings();
```

## 📊 Key Enums & Constants

### Dialogue Node Types
- `TEXT` - Regular dialogue
- `CHOICE` - Player choice
- `CONDITION` - Conditional branch
- `ACTION` - Trigger action
- `BRANCH` - Narrative branch
- `END` - End dialogue

### Relationship Levels
- `HATED` (-100 to -80)
- `HOSTILE` (-79 to -60)
- `UNFRIENDLY` (-59 to -40)
- `NEUTRAL` (-39 to -20)
- `ACQUAINTANCE` (-19 to 0)
- `FRIENDLY` (1 to 20)
- `LIKED` (21 to 40)
- `TRUSTED` (41 to 60)
- `CLOSE` (61 to 80)
- `DEVOTED` (81 to 100)
- `ROMANCED` (special state)

### Quest Types
- `MAIN_STORY`
- `SIDE_QUEST`
- `FACTION`
- `ROMANCE`
- `EXPLORATION`
- `COLLECTION`
- `BOUNTY`
- `REPEATABLE`
- `HIDDEN`

### Quest Status
- `LOCKED` - Not yet available
- `AVAILABLE` - Can be started
- `ACTIVE` - Currently active
- `COMPLETED` - Finished
- `FAILED` - Failed
- `ABANDONED` - Abandoned

## 🎯 Event Callbacks

### Dialogue Callbacks
```cpp
Story::DialogueCallbacks callbacks;

callbacks.onDialogueStart = [](const std::string& id) {
    // Dialogue started
};

callbacks.onChoiceMade = [](const Story::DialogueChoice& choice) {
    // Choice was made
};

callbacks.onDialogueEnd = [](const std::string& id) {
    // Dialogue ended
};

dialogue.setCallbacks(callbacks);
```

### Relationship Callbacks
```cpp
relationships.onRelationshipChanged([](const auto& event) {
    // Relationship changed
    // event.npcId, event.oldLevel, event.newLevel
});
```

### Quest Callbacks
```cpp
journal.onQuestStatusChanged([](const std::string& id, auto oldStatus, auto newStatus) {
    // Quest status changed
});

journal.onObjectiveCompleted([](const std::string& questId, const std::string& objectiveId) {
    // Objective completed
});
```

## 🌍 Localization

### Set Language
```cpp
dialogue.setLanguage("es_ES"); // Spanish
dialogue.setLanguage("ja_JP"); // Japanese
dialogue.setLanguage("en_US"); // English (default)
```

### Get Current Language
```cpp
std::string lang = dialogue.getCurrentLanguage();
```

## 🎬 Cutscenes

### Build Cutscene
```cpp
Story::CutsceneBuilder builder("cutscene_id");

builder
    .addActor("hero", "hero_model", 0, 0, 0)
    .fadeIn(1.0f)
    .dialogue("hero", "Let's do this!", 3.0f)
    .moveCamera(5, 5, 10, 2.0f)
    .fadeOut(1.0f);

auto cutscene = builder.build();
```

### Play Cutscene
```cpp
Story::CutscenePlayer player;
player.initialize();
player.registerCutscene(cutscene);
player.play("cutscene_id");

// In game loop
player.update(deltaTime);
```

## 📝 JSON Structure Quick Reference

### Dialogue Node
```json
{
  "id": "node_id",
  "type": "TEXT",
  "speakerId": "npc_id",
  "text": "Dialogue text",
  "nextNodeId": "next_node",
  "choices": [...]
}
```

### Dialogue Choice
```json
{
  "id": "choice_id",
  "text": "Choice text",
  "nextNodeId": "next_node",
  "moralityShift": 10,
  "modifyRelationships": [["npc_id", 15]],
  "startQuests": ["quest_id"]
}
```

### NPC
```json
{
  "id": "npc_id",
  "name": "NPC Name",
  "personality_traits": ["HONEST", "BRAVE"],
  "likes": {"item_id": 20},
  "romanceable": true,
  "defaultDialogue": "dialogue_id"
}
```

### Quest
```json
{
  "id": "quest_id",
  "title": "Quest Title",
  "type": "MAIN_STORY",
  "stages": [{
    "stage": 0,
    "objectives": [{
      "id": "obj_id",
      "type": "COLLECT",
      "requiredProgress": 5
    }]
  }]
}
```

## 🔍 Debug Functions

```cpp
// Enable debug mode
dialogue.setDebugMode(true);
narrative.setDebugMode(true);
journal.setDebugMode(true);

// Export dialogue graph
dialogue.exportDialogueGraph("dialogue_id", "output.dot");

// Dump state
narrative.dumpState("narrative_state.txt");
journal.dumpQuestState("quest_state.txt");
relationships.dumpRelationships("relationships.txt");

// Unlock all for testing
narrative.unlockAllChapters();
journal.unlockAllQuests();
```

## 📚 Documentation Locations

- **Full API Docs**: `src/Story/README.md`
- **Project Overview**: `STORY_SYSTEM.md`
- **Implementation Details**: `IMPLEMENTATION_SUMMARY.md`
- **Code Examples**: `examples/demo.cpp`

## 🎨 Story Content Overview

### Main Characters
- **Cornelius** - Sarcastic ghost guide
- **Margaret** - Innkeeper (romanceable)
- **Theodore** - Incompetent wizard
- **Grimgar** - Poetic blacksmith (romanceable)
- **Lord Malevolus** - Theatrical villain

### Story Structure
- **Prologue**: Dungeon escape
- **Chapter 1**: Village arrival
- **Chapter 2**: Crystal choice (major branch)
- **Chapter 3**: Festival chaos
- **Chapter 4**: Conspiracy revealed
- **Chapter 5**: Fortress assault
- **Chapter 6**: Final confrontation
- **Epilogue**: Variable based on choices

### Endings
1. Hero's Triumph
2. Villain's Victory
3. Redemption Arc
4. Independent Path
5. Union Ending
6. Friendship Ending
7. Musical Ending (secret)

## ⚡ Performance Tips

- Use dialogue IDs for lookups (O(1))
- Cache getCurrentNode() if calling multiple times
- Batch relationship updates when possible
- Use quest tracking to avoid iterating all quests
- Enable lazy loading for large dialogue sets

## 🐛 Common Issues

### Dialogue won't start
```cpp
// Check if dialogue exists
if (!dialogue.loadDialogue("path/to/dialogue.json")) {
    // Handle error
}

// Check if node exists
if (!dialogue.startDialogue("dialogue_id", "start_node")) {
    // Handle error
}
```

### Choice unavailable
```cpp
// Check requirements
auto choices = dialogue.getAvailableChoices();
// Only shows choices that meet requirements
```

### Quest won't unlock
```cpp
// Check requirements
auto requirements = journal.getQuestRequirements("quest_id");
// Verify all requirements are met
```

## 🎯 Best Practices

1. **Always check return values** - Most functions return bool for success
2. **Use callbacks** - Don't poll, use event callbacks
3. **Cache frequently accessed data** - Like current node, active quests
4. **Validate JSON data** - Check dialogue trees for errors
5. **Test all branches** - Verify all dialogue paths work
6. **Localize early** - Add localization from the start
7. **Track consequences** - Use consequence tags for complex branching

## 🚀 Next Steps

1. Read `src/Story/README.md` for full API
2. Run `./bin/StorySystemDemo` to see it in action
3. Explore JSON files in `data/` for content examples
4. Integrate into your game
5. Start writing your own story!

---

**Need Help?**
- Check API docs: `src/Story/README.md`
- See examples: `examples/demo.cpp`
- Read overview: `STORY_SYSTEM.md`

**"May your code compile and your stories captivate!"**
