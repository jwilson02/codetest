# SCUMM VM-Style Dialogue & Story System

A comprehensive narrative engine for ARPG games featuring branching dialogues, relationship management, quest tracking, and multiple endings.

## Overview

This system provides everything needed to create rich, branching narratives with:

- **Dialogue System** - Rich text rendering, portraits, voice acting support (TTS-ready)
- **Branching Narratives** - Player choices that matter with real consequences
- **Relationship System** - Dynamic NPC relationships, romance options, faction reputations
- **Quest Journal** - Quest tracking, objectives, lore, bestiary
- **Cutscene Engine** - Scripted cutscenes with camera control
- **Localization** - Multi-language support (English, Spanish, Japanese included)

## Features

### Dialogue System (`DialogueSystem.h/cpp`)

- **Branching dialogue trees** with unlimited choices
- **Rich text formatting** - colors, styles, portraits
- **Dynamic text variables** - `{player_name}`, `{variables}` replacement
- **Voice acting support** - TTS integration ready
- **Multiple speaker support** - Character portraits and voices
- **Dialogue history** - Track conversation progression
- **Conditional branches** - Show/hide choices based on flags, relationships, items
- **Choice consequences** - Morality shifts, relationship changes, quest triggers

### Narrative Engine (`NarrativeEngine.h/cpp`)

- **Story progression** - Prologue → Act I → Act II → Act III → Climax → Epilogue
- **Chapter system** - Structured story with estimated playtime
- **Story flags** - Track player decisions and world state
- **Morality system** - -100 (evil) to +100 (good) alignment tracking
- **Faction reputation** - Track relationships with organizations
- **Multiple endings** - 7+ unique endings based on choices
- **Variable system** - Int, string, bool variables for dynamic content
- **Event system** - Custom event callbacks

### Relationship System (`RelationshipSystem.h/cpp`)

- **11 relationship levels** - From "Hated" to "Romanced"
- **Dynamic NPC reactions** - NPCs respond based on relationship
- **Romance system** - Multiple romanceable characters
- **Gift giving** - Give items to improve relationships
- **Faction membership** - NPC faction affiliations affect relationships
- **Personality traits** - 14 personality traits (Honest, Brave, Kind, etc.)
- **Social network** - NPCs have relationships with each other
- **Contextual dialogue** - Different dialogue based on relationship level

### Quest Journal (`Journal.h/cpp`)

- **8 quest types** - Main Story, Side Quest, Romance, Faction, etc.
- **Multi-stage quests** - Complex quest chains with stages
- **Objective tracking** - Kill, Collect, Talk, Reach, Escort, etc.
- **Quest rewards** - Experience, gold, items, unlocks
- **Lore system** - Discover world lore through gameplay
- **Bestiary** - Track discovered enemies and kill counts
- **Quest search** - Search through quests, journal entries, lore
- **Time-limited quests** - Optional time limits

### Cutscene Engine (`Cutscene.h/cpp`)

- **Timeline-based** - Commands executed at specific timestamps
- **Camera control** - Move, zoom, focus on actors
- **Actor management** - Spawn, move, animate actors
- **Branching cutscenes** - Player choices within cutscenes
- **Skippable/Pausable** - Optional skip and pause functionality
- **Cinematic bars** - Letterbox/pillarbox support
- **Music & sound** - Background music and sound effects
- **Fade effects** - Fade in/out transitions

### Dialogue Tree Analysis (`DialogueTree.h/cpp`)

- **Tree visualization** - Export to Graphviz format
- **Flow analysis** - Calculate branching factor, depth, endings
- **Consequence tracking** - Track narrative consequences
- **Replay value calculation** - Estimate replayability
- **Validation** - Detect orphaned nodes, circular references
- **Critical path finding** - Identify main story path

## Story Content

### Main Story (25+ hours)

The system includes a complete narrative:

1. **Prologue** - Awaken in dungeon with amnesia, meet Cornelius the Ghost
2. **Chapter 1** - Escape to Village of Last Resort, meet quirky NPCs
3. **Chapter 2** - Choose your power crystal (major branching point)
4. **Chapter 3** - Festival chaos, romance subplot, dragon with stage fright
5. **Chapter 4** - Conspiracy revealed, memory recovery, trust decisions
6. **Chapter 5** - Storm the fortress, optional minion revolution
7. **Chapter 6** - Final battle with multiple resolution paths
8. **Epilogue** - Ending variations based on choices

