# 2D Rendering System for ARPG Game

A powerful, high-performance 2D rendering system built with SDL2 and OpenGL for Action RPG games. Features sprite batching, animations, particle effects, dynamic lighting, and post-processing.

## Features

### Core Rendering
- **Sprite Batching**: Efficiently render thousands of sprites with automatic batching
- **Multi-texture Support**: Batch up to 32 different textures in a single draw call
- **Z-buffering**: Proper depth sorting for layered rendering
- **Alpha Blending**: Full transparency support with proper alpha blending

### Animation System
- **Sprite Sheet Support**: Easy sprite sheet grid-based animations
- **Animation Controller**: State machine for managing multiple animation clips
- **Frame Events**: Callbacks for animation completion and frame changes
- **Looping & Non-looping**: Support for both looping and one-shot animations

### Particle System
- **High-performance**: Optimized for thousands of particles on screen
- **Configurable Emitters**: Extensive configuration options for particle behavior
- **Custom Update Functions**: Extend particle behavior with custom code
- **Particle Properties**: Position, velocity, acceleration, color, size, rotation

### Camera System
- **Smooth Following**: Interpolated camera following with adjustable speed
- **Zoom**: Dynamic zoom with smooth transitions
- **Screen Shake**: Camera shake effect for game feel
- **Bounds Constraints**: Optional world boundaries
- **Coordinate Conversion**: Screen-to-world and world-to-screen conversion

### Lighting System
- **Dynamic Lights**: Multiple point lights with attenuation
- **Ambient Lighting**: Global ambient light control
- **Colored Lights**: RGB colored light sources
- **Radius Control**: Adjustable light radius and intensity

### Post-Processing
- **Bloom Effect**: HDR bloom with configurable threshold and intensity
- **Color Grading**: Brightness, contrast, saturation, and tint adjustments
- **Render-to-Texture**: Full FBO support for post-processing pipeline
- **Extensible**: Easy to add custom post-processing effects

### Layer-Based Rendering
- Background Layer
- Game Layer
- Foreground Layer
- UI Layer

## Architecture

### Class Overview

```
Renderer (Singleton)
├── Camera
├── SpriteBatch
├── ParticleSystemManager
│   └── ParticleEmitter[]
├── Framebuffer[]
└── PostProcessEffect[]
    ├── BloomEffect
    └── ColorGradingEffect
```

### Key Classes

#### `Renderer`
Main rendering manager that coordinates all rendering operations.
- Singleton pattern for global access
- Manages render layers and viewport
- Handles post-processing pipeline
- Tracks render statistics

#### `SpriteBatch`
High-performance sprite batch renderer.
- Automatic batching by texture
- Up to 32 texture slots per batch
- Configurable max sprites (default 10,000)
- Automatic flushing when full

#### `Camera`
2D camera with advanced features.
- Orthographic projection
- Smooth interpolation
- Screen shake effects
- Bounds constraints

#### `AnimationController`
Animation state machine.
- Multiple animation clips
- Frame-based timing
- Event callbacks
- Speed control

#### `ParticleEmitter`
Particle effect emitter.
- Continuous and burst emission
- Customizable particle properties
- GPU-accelerated rendering

## Usage Examples

### Basic Setup

```cpp
#include "Renderer.h"
#include <SDL2/SDL.h>

// Initialize SDL and create window
SDL_Init(SDL_INIT_VIDEO);
SDL_Window* window = SDL_CreateWindow("Game",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1280, 720, SDL_WINDOW_OPENGL);

// Initialize renderer
auto& renderer = Renderer::GetInstance();
renderer.Initialize(window);

// Load shaders
auto spriteShader = std::make_shared<Shader>();
spriteShader->LoadFromFiles(
    "assets/shaders/sprite.vert",
    "assets/shaders/sprite.frag");

renderer.GetSpriteBatch().SetShader(spriteShader);
```

### Drawing Sprites

```cpp
auto& renderer = Renderer::GetInstance();
auto& spriteBatch = renderer.GetSpriteBatch();

// Load texture
auto texture = TextureManager::GetInstance().Load("player.png");

// Begin frame
renderer.BeginFrame();
renderer.Clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

// Set camera
spriteBatch.SetViewProjectionMatrix(camera->GetViewProjectionMatrix());

// Draw sprites
spriteBatch.Begin();

spriteBatch.DrawSprite(
    texture,
    glm::vec2(100.0f, 100.0f),  // position
    glm::vec2(64.0f, 64.0f),     // size
    glm::vec4(1.0f),             // color
    0.0f,                        // rotation
    0.0f                         // depth
);

spriteBatch.End();

// End frame
renderer.EndFrame();
```

