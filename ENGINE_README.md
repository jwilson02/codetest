# SCUMM VM-Style ARPG Game Engine

A robust, production-quality C++17 game engine architecture designed for SCUMM VM-style action RPGs. Features modern AAA game engine patterns with comprehensive error handling, memory management, and extensibility.

## Architecture Overview

### Core Systems

#### 1. **Game Engine** (`src/Engine/GameEngine.h/cpp`)
- Main engine class with singleton pattern
- Fixed timestep game loop (à la Gaffer on Games)
- SDL2 window and OpenGL context management
- Comprehensive initialization and shutdown lifecycle
- Event-driven architecture integration

**Key Features:**
- Configurable engine settings (resolution, VSync, target FPS)
- Fixed timestep physics updates (separate from rendering)
- Proper frame pacing and delta time management
- Window management (fullscreen toggle, resize handling)

#### 2. **Entity Component System** (`src/Engine/ECS/`)
Modern ECS architecture for flexible game object composition.

**Entity.h/cpp:**
- Entity container with unique IDs
- Component attachment/detachment
- Type-safe component access with templates
- Automatic component lifecycle management
- Update and FixedUpdate propagation

**Component.h:**
- Base component interface
- Virtual lifecycle hooks (OnAttach, OnDetach)
- Update methods (variable and fixed timestep)
- ComponentBase template for automatic type registration
- Active state management

**Usage Example:**
```cpp
class TransformComponent : public ComponentBase<TransformComponent> {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1.0f);
};

auto entity = scene->CreateEntity("Player");
auto transform = entity->AddComponent<TransformComponent>();
transform->position = glm::vec3(0, 0, 0);
```

#### 3. **Scene Management** (`src/Engine/SceneManager.h/cpp`)
Hierarchical scene system for organizing game levels and states.

**Scene:**
- Entity container and manager
- Virtual lifecycle hooks (OnEnter, OnExit)
- Update/FixedUpdate/Render methods
- Entity creation and destruction
- Entity lookup by ID or name

**SceneManager:**
- Singleton scene management
- Scene registration and switching
- Active scene tracking
- Type-safe scene creation with templates

**Usage Example:**
```cpp
class MainMenuScene : public Scene {
public:
    MainMenuScene() : Scene("MainMenu") {}

    void OnEnter() override {
        // Load menu assets
    }

    void Render() override {
        // Render menu UI
    }
};

auto& sceneManager = SceneManager::GetInstance();
sceneManager.CreateScene<MainMenuScene>("MainMenu");
sceneManager.SwitchScene("MainMenu");
```

#### 4. **Event System** (`src/Engine/EventSystem.h/cpp`)
Type-safe, publish-subscribe event system for decoupled communication.

**Features:**
- Template-based type safety
- Observer pattern implementation
- Immediate event dispatch
- Event handler registration/unregistration
- Built-in common events (KeyPressed, WindowResize, etc.)

**Usage Example:**
```cpp
auto& eventSystem = EventSystem::GetInstance();

// Subscribe to events
auto listenerId = eventSystem.Subscribe<KeyPressedEvent>(
    [](const KeyPressedEvent& event) {
        if (event.GetKeyCode() == SDLK_SPACE) {
            // Handle spacebar press
        }
    }
);

// Publish events
eventSystem.Publish(KeyPressedEvent(SDLK_a, false));

// Unsubscribe
eventSystem.Unsubscribe<KeyPressedEvent>(listenerId);
```

**Built-in Events:**
- `KeyPressedEvent` - Keyboard key down
- `KeyReleasedEvent` - Keyboard key up
- `WindowResizeEvent` - Window size changed
- `WindowCloseEvent` - Window close requested

#### 5. **Time System** (`src/Engine/Time.h/cpp`)
High-precision timing for fixed timestep game loops.

**Features:**
- Delta time calculation
- Fixed timestep accumulator
- Frame count tracking
- FPS calculation
- Time scaling support

**Fixed Timestep Implementation:**
```cpp
while (engine.IsRunning()) {
    Time::Update();

    // Fixed updates (physics)
    while (Time::ShouldDoFixedUpdate()) {
        FixedUpdate(Time::GetFixedDeltaTime());
        Time::ConsumeFixedTimestep();
    }

    // Variable update (rendering)
    Update(Time::GetDeltaTime());
    Render();
}
```

## Build System

### CMakeLists.txt Features
- CMake 3.15+ support
- C++17 standard enforcement
- Compiler warnings as errors
- Cross-platform configuration (Windows/Linux/macOS)
- Dependency management (SDL2, OpenGL, GLM)

### Building the Project

```bash
# Create build directory
mkdir build && cd build

# Configure (Linux/macOS)
cmake ..

# Configure (Windows with Visual Studio)
cmake .. -G "Visual Studio 16 2019"

# Build
cmake --build . --config Release

# Run
./bin/SCUMMStyleARPG
```

### Dependencies
- **SDL2** - Window management, input handling
- **OpenGL 3.3+** - Graphics rendering
- **GLM** - Mathematics library for 3D graphics

## Code Quality Features

### Memory Management
- Smart pointers throughout (std::shared_ptr, std::unique_ptr)
- RAII principles for resource management
- No raw news/deletes
- Automatic cleanup in destructors