### NPCs

#### Main Characters

- **Cornelius** - Sarcastic ghost guide (200 years dead, still complaining)
- **Margaret** - Innkeeper running a "slightly on fire" establishment
- **Theodore** - Wizard whose spells have 50% success rate (rest = chickens)
- **Grimgar** - Gruff blacksmith who secretly writes poetry
- **Lord Malevolus** - Theatrical villain with excellent employee benefits

### Features Showcase

- **Humor & Personality** - Every NPC has unique voice and humor
- **Meaningful Choices** - Decisions affect story, relationships, endings
- **Romance Options** - Multiple romantic storylines
- **7 Different Endings**:
  - Hero's Triumph
  - Villain's Victory
  - Redemption Arc
  - Independent Path
  - Union Ending (organize the minions!)
  - Friendship Ending (everyone gets along)
  - Musical Ending (secret ending - turn battle into musical)

## Usage Examples

### Starting a Dialogue

```cpp
#include "DialogueSystem.h"

Story::DialogueSystem dialogueSystem;
dialogueSystem.initialize();

// Register speakers
Story::Speaker player;
player.id = "player";
player.name = "Hero";
player.portraitPath = "assets/portraits/player.png";
dialogueSystem.registerSpeaker(player);

// Load dialogue
dialogueSystem.loadDialogue("data/dialogues/main_story.json");

// Start dialogue
dialogueSystem.startDialogue("prologue_awakening");

// Get current node
const auto* node = dialogueSystem.getCurrentNode();
if (node) {
    std::cout << node->text << std::endl;
}

// Get available choices
auto choices = dialogueSystem.getAvailableChoices();
for (size_t i = 0; i < choices.size(); ++i) {
    std::cout << i + 1 << ". " << choices[i].text << std::endl;
}

// Select choice
dialogueSystem.selectChoice(0);
```

### Managing Relationships

```cpp
#include "RelationshipSystem.h"

Story::RelationshipSystem relationshipSystem;
relationshipSystem.initialize();

// Load NPCs
relationshipSystem.loadNPCs("data/dialogues/npcs.json");

// Modify relationship
relationshipSystem.modifyRelationship("innkeeper_margaret", 15, "helped_with_fire");

// Check relationship level
auto level = relationshipSystem.getRelationshipLevel("innkeeper_margaret");
std::string levelName = relationshipSystem.getRelationshipLevelName("innkeeper_margaret");

// Give gift
relationshipSystem.giveGift("innkeeper_margaret", "fireproof_cookbook");

// Start romance
if (relationshipSystem.canStartRomance("innkeeper_margaret")) {
    relationshipSystem.startRomance("innkeeper_margaret");
}
```

### Quest Management

```cpp
#include "Journal.h"

Story::Journal journal;
journal.initialize();

// Load quests
journal.loadQuests("data/story/chapters.json");

// Start quest
journal.startQuest("main_quest_escape_dungeon");

// Update objective
journal.updateObjective("main_quest_escape_dungeon", "find_key", 1);

// Track quest
journal.setTrackedQuest("main_quest_escape_dungeon");

// Get active quests
auto activeQuests = journal.getActiveQuests();
for (const auto* quest : activeQuests) {
    std::cout << quest->title << ": "
              << quest->stages[quest->currentStage].description << std::endl;
}
```

### Playing Cutscenes

```cpp
#include "Cutscene.h"

Story::CutscenePlayer cutscenePlayer;
cutscenePlayer.initialize();

// Build cutscene
Story::CutsceneBuilder builder("intro_cutscene");
builder
    .addActor("hero", "hero_model", 0, 0, 0)
    .addActor("villain", "villain_model", 10, 0, 0)
    .fadeIn(2.0f)
    .focusOn("hero", 1.0f)
    .dialogue("hero", "We meet at last!", 3.0f)
    .focusOn("villain", 1.0f)
    .dialogue("villain", "Indeed! *swirls cape*", 3.0f)
    .fadeOut(2.0f);

auto cutscene = builder.build();
cutscenePlayer.registerCutscene(cutscene);

// Play
cutscenePlayer.play("intro_cutscene");

// Update each frame
cutscenePlayer.update(deltaTime);
```

### Narrative Engine

