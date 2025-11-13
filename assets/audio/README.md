# Audio Asset Creation Guide

This guide provides comprehensive information for creating and integrating audio assets into the game's professional audio system.

## Table of Contents

1. [Overview](#overview)
2. [Audio Formats](#audio-formats)
3. [Music Guidelines](#music-guidelines)
4. [Sound Effects Guidelines](#sound-effects-guidelines)
5. [Voice Acting Guidelines](#voice-acting-guidelines)
6. [3D Positional Audio](#3d-positional-audio)
7. [Ambient Sounds](#ambient-sounds)
8. [Implementation](#implementation)
9. [Tools and Software](#tools-and-software)
10. [Best Practices](#best-practices)

---

## Overview

The game uses SDL_mixer for audio playback with support for:
- Adaptive music with dynamic layers
- 3D positional audio with attenuation models
- Sound effect categories and priorities
- Audio ducking and crossfading
- Voice acting and dialogue
- Ambient soundscapes
- Audio occlusion and reverb effects

### System Capabilities

- **Sample Rate**: 44.1 kHz (CD quality)
- **Bit Depth**: 16-bit
- **Channels**: Stereo (2 channels)
- **Max Simultaneous Sounds**: 32 channels
- **Supported Formats**: WAV, OGG, MP3, FLAC

---

## Audio Formats

### Recommended Formats by Use Case

| Type | Format | Reason |
|------|--------|--------|
| Music | OGG Vorbis | Best compression/quality ratio, looping support |
| Long Sound Effects | OGG Vorbis | Good for ambient loops and longer effects |
| Short Sound Effects | WAV | Uncompressed, instant playback, no decoding latency |
| Voice/Dialogue | OGG Vorbis | Good compression for large dialogue sets |

### Compression Settings

#### OGG Vorbis
- **Quality Level**: 5-7 (for music)
- **Quality Level**: 4-6 (for sound effects)
- **Bitrate**: Variable (let encoder decide)
- **Sample Rate**: 44100 Hz

#### WAV
- **Format**: PCM signed 16-bit
- **Sample Rate**: 44100 Hz
- **Channels**: Mono for 3D sounds, Stereo for music

---

## Music Guidelines

### File Structure
```
assets/audio/music/
├── exploration_main.ogg
├── combat_base.ogg
├── combat_percussion.ogg      # Layer 1
├── combat_intensity.ogg        # Layer 2
├── boss_battle.ogg
├── stealth_ambient.ogg
├── dialogue_soft.ogg
├── menu_main.ogg
└── ambient_forest.ogg
```

### Adaptive Music with Layers

For dynamic music that responds to gameplay:

1. **Base Layer**: Core melody and harmony (always playing)
2. **Percussion Layer**: Adds intensity during combat
3. **Intensity Layer**: Maximum intensity for boss battles

#### Creating Layered Music

**Requirements:**
- All layers must be **exactly the same length**
- All layers must have the **same BPM and time signature**
- Layers should be synchronized to start at the same point
- Each layer must loop seamlessly

**Example Setup:**
```json
{
  "name": "combat_theme",
  "hasLayers": true,
  "layers": [
    {
      "name": "combat_base",
      "file": "assets/audio/music/combat_base.ogg"
    },
    {
      "name": "combat_percussion",
      "file": "assets/audio/music/combat_percussion.ogg"
    },
    {
      "name": "combat_intensity",
      "file": "assets/audio/music/combat_intensity.ogg"
    }
  ]
}
```

### Loop Points

To create seamless loops:

1. Compose music in measures (4/4, 3/4, etc.)
2. Ensure the end flows naturally back to the beginning
3. Add loop markers in your DAW
4. Export with loop point metadata (OGG supports this)
5. Test the loop for clicks or pops

### Technical Specifications

- **Duration**: 1-3 minutes (for looping tracks)
- **Format**: OGG Vorbis, quality 6
- **Sample Rate**: 44100 Hz
- **Channels**: Stereo
- **Normalization**: -6 dB peak (leave headroom)
- **Fade In/Out**: Handle via code, not baked into audio

---

## Sound Effects Guidelines

### File Structure
```
assets/audio/sfx/
├── footsteps/
│   ├── grass_01.wav
│   ├── grass_02.wav
│   ├── stone_01.wav
│   └── water_01.wav
├── combat/
│   ├── sword_swing_01.wav
│   ├── sword_hit_01.wav
│   └── magic_cast.wav
├── ui/
│   ├── click.wav
│   ├── hover.wav
│   └── menu_open.wav
├── environment/
│   ├── door_open_01.wav
│   ├── chest_open.wav
│   └── torch_light.wav
└── pickup/
    ├── coin_01.wav
    └── item.wav
```

### Sound Effect Categories

#### 1. UI Sounds
- **Format**: WAV (for instant response)
- **Duration**: 0.1 - 0.5 seconds
- **Volume**: Consistent across all UI sounds
- **No 3D positioning**: Always plays at full volume

#### 2. Footsteps
- **Create variations**: At least 3-4 per surface type
- **Duration**: 0.2 - 0.4 seconds
- **Format**: WAV
- **Mono**: For 3D positioning
- **Add pitch variation**: ±10% in code
- **Add volume variation**: ±15% in code

#### 3. Combat Sounds
- **Impacts**: Short, punchy (0.1 - 0.3s)
- **Swings**: Whoosh sounds (0.3 - 0.6s)
- **Format**: WAV for instant feedback
- **Mono**: For 3D positioning
- **Variations**: 3-4 per weapon type

#### 4. Environment Sounds
- **Doors, Chests**: 0.5 - 2 seconds
- **Format**: WAV or OGG
- **Mono**: For 3D positioning
- **Add reverb tail**: For interior spaces

### Variation System

To avoid repetitive sounds, create multiple variations:

```json
{
  "name": "footstep_grass",
  "variations": [
    "assets/audio/sfx/footsteps/grass_01.wav",
    "assets/audio/sfx/footsteps/grass_02.wav",
    "assets/audio/sfx/footsteps/grass_03.wav",
    "assets/audio/sfx/footsteps/grass_04.wav"
  ],
  "pitchVariation": 0.1,
  "volumeVariation": 0.15
}
```

The system will:
- Randomly select a variation
- Apply random pitch shift (±10%)
- Apply random volume variation (±15%)

### Technical Specifications

- **Short Effects** (<1s): WAV, 16-bit, 44100 Hz, Mono
- **Long Effects** (>1s): OGG Vorbis, quality 5, Mono
- **Peak Level**: -3 dB
- **No silence** at start/end (trim)
- **No fades** (handled by code)

---

## Voice Acting Guidelines

### File Structure
```
assets/audio/voice/
├── player/
│   ├── hurt_01.wav
│   ├── hurt_02.wav
│   ├── death.wav
│   └── effort_01.wav
├── npc/
│   ├── greeting_01.ogg
│   ├── quest_01.ogg
│   └── farewell_01.ogg
└── narrator/
    ├── intro.ogg
    └── chapter_01.ogg
```

### Recording Guidelines

#### Equipment
- **Microphone**: Condenser or dynamic with pop filter
- **Environment**: Quiet room with minimal echo
- **Recording Level**: -12 dB to -6 dB peak

#### Processing
1. **Noise Reduction**: Remove background noise
2. **EQ**: Enhance clarity (cut below 80 Hz, boost 2-5 kHz)
3. **Compression**: Gentle 3:1 ratio
4. **Normalization**: -6 dB peak
5. **De-essing**: Reduce harsh 's' sounds

#### Delivery
- **Short Exclamations** (hurt, effort): 0.5 - 1 second
- **Dialogue Lines**: 2 - 10 seconds
- **Narration**: Up to 30 seconds per file

### Technical Specifications

- **Format**: OGG Vorbis, quality 6
- **Sample Rate**: 44100 Hz
- **Channels**: Mono (for 3D positioning) or Stereo (for UI)
- **Naming**: Descriptive with numbering

---

## 3D Positional Audio

### Mono vs Stereo

- **Mono**: Required for 3D positional sounds
  - Footsteps, combat sounds, environment sounds
  - System applies panning and distance attenuation

- **Stereo**: For non-positional sounds
  - Music, UI sounds, voices (optional)

### Attenuation Models

The system supports 4 attenuation models:

#### 1. Linear
```
volume = 1.0 - (distance / maxDistance)
```
- Smooth, gradual falloff
- Good for ambient sounds

#### 2. Inverse
```
volume = minDistance / distance
```
- More realistic than linear
- Good for most sound effects

#### 3. Inverse Square (Recommended)
```
volume = 1.0 / (1.0 + (distance/minDistance)^2)
```
- Physically accurate
- Best for realistic sound propagation
- Use for important gameplay sounds

#### 4. Exponential
```
volume = exp(-rolloff * distance)
```
- Sharp cutoff
- Good for localized sounds

### Distance Parameters

Configure in `sound_mapping.json`:

```json
{
  "name": "sword_hit",
  "minDistance": 2.0,      // Full volume within this radius
  "maxDistance": 60.0,     // Silent beyond this radius
  "attenuationModel": "inversesquare"
}
```

**Guidelines:**
- **UI Sounds**: No distance (2D)
- **Footsteps**: min: 1.0, max: 20.0
- **Combat**: min: 2.0, max: 50-60
- **Explosions**: min: 5.0, max: 100+
- **Ambient**: min: 5-10, max: 80-150

---

## Ambient Sounds

### Types of Ambient Sounds

#### 1. Looping Ambient
Continuous environmental sounds:
- Wind, rain, water flowing
- Fire crackling, machinery humming
- Forest ambience, cave drips

**Requirements:**
- Must loop seamlessly
- Mono for 3D positioning
- OGG format for compression
- 10-30 seconds in length

#### 2. Ambient One-Shots
Occasional environmental sounds:
- Bird chirps, distant thunder
- Leaves rustling, footsteps
- Random creature sounds

**Playback:**
- Triggered at intervals
- Random pitch/volume variation
- Multiple variations

### Creating Seamless Loops

1. **Record longer than needed**: 30-60 seconds
2. **Find natural loop points**: Where waveform crosses zero
3. **Use crossfade loop**: Overlap beginning and end (1-2s)
4. **Test extensively**: Listen for clicks or pattern recognition

**Tools:**
- Audacity: Analyze → Plot Spectrum (find loop points)
- Reaper: Loop points with crossfade
- Logic Pro: Flex Time for loop matching

### File Structure
```
assets/audio/ambient/
├── wind_loop.ogg
├── fire_loop.ogg
├── water_loop.ogg
├── birds_loop.ogg
├── cave_drips_loop.ogg
└── night_forest_loop.ogg
```

### Implementation Example

```json
{
  "name": "ambient_wind",
  "category": "ambient",
  "file": "assets/audio/ambient/wind_loop.ogg",
  "looping": true,
  "volume": 0.4,
  "minDistance": 5.0,
  "maxDistance": 100.0,
  "attenuationModel": "linear"
}
```

---

## Implementation

### Loading Sounds

#### Via Code
```cpp
AudioSystem& audio = AudioSystem::getInstance();

// Load single sound effect
audio.loadSoundEffect("sword_hit", "assets/audio/sfx/combat/sword_hit_01.wav");

// Load music track
audio.loadMusic("exploration_theme", "assets/audio/music/exploration.ogg");
```

#### Via JSON
```cpp
// Load sound mappings from JSON
audio.loadSoundMapping("data/audio/sound_mapping.json");

// Load music playlist from JSON
audio.loadMusicPlaylist("data/audio/music_tracks.json");
```

### Playing Sounds

#### 2D (Non-positional)
```cpp
// Play UI sound
audio.playSoundEffect("ui_click", 1.0f);

// Play with volume and looping
audio.playSoundEffect("background_hum", 0.5f, -1); // Loop infinitely
```

#### 3D (Positional)
```cpp
// Play sound at world position
float x = 10.0f, y = 0.0f, z = 5.0f;
audio.playSoundEffect3D("sword_hit", x, y, z);

// With volume control
audio.playSoundEffect3D("explosion", x, y, z, 0.9f);
```

### Music Control

```cpp
MusicPlayer* music = audio.getMusicPlayer();

// Play track
music->play("exploration_theme", -1); // Loop infinitely

// Crossfade to new track
music->transitionTo("combat_theme",
                    MusicTransition::Type::Crossfade,
                    2.0f); // 2 second crossfade

// Control layers (for adaptive music)
music->enableLayer("combat_percussion");
music->fadeInLayer("combat_intensity", 1.5f);
music->fadeOutLayer("combat_percussion", 2.0f);
```

### Adaptive Music by Game State

```cpp
// Map tracks to game states
music->setStateTrack(AudioState::Exploration, "exploration_theme");
music->setStateTrack(AudioState::Combat, "combat_theme");
music->setStateTrack(AudioState::Stealth, "stealth_theme");

// Change game state (music adapts automatically)
audio.setAudioState(AudioState::Combat);
```

---

## Tools and Software

### Digital Audio Workstations (DAWs)

#### Free
- **Audacity**: Basic editing, good for loops
- **Reaper**: Full-featured (free trial, affordable)
- **Cakewalk by BandLab**: Free, Windows only

#### Commercial
- **FL Studio**: Great for music composition
- **Ableton Live**: Excellent for loops and layers
- **Logic Pro**: Mac only, professional
- **Cubase**: Industry standard

### Audio Editing Tools

- **Audacity**: Free, cross-platform
- **ocenaudio**: Free, simple interface
- **Adobe Audition**: Professional (subscription)

### Sound Libraries

#### Free
- **Freesound.org**: Community-driven, CC licensed
- **Zapsplat**: Free with attribution
- **OpenGameArt.org**: Game-focused

#### Commercial
- **AudioJungle**: Pay-per-asset
- **Sonniss**: Premium game audio
- **Pro Sound Effects**: Professional library

### Compression Tools

- **Audacity**: Can export OGG Vorbis
- **FFmpeg**: Command-line encoding
- **foobar2000**: Batch conversion (Windows)

---

## Best Practices

### General Guidelines

1. **Consistency**: Maintain consistent volume levels across similar sounds
2. **Headroom**: Leave -3 to -6 dB headroom for mixing
3. **No Clipping**: Ensure peaks don't exceed 0 dB
4. **Clean Starts/Ends**: Trim silence, no clicks or pops
5. **Metadata**: Include descriptive file names

### Naming Conventions

```
category_name_variation_number.ext

Examples:
footstep_grass_01.wav
sword_swing_heavy_03.wav
ambient_wind_forest_loop.ogg
ui_button_click.wav
voice_player_hurt_02.wav
music_combat_intensity_layer.ogg
```

### File Organization

```
assets/audio/
├── music/
│   ├── exploration/
│   ├── combat/
│   └── ambient/
├── sfx/
│   ├── ui/
│   ├── footsteps/
│   ├── combat/
│   ├── environment/
│   └── pickup/
├── voice/
│   ├── player/
│   ├── npc/
│   └── narrator/
└── ambient/
    ├── exterior/
    └── interior/
```

### Performance Optimization

1. **Use OGG for long files**: Music and long ambient loops
2. **Use WAV for short effects**: UI sounds and quick feedback
3. **Mono for 3D sounds**: Saves memory, better spatialization
4. **Limit variations**: 3-4 is usually enough
5. **Unload unused assets**: Free memory when not needed

### Testing Checklist

- [ ] No clicks, pops, or artifacts
- [ ] Loops seamlessly (for looping sounds)
- [ ] Volume appropriate for category
- [ ] No clipping or distortion
- [ ] Variations sound different enough
- [ ] 3D positioning works correctly
- [ ] Attenuation curves feel natural
- [ ] Music layers align perfectly
- [ ] Crossfades are smooth
- [ ] All files properly named and organized

### Quality Control

#### Technical Check
```bash
# Check audio properties (using ffprobe)
ffprobe -v error -show_streams assets/audio/sfx/footstep.wav

# Verify sample rate and bit depth
# Should show: sample_rate=44100, sample_fmt=s16
```

#### In-Game Testing
1. Play sound in isolation
2. Test with other sounds playing
3. Test at various distances (3D sounds)
4. Test music transitions
5. Test layer combinations
6. Check volume balance

---

## Troubleshooting

### Common Issues

#### "Sound doesn't play"
- Check file path is correct
- Verify format is supported
- Ensure file isn't corrupted
- Check volume isn't set to 0

#### "3D sound has no panning"
- Ensure file is mono (not stereo)
- Check min/max distance settings
- Verify listener position is updated

#### "Music loop has a gap"
- Check for silence at end of file
- Verify loop points are set correctly
- Ensure file hasn't been trimmed incorrectly

#### "Crackling or popping"
- Check buffer size (increase if needed)
- Ensure no DC offset in audio
- Verify sample rate matches system

#### "Layers don't align"
- Ensure all layers are same length
- Check BPM and time signature match
- Verify loop points are identical

---

## Audio System Features Reference

### Categories
- Music
- SFX
- Ambient
- Voice
- UI
- Footsteps
- Combat
- Environment

### Priorities
- Low (0): Can be interrupted
- Normal (1): Standard priority
- High (2): Important sounds
- Critical (3): Never interrupt (UI, dialogue)

### Audio States
- Exploration
- Combat
- Stealth
- Dialogue
- Cutscene
- Menu

### Transition Types
- Immediate: Instant switch
- Crossfade: Blend between tracks
- FadeOut: Fade out then play new
- Sequential: Wait for finish, then play

---

## Contact and Support

For questions or issues with audio integration:
- Check the audio system documentation
- Review example configurations in `data/audio/`
- Test with provided example files

## Version History

- **v1.0** (2025): Initial audio system with SDL_mixer
  - Multi-format support
  - 3D positional audio
  - Adaptive music layers
  - Audio ducking and crossfading

---

**Remember**: Great audio enhances immersion. Take time to create and polish your audio assets!