### Animation System

```cpp
// Create sprite sheet (8 columns, 4 rows)
auto spriteSheet = std::make_shared<SpriteSheet>(texture, 8, 4);

// Create animation clips
auto idleAnim = spriteSheet->CreateAnimation("idle", 0, 7, 0.1f, true);
auto runAnim = spriteSheet->CreateAnimation("run", 8, 15, 0.08f, true);

// Setup animation controller
AnimationController animController;
animController.AddClip("idle", idleAnim);
animController.AddClip("run", runAnim);
animController.Play("idle");

// Update (call every frame)
animController.Update(deltaTime);

// Get current frame UV
glm::vec4 currentFrame = animController.GetCurrentFrameUV();

// Draw animated sprite
spriteBatch.DrawSprite(texture, position, size, currentFrame);
```

### Particle Effects

```cpp
// Configure particle emitter
ParticleEmitterConfig config;
config.emissionRate = 50.0f;
config.maxParticles = 500;
config.lifetime = 2.0f;
config.startColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
config.endColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);   // Fade to transparent
config.startSize = 20.0f;
config.endSize = 5.0f;
config.velocity = glm::vec2(0.0f, -100.0f);
config.acceleration = glm::vec2(0.0f, -50.0f);

// Create emitter
auto& particleSystem = renderer.GetParticleSystem();
auto emitter = particleSystem.CreateEmitter(config);
emitter->SetPosition(glm::vec2(400.0f, 300.0f));
emitter->Play();

// Update and render (called automatically by renderer)
particleSystem.Update(deltaTime);
particleSystem.Render(camera->GetViewProjectionMatrix());
```

### Camera Control

```cpp
auto camera = std::make_shared<Camera>(1280.0f, 720.0f);

// Setup smooth following
camera->SetSmoothFollow(true);
camera->SetFollowSpeed(0.1f); // 0.0 = no follow, 1.0 = instant

// Set target (player position)
camera->SetTarget(playerPosition);

// Zoom
camera->SetTargetZoom(2.0f); // 2x zoom

// Camera shake
camera->Shake(20.0f, 0.5f); // intensity, duration

// Set bounds (optional)
camera->SetBounds(glm::vec4(-1000, -1000, 1000, 1000));

// Update camera
camera->Update(deltaTime);
```

### Dynamic Lighting

```cpp
// Set ambient light
renderer.SetAmbientLight(glm::vec3(0.3f, 0.3f, 0.4f), 0.5f);

// Add point lights
Renderer::Light light;
light.position = glm::vec2(400.0f, 300.0f);
light.color = glm::vec3(1.0f, 0.9f, 0.7f);
light.intensity = 2.0f;
light.radius = 200.0f;
renderer.AddLight(light);
```

### Post-Processing

```cpp
// Bloom effect
auto bloom = std::make_shared<BloomEffect>();
bloom->Initialize(1280, 720);
bloom->SetThreshold(0.8f);
bloom->SetIntensity(0.5f);
bloom->SetBlurPasses(5);
renderer.AddPostProcessEffect(bloom);

// Color grading
auto colorGrading = std::make_shared<ColorGradingEffect>();
colorGrading->Initialize();
colorGrading->SetBrightness(0.05f);
colorGrading->SetContrast(1.1f);
colorGrading->SetSaturation(1.2f);
renderer.AddPostProcessEffect(colorGrading);

// Enable post-processing
renderer.SetPostProcessingEnabled(true);
```

## Shader Files

The system includes several GLSL shaders:

### Sprite Shaders
- `sprite.vert` - Vertex shader for sprite rendering
- `sprite.frag` - Fragment shader with multi-texture support

### Particle Shaders
- `particle.vert` - Vertex shader for particles
- `particle.frag` - Fragment shader with texture and procedural shapes

### Post-Processing Shaders
- `fullscreen.vert` - Full-screen quad vertex shader
- `bloom_bright.frag` - Bright pass extraction
- `blur.frag` - Gaussian blur
- `bloom_combine.frag` - Bloom combination with tone mapping
- `color_grading.frag` - Color grading adjustments
- `lighting.frag` - Dynamic lighting

## Performance Optimization

### Sprite Batching
- Minimize texture switches by grouping sprites by texture
- Use texture atlases to combine multiple sprites
- Default batch size: 10,000 sprites
- Automatic flushing when texture slots or sprite limit reached

### Particle System
- Use object pooling (particles are reused)
- Configurable max particles per emitter
- GPU-accelerated rendering

### Render Layers
- Organize rendering by layers to reduce state changes
- Background → Game → Foreground → UI

