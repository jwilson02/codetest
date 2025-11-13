# 2D Rendering System - Quick Start Guide

## Minimal Example (5 Minutes)

```cpp
#include "Renderer.h"
#include <SDL2/SDL.h>

int main() {
    // 1. Initialize SDL and create window
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_OPENGL);

    // 2. Initialize renderer
    auto& renderer = Rendering::Renderer::GetInstance();
    renderer.Initialize(window);

    // 3. Load shader
    auto shader = std::make_shared<Rendering::Shader>();
    shader->LoadFromFiles("assets/shaders/sprite.vert",
                         "assets/shaders/sprite.frag");
    renderer.GetSpriteBatch().SetShader(shader);

    // 4. Load texture
    auto texture = Rendering::TextureManager::GetInstance()
        .Load("assets/textures/sprite.png");

    // 5. Game loop
    bool running = true;
    while (running) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Render
        renderer.BeginFrame();
        renderer.Clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

        auto& batch = renderer.GetSpriteBatch();
        batch.SetViewProjectionMatrix(
            renderer.GetCamera()->GetViewProjectionMatrix());

        batch.Begin();
        batch.DrawSprite(texture, glm::vec2(100, 100), glm::vec2(64, 64));
        batch.End();

        renderer.EndFrame();
    }

    // 6. Cleanup
    renderer.Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

## Common Tasks

### Load and Draw a Sprite
```cpp
auto texture = TextureManager::GetInstance().Load("player.png");
batch.DrawSprite(texture, position, size, color, rotation, depth);
```

### Animate a Sprite
```cpp
// Create sprite sheet (8x4 grid)
SpriteSheet sheet(texture, 8, 4);
auto anim = sheet.CreateAnimation("walk", 0, 7, 0.1f, true);

AnimationController controller;
controller.AddClip("walk", anim);
controller.Play("walk");

// In update loop
controller.Update(deltaTime);
glm::vec4 uv = controller.GetCurrentFrameUV();
batch.DrawSprite(texture, pos, size, uv);
```

### Create Particle Effect
```cpp
ParticleEmitterConfig config;
config.emissionRate = 50.0f;
config.maxParticles = 500;
config.startColor = glm::vec4(1, 0.5, 0, 1);
config.endColor = glm::vec4(1, 0, 0, 0);

auto emitter = renderer.GetParticleSystem().CreateEmitter(config);
emitter->SetPosition(glm::vec2(400, 300));
emitter->Play();
```

### Control Camera
```cpp
auto camera = renderer.GetCamera();
camera->SetTarget(playerPos);
camera->SetZoom(2.0f);
camera->Shake(10.0f, 0.5f);
camera->Update(deltaTime);
```

### Add Lighting
```cpp
renderer.SetAmbientLight(glm::vec3(0.3f), 0.5f);

Renderer::Light light;
light.position = glm::vec2(400, 300);
light.color = glm::vec3(1, 0.9, 0.7);
light.intensity = 2.0f;
light.radius = 200.0f;
renderer.AddLight(light);
```

### Enable Post-Processing
```cpp
auto bloom = std::make_shared<BloomEffect>();
bloom->Initialize(1280, 720);
renderer.AddPostProcessEffect(bloom);
renderer.SetPostProcessingEnabled(true);
```

## Cheat Sheet

### Renderer
```cpp
Renderer::GetInstance().Initialize(window);
renderer.BeginFrame();
renderer.Clear(color);
renderer.SetRenderLayer(RenderLayer::Game);
renderer.EndFrame();
```

### SpriteBatch
```cpp
batch.SetShader(shader);
batch.SetViewProjectionMatrix(vp);
batch.Begin();
batch.DrawSprite(texture, pos, size);
batch.End();
```

### Camera
```cpp
camera->SetPosition(pos);
camera->SetTarget(pos);
camera->SetZoom(2.0f);
camera->Shake(intensity, duration);
camera->Update(deltaTime);
```

### Animation
```cpp
controller.AddClip(name, clip);
controller.Play(name);
controller.Update(deltaTime);
controller.GetCurrentFrameUV();
```

### Particles
```cpp
emitter->SetPosition(pos);
emitter->Play();
emitter->Stop();
emitter->Burst(count);
```

### Texture
```cpp
TextureManager::GetInstance().Load(path);
texture->Bind(unit);
texture->SetFilterMode(min, mag);
```

### Shader
```cpp
shader->LoadFromFiles(vertPath, fragPath);
shader->Use();
shader->SetInt(name, value);
shader->SetFloat(name, value);
shader->SetVec2(name, value);
shader->SetMat4(name, value);
```

## Performance Tips

1. **Batch sprites by texture** - Minimize texture switches
2. **Use texture atlases** - Combine multiple textures
3. **Limit particles** - Set reasonable max particles
4. **Update only visible objects** - Cull off-screen objects
5. **Profile regularly** - Check renderer.GetStats()

## Common Issues

### Sprites not visible
- Check camera position and zoom
- Verify shader is loaded
- Check sprite depth/z-order

### Poor performance
- Too many draw calls? → Use batching
- Too many particles? → Reduce maxParticles
- Texture thrashing? → Use texture atlas

### Shader errors
- Check file paths are correct
- Verify GLSL version (3.3+)
- Check uniform names match

## Next Steps

1. Read full documentation in `RENDERING_README.md`
2. Explore `RenderingExample.cpp` for complete example
3. Experiment with post-processing effects
4. Create custom particle effects
5. Add your game logic!

## File Locations

### Source Files
```
src/Rendering/
  ├── Renderer.h/cpp
  ├── Shader.h/cpp
  ├── Texture.h/cpp
  ├── Camera.h/cpp
  ├── Sprite.h/cpp
  ├── Animation.h/cpp
  └── ParticleSystem.h/cpp
```

### Shaders
```
assets/shaders/
  ├── sprite.vert/frag
  ├── particle.vert/frag
  ├── bloom_bright.frag
  ├── blur.frag
  ├── bloom_combine.frag
  ├── color_grading.frag
  ├── lighting.frag
  └── fullscreen.vert
```

## Building

```bash
# Linux
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Run example
./RenderingExample
```

## Dependencies

- SDL2 - Window/Input
- OpenGL 3.3+ - Graphics
- GLEW - OpenGL Extensions
- GLM - Math
- stb_image.h - Image Loading

Happy rendering! 🎮
