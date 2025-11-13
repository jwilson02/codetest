# Audio System

A professional audio system built with SDL_mixer featuring:

- **Advanced Music System**: Dynamic layers, adaptive music, smooth transitions
- **3D Positional Audio**: Realistic sound spatialization with multiple attenuation models
- **Sound Effect Manager**: Categories, priorities, variations, and polyphony limiting
- **Audio Mixing**: Volume control per category, audio ducking, crossfading
- **Ambient Sound System**: Looping environmental audio, audio zones
- **Voice Acting Support**: Dialogue system with priority management
- **Audio Effects**: Occlusion, reverb, Doppler effect (partially supported)

## Features

### Audio Manager (AudioSystem)
- Singleton pattern for easy access
- Configurable sample rate, channels, and buffer size
- Master volume and per-category volume control
- Mute/unmute functionality
- 3D audio listener (camera/player position)
- Audio state management for adaptive music
- Audio ducking (automatic volume reduction)
- Resource loading from files or JSON configuration

### Music Player (MusicPlayer)
- Multiple track loading and management
- Smooth transitions: immediate, crossfade, fade-out, sequential
- Dynamic music layers for adaptive soundtracks
- State-based music (exploration, combat, stealth, etc.)
- Volume control and tempo/pitch adjustment (limited by SDL_mixer)
- Loop control and position tracking

### Sound Effect System (SoundEffect)
- Sound variations for natural randomness
- Pitch and volume randomization
- Priority-based playback
- Polyphony limiting per sound and category
- 3D positional audio support
- Distance-based attenuation with multiple models

### 3D Audio (AudioSource)
- Position and velocity (for Doppler)
- Four attenuation models: Linear, Inverse, InverseSquare, Exponential
- Directional audio cones
- Occlusion and reverb control
- Min/max distance configuration
- Rolloff factor for attenuation curve control

## Building

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler
- SDL2
- SDL2_mixer

### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libsdl2-dev libsdl2-mixer-dev

# Build
cd src/Audio
mkdir build
cd build
cmake ..
make
```

### macOS
```bash
# Install dependencies
brew install sdl2 sdl2_mixer

# Build
cd src/Audio
mkdir build
cd build
cmake ..
make
```

### Windows
```bash
# Install dependencies via vcpkg
vcpkg install sdl2 sdl2-mixer

# Build with CMake
cd src/Audio
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Quick Start

### Basic Setup

```cpp
#include "AudioSystem.h"

using namespace Audio;

int main() {
    // Get audio system instance
    AudioSystem& audio = AudioSystem::getInstance();

    // Initialize with default config
    if (!audio.initialize()) {
        std::cerr << "Failed to initialize audio system" << std::endl;
        return 1;
    }

    // Load and play sound
    audio.loadSoundEffect("click", "assets/audio/ui/click.wav");
    audio.playSoundEffect("click");

    // Load and play music
    audio.loadMusic("theme", "assets/audio/music/main_theme.ogg");
    audio.getMusicPlayer()->play("theme", -1);  // Loop infinitely

    // Update in game loop
    while (gameRunning) {
        float deltaTime = calculateDeltaTime();
        audio.update(deltaTime);

        // ... rest of game loop
    }

    // Cleanup
    audio.shutdown();
    return 0;
}
```

### 3D Positional Audio

```cpp
// Set listener position (usually player/camera)
audio.setListenerPosition(playerX, playerY, playerZ);

// Play sound at world position
audio.playSoundEffect3D("explosion", enemyX, enemyY, enemyZ);

// Use AudioSource for more control
AudioSource fireSound;
fireSound.setSound("fire_loop");
fireSound.setPosition(10.0f, 0.0f, 5.0f);
fireSound.setLooping(true);
fireSound.setMinDistance(2.0f);
fireSound.setMaxDistance(50.0f);
fireSound.setAttenuationModel(AttenuationModel::InverseSquare);
fireSound.play(-1);

// Update each frame
fireSound.update(deltaTime, audio.getListener());
```

### Adaptive Music