### Error Handling
- Comprehensive initialization checks
- Graceful failure handling
- Informative error messages
- Exception safety

### Modern C++17 Features
- Template metaprogramming for type safety
- Move semantics
- Lambda expressions
- Type traits for compile-time checks
- Structured bindings (where appropriate)

### Design Patterns
- **Singleton** - Engine, SceneManager, EventSystem
- **Observer** - Event system
- **Component** - ECS architecture
- **Template Method** - Scene lifecycle
- **Factory** - Entity/Component creation

## Engine Flow

### Initialization
```
1. GameEngine::Initialize()
   ├─ Initialize SDL2
   ├─ Create OpenGL context
   ├─ Initialize Time system
   └─ Subscribe to system events

2. Create scenes
   └─ SceneManager::CreateScene<T>()

3. Switch to initial scene
   └─ SceneManager::SwitchScene()
```

### Game Loop
```
GameEngine::Run()
├─ Time::Update()
├─ ProcessEvents()
│  └─ Dispatch to EventSystem
├─ Fixed timestep loop
│  ├─ FixedUpdate() - Physics
│  └─ Time::ConsumeFixedTimestep()
├─ Update() - Game logic
└─ Render() - Draw frame
```

### Shutdown
```
GameEngine::Shutdown()
├─ Unsubscribe from events
├─ Clear all scenes
├─ Destroy OpenGL context
└─ Cleanup SDL2
```

## Usage Examples

### Creating a Game

```cpp
int main() {
    // Configure engine
    EngineConfig config;
    config.windowTitle = "My ARPG Game";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.fixedTimestep = 1.0 / 60.0;

    // Initialize
    auto& engine = GameEngine::GetInstance();
    if (!engine.Initialize(config)) {
        return 1;
    }

    // Set up game
    auto& sceneManager = SceneManager::GetInstance();
    sceneManager.CreateScene<GameScene>("Game");
    sceneManager.SwitchScene("Game");

    // Run
    engine.Run();

    // Cleanup
    engine.Shutdown();
    return 0;
}
```

### Creating a Component

```cpp
class HealthComponent : public ComponentBase<HealthComponent> {
public:
    HealthComponent(int maxHealth)
        : m_MaxHealth(maxHealth), m_CurrentHealth(maxHealth) {}

    void TakeDamage(int damage) {
        m_CurrentHealth = std::max(0, m_CurrentHealth - damage);
        if (m_CurrentHealth == 0) {
            OnDeath();
        }
    }

    void Heal(int amount) {
        m_CurrentHealth = std::min(m_MaxHealth, m_CurrentHealth + amount);
    }

private:
    void OnDeath() {
        // Handle death
        GetEntity()->SetActive(false);
    }

    int m_MaxHealth;
    int m_CurrentHealth;
};
```

## Performance Considerations

### Fixed Timestep Benefits
- Deterministic physics simulation
- Consistent game logic regardless of frame rate
- Network synchronization support
- Replay system capability

### Memory Optimization
- Entity pooling (can be added)
- Component memory locality
- Smart pointer overhead minimized with shared_ptr
- String interning for entity/scene names (future optimization)

## Extension Points

The architecture is designed for easy extension:

### Add New Components
Inherit from `ComponentBase<T>` and implement Update/FixedUpdate

### Add New Events
Inherit from `Event` and implement GetTypeName()

### Add New Scenes
Inherit from `Scene` and override lifecycle methods

### Add New Systems
Follow singleton pattern or create system managers

## File Structure

```
/home/user/codetest/
├── CMakeLists.txt
└── src/
    ├── main.cpp
    └── Engine/
        ├── GameEngine.h/cpp       - Main engine class
        ├── Time.h/cpp             - Time management
        ├── EventSystem.h/cpp      - Event handling
        ├── SceneManager.h/cpp     - Scene management
        └── ECS/
            ├── Component.h        - Component base class
            └── Entity.h/cpp       - Entity management
```

## Testing

### Manual Testing
The provided `main.cpp` includes:
- Example components (Transform, PlayerController)
- Example scene (GameScene)
- Keyboard input handling
- FPS display
- Entity creation demonstration

### Controls (Demo)
- **WASD / Arrow Keys** - Move player
- **SPACE** - Action
- **ESC** - Quit

## Future Enhancements

Potential additions to the engine:
- [ ] Resource Manager for assets (textures, sounds, models)
- [ ] Rendering System (sprite batching, camera, layers)
- [ ] Physics System (collision detection, rigidbodies)
- [ ] Audio System (music, sound effects, 3D audio)
- [ ] Input Manager (action mapping, gamepad support)
- [ ] Serialization System (save/load, scene persistence)
- [ ] Scripting Integration (Lua, Python bindings)
- [ ] Networking (multiplayer support)
- [ ] Profiler (performance metrics)
- [ ] Asset Hot-Reloading

## License

This is a demonstration/template engine architecture. Use freely for your projects.

## Credits

Architecture inspired by:
- Unity Engine's component model
- Unreal Engine's actor system
- Gaffer on Games fixed timestep article
- Game Programming Patterns by Robert Nystrom
