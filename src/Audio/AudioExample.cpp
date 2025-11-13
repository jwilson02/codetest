/**
 * @file AudioExample.cpp
 * @brief Example usage of the Audio System
 *
 * This file demonstrates how to use the various features of the audio system
 * including music playback, sound effects, 3D audio, and adaptive music.
 */

#include "AudioSystem.h"
#include "MusicPlayer.h"
#include "SoundEffect.h"
#include "AudioSource.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Audio;

// Simple delay helper
void wait(float seconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

/**
 * Example 1: Basic Audio System Setup
 */
void example_BasicSetup() {
    std::cout << "\n=== Example 1: Basic Audio System Setup ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();

    // Configure audio system
    AudioConfig config;
    config.frequency = 44100;
    config.channels = 2;
    config.maxChannels = 32;
    config.masterVolume = 1.0f;
    config.musicVolume = 0.7f;
    config.sfxVolume = 0.8f;

    // Initialize
    if (audio.initialize(config)) {
        std::cout << "Audio system initialized successfully!" << std::endl;
        std::cout << "Supported formats: ";
        for (const auto& fmt : AudioSystem::getSupportedFormats()) {
            std::cout << fmt << " ";
        }
        std::cout << std::endl;
    }

    // Shutdown
    audio.shutdown();
}

/**
 * Example 2: Loading and Playing Sound Effects
 */
void example_SoundEffects() {
    std::cout << "\n=== Example 2: Sound Effects ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    // Load sound effects
    audio.loadSoundEffect("sword_hit", "assets/audio/sfx/combat/sword_hit_01.wav");
    audio.loadSoundEffect("ui_click", "assets/audio/sfx/ui/click.wav");
    audio.loadSoundEffect("footstep", "assets/audio/sfx/footsteps/grass_01.wav");

    // Play sound effects
    std::cout << "Playing sword hit..." << std::endl;
    audio.playSoundEffect("sword_hit", 1.0f);
    wait(1.0f);

    std::cout << "Playing UI click..." << std::endl;
    audio.playSoundEffect("ui_click", 0.8f);
    wait(0.5f);

    // Load from JSON configuration
    audio.loadSoundMapping("data/audio/sound_mapping.json");
    std::cout << "Loaded " << audio.getLoadedSoundsCount() << " sounds" << std::endl;

    audio.shutdown();
}

/**
 * Example 3: 3D Positional Audio
 */
void example_3DAudio() {
    std::cout << "\n=== Example 3: 3D Positional Audio ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    // Load a sound effect
    audio.loadSoundEffect("explosion", "assets/audio/sfx/combat/explosion.wav");

    // Set listener position (player/camera)
    audio.setListenerPosition(0.0f, 0.0f, 0.0f);

    // Play sound at different positions
    std::cout << "Playing explosion at (10, 0, 0) - to the right" << std::endl;
    audio.playSoundEffect3D("explosion", 10.0f, 0.0f, 0.0f, 1.0f);
    wait(2.0f);

    std::cout << "Playing explosion at (-10, 0, 0) - to the left" << std::endl;
    audio.playSoundEffect3D("explosion", -10.0f, 0.0f, 0.0f, 1.0f);
    wait(2.0f);

    std::cout << "Playing explosion at (0, 0, 20) - in front" << std::endl;
    audio.playSoundEffect3D("explosion", 0.0f, 0.0f, 20.0f, 1.0f);
    wait(2.0f);

    // Move listener while sound plays
    std::cout << "Moving listener while sound plays..." << std::endl;
    int channel = audio.playSoundEffect3D("explosion", 30.0f, 0.0f, 0.0f, 1.0f);

    for (int i = 0; i < 10; ++i) {
        audio.setListenerPosition(i * 3.0f, 0.0f, 0.0f);
        audio.update(0.1f);
        wait(0.1f);
    }

    audio.shutdown();
}

/**
 * Example 4: Music System with Transitions
 */
void example_MusicSystem() {
    std::cout << "\n=== Example 4: Music System ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    MusicPlayer* music = audio.getMusicPlayer();

    // Load music tracks
    music->loadTrack("exploration", "assets/audio/music/exploration.ogg", AudioState::Exploration);
    music->loadTrack("combat", "assets/audio/music/combat.ogg", AudioState::Combat);

    // Play exploration music
    std::cout << "Playing exploration music..." << std::endl;
    music->play("exploration", -1);  // Loop infinitely
    wait(3.0f);

    // Crossfade to combat music
    std::cout << "Transitioning to combat music (crossfade)..." << std::endl;
    music->transitionTo("combat", MusicTransition::Type::Crossfade, 2.0f, -1);
    wait(4.0f);

    // Fade out
    std::cout << "Fading out music..." << std::endl;
    music->fadeOut(2.0f);
    wait(3.0f);

    audio.shutdown();
}

/**
 * Example 5: Adaptive Music with Layers
 */
void example_AdaptiveMusic() {
    std::cout << "\n=== Example 5: Adaptive Music with Layers ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    MusicPlayer* music = audio.getMusicPlayer();

    // Load layered music track
    std::vector<std::string> combatLayers = {
        "assets/audio/music/combat_base.ogg",
        "assets/audio/music/combat_percussion.ogg",
        "assets/audio/music/combat_intensity.ogg"
    };
    music->loadTrackWithLayers("combat_layered", combatLayers, AudioState::Combat);

    // Play base layer
    std::cout << "Playing combat music (base layer only)..." << std::endl;
    music->play("combat_layered", -1);
    wait(3.0f);

    // Add percussion layer
    std::cout << "Adding percussion layer..." << std::endl;
    music->fadeInLayer("combat_layered_layer1", 1.5f);
    wait(3.0f);

    // Add intensity layer
    std::cout << "Adding intensity layer (boss fight!)..." << std::endl;
    music->fadeInLayer("combat_layered_layer2", 1.0f);
    wait(3.0f);

    // Remove layers (combat ending)
    std::cout << "Removing intensity layer..." << std::endl;
    music->fadeOutLayer("combat_layered_layer2", 2.0f);
    wait(2.5f);

    std::cout << "Removing percussion layer..." << std::endl;
    music->fadeOutLayer("combat_layered_layer1", 2.0f);
    wait(3.0f);

    audio.shutdown();
}

/**
 * Example 6: Audio State Management (Adaptive Music)
 */
void example_AudioStates() {
    std::cout << "\n=== Example 6: Audio State Management ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    MusicPlayer* music = audio.getMusicPlayer();

    // Load music for different states
    music->loadTrack("exploration", "assets/audio/music/exploration.ogg");
    music->loadTrack("combat", "assets/audio/music/combat.ogg");
    music->loadTrack("stealth", "assets/audio/music/stealth.ogg");

    // Map tracks to states
    music->setStateTrack(AudioState::Exploration, "exploration");
    music->setStateTrack(AudioState::Combat, "combat");
    music->setStateTrack(AudioState::Stealth, "stealth");

    // Simulate gameplay state changes
    std::cout << "Entering exploration..." << std::endl;
    audio.setAudioState(AudioState::Exploration);
    music->playForState(AudioState::Exploration);
    wait(3.0f);

    std::cout << "Entering combat!" << std::endl;
    audio.setAudioState(AudioState::Combat);
    wait(3.0f);

    std::cout << "Back to exploration..." << std::endl;
    audio.setAudioState(AudioState::Exploration);
    wait(3.0f);

    std::cout << "Entering stealth mode..." << std::endl;
    audio.setAudioState(AudioState::Stealth);
    wait(3.0f);

    audio.shutdown();
}

/**
 * Example 7: Volume Control and Audio Ducking
 */
void example_VolumeControl() {
    std::cout << "\n=== Example 7: Volume Control and Ducking ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    // Load audio
    audio.loadMusic("background", "assets/audio/music/exploration.ogg");
    audio.loadSoundEffect("voice", "assets/audio/voice/dialogue_01.ogg");

    MusicPlayer* music = audio.getMusicPlayer();
    music->play("background", -1);

    std::cout << "Playing background music at full volume..." << std::endl;
    wait(2.0f);

    // Enable ducking for music when voice plays
    std::cout << "Playing voice with music ducking..." << std::endl;
    audio.enableDucking(AudioCategory::Music, 0.4f);  // Duck to 60% volume
    audio.playSoundEffect("voice", 1.0f);

    wait(3.0f);

    // Disable ducking
    std::cout << "Voice finished, restoring music volume..." << std::endl;
    audio.disableDucking(AudioCategory::Music);
    wait(2.0f);

    // Demonstrate volume control
    std::cout << "Adjusting volume levels..." << std::endl;
    audio.setMusicVolume(0.5f);
    wait(1.0f);
    audio.setMusicVolume(1.0f);
    wait(1.0f);

    // Mute/unmute
    std::cout << "Muting audio..." << std::endl;
    audio.setMuted(true);
    wait(1.0f);
    std::cout << "Unmuting audio..." << std::endl;
    audio.setMuted(false);
    wait(1.0f);

    audio.shutdown();
}

/**
 * Example 8: Advanced AudioSource with Effects
 */
void example_AudioSource() {
    std::cout << "\n=== Example 8: Advanced AudioSource ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    audio.loadSoundEffect("ambient_fire", "assets/audio/ambient/fire_loop.ogg");

    // Create audio source
    AudioSource fireSource;
    fireSource.setSound("ambient_fire");
    fireSource.setPosition(10.0f, 0.0f, 5.0f);
    fireSource.setVolume(0.8f);
    fireSource.setLooping(true);

    // Configure distance attenuation
    fireSource.setMinDistance(2.0f);
    fireSource.setMaxDistance(50.0f);
    fireSource.setAttenuationModel(AttenuationModel::InverseSquare);
    fireSource.setRolloffFactor(1.0f);

    // Configure directional cone (fire facing forward)
    fireSource.setCone(90.0f, 180.0f, 0.3f);  // 90° inner, 180° outer, 30% outer volume
    fireSource.setConeOrientation(0.0f, 0.0f, 1.0f);  // Facing forward

    // Set effects
    fireSource.setOcclusion(0.0f);  // No walls blocking
    fireSource.setReverbSend(0.5f);  // Some reverb

    // Play the source
    std::cout << "Playing fire ambient sound..." << std::endl;
    fireSource.play(-1);

    // Simulate listener movement
    audio.setListenerPosition(0.0f, 0.0f, 0.0f);

    std::cout << "Moving listener around the fire source..." << std::endl;
    for (int i = 0; i < 20; ++i) {
        float angle = i * 0.314f;  // ~18 degrees per step
        float x = 15.0f * std::cos(angle);
        float z = 5.0f + 15.0f * std::sin(angle);

        audio.setListenerPosition(x, 0.0f, z);
        fireSource.update(0.1f, audio.getListener());
        audio.update(0.1f);

        wait(0.1f);
    }

    fireSource.stop();
    audio.shutdown();
}

/**
 * Example 9: Audio Categories and Priorities
 */
void example_CategoriesAndPriorities() {
    std::cout << "\n=== Example 9: Audio Categories and Priorities ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    // Load sounds with different categories
    audio.loadSoundEffect("ui_click", "assets/audio/sfx/ui/click.wav");
    audio.loadSoundEffect("footstep", "assets/audio/sfx/footsteps/grass_01.wav");
    audio.loadSoundEffect("explosion", "assets/audio/sfx/combat/explosion.wav");

    // Set category volumes
    audio.setCategoryVolume(AudioCategory::UI, 1.0f);
    audio.setCategoryVolume(AudioCategory::Footsteps, 0.6f);
    audio.setCategoryVolume(AudioCategory::Combat, 0.9f);

    // Play sounds in different categories
    std::cout << "Playing UI sound (full volume)..." << std::endl;
    audio.playSoundEffect("ui_click", 1.0f, 0, AudioCategory::UI);
    wait(0.5f);

    std::cout << "Playing footstep (60% volume)..." << std::endl;
    audio.playSoundEffect("footstep", 1.0f, 0, AudioCategory::Footsteps);
    wait(0.5f);

    std::cout << "Playing explosion (90% volume)..." << std::endl;
    audio.playSoundEffect("explosion", 1.0f, 0, AudioCategory::Combat);
    wait(2.0f);

    // Adjust category volumes
    std::cout << "Reducing combat sounds volume..." << std::endl;
    audio.setCategoryVolume(AudioCategory::Combat, 0.3f);
    audio.playSoundEffect("explosion", 1.0f, 0, AudioCategory::Combat);
    wait(2.0f);

    audio.shutdown();
}

/**
 * Example 10: Complete Game Loop Integration
 */
void example_GameLoop() {
    std::cout << "\n=== Example 10: Complete Game Loop Integration ===" << std::endl;

    AudioSystem& audio = AudioSystem::getInstance();
    audio.initialize();

    // Load configuration files
    audio.loadSoundMapping("data/audio/sound_mapping.json");
    audio.loadMusicPlaylist("data/audio/music_tracks.json");

    MusicPlayer* music = audio.getMusicPlayer();

    // Start with menu music
    std::cout << "Game starting - menu music..." << std::endl;
    audio.setAudioState(AudioState::Menu);
    music->playForState(AudioState::Menu);
    wait(2.0f);

    // Enter game - exploration
    std::cout << "Starting game - exploration music..." << std::endl;
    audio.setAudioState(AudioState::Exploration);
    wait(2.0f);

    // Simulate game events
    std::cout << "Player walking..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        audio.playSoundEffect("footstep_grass", 1.0f, 0, AudioCategory::Footsteps);
        wait(0.4f);
    }

    // Enter combat
    std::cout << "Combat started!" << std::endl;
    audio.setAudioState(AudioState::Combat);
    wait(1.0f);

    std::cout << "Player attacks..." << std::endl;
    audio.playSoundEffect("sword_swing", 1.0f, 0, AudioCategory::Combat);
    wait(0.5f);
    audio.playSoundEffect("sword_hit", 1.0f, 0, AudioCategory::Combat);
    wait(1.5f);

    // Combat ends
    std::cout << "Combat ended - back to exploration..." << std::endl;
    audio.setAudioState(AudioState::Exploration);
    wait(2.0f);

    // Game loop (simplified)
    std::cout << "Running game loop..." << std::endl;
    float deltaTime = 1.0f / 60.0f;  // 60 FPS
    for (int frame = 0; frame < 180; ++frame) {  // 3 seconds at 60 FPS
        audio.update(deltaTime);
        wait(deltaTime);
    }

    // Shutdown
    std::cout << "Game closing..." << std::endl;
    music->fadeOut(2.0f);
    wait(2.5f);

    audio.shutdown();
}

/**
 * Main function - runs all examples
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Audio System Examples ===" << std::endl;
    std::cout << "This demonstrates the various features of the audio system.\n" << std::endl;

    try {
        // Run examples (comment out ones you don't want to run)
        example_BasicSetup();
        example_SoundEffects();
        example_3DAudio();
        example_MusicSystem();
        example_AdaptiveMusic();
        example_AudioStates();
        example_VolumeControl();
        example_AudioSource();
        example_CategoriesAndPriorities();
        example_GameLoop();

        std::cout << "\n=== All examples completed! ===" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