### Post-Processing
- Render-to-texture only when effects are enabled
- Effects can be toggled individually
- Use lower resolution framebuffers for bloom (half resolution)

## Building

### Requirements
- C++17 or later
- CMake 3.10+
- SDL2
- OpenGL 3.3+
- GLEW
- GLM

### Linux
```bash
# Install dependencies
sudo apt-get install libsdl2-dev libglew-dev libglm-dev

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Windows (Visual Studio)
```bash
# Install dependencies via vcpkg
vcpkg install sdl2 glew glm

# Build
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
```

## File Structure

```
src/Rendering/
├── Renderer.h/cpp         # Main renderer
├── Shader.h/cpp           # Shader management
├── Texture.h/cpp          # Texture loading and caching
├── Camera.h/cpp           # Camera system
├── Sprite.h/cpp           # Sprite batching
├── Animation.h/cpp        # Animation system
├── ParticleSystem.h/cpp   # Particle effects
└── RenderingExample.cpp   # Example usage

assets/shaders/
├── sprite.vert/frag       # Sprite shaders
├── particle.vert/frag     # Particle shaders
├── fullscreen.vert        # Full-screen quad
├── bloom_bright.frag      # Bloom bright pass
├── blur.frag              # Gaussian blur
├── bloom_combine.frag     # Bloom combination
├── color_grading.frag     # Color grading
└── lighting.frag          # Dynamic lighting
```

## Dependencies

### Required Libraries
- **SDL2**: Window management and OpenGL context
- **OpenGL 3.3+**: Graphics API
- **GLEW**: OpenGL extension loading
- **GLM**: Mathematics library
- **stb_image**: Image loading (single-header, included)

### Header-Only Libraries (to include)
You'll need to download `stb_image.h`:
```cpp
// Add to your project
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```

Download from: https://github.com/nothings/stb/blob/master/stb_image.h

## Performance Statistics

The renderer tracks the following statistics:
- Draw calls per frame
- Sprites rendered
- Particles rendered
- Frame time (ms)

Access via:
```cpp
const auto& stats = renderer.GetStats();
std::cout << "FPS: " << (1.0f / stats.frameTime) << std::endl;
```

## Advanced Features

### Custom Post-Processing Effects
Extend `PostProcessEffect` to create custom effects:

```cpp
class CustomEffect : public PostProcessEffect {
public:
    void Apply(std::shared_ptr<Texture> input,
               std::shared_ptr<Framebuffer> output) override {
        // Your effect implementation
    }
};
```

### Custom Particle Behavior
```cpp
emitter->SetCustomUpdate([](Particle& p, float dt) {
    // Custom particle update logic
    p.velocity.x += std::sin(p.age) * 10.0f * dt;
});
```

### Render-to-Texture
```cpp
auto framebuffer = std::make_shared<Framebuffer>();
framebuffer->Create(512, 512, true);

framebuffer->Bind();
// Render here
framebuffer->Unbind();

// Use framebuffer texture
auto texture = framebuffer->GetColorTexture();
```

## Best Practices

1. **Texture Management**: Use TextureManager for caching
2. **Sprite Sorting**: Sort sprites by texture to maximize batching
3. **Layer Organization**: Group similar objects in same layer
4. **Camera Updates**: Update camera before rendering
5. **Particle Limits**: Set reasonable max particles based on target hardware
6. **Shader Compilation**: Check for shader errors during initialization
7. **Alpha Blending**: Use premultiplied alpha for correct blending

## Troubleshooting

### Black Screen
- Check shader compilation errors
- Verify textures are loaded correctly
- Ensure camera is properly configured
- Check viewport settings

### Poor Performance
- Reduce max particles
- Disable post-processing effects
- Check for texture thrashing (too many texture swaps)
- Profile draw calls

### Shader Errors
- Verify GLSL version compatibility
- Check uniform variable names
- Ensure shaders are loaded before use

## License

This rendering system is part of the ARPG game project. See LICENSE for details.

## Credits

- Uses stb_image by Sean Barrett for image loading
- Built with SDL2, OpenGL, and GLEW
- Mathematics powered by GLM

## Future Enhancements

- [ ] Normal mapping for lighting
- [ ] Shadow casting system
- [ ] Tilemap rendering optimization
- [ ] GPU particle system with compute shaders
- [ ] Deferred rendering pipeline
- [ ] SSAO (Screen Space Ambient Occlusion)
- [ ] Motion blur effect
- [ ] Distortion effects
- [ ] Text rendering system

## Contact

For questions or contributions, please open an issue on the project repository.