```cpp
#include "NarrativeEngine.h"

Story::NarrativeEngine narrativeEngine;
narrativeEngine.initialize();

// Load chapters
narrativeEngine.loadChapters("data/story/chapters.json");

// Set player data
narrativeEngine.setPlayerName("The Chosen One");
narrativeEngine.setPlayerClass("Rogue");
narrativeEngine.setPlayerLevel(10);

// Set story flags
narrativeEngine.setFlag("escaped_dungeon", true);
narrativeEngine.setFlag("met_cornelius", true);

// Adjust morality
narrativeEngine.adjustMorality(15); // Good choice

// Record decision
Story::StoryDecision decision;
decision.id = "spared_enemy";
decision.description = "Spared the bandit leader";
decision.impact = Story::StoryDecision::Impact::MODERATE;
narrativeEngine.recordDecision(decision);

// Check available endings
auto endings = narrativeEngine.getAvailableEndings();
for (const auto* ending : endings) {
    std::cout << "Available: " << ending->title << std::endl;
}
```

## Building

### Prerequisites

- C++17 or later
- CMake 3.15+
- JSON library (nlohmann/json recommended)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

### Integration

Include the headers in your project:

```cpp
#include "Story/DialogueSystem.h"
#include "Story/NarrativeEngine.h"
#include "Story/RelationshipSystem.h"
#include "Story/Journal.h"
#include "Story/Cutscene.h"
```

Link against the Story library:

```cmake
target_link_libraries(YourGame Story)
```

## Data Files

### Dialogue JSON Structure

```json
{
  "dialogues": [{
    "id": "dialogue_id",
    "nodes": [{
      "id": "node_id",
      "type": "TEXT",
      "speakerId": "npc_id",
      "text": "Dialogue text here",
      "nextNodeId": "next_node",
      "choices": [...]
    }]
  }]
}
```

### NPC JSON Structure

```json
{
  "npcs": [{
    "id": "npc_id",
    "name": "NPC Name",
    "personality_traits": ["HONEST", "BRAVE"],
    "likes": { "item_id": 20 },
    "dislikes": { "item_id": -15 },
    "romanceable": true,
    "defaultDialogue": "dialogue_id"
  }]
}
```

## Localization

Supported languages:
- English (en_US) - Complete
- Spanish (es_ES) - Complete
- Japanese (ja_JP) - Complete

To add a new language:
1. Create `data/localization/{language_code}.json`
2. Copy structure from `en_US.json`
3. Translate all strings
4. Set language: `dialogueSystem.setLanguage("es_ES");`

## Architecture

### Class Hierarchy

```
DialogueSystem
├── DialogueNode (text, choices, conditions)
├── DialogueChoice (text, effects, requirements)
├── DialogueSession (current state)
└── Speaker (NPC data)

NarrativeEngine
├── Chapter (story structure)
├── StoryFlag (world state)
├── StoryDecision (player choices)
├── WorldStateChange (state modifications)
└── Ending (multiple endings)

RelationshipSystem
├── NPC (character data)
├── Relationship (player-NPC relationship)
├── Faction (group affiliations)
└── BestiaryEntry (enemy tracking)

Journal
├── Quest (quest data)
├── QuestStage (multi-stage quests)
├── QuestObjective (objectives)
├── JournalEntry (player journal)
└── LoreEntry (world lore)

CutscenePlayer
├── Cutscene (cutscene data)
├── CutsceneCommand (timeline commands)
├── CutsceneActor (characters in scene)
└── CameraCommand (camera control)
```

## Performance

- Dialogue trees support unlimited branching
- O(1) dialogue node lookup
- Lazy loading for large dialogue sets
- Efficient relationship queries
- Quest system optimized for hundreds of active quests

## Future Enhancements

- [ ] Visual dialogue editor
- [ ] Voice acting recording integration
- [ ] Procedural dialogue generation
- [ ] AI-driven NPC responses
- [ ] Advanced emotion system
- [ ] Gesture and animation sync
- [ ] Multiplayer dialogue choices (voting system)

## Credits

Created as a comprehensive SCUMM VM-style narrative system for modern ARPGs.

Inspired by:
- LucasArts SCUMM games
- BioWare dialogue systems (Mass Effect, Dragon Age)
- Obsidian Entertainment (Fallout: New Vegas, Pillars of Eternity)
- Disco Elysium's dialogue system

## License

See LICENSE file for details.

## Support

For issues, questions, or contributions, please check the documentation or file an issue.

---

**"May your choices be interesting and your endings satisfying!"** - Cornelius the Ghost