```cpp
MusicPlayer* music = audio.getMusicPlayer();

// Load layered music
std::vector<std::string> layers = {
    "assets/audio/music/combat_base.ogg",
    "assets/audio/music/combat_percussion.ogg",
    "assets/audio/music/combat_intensity.ogg"
};
music->loadTrackWithLayers("combat", layers, AudioState::Combat);

// Play base layer
music->play("combat", -1);

// Add layers dynamically
music->fadeInLayer("combat_layer1", 1.5f);  // Add percussion
music->fadeInLayer("combat_layer2", 1.0f);  // Add intensity

// Remove layers
music->fadeOutLayer("combat_layer2", 2.0f);
```

### Audio States

```cpp
// Map music to game states
music->setStateTrack(AudioState::Exploration, "exploration_theme");
music->setStateTrack(AudioState::Combat, "combat_theme");
music->setStateTrack(AudioState::Stealth, "stealth_theme");

// Change state (music adapts automatically)
audio.setAudioState(AudioState::Combat);
music->playForState(AudioState::Combat);

// Music will crossfade automatically when state changes
```

### Audio Ducking

```cpp
// Duck music when voice plays
audio.enableDucking(AudioCategory::Music, 0.5f);  // Duck to 50%

// Play voice
audio.playSoundEffect("dialogue", 1.0f, 0, AudioCategory::Voice);

// Music automatically ducks down and restores when done
audio.disableDucking(AudioCategory::Music);
```

### Volume Control

```cpp
// Master volume
audio.setMasterVolume(0.8f);

// Category volumes
audio.setMusicVolume(0.7f);
audio.setSFXVolume(0.9f);
audio.setAmbientVolume(0.5f);
audio.setVoiceVolume(1.0f);

// Per-category control
audio.setCategoryVolume(AudioCategory::Combat, 0.85f);

// Mute/unmute
audio.setMuted(true);
audio.toggleMute();
```

## Configuration Files

### Sound Mapping (data/audio/sound_mapping.json)

Defines all sound effects with their properties:

```json
{
  "sounds": [
    {
      "name": "footstep_grass",
      "category": "footsteps",
      "priority": "low",
      "volume": 0.6,
      "variations": [
        "assets/audio/sfx/footsteps/grass_01.wav",
        "assets/audio/sfx/footsteps/grass_02.wav"
      ],
      "pitchVariation": 0.1,
      "volumeVariation": 0.15,
      "minDistance": 1.0,
      "maxDistance": 20.0,
      "attenuationModel": "inverse"
    }
  ]
}
```

### Music Tracks (data/audio/music_tracks.json)

Defines music tracks and layered compositions:

```json
{
  "tracks": [
    {
      "name": "combat_theme",
      "state": "combat",
      "hasLayers": true,
      "layers": [
        {
          "name": "combat_base",
          "file": "assets/audio/music/combat_base.ogg"
        },
        {
          "name": "combat_percussion",
          "file": "assets/audio/music/combat_percussion.ogg"
        }
      ]
    }
  ]
}
```

## API Reference

### AudioSystem

**Initialization**
- `initialize(config)` - Initialize audio system
- `shutdown()` - Cleanup and shutdown
- `update(deltaTime)` - Update audio processing (call per frame)

**Volume Control**
- `setMasterVolume(volume)` - Set master volume (0.0 - 1.0)
- `setMusicVolume(volume)` - Set music category volume
- `setSFXVolume(volume)` - Set SFX category volume
- `setCategoryVolume(category, volume)` - Set volume for specific category

**Playback**
- `playSoundEffect(name, volume, loops, category)` - Play 2D sound
- `playSoundEffect3D(name, x, y, z, volume, category)` - Play 3D sound
- `stopChannel(channel)` - Stop specific channel
- `stopAllChannels()` - Stop all playing sounds

**Listener**
- `setListenerPosition(x, y, z)` - Set 3D listener position
- `setListenerOrientation(forward, up)` - Set listener orientation

**State**
- `setAudioState(state)` - Change audio state for adaptive music

**Effects**
- `enableDucking(category, amount)` - Enable audio ducking
- `setReverbAmount(amount)` - Set global reverb
- `setOcclusionFactor(factor)` - Set occlusion amount

### MusicPlayer

**Loading**
- `loadTrack(name, filepath, state)` - Load single track
- `loadTrackWithLayers(name, layerPaths, state)` - Load layered track

