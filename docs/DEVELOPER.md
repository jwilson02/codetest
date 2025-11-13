# Developer Guide

Welcome to the SCUMM Style ARPG development guide! This document will help you get started with developing features, extending the engine, and contributing to the project.

## Table of Contents

- [Development Environment Setup](#development-environment-setup)
- [Project Structure](#project-structure)
- [Core Concepts](#core-concepts)
- [Creating Game Content](#creating-game-content)
- [Extending the Engine](#extending-the-engine)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Debugging](#debugging)
- [Best Practices](#best-practices)
- [Common Patterns](#common-patterns)

## Development Environment Setup

### IDE Recommendations

#### Visual Studio Code (Cross-platform)

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
   - CMake (twxs)
   - C++ TestMate
   - GitLens

2. Configure `.vscode/settings.json`:
```json
{
    "cmake.configureOnOpen": true,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "files.associations": {
        "*.h": "cpp",
        "*.cpp": "cpp"
    }
}
```

#### CLion (JetBrains)

- Open CMakeLists.txt as project
- Configure CMake profiles for Debug/Release
- Use built-in debugging and profiling tools

#### Visual Studio (Windows)

- Use CMake integration or generate .sln files
- Install C++ desktop development workload
- Configure CMake settings via CMakeSettings.json

### Tools and Utilities

**Essential Tools:**
- Git for version control
- CMake 3.15+ for building
- Debugger (GDB, LLDB, or MSVC debugger)
- Valgrind (Linux) for memory debugging

**Recommended Tools:**
- Clang-format for code formatting
- Clang-tidy for static analysis
- Doxygen for documentation generation
- Perf/gprof for profiling

## Project Structure

### Source Organization

```
src/
├── Engine/          # Core engine systems
│   ├── GameEngine   # Main engine loop
│   ├── Time         # Time management
│   ├── EventSystem  # Event handling
│   ├── SceneManager # Scene management
│   └── ECS/         # Entity-Component-System
│
├── Rendering/       # Graphics rendering
├── Audio/           # Sound and music
├── UI/              # User interface
├── AI/              # Artificial intelligence
├── Combat/          # Combat mechanics
├── RPG/             # RPG systems (stats, leveling)
├── Quest/           # Quest system
├── Inventory/       # Items and inventory
├── Story/           # Narrative systems
├── World/           # World and environment
├── Crafting/        # Crafting system
├── Save/            # Save/load functionality
├── Environment/     # Weather, day/night cycle
├── Network/         # Networking (multiplayer)
├── Assets/          # Asset management
├── Performance/     # Profiling tools
└── Launcher/        # Game launcher
```

### File Naming Conventions

- **Header files**: `ClassName.h`
- **Implementation files**: `ClassName.cpp`
- **Test files**: `ClassName_test.cpp`
- **Example files**: `ClassNameExample.cpp`

## Core Concepts

### Entity-Component-System (ECS)

The engine uses ECS for game object composition.

#### Creating Components

```cpp
// Define a component
class HealthComponent : public ComponentBase<HealthComponent> {
public:
    HealthComponent(int maxHealth)
        : m_MaxHealth(maxHealth)
        , m_CurrentHealth(maxHealth) {}

    // Called when attached to an entity
    void OnAttach() override {
        std::cout << "HealthComponent attached" << std::endl;
    }

    // Update every frame
    void Update(double deltaTime) override {
        // Regenerate health over time
        if (m_CurrentHealth < m_MaxHealth) {
            m_CurrentHealth += m_RegenRate * deltaTime;
            m_CurrentHealth = std::min(m_CurrentHealth, m_MaxHealth);
        }
    }

    // Fixed update for physics/game logic
    void FixedUpdate(double fixedDeltaTime) override {
        // Consistent updates independent of frame rate
    }

    void TakeDamage(int damage) {
        m_CurrentHealth = std::max(0, m_CurrentHealth - damage);
        if (m_CurrentHealth == 0) {
            OnDeath();
        }
    }

private:
    void OnDeath() {
        auto& eventSystem = EventSystem::GetInstance();
        eventSystem.Publish(EntityDeathEvent(GetEntity()->GetID()));
    }

    int m_MaxHealth;
    int m_CurrentHealth;
    float m_RegenRate = 5.0f; // HP per second
};
```

#### Creating Entities

```cpp
// In a scene
void GameScene::OnEnter() {
    // Create player entity
    auto player = CreateEntity("Player");

    // Add components
    auto transform = player->AddComponent<TransformComponent>();
    transform->position = glm::vec3(0.0f, 0.0f, 0.0f);

    auto health = player->AddComponent<HealthComponent>(100);
    auto sprite = player->AddComponent<SpriteComponent>();
    sprite->SetTexture("player.png");

    auto controller = player->AddComponent<PlayerControllerComponent>();

    // Store reference for later use
    m_Player = player;
}
```

### Event System

Use events for decoupled communication between systems.

#### Defining Events

```cpp
// Define a custom event
class PlayerLevelUpEvent : public Event {
public:
    PlayerLevelUpEvent(int newLevel, int skillPoints)
        : m_NewLevel(newLevel)
        , m_SkillPoints(skillPoints) {}

    std::string GetTypeName() const override {
        return "PlayerLevelUpEvent";
    }

    int GetNewLevel() const { return m_NewLevel; }
    int GetSkillPoints() const { return m_SkillPoints; }

private:
    int m_NewLevel;
    int m_SkillPoints;
};
```

#### Publishing Events

```cpp
void LevelingSystem::AddExperience(int xp) {
    m_CurrentXP += xp;

    if (m_CurrentXP >= m_XPToNextLevel) {
        m_Level++;
        m_CurrentXP -= m_XPToNextLevel;

        // Publish event
        auto& eventSystem = EventSystem::GetInstance();
        eventSystem.Publish(PlayerLevelUpEvent(m_Level, 5));
    }
}
```

#### Subscribing to Events

```cpp
class UISystem {
public:
    void Initialize() {
        auto& eventSystem = EventSystem::GetInstance();

        // Subscribe to level up events
        m_LevelUpListener = eventSystem.Subscribe<PlayerLevelUpEvent>(
            [this](const PlayerLevelUpEvent& event) {
                ShowLevelUpNotification(event.GetNewLevel());
            }
        );
    }

    ~UISystem() {
        auto& eventSystem = EventSystem::GetInstance();
        eventSystem.Unsubscribe<PlayerLevelUpEvent>(m_LevelUpListener);
    }

private:
    void ShowLevelUpNotification(int level) {
        // Display UI notification
    }

    EventListenerID m_LevelUpListener;
};
```

### Scene Management

Organize game content into scenes.

#### Creating a Scene

```cpp
class MainMenuScene : public Scene {
public:
    MainMenuScene() : Scene("MainMenu") {}

    void OnEnter() override {
        std::cout << "Entering Main Menu" << std::endl;

        // Load menu assets
        LoadMenuUI();

        // Subscribe to menu events
        SubscribeToEvents();
    }

    void OnExit() override {
        std::cout << "Exiting Main Menu" << std::endl;

        // Cleanup
        UnsubscribeFromEvents();
    }

    void Update(double deltaTime) override {
        Scene::Update(deltaTime); // Update entities

        // Update menu logic
        UpdateMenuAnimations(deltaTime);
    }

    void Render() override {
        // Render menu
        RenderMenuBackground();
        RenderMenuButtons();
    }

private:
    void LoadMenuUI() {
        // Create menu buttons, background, etc.
    }

    void SubscribeToEvents() {
        // Subscribe to input events
    }

    void UnsubscribeFromEvents() {
        // Unsubscribe from events
    }
};
```

#### Switching Scenes

```cpp
void MainMenuScene::OnPlayButtonClicked() {
    auto& sceneManager = SceneManager::GetInstance();
    sceneManager.SwitchScene("GameScene");
}
```

## Creating Game Content

### Adding New Gameplay Features

1. **Design the feature**
   - Define requirements
   - Plan component interactions
   - Consider event flow

2. **Create components**
   - Inherit from ComponentBase
   - Implement lifecycle methods
   - Add data and behavior

3. **Integrate with systems**
   - Use event system for communication
   - Update relevant managers
   - Add UI elements if needed

4. **Test thoroughly**
   - Unit tests for logic
   - Integration tests with other systems
   - Playtest for feel and balance

### Example: Adding a Stamina System

```cpp
// 1. Create StaminaComponent
class StaminaComponent : public ComponentBase<StaminaComponent> {
public:
    StaminaComponent(float maxStamina)
        : m_MaxStamina(maxStamina)
        , m_CurrentStamina(maxStamina) {}

    void Update(double deltaTime) override {
        // Regenerate stamina when not exhausted
        if (!m_Exhausted && m_CurrentStamina < m_MaxStamina) {
            m_CurrentStamina += m_RegenRate * deltaTime;
            m_CurrentStamina = std::min(m_CurrentStamina, m_MaxStamina);
        }

        // Check if recovered from exhaustion
        if (m_Exhausted && m_CurrentStamina >= m_MaxStamina * 0.5f) {
            m_Exhausted = false;
        }
    }

    bool ConsumeStamina(float amount) {
        if (m_CurrentStamina >= amount) {
            m_CurrentStamina -= amount;

            if (m_CurrentStamina <= 0) {
                m_Exhausted = true;
                OnExhausted();
            }

            return true;
        }
        return false;
    }

    float GetCurrentStamina() const { return m_CurrentStamina; }
    float GetMaxStamina() const { return m_MaxStamina; }
    bool IsExhausted() const { return m_Exhausted; }

private:
    void OnExhausted() {
        // Publish exhaustion event
        auto& eventSystem = EventSystem::GetInstance();
        eventSystem.Publish(StaminaExhaustedEvent(GetEntity()->GetID()));
    }

    float m_MaxStamina;
    float m_CurrentStamina;
    float m_RegenRate = 10.0f; // Per second
    bool m_Exhausted = false;
};

// 2. Integrate with movement/combat
class PlayerControllerComponent : public ComponentBase<PlayerControllerComponent> {
public:
    void Update(double deltaTime) override {
        auto stamina = GetEntity()->GetComponent<StaminaComponent>();

        if (IsSprintKeyPressed() && stamina) {
            if (stamina->ConsumeStamina(m_SprintCost * deltaTime)) {
                m_CurrentSpeed = m_SprintSpeed;
            } else {
                m_CurrentSpeed = m_WalkSpeed;
            }
        } else {
            m_CurrentSpeed = m_WalkSpeed;
        }

        // Move character
        Move(deltaTime);
    }

private:
    float m_WalkSpeed = 5.0f;
    float m_SprintSpeed = 10.0f;
    float m_SprintCost = 20.0f; // Per second
    float m_CurrentSpeed = m_WalkSpeed;
};

// 3. Add UI display
class HUDComponent : public UIComponent {
public:
    void Update(double deltaTime) override {
        if (auto player = GetPlayerEntity()) {
            if (auto stamina = player->GetComponent<StaminaComponent>()) {
                UpdateStaminaBar(stamina->GetCurrentStamina(),
                                stamina->GetMaxStamina());
            }
        }
    }

private:
    void UpdateStaminaBar(float current, float max) {
        // Update stamina bar UI
        float percentage = current / max;
        m_StaminaBar->SetFillPercentage(percentage);

        // Change color if exhausted
        if (percentage <= 0.0f) {
            m_StaminaBar->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            m_StaminaBar->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
    }
};
```

## Extending the Engine

### Adding a New System

1. **Create system class**
```cpp
// ParticleSystem.h
class ParticleSystem {
public:
    static ParticleSystem& GetInstance() {
        static ParticleSystem instance;
        return instance;
    }

    bool Initialize();
    void Update(double deltaTime);
    void Render();
    void Shutdown();

    // System-specific methods
    void EmitParticles(const glm::vec3& position, int count);
    void CreateEmitter(const std::string& name, ParticleEmitter emitter);

private:
    ParticleSystem() = default;
    ~ParticleSystem() = default;
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    std::vector<Particle> m_Particles;
    std::unordered_map<std::string, ParticleEmitter> m_Emitters;
};
```

2. **Integrate with engine**
```cpp
// In GameEngine::Initialize()
if (!ParticleSystem::GetInstance().Initialize()) {
    std::cerr << "Failed to initialize particle system" << std::endl;
    return false;
}

// In GameEngine::Update()
ParticleSystem::GetInstance().Update(deltaTime);

// In GameEngine::Render()
ParticleSystem::GetInstance().Render();

// In GameEngine::Shutdown()
ParticleSystem::GetInstance().Shutdown();
```

### Adding Resource Types

```cpp
// Define resource type
class AudioClip : public Resource {
public:
    AudioClip(const std::string& path);
    ~AudioClip();

    bool Load() override;
    void Unload() override;

    Mix_Chunk* GetChunk() const { return m_Chunk; }

private:
    Mix_Chunk* m_Chunk = nullptr;
};

// Register with AssetManager
AssetManager::GetInstance().RegisterLoader<AudioClip>(".wav");
AssetManager::GetInstance().RegisterLoader<AudioClip>(".ogg");

// Load and use
auto clip = AssetManager::GetInstance().Load<AudioClip>("sounds/jump.wav");
Mix_PlayChannel(-1, clip->GetChunk(), 0);
```

## Coding Standards

### C++ Style Guide

**Naming Conventions:**
- Classes: `PascalCase` (e.g., `GameEngine`, `PlayerController`)
- Functions: `PascalCase` (e.g., `Initialize`, `Update`)
- Variables: `camelCase` (e.g., `deltaTime`, `playerHealth`)
- Member variables: `m_camelCase` (e.g., `m_CurrentHealth`, `m_MaxSpeed`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_PLAYERS`, `DEFAULT_VOLUME`)
- Namespaces: `PascalCase` (e.g., `Engine`, `Rendering`)

**Code Formatting:**
```cpp
// Use 4 spaces for indentation
class MyClass {
public:
    MyClass()
        : m_Value(0)
        , m_Name("default") {}

    void DoSomething() {
        if (m_Value > 0) {
            ProcessValue();
        } else {
            HandleError();
        }
    }

private:
    int m_Value;
    std::string m_Name;
};
```

**Best Practices:**
- Use smart pointers (`std::shared_ptr`, `std::unique_ptr`)
- Avoid raw `new`/`delete`
- Use RAII for resource management
- Prefer `const` correctness
- Use `nullptr` instead of `NULL`
- Use `auto` for complex type names
- Use `enum class` instead of `enum`

### Header Organization

```cpp
#pragma once // Use pragma once instead of include guards

// System includes
#include <vector>
#include <memory>
#include <string>

// Third-party includes
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

// Project includes
#include "Engine/Component.h"
#include "Engine/Entity.h"

namespace MyNamespace {

// Forward declarations
class SomeOtherClass;

/**
 * @brief Brief description of the class
 *
 * Detailed description...
 */
class MyClass {
public:
    // Public interface

private:
    // Private implementation
};

} // namespace MyNamespace
```

## Testing

### Unit Testing

```cpp
// Example using a testing framework
#include <gtest/gtest.h>
#include "RPG/CharacterStats.h"

TEST(CharacterStatsTest, DefaultConstruction) {
    CharacterStats stats;
    EXPECT_EQ(stats.GetLevel(), 1);
    EXPECT_EQ(stats.GetExperience(), 0);
}

TEST(CharacterStatsTest, LevelUp) {
    CharacterStats stats;
    stats.AddExperience(1000);
    EXPECT_EQ(stats.GetLevel(), 2);
    EXPECT_GT(stats.GetHealth(), 100); // Health increases on level up
}

TEST(CharacterStatsTest, StatModifiers) {
    CharacterStats stats;
    stats.AddModifier(StatType::Strength, 10);
    EXPECT_EQ(stats.GetStat(StatType::Strength), 20); // Base 10 + modifier 10
}
```

### Integration Testing

```cpp
TEST(GameEngineTest, InitializeAndShutdown) {
    EngineConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;

    auto& engine = GameEngine::GetInstance();
    ASSERT_TRUE(engine.Initialize(config));

    engine.Shutdown();
}

TEST(SceneManagerTest, SceneSwitching) {
    auto& sceneManager = SceneManager::GetInstance();

    sceneManager.CreateScene<TestScene>("Test");
    ASSERT_TRUE(sceneManager.SwitchScene("Test"));
    EXPECT_EQ(sceneManager.GetCurrentScene()->GetName(), "Test");
}
```

## Debugging

### Debug Logging

```cpp
// Use different log levels
#ifdef DEBUG
    #define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
    #define LOG_DEBUG(msg)
#endif

#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_WARNING(msg) std::cerr << "[WARNING] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl

// Usage
LOG_DEBUG("Player position: " << player->GetPosition());
LOG_INFO("Game started");
LOG_WARNING("Low health: " << health);
LOG_ERROR("Failed to load texture: " << path);
```

### Using Debuggers

**GDB (Linux/macOS):**
```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run with GDB
gdb ./build/bin/SCUMMStyleARPG

# Common GDB commands
(gdb) break main              # Set breakpoint
(gdb) run                     # Start program
(gdb) next                    # Step over
(gdb) step                    # Step into
(gdb) print variable          # Print variable
(gdb) backtrace               # Show call stack
```

**LLDB (macOS):**
```bash
lldb ./build/bin/SCUMMStyleARPG
(lldb) breakpoint set --name main
(lldb) run
(lldb) next
(lldb) print variable
```

**Visual Studio:**
- Set breakpoints by clicking line numbers
- F5 to start debugging
- F10 to step over, F11 to step into
- Watch window for variables

### Memory Debugging

**Valgrind (Linux):**
```bash
# Check for memory leaks
valgrind --leak-check=full ./build/bin/SCUMMStyleARPG

# Check for memory errors
valgrind --tool=memcheck ./build/bin/SCUMMStyleARPG
```

**Address Sanitizer:**
```bash
# Build with ASan
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address -g"
cmake --build .

# Run (ASan will report errors automatically)
./build/bin/SCUMMStyleARPG
```

## Best Practices

### Performance Tips

1. **Minimize dynamic allocations in hot paths**
```cpp
// Bad: Allocates every frame
void Update() {
    auto tempVector = std::make_shared<std::vector<int>>();
    // ...
}

// Good: Reuse container
class MySystem {
    std::vector<int> m_TempStorage;

    void Update() {
        m_TempStorage.clear();
        // ...
    }
};
```

2. **Use const references for large objects**
```cpp
// Bad: Copies the entity
void ProcessEntity(Entity entity);

// Good: Uses reference
void ProcessEntity(const Entity& entity);
```

3. **Cache frequently accessed components**
```cpp
// Bad: Searches for component every frame
void Update(double deltaTime) {
    auto transform = GetEntity()->GetComponent<TransformComponent>();
    // ...
}

// Good: Cache component reference
void OnAttach() override {
    m_Transform = GetEntity()->GetComponent<TransformComponent>();
}

void Update(double deltaTime) override {
    // Use cached m_Transform
}
```

### Error Handling

```cpp
// Check return values
if (!AssetManager::Load("texture.png")) {
    LOG_ERROR("Failed to load texture");
    return false;
}

// Use exceptions for exceptional cases
try {
    config.LoadFromFile("config.json");
} catch (const std::exception& e) {
    LOG_ERROR("Failed to load config: " << e.what());
    // Use default config
}

// Validate input
bool CharacterStats::SetLevel(int level) {
    if (level < 1 || level > MAX_LEVEL) {
        LOG_WARNING("Invalid level: " << level);
        return false;
    }
    m_Level = level;
    return true;
}
```

## Common Patterns

### Singleton Pattern

```cpp
class MySingleton {
public:
    static MySingleton& GetInstance() {
        static MySingleton instance;
        return instance;
    }

    // Delete copy and move
    MySingleton(const MySingleton&) = delete;
    MySingleton& operator=(const MySingleton&) = delete;
    MySingleton(MySingleton&&) = delete;
    MySingleton& operator=(MySingleton&&) = delete;

private:
    MySingleton() = default;
    ~MySingleton() = default;
};
```

### Factory Pattern

```cpp
class ItemFactory {
public:
    static std::unique_ptr<Item> CreateItem(ItemType type) {
        switch (type) {
            case ItemType::Weapon:
                return std::make_unique<Weapon>();
            case ItemType::Armor:
                return std::make_unique<Armor>();
            case ItemType::Potion:
                return std::make_unique<Potion>();
            default:
                return nullptr;
        }
    }
};
```

### Object Pool

```cpp
template<typename T>
class ObjectPool {
public:
    T* Acquire() {
        if (m_Available.empty()) {
            return new T();
        }

        T* obj = m_Available.back();
        m_Available.pop_back();
        m_InUse.push_back(obj);
        return obj;
    }

    void Release(T* obj) {
        auto it = std::find(m_InUse.begin(), m_InUse.end(), obj);
        if (it != m_InUse.end()) {
            m_InUse.erase(it);
            m_Available.push_back(obj);
        }
    }

private:
    std::vector<T*> m_Available;
    std::vector<T*> m_InUse;
};
```

## Resources

### Further Reading

- [Game Programming Patterns](https://gameprogrammingpatterns.com/) by Robert Nystrom
- [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/) by Scott Meyers
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Gaffer on Games](https://gafferongames.com/) - Game networking and physics

### Community

- [GitHub Discussions](https://github.com/yourusername/scumm-arpg/discussions)
- [Discord Server](#) (if applicable)
- [Wiki](https://github.com/yourusername/scumm-arpg/wiki)

---

**Happy Coding!**

For more information, see:
- [ARCHITECTURE.md](ARCHITECTURE.md) - System design details
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [BUILDING.md](BUILDING.md) - Build instructions
