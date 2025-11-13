# Architecture Overview

This document provides a comprehensive overview of the SCUMM Style ARPG engine architecture, system designs, and implementation details.

## Table of Contents

- [High-Level Architecture](#high-level-architecture)
- [Core Engine Systems](#core-engine-systems)
- [Game Systems](#game-systems)
- [Rendering Pipeline](#rendering-pipeline)
- [Audio Architecture](#audio-architecture)
- [Data Flow](#data-flow)
- [Performance Considerations](#performance-considerations)
- [Design Patterns](#design-patterns)

## High-Level Architecture

### System Layers

```
┌─────────────────────────────────────────┐
│         Game Content Layer              │
│  (Scenes, Game Logic, Custom Scripts)   │
├─────────────────────────────────────────┤
│         Game Systems Layer              │
│  (RPG, Combat, Quest, Inventory, AI)    │
├─────────────────────────────────────────┤
│         Engine Systems Layer            │
│  (ECS, Scene Manager, Event System)     │
├─────────────────────────────────────────┤
│      Subsystems Layer                   │
│  (Rendering, Audio, Physics, Input)     │
├─────────────────────────────────────────┤
│      Platform Abstraction Layer         │
│         (SDL2, OpenGL, OS)              │
└─────────────────────────────────────────┘
```

### Module Dependencies

```
GameEngine (Core)
├── Time Management
├── Event System
├── Scene Manager
│   └── Entity Component System (ECS)
├── Rendering System
│   ├── Camera
│   ├── Sprite Batch
│   ├── Animation
│   ├── Particles
│   └── Post-Processing
├── Audio System
│   ├── Music Player
│   ├── Sound Effects
│   └── 3D Audio
├── Game Systems
│   ├── RPG Systems
│   ├── Combat System
│   ├── Quest System
│   ├── Inventory System
│   ├── Crafting System
│   └── Story System
└── Utilities
    ├── Asset Manager
    ├── Save System
    └── Profiler
```

## Core Engine Systems

### Game Engine Loop

The game engine uses a fixed timestep loop for deterministic physics and variable frame rate rendering.

```cpp
// Simplified game loop
while (m_Running) {
    // 1. Update time
    Time::Update();

    // 2. Process input events
    ProcessEvents();

    // 3. Fixed timestep updates (physics, game logic)
    while (Time::ShouldDoFixedUpdate()) {
        FixedUpdate(Time::GetFixedDeltaTime());
        Time::ConsumeFixedTimestep();
    }

    // 4. Variable update (rendering, animations)
    Update(Time::GetDeltaTime());

    // 5. Render frame
    Render();

    // 6. Present to screen
    Present();
}
```

**Benefits:**
- Deterministic simulation
- Consistent physics regardless of frame rate
- Smooth rendering with interpolation
- Network synchronization support

### Entity Component System (ECS)

#### Design

```
Entity (Container)
├── Component A (Data + Behavior)
├── Component B (Data + Behavior)
└── Component C (Data + Behavior)
```

**Key Concepts:**
- **Entity**: Unique ID + component container
- **Component**: Data and behavior attached to entities
- **System**: Processes entities with specific component combinations

#### Implementation

```cpp
// Entity owns components via shared pointers
class Entity {
    std::unordered_map<std::type_index, std::shared_ptr<IComponent>> m_Components;

    template<typename T>
    std::shared_ptr<T> AddComponent(Args&&... args) {
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        component->SetEntity(this);
        m_Components[typeid(T)] = component;
        component->OnAttach();
        return component;
    }

    template<typename T>
    std::shared_ptr<T> GetComponent() {
        auto it = m_Components.find(typeid(T));
        return (it != m_Components.end())
            ? std::static_pointer_cast<T>(it->second)
            : nullptr;
    }
};

// Components inherit from base
class IComponent {
    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void Update(double deltaTime) {}
    virtual void FixedUpdate(double fixedDeltaTime) {}
};

// Concrete component
class TransformComponent : public ComponentBase<TransformComponent> {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale = glm::vec3(1.0f);
};
```

### Scene Management

#### Scene Hierarchy

```
SceneManager
├── Scene: MainMenu
│   ├── Entity: Background
│   ├── Entity: StartButton
│   └── Entity: OptionsButton
├── Scene: GameWorld
│   ├── Entity: Player
│   ├── Entity: Enemy_1
│   ├── Entity: Enemy_2
│   └── Entity: Chest
└── Scene: PauseMenu
    ├── Entity: PauseBackground
    └── Entity: ResumeButton
```

**Lifecycle:**
1. **Creation**: `SceneManager::CreateScene<T>()`
2. **Activation**: `OnEnter()` called
3. **Update**: `Update()` and `Render()` every frame
4. **Deactivation**: `OnExit()` called
5. **Destruction**: Scene removed from manager

### Event System

#### Publisher-Subscriber Pattern

```
EventSystem (Mediator)
│
├── EventType: KeyPressed
│   ├── Subscriber: PlayerController
│   └── Subscriber: UISystem
│
├── EventType: EntityDeath
│   ├── Subscriber: QuestSystem
│   ├── Subscriber: AudioSystem
│   └── Subscriber: ParticleSystem
│
└── EventType: LevelUp
    ├── Subscriber: UISystem
    └── Subscriber: AchievementSystem
```

**Features:**
- Type-safe event dispatch
- Automatic listener management
- Immediate event processing
- Support for custom event types

## Game Systems

### RPG Systems Architecture

```
Character System
├── Stats (Health, Mana, Stamina)
├── Attributes (Strength, Dexterity, etc.)
├── Leveling (XP, Level progression)
├── Class System (Warrior, Mage, etc.)
├── Skill Tree (Talents and abilities)
└── Achievements (Unlockables)
```

**Data Flow:**
```
Player Action → XP Gained → Level Up Check → Stat Increase → UI Update
```

### Combat System Architecture

```
Combat System
├── Damage Calculation
│   ├── Base Damage
│   ├── Modifiers (Stats, Buffs)
│   └── Resistances
├── Weapon System
│   ├── Weapon Types
│   ├── Attack Patterns
│   └── Special Effects
├── Skill System
│   ├── Active Skills
│   ├── Passive Skills
│   └── Cooldown Management
├── Status Effects
│   ├── Buffs
│   ├── Debuffs
│   └── DoT/HoT
└── Combo System
    ├── Chain Detection
    └── Bonus Calculation
```

**Combat Flow:**
```
Input → Skill Activation → Target Selection → Damage Calculation →
Apply Effects → Update Stats → Trigger Events → UI Feedback
```

### Quest System Architecture

```
Quest System
├── Quest Manager
│   ├── Active Quests
│   ├── Completed Quests
│   └── Available Quests
├── Quest Structure
│   ├── Objectives
│   │   ├── Kill Objectives
│   │   ├── Collect Objectives
│   │   ├── Talk Objectives
│   │   └── Reach Objectives
│   ├── Rewards
│   │   ├── XP
│   │   ├── Items
│   │   ├── Gold
│   │   └── Reputation
│   └── Prerequisites
│       ├── Level Requirements
│       ├── Quest Dependencies
│       └── Item Requirements
└── Quest Generator
    └── Procedural Quest Creation
```

### Inventory System Architecture

```
Inventory System
├── Inventory Manager
│   ├── Storage (Grid/List)
│   ├── Capacity Management
│   └── Item Organization
├── Item System
│   ├── Item Types
│   │   ├── Consumables
│   │   ├── Equipment
│   │   ├── Materials
│   │   └── Quest Items
│   ├── Item Properties
│   │   ├── Stats
│   │   ├── Rarity
│   │   └── Stackability
│   └── Item Database
├── Equipment System
│   ├── Equipment Slots
│   ├── Stat Bonuses
│   └── Set Bonuses
└── Crafting System
    ├── Recipes
    ├── Materials
    └── Crafting Stations
```

## Rendering Pipeline

### Rendering Architecture

```
Frame Rendering
│
├── 1. Begin Frame
│   └── Clear buffers
│
├── 2. Render to Framebuffer (if post-processing enabled)
│   ├── Background Layer
│   ├── Game Layer
│   ├── Foreground Layer
│   └── UI Layer
│
├── 3. Apply Post-Processing
│   ├── Bloom
│   ├── Color Grading
│   └── Custom Effects
│
├── 4. Render to Screen
│   └── Draw final framebuffer
│
└── 5. Present
    └── Swap buffers
```

### Sprite Batch System

**Purpose**: Minimize draw calls by batching sprites with the same texture.

```
Sprite Batch Process:
1. Collect sprites for current frame
2. Sort by texture and depth
3. Group sprites with same texture
4. Create vertex buffer for each group
5. Submit single draw call per group
```

**Performance Benefit:**
- Without batching: 1000 sprites = 1000 draw calls
- With batching: 1000 sprites = ~10 draw calls (assuming 100 different textures)

### Camera System

```
Camera Types:
├── Orthographic (2D)
│   ├── Fixed Position
│   ├── Follow Target
│   └── Pan & Zoom
└── Perspective (3D)
    ├── First Person
    ├── Third Person
    └── Isometric
```

**View Matrix Calculation:**
```cpp
View Matrix = Translation × Rotation × Zoom
```

## Audio Architecture

### Audio System Components

```
Audio System
├── Mixer
│   ├── Master Channel
│   ├── Music Channel
│   ├── SFX Channel
│   ├── Ambient Channel
│   └── Voice Channel
│
├── Music Player
│   ├── Playlist Management
│   ├── Crossfading
│   └── Adaptive Music (state-based)
│
├── Sound Effects
│   ├── 3D Positional Audio
│   ├── Priority System
│   └── Effect Pool
│
└── Audio Processing
    ├── Volume Control (per-category)
    ├── Ducking (automatic volume reduction)
    ├── Reverb
    └── Occlusion
```

### 3D Audio Processing

```
3D Sound Calculation:
1. Calculate distance from listener
2. Apply attenuation based on distance
3. Calculate stereo panning based on angle
4. Apply occlusion if enabled
5. Mix into appropriate channel
```

**Formula:**
```
Volume = BaseVolume × Attenuation × CategoryVolume × MasterVolume
Attenuation = 1.0 / (1.0 + distance / maxDistance)
Pan = sin(angle) // -1.0 (left) to 1.0 (right)
```

## Data Flow

### Game Loop Data Flow

```
Input Events → Event System → Components → Game Logic →
State Changes → Rendering System → Screen Output
     ↓
Audio System → Sound Output
```

### Save/Load Flow

```
Save:
Game State → Serialize → JSON/Binary → File System → Cloud (optional)

Load:
File System → Deserialize → Restore Game State → Resume
```

### Asset Loading Flow

```
Asset Request → Asset Manager → Check Cache →
  ├── Cached: Return asset
  └── Not Cached:
      ├── Load from disk
      ├── Process/Parse
      ├── Store in cache
      └── Return asset
```

## Performance Considerations

### Memory Management

**Smart Pointer Usage:**
- `std::shared_ptr` for entities and components (shared ownership)
- `std::unique_ptr` for single-owner resources
- `std::weak_ptr` for avoiding circular references

**Object Pooling:**
```cpp
// Particle pool to avoid allocations
ParticlePool {
    std::vector<Particle> pool;
    std::vector<Particle*> available;

    Particle* Acquire() {
        if (available.empty()) {
            pool.emplace_back();
            return &pool.back();
        }
        auto* particle = available.back();
        available.pop_back();
        return particle;
    }

    void Release(Particle* p) {
        available.push_back(p);
    }
}
```

### CPU Optimization

**Hot Path Optimization:**
1. Minimize allocations in Update/Render
2. Use cache-friendly data structures
3. Batch similar operations
4. Profile before optimizing

**Multithreading:**
- Main thread: Game logic, rendering
- Audio thread: Audio processing (SDL2_mixer)
- Async loading: Asset streaming (future)

### GPU Optimization

**Rendering Optimizations:**
1. Sprite batching (reduce draw calls)
2. Texture atlasing (minimize texture switches)
3. Frustum culling (don't render off-screen)
4. Level of detail (LOD) for distant objects
5. Occlusion culling (future enhancement)

## Design Patterns

### Patterns Used

1. **Singleton**
   - GameEngine, SceneManager, EventSystem
   - Ensures single instance, global access

2. **Observer (Pub-Sub)**
   - Event System
   - Decoupled communication between systems

3. **Component**
   - Entity-Component-System
   - Flexible composition over inheritance

4. **Factory**
   - Entity/Component creation
   - Scene creation

5. **Object Pool**
   - Particle systems
   - Bullet/projectile systems

6. **State Machine**
   - AI behaviors
   - Character states
   - Game states

7. **Strategy**
   - Different combat behaviors
   - AI decision making

8. **Command**
   - Input handling
   - Undo/redo systems

### Anti-Patterns to Avoid

1. **God Object** - Don't put everything in GameEngine
2. **Spaghetti Code** - Use events instead of direct coupling
3. **Premature Optimization** - Profile first
4. **Magic Numbers** - Use named constants
5. **Global State** - Minimize global variables

## System Interactions

### Example: Player Takes Damage

```
1. Collision System detects hit
   └── Publishes CollisionEvent

2. Combat System receives event
   ├── Calculates damage
   ├── Applies resistances
   └── Publishes DamageEvent

3. Health Component receives DamageEvent
   ├── Reduces health
   ├── Checks for death
   └── Publishes HealthChangedEvent (or DeathEvent)

4. Multiple systems respond:
   ├── UI System: Updates health bar
   ├── Audio System: Plays hurt sound
   ├── Animation System: Triggers hit animation
   ├── Particle System: Shows blood effect
   └── Quest System: Checks objectives

5. If death occurred:
   ├── Respawn System: Handles respawn
   ├── Achievement System: Checks death-related achievements
   └── Save System: Updates death counter
```

## Future Enhancements

### Planned Architectural Improvements

1. **Job System**
   - Parallel task execution
   - Better CPU utilization

2. **Scripting Integration**
   - Lua/Python bindings
   - Hot-reload support

3. **Asset Streaming**
   - Background loading
   - Seamless world transitions

4. **Advanced Physics**
   - Rigid body dynamics
   - Soft body simulation

5. **Networking Layer**
   - Client-server architecture
   - State synchronization
   - Lag compensation

6. **Editor Tools**
   - Level editor
   - Visual scripting
   - Real-time editing

## References

### Influential Architectures

- **Unity**: Component-based architecture
- **Unreal Engine**: Actor-component model
- **Godot**: Node-based scene system
- **Entity Component System**: Data-oriented design

### Recommended Reading

- "Game Programming Patterns" by Robert Nystrom
- "Game Engine Architecture" by Jason Gregory
- "Real-Time Rendering" by Akenine-Möller et al.
- "Game Physics Engine Development" by Ian Millington

---

**For implementation details, see:**
- [DEVELOPER.md](DEVELOPER.md) - Development guide
- [BUILDING.md](BUILDING.md) - Build instructions
- [Source Code](../src/) - Implementation
