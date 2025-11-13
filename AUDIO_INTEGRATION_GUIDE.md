# Audio System Integration Guide

This guide will help you integrate the professional audio system into your game project.

## Table of Contents

1. [Installation](#installation)
2. [Project Structure](#project-structure)
3. [Building the Audio System](#building-the-audio-system)
4. [Integration Steps](#integration-steps)
5. [Basic Usage](#basic-usage)
6. [Advanced Features](#advanced-features)
7. [Performance Tips](#performance-tips)
8. [Troubleshooting](#troubleshooting)

---

## Installation

### Step 1: Install Dependencies

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libsdl2-dev libsdl2-mixer-dev
```

#### macOS
```bash
brew install cmake
brew install sdl2 sdl2_mixer
```

#### Windows
Using vcpkg:
```powershell
vcpkg install sdl2:x64-windows
vcpkg install sdl2-mixer:x64-windows
```

### Step 2: Verify Installation

```bash
# Linux/macOS
pkg-config --modversion sdl2
pkg-config --modversion SDL2_mixer

# Should output version numbers like 2.0.14
```

---

## Project Structure

After integration, your project should look like this:

```
your-game-project/
├── src/
│   ├── Audio/                  # Audio system (this)
│   │   ├── AudioSystem.h/cpp
│   │   ├── MusicPlayer.h/cpp
│   │   ├── SoundEffect.h/cpp
│   │   ├── AudioSource.h/cpp
│   │   ├── CMakeLists.txt
│   │   └── README.md
│   ├── Game/                   # Your game code
│   │   ├── main.cpp
│   │   └── ...
│   └── ...
├── data/
│   └── audio/
│       ├── music_tracks.json
│       └── sound_mapping.json
├── assets/
│   └── audio/
│       ├── music/
│       ├── sfx/
│       ├── voice/
│       ├── ambient/
│       └── README.md           # Asset creation guide
├── CMakeLists.txt              # Root CMakeLists
└── README.md
```

---

## Building the Audio System

### Option 1: As a Static Library

Add to your root `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(YourGame)

set(CMAKE_CXX_STANDARD 17)

# Add audio system subdirectory
add_subdirectory(src/Audio)

# Your game executable
add_executable(YourGame
    src/Game/main.cpp
    src/Game/Player.cpp
    # ... other source files
)

# Link audio system
target_link_libraries(YourGame PRIVATE AudioSystem)

# Include directories
target_include_directories(YourGame PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
```

Build:
```bash
mkdir build
cd build
cmake ..
make
./YourGame
```

### Option 2: Direct Integration

Copy the audio system files directly into your project and add them to your existing build system:

```cmake
# Add audio system sources to your executable
add_executable(YourGame
    src/Game/main.cpp
    src/Audio/AudioSystem.cpp
    src/Audio/MusicPlayer.cpp
    src/Audio/SoundEffect.cpp
    src/Audio/AudioSource.cpp
    # ... other sources
)

# Find and link SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

target_link_libraries(YourGame
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

target_include_directories(YourGame PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)
```

---

## Integration Steps

### Step 1: Initialize Audio System

In your game initialization code (typically `main.cpp` or `Game::init()`):

```cpp
#include "Audio/AudioSystem.h"

using namespace Audio;

bool Game::init() {
    // ... other initialization

    // Get audio system instance
    AudioSystem& audio = AudioSystem::getInstance();

    // Configure audio
    AudioConfig config;
    config.frequency = 44100;
    config.channels = 2;
    config.maxChannels = 32;
    config.masterVolume = 0.8f;
    config.musicVolume = 0.7f;
    config.sfxVolume = 0.8f;

    // Initialize
    if (!audio.initialize(config)) {
        std::cerr << "Failed to initialize audio system!" << std::endl;
        return false;
    }

    // Load audio configuration
    audio.loadSoundMapping("data/audio/sound_mapping.json");
    audio.loadMusicPlaylist("data/audio/music_tracks.json");

    std::cout << "Audio system initialized successfully!" << std::endl;
    return true;
}
```

### Step 2: Update Audio System

In your main game loop:

```cpp
void Game::update(float deltaTime) {
    // Update audio system
    AudioSystem& audio = AudioSystem::getInstance();
    audio.update(deltaTime);

    // Update listener position (camera or player)
    Vector3 playerPos = player.getPosition();
    audio.setListenerPosition(playerPos.x, playerPos.y, playerPos.z);

    // ... rest of game logic
}
```

### Step 3: Shutdown

In your cleanup code:

```cpp
void Game::shutdown() {
    // ... other cleanup

    // Shutdown audio system
    AudioSystem& audio = AudioSystem::getInstance();
    audio.shutdown();
}
```

---

## Basic Usage

### Playing Sound Effects

```cpp
#include "Audio/AudioSystem.h"

void Player::jump() {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.playSoundEffect("player_jump");
}

void UI::onButtonClick() {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.playSoundEffect("ui_click", 1.0f, 0, AudioCategory::UI);
}
```

### Playing 3D Sounds

```cpp
void Enemy::takeDamage(float x, float y, float z) {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.playSoundEffect3D("enemy_hurt", x, y, z);
}

void Explosion::explode() {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.playSoundEffect3D("explosion",
                           position.x,
                           position.y,
                           position.z,
                           1.0f,
                           AudioCategory::Combat);
}
```

### Playing Music

```cpp
void GameState::enterExploration() {
    AudioSystem& audio = AudioSystem::getInstance();
    MusicPlayer* music = audio.getMusicPlayer();

    // Crossfade to exploration music
    music->transitionTo("exploration_theme",
                       MusicTransition::Type::Crossfade,
                       2.0f,   // 2 second crossfade
                       -1);    // Loop infinitely
}

void GameState::enterCombat() {
    AudioSystem& audio = AudioSystem::getInstance();
    audio.setAudioState(AudioState::Combat);

    // Music will automatically adapt if configured
}
```

---

## Advanced Features

### 1. Adaptive Music with Layers

```cpp
class CombatManager {
public:
    void startCombat() {
        AudioSystem& audio = AudioSystem::getInstance();
        MusicPlayer* music = audio.getMusicPlayer();

        // Start combat music (base layer only)
        audio.setAudioState(AudioState::Combat);
        music->play("combat_theme", -1);
    }

    void onBossPhase2() {
        MusicPlayer* music = AudioSystem::getInstance().getMusicPlayer();

        // Add intensity layer
        music->fadeInLayer("combat_theme_layer1", 1.5f);
    }

    void onLowHealth() {
        MusicPlayer* music = AudioSystem::getInstance().getMusicPlayer();

        // Add dramatic layer
        music->fadeInLayer("combat_theme_layer2", 1.0f);
    }

    void endCombat() {
        AudioSystem& audio = AudioSystem::getInstance();
        MusicPlayer* music = audio.getMusicPlayer();

        // Fade out all layers
        music->fadeOutLayer("combat_theme_layer1", 2.0f);
        music->fadeOutLayer("combat_theme_layer2", 2.0f);

        // Return to exploration
        audio.setAudioState(AudioState::Exploration);
    }
};
```

### 2. Audio Emitters for Game Objects

```cpp
class Torch : public GameObject {
private:
    AudioEmitter emitter;

public:
    void init() {
        emitter.setPosition(position.x, position.y, position.z);
        emitter.playAmbient("fire_crackling", 0.6f, 30.0f);
    }

    void update(float deltaTime) {
        // Position updates automatically if emitter is updated
        emitter.setPosition(position.x, position.y, position.z);
    }

    void cleanup() {
        emitter.stopAmbient();
    }
};
```

### 3. Audio Zones for Area-Based Ambience

```cpp
class DungeonRoom {
private:
    AudioZone audioZone;

public:
    void init(const Bounds& bounds) {
        audioZone.setBounds(bounds.minX, bounds.minY, bounds.minZ,
                           bounds.maxX, bounds.maxY, bounds.maxZ);
        audioZone.setSound("ambient_dungeon", 0.5f);
        audioZone.setFadeDistance(10.0f);
    }

    void update(float deltaTime) {
        AudioSystem& audio = AudioSystem::getInstance();
        audioZone.update(deltaTime, audio.getListener());
    }
};
```

### 4. Audio Ducking for Dialogue

```cpp
class DialogueSystem {
public:
    void startDialogue(const std::string& dialogueFile) {
        AudioSystem& audio = AudioSystem::getInstance();

        // Duck music and ambient sounds
        audio.enableDucking(AudioCategory::Music, 0.5f);
        audio.enableDucking(AudioCategory::Ambient, 0.6f);

        // Play dialogue
        audio.playSoundEffect(dialogueFile, 1.0f, 0, AudioCategory::Voice);
    }

    void endDialogue() {
        AudioSystem& audio = AudioSystem::getInstance();

        // Restore volumes
        audio.disableDucking(AudioCategory::Music);
        audio.disableDucking(AudioCategory::Ambient);
    }
};
```

### 5. Footstep System

```cpp
class FootstepManager {
private:
    float stepTimer = 0.0f;
    float stepInterval = 0.4f;
    SurfaceType currentSurface = SurfaceType::Grass;

public:
    void update(float deltaTime, bool isMoving) {
        if (!isMoving) {
            stepTimer = 0.0f;
            return;
        }

        stepTimer += deltaTime;
        if (stepTimer >= stepInterval) {
            playFootstep();
            stepTimer = 0.0f;
        }
    }

    void playFootstep() {
        AudioSystem& audio = AudioSystem::getInstance();

        std::string soundName;
        switch (currentSurface) {
            case SurfaceType::Grass:
                soundName = "footstep_grass";
                break;
            case SurfaceType::Stone:
                soundName = "footstep_stone";
                break;
            case SurfaceType::Water:
                soundName = "footstep_water";
                break;
        }

        audio.playSoundEffect(soundName, 0.8f, 0, AudioCategory::Footsteps);
    }

    void setSurface(SurfaceType surface) {
        currentSurface = surface;
    }
};
```

---

## Performance Tips

### 1. Limit Simultaneous Sounds

```cpp
// Configure max channels
AudioConfig config;
config.maxChannels = 32;  // Balance between quality and performance

// Limit instances per sound
SoundEffectManager* sfxManager = /* get manager */;
sfxManager->setMaxInstancesPerSound(4);
```

### 2. Use Appropriate Audio Formats

```cpp
// Short, frequent sounds: WAV (no decode latency)
audio.loadSoundEffect("footstep", "assets/audio/sfx/footstep.wav");

// Long sounds, music: OGG (compressed)
audio.loadMusic("theme", "assets/audio/music/theme.ogg");
```

### 3. Unload Unused Audio

```cpp
// When changing levels
void Level::unload() {
    AudioSystem& audio = AudioSystem::getInstance();

    // Stop all sounds
    audio.stopAllChannels();

    // Unload level-specific sounds
    // (implement unload functionality if needed)
}
```

### 4. Distance Culling

```cpp
// Don't play sounds that are too far away
float distance = calculateDistance(soundPos, listenerPos);
if (distance < maxAudibleDistance) {
    audio.playSoundEffect3D("sound", x, y, z);
}
```

### 5. Priority System

```cpp
// Use priorities to ensure important sounds play
audio.playSoundEffect("player_death", 1.0f, 0, AudioCategory::Voice);
// This has higher priority than footsteps or ambient sounds
```

---

## Troubleshooting

### Audio Not Playing

**Problem**: No sound is heard when playing audio.

**Solutions**:
1. Check audio system is initialized: `audio.isInitialized()`
2. Verify volume is not zero: `audio.setMasterVolume(1.0f)`
3. Check audio isn't muted: `audio.setMuted(false)`
4. Verify file paths are correct
5. Check SDL_mixer is properly installed

### 3D Audio Not Working

**Problem**: 3D positional audio has no panning or distance effect.

**Solutions**:
1. Ensure sound files are **mono**, not stereo
2. Verify listener position is being updated
3. Check min/max distance values are reasonable
4. Verify `enable3DAudio` is true in config

### Music Won't Load

**Problem**: `loadMusic()` fails or music doesn't play.

**Solutions**:
1. Check file format is supported (OGG, MP3, FLAC)
2. Verify file path is correct
3. Check SDL_mixer codecs are initialized
4. Try with a known-good audio file (test.ogg)

### Crackling or Popping Sounds

**Problem**: Audio has artifacts or glitches.

**Solutions**:
1. Increase buffer size: `config.chunkSize = 4096`
2. Check CPU isn't overloaded
3. Verify audio files aren't corrupted
4. Reduce number of simultaneous sounds

### Memory Leaks

**Problem**: Memory usage increases over time.

**Solutions**:
1. Ensure `audio.shutdown()` is called on exit
2. Stop sounds that are no longer needed
3. Unload unused audio resources
4. Check for AudioSource objects not being cleaned up

---

## Example: Complete Game Integration

Here's a complete example of integrating the audio system into a simple game:

```cpp
#include <SDL2/SDL.h>
#include "Audio/AudioSystem.h"
#include "Game/Player.h"
#include "Game/World.h"

class Game {
private:
    bool running = false;
    Player player;
    World world;

public:
    bool init() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            return false;
        }

        // Initialize audio system
        AudioSystem& audio = AudioSystem::getInstance();
        if (!audio.initialize()) {
            return false;
        }

        // Load audio resources
        audio.loadSoundMapping("data/audio/sound_mapping.json");
        audio.loadMusicPlaylist("data/audio/music_tracks.json");

        // Set up state-based music
        MusicPlayer* music = audio.getMusicPlayer();
        music->setStateTrack(AudioState::Menu, "menu_theme");
        music->setStateTrack(AudioState::Exploration, "exploration_theme");
        music->setStateTrack(AudioState::Combat, "combat_theme");

        // Start menu music
        audio.setAudioState(AudioState::Menu);
        music->playForState(AudioState::Menu);

        running = true;
        return true;
    }

    void run() {
        Uint32 lastTime = SDL_GetTicks();

        while (running) {
            Uint32 currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            handleInput();
            update(deltaTime);
            render();
        }
    }

    void update(float deltaTime) {
        AudioSystem& audio = AudioSystem::getInstance();

        // Update game objects
        player.update(deltaTime);
        world.update(deltaTime);

        // Update audio listener to follow player
        Vector3 playerPos = player.getPosition();
        audio.setListenerPosition(playerPos.x, playerPos.y, playerPos.z);

        // Update audio system
        audio.update(deltaTime);

        // Handle game state changes
        if (world.isInCombat() && audio.getAudioState() != AudioState::Combat) {
            audio.setAudioState(AudioState::Combat);
        } else if (!world.isInCombat() && audio.getAudioState() == AudioState::Combat) {
            audio.setAudioState(AudioState::Exploration);
        }
    }

    void shutdown() {
        AudioSystem& audio = AudioSystem::getInstance();
        audio.shutdown();
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }

    game.run();
    game.shutdown();

    return 0;
}
```

---

## Next Steps

1. **Read the Audio Asset Guide**: See `assets/audio/README.md` for detailed information on creating audio assets
2. **Run the Examples**: Check out `src/Audio/AudioExample.cpp` for comprehensive usage examples
3. **Configure Your Audio**: Edit `data/audio/sound_mapping.json` and `music_tracks.json`
4. **Create Your Assets**: Follow the guidelines in the asset creation guide
5. **Test Thoroughly**: Test audio on different platforms and with various hardware

---

## Support and Resources

- **Audio System README**: `src/Audio/README.md` - API reference
- **Asset Creation Guide**: `assets/audio/README.md` - How to create audio files
- **Example Code**: `src/Audio/AudioExample.cpp` - Usage examples
- **SDL_mixer Documentation**: https://www.libsdl.org/projects/SDL_mixer/

---

## Tips for Best Results

1. **Test early and often**: Audio bugs can be subtle
2. **Use headphones**: Better for testing 3D audio
3. **Profile performance**: Audio can impact frame rate
4. **Plan your audio states**: Design adaptive music system early
5. **Create variations**: Avoid repetitive sounds
6. **Balance volumes**: Mix audio across all categories
7. **Consider accessibility**: Provide volume controls and subtitles

Good luck with your audio integration!