**Playback**
- `play(trackName, loops)` - Play track
- `stop()` - Stop playback
- `pause()` / `resume()` - Pause/resume
- `fadeOut(duration)` / `fadeIn(track, duration)` - Fade effects

**Transitions**
- `transitionTo(track, type, duration, loops)` - Transition between tracks

**Layers**
- `enableLayer(layerName)` / `disableLayer(layerName)` - Layer control
- `fadeInLayer(layerName, duration)` - Fade in layer
- `fadeOutLayer(layerName, duration)` - Fade out layer

**State**
- `onStateChanged(newState)` - Handle state change
- `setStateTrack(state, trackName)` - Map state to track

### AudioSource

**Configuration**
- `setPosition(x, y, z)` - Set source position
- `setVelocity(vx, vy, vz)` - Set velocity (Doppler)
- `setSound(soundName)` - Assign sound
- `setVolume(volume)` - Set volume
- `setMinDistance(distance)` / `setMaxDistance(distance)` - Distance range
- `setAttenuationModel(model)` - Set attenuation model
- `setLooping(loop)` - Enable/disable looping

**Effects**
- `setCone(innerAngle, outerAngle, outerVolume)` - Directional sound
- `setOcclusion(amount)` - Set occlusion
- `setReverbSend(amount)` - Set reverb send
- `setDopplerFactor(factor)` - Doppler effect strength

**Playback**
- `play(loops)` - Start playback
- `stop()` - Stop playback
- `update(deltaTime, listener)` - Update (call per frame)

## Audio Categories

- `Music` - Background music
- `SFX` - General sound effects
- `Ambient` - Ambient environmental sounds
- `Voice` - Dialogue and voice acting
- `UI` - User interface sounds
- `Footsteps` - Character footsteps
- `Combat` - Combat-related sounds
- `Environment` - Environmental interactions

## Audio States

- `Exploration` - Normal exploration
- `Combat` - Active combat
- `Stealth` - Stealth/sneaking
- `Dialogue` - Dialogue/conversation
- `Cutscene` - Cinematic cutscene
- `Menu` - Menu/UI state

## Attenuation Models

- `Linear` - Linear distance falloff
- `Inverse` - 1/distance (realistic)
- `InverseSquare` - 1/distance² (most realistic)
- `Exponential` - Exponential decay

## Limitations

### SDL_mixer Limitations

- **No true layered music**: SDL_mixer supports only one music track at a time
  - Workaround: Use Mix_Chunk for layers (play as sound effects)
  - Or pre-mix layers and switch between different mixes

- **No pitch shifting**: SDL_mixer doesn't support pitch changes
  - Doppler effect is calculated but not applied
  - Requires advanced audio library (FMOD, Wwise) or manual implementation

- **Limited reverb**: No built-in reverb or DSP effects
  - Reverb is tracked but not actually applied
  - Would need additional audio processing library

- **No real-time mixing**: Can't adjust music layer volumes independently in real-time

### Recommended Alternatives for Advanced Features

For production games requiring full features:
- **FMOD Studio**: Industry-standard, full feature set
- **Wwise**: Advanced, used in AAA games
- **OpenAL**: Open-source 3D audio
- **miniaudio**: Lightweight, single-header library

## Performance Considerations

- **Limit simultaneous sounds**: Configure `maxChannels` appropriately (default: 32)
- **Use OGG for long audio**: Better compression than WAV
- **Use WAV for short effects**: No decoding latency
- **Mono for 3D sounds**: Stereo files can't be positioned properly
- **Unload unused assets**: Free memory when not needed
- **Limit polyphony**: Use `setMaxInstancesPerSound()` to prevent audio spam

## Examples

See `AudioExample.cpp` for complete usage examples covering:
1. Basic setup and initialization
2. Loading and playing sound effects
3. 3D positional audio
4. Music system with transitions
5. Adaptive music with layers
6. Audio state management
7. Volume control and ducking
8. Advanced AudioSource usage
9. Categories and priorities
10. Complete game loop integration

## License

This audio system is provided as-is for use in game development projects.

## Credits

Built with:
- SDL2 (https://www.libsdl.org/)
- SDL_mixer (https://www.libsdl.org/projects/SDL_mixer/)
