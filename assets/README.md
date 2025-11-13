# Asset Creation Guidelines

This document provides guidelines and best practices for creating and managing game assets for optimal performance and compatibility with the asset pipeline.

## Table of Contents

- [Directory Structure](#directory-structure)
- [Asset Types](#asset-types)
- [Texture Guidelines](#texture-guidelines)
- [Audio Guidelines](#audio-guidelines)
- [Model Guidelines](#model-guidelines)
- [Shader Guidelines](#shader-guidelines)
- [Asset Naming Conventions](#asset-naming-conventions)
- [Optimization Tips](#optimization-tips)
- [Asset Pipeline Tools](#asset-pipeline-tools)
- [Hot-Reloading](#hot-reloading)
- [Memory Budget](#memory-budget)

## Directory Structure

Organize assets by type in the following structure:

```
assets/
├── textures/           # Texture images
│   ├── characters/     # Character sprites
│   ├── environment/    # Environment textures
│   ├── ui/            # UI elements
│   └── effects/       # Visual effects
├── audio/             # Audio files
│   ├── music/         # Background music
│   ├── sfx/           # Sound effects
│   └── voice/         # Voice acting
├── shaders/           # Shader programs
├── models/            # 3D models
├── maps/              # Tile maps and levels
├── fonts/             # Font files
└── data/              # Game data (JSON, XML)
```

## Asset Types

### Supported Formats

| Type | Supported Formats | Recommended |
|------|------------------|-------------|
| Textures | PNG, JPG, BMP, TGA | PNG (with alpha), JPG (photos) |
| Audio | WAV, MP3, OGG | OGG (compressed), WAV (uncompressed) |
| Models | OBJ, FBX, GLTF, GLB | GLTF/GLB |
| Shaders | GLSL, .vert, .frag | .vert/.frag |
| Maps | TMX, JSON | JSON |
| Fonts | TTF, OTF | TTF |

## Texture Guidelines

### Resolution

- **UI Elements**: Power of 2 sizes (64x64, 128x128, 256x256, etc.)
- **Character Sprites**: 64x64 to 256x256 typical
- **Environment**: 512x512 to 2048x2048
- **Atlas Pages**: Maximum 2048x2048 (for compatibility)

### Format Recommendations

- **With transparency**: PNG-24 or PNG-32
- **No transparency**: JPG (quality 85-95) or PNG-8
- **Small sprites**: Use texture atlases (see [Asset Pipeline Tools](#asset-pipeline-tools))

### Best Practices

1. **Use Texture Atlases**: Combine multiple small textures into atlases using `generate_atlas.py`
2. **Trim Transparent Pixels**: Remove unnecessary transparent borders
3. **Power-of-Two**: Use power-of-two dimensions when possible for better GPU performance
4. **Compression**: Enable mipmaps for 3D textures
5. **Color Depth**: Use appropriate bit depth (RGB vs RGBA)

### Example

```bash
# Generate a texture atlas from individual sprites
./tools/generate_atlas.py assets/textures/characters/ output/characters_atlas.png
```

## Audio Guidelines

### Format Selection

| Use Case | Format | Settings |
|----------|--------|----------|
| Music (looping) | OGG | Quality 6-8, Stereo |
| Sound Effects (short) | WAV | 16-bit, Mono/Stereo |
| Sound Effects (long) | OGG | Quality 4-6, Mono |
| Voice | OGG | Quality 5-7, Mono |

### Best Practices

1. **Sample Rate**: 44.1 kHz for most audio, 22 kHz for low-quality SFX
2. **Bit Depth**: 16-bit for most cases
3. **Channels**: Mono for SFX and voice, Stereo for music
4. **Normalization**: Normalize audio levels to prevent clipping
5. **Looping**: Ensure seamless loops for music and ambient sounds

### File Size Targets

- **Music**: 2-5 MB per minute (OGG)
- **Short SFX**: < 100 KB
- **Voice clips**: < 500 KB per line

## Model Guidelines

### Polygon Count

| Type | Target Triangles |
|------|-----------------|
| Simple props | < 500 |
| Characters | 2,000 - 5,000 |
| Hero characters | 5,000 - 10,000 |
| Environment | 10,000 - 50,000 |

### Best Practices

1. **UV Mapping**: Ensure clean UV layouts with minimal seams
2. **Normals**: Include smooth normals for better lighting
3. **Materials**: Keep material count low (1-3 per model)
4. **Pivots**: Set correct pivot points for rotation/animation
5. **Scale**: Use consistent scale (1 unit = 1 meter recommended)

### Supported Features

- ✅ Static meshes
- ✅ UV coordinates
- ✅ Vertex colors
- ✅ Multiple materials
- ⚠️ Skeletal animation (limited support)
- ❌ Morph targets (not yet supported)

## Shader Guidelines

### Structure

Organize shaders in pairs:

```
shaders/
├── sprite.vert          # Vertex shader
├── sprite.frag          # Fragment shader
├── particle.vert
├── particle.frag
└── post_process.frag
```

### Best Practices

1. **Uniforms**: Use uniform buffers for shared data
2. **Precision**: Use appropriate precision (lowp, mediump, highp)
3. **Branching**: Minimize conditional branches in fragment shaders
4. **Sampling**: Batch texture samples when possible
5. **Comments**: Document uniforms and key functionality

### Example Shader

```glsl
// sprite.vert
#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 fragTexCoord;

void main() {
    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
    fragTexCoord = texCoord;
}
```

## Asset Naming Conventions

### General Rules

- Use **lowercase** with **underscores**: `player_walk_01.png`
- Be **descriptive**: `sword_iron_short.obj` not `item_03.obj`
- Include **variants**: `tree_oak_01.obj`, `tree_oak_02.obj`
- Use **sequential numbers**: `_01`, `_02`, `_03`

### Examples

```
✅ Good:
  - character_hero_idle.png
  - sfx_footstep_stone_01.wav
  - shader_water_animated.frag
  - ui_button_start_normal.png

❌ Bad:
  - CharHero-Idle.png (mixed case)
  - footstep1.wav (not descriptive)
  - myshader.frag (too vague)
  - button.png (ambiguous)
```

### Animation Frame Naming

```
character_hero_walk_00.png
character_hero_walk_01.png
character_hero_walk_02.png
...
character_hero_walk_15.png
```

## Optimization Tips

### Memory Optimization

1. **Texture Atlases**: Reduce draw calls and memory overhead
2. **Compression**: Use compressed texture formats (DXT, ETC, ASTC)
3. **Mipmaps**: Generate for 3D textures, skip for UI/2D
4. **Resolution**: Use lowest acceptable resolution
5. **Streaming**: Enable streaming for large assets

### Loading Performance

1. **Async Loading**: Use async loading for non-critical assets
2. **Preloading**: Preload assets for upcoming levels
3. **Lazy Loading**: Delay loading of rarely-used assets
4. **Bundle Assets**: Use asset bundles for faster loading

### Runtime Performance

1. **Asset Pooling**: Reuse frequently created/destroyed objects
2. **LOD**: Use level-of-detail for distant objects
3. **Culling**: Enable frustum culling for off-screen assets
4. **Batching**: Group similar assets for batch rendering

## Asset Pipeline Tools

### Texture Atlas Generator

Generate optimized texture atlases:

```bash
# Basic usage
./tools/generate_atlas.py assets/textures/characters/ output/character_atlas.png

# Advanced options
./tools/generate_atlas.py assets/textures/ui/ output/ui_atlas.png \
  --width 2048 \
  --height 2048 \
  --padding 4 \
  --algorithm maxrects \
  --no-trim
```

### Asset Packer

Pack and compress assets for distribution:

```bash
# Pack all assets
./tools/pack_assets.py assets/ packed/ --compression 6 --bundle

# Pack specific types
./tools/pack_assets.py assets/ packed/ --types textures audio --bundle

# Validate packed assets
./tools/pack_assets.py assets/ packed/ --validate
```

## Hot-Reloading

Hot-reloading allows you to see asset changes immediately without restarting.

### Enable Hot-Reloading

```cpp
// In your initialization code
AssetManager::GetInstance().SetHotReloadEnabled(true);

// Call every frame
AssetManager::GetInstance().Update(deltaTime);
```

### Supported Assets

- ✅ Textures
- ✅ Shaders
- ✅ Data files (JSON, XML)
- ⚠️ Audio (limited support)
- ❌ Models (requires restart)

### Workflow

1. Run game in development mode
2. Edit asset file (e.g., texture in Photoshop)
3. Save changes
4. Asset automatically reloads in-game (within 1-2 seconds)

## Memory Budget

Default memory budgets (configurable):

| Asset Type | Default Budget | Notes |
|------------|---------------|-------|
| Textures | 256 MB | Largest budget |
| Audio | 128 MB | Streamed when possible |
| Models | 128 MB | Shared meshes |
| Other | 64 MB | Shaders, data, etc. |
| **Total** | **512 MB** | Can be increased |

### Configure Memory Budget

```cpp
MemoryBudget budget;
budget.maxTotalMemory = 1024 * 1024 * 1024;  // 1 GB
budget.maxTextureMemory = 512 * 1024 * 1024;  // 512 MB
budget.maxAudioMemory = 256 * 1024 * 1024;    // 256 MB
budget.warningThreshold = 80;  // 80%

AssetManager::GetInstance().Initialize(budget);
```

### Memory Management

The asset system automatically:

- **Tracks** memory usage per asset type
- **Warns** when approaching budget limits
- **Evicts** least-recently-used assets when needed
- **Prevents** loading when budget would be exceeded

### Monitor Memory Usage

```cpp
// Get current memory statistics
auto stats = AssetManager::GetInstance().GetMemoryStats();

std::cout << "Total memory: " << stats.totalUsed / (1024*1024) << " MB" << std::endl;
std::cout << "Textures: " << stats.textureUsed / (1024*1024) << " MB" << std::endl;
std::cout << "Audio: " << stats.audioUsed / (1024*1024) << " MB" << std::endl;
std::cout << "Asset count: " << stats.assetCount << std::endl;
```

## Asset Validation

Before adding assets to the project:

1. **Check format compatibility**
2. **Verify dimensions** (power-of-two for textures)
3. **Test in-game** appearance
4. **Measure file size** and memory usage
5. **Validate naming** conventions

### Automated Validation

```bash
# Run asset validation
./tools/pack_assets.py assets/ packed/ --validate
```

## Performance Checklist

Before shipping:

- [ ] All textures optimized and atlased
- [ ] Audio files compressed (OGG for music/long sounds)
- [ ] Unused assets removed
- [ ] Asset bundles created for faster loading
- [ ] Memory budget tested on target hardware
- [ ] Loading times measured and acceptable
- [ ] Hot-reloading disabled for production builds

## Additional Resources

- **Texture Atlas Documentation**: See `src/Assets/TextureAtlas.h`
- **Asset Manager API**: See `src/Assets/AssetManager.h`
- **Resource Pooling**: See `src/Assets/ResourcePool.h`
- **Async Loading**: See `src/Assets/AssetLoader.h`

## Getting Help

If you encounter issues with assets:

1. Check the console for error messages
2. Verify asset file paths are correct
3. Ensure assets meet format requirements
4. Test with a known-good asset
5. Review memory budget settings

For questions or issues, consult the main project documentation or contact the development team.
