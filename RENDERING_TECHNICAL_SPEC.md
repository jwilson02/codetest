# 2D Rendering System - Technical Specification

## Overview

High-performance 2D rendering engine for ARPG games using modern OpenGL (3.3+) with SDL2. Designed for rendering thousands of sprites with minimal draw calls through intelligent batching.

## Architecture

### Design Patterns

1. **Singleton Pattern**: Renderer class for global access
2. **Object Pool Pattern**: Particles reuse inactive instances
3. **Command Pattern**: Render layers organize draw calls
4. **Observer Pattern**: Animation events and callbacks
5. **Factory Pattern**: Texture manager for resource caching

### Core Components

```
┌─────────────────────────────────────────────────────────┐
│                        Renderer                         │
│  - Frame management                                     │
│  - Layer organization                                   │
│  - Post-processing pipeline                             │
│  - Statistics tracking                                  │
└─────────────────┬───────────────────────────────────────┘
                  │
      ┌───────────┼───────────┬───────────────┐
      │           │           │               │
┌─────▼─────┐ ┌──▼────────┐ ┌▼──────────┐ ┌─▼──────────┐
│  Camera   │ │SpriteBatch│ │ Particle  │ │Post-Process│
│           │ │           │ │  System   │ │  Effects   │
└───────────┘ └───────────┘ └───────────┘ └────────────┘
```

## Rendering Pipeline

### Frame Flow

```
1. BeginFrame()
   ├── Bind main framebuffer (if post-processing enabled)
   └── Reset statistics

2. Render Layers (Background → Game → Foreground → UI)
   ├── Set layer
   ├── SpriteBatch.Begin()
   ├── DrawSprite() calls (batched)
   ├── SpriteBatch.End()
   └── Flush batches

3. Apply Post-Processing
   ├── For each effect:
   │   ├── Bind target framebuffer
   │   ├── Apply effect shader
   │   └── Draw full-screen quad
   └── Final render to screen

4. EndFrame()
   ├── Swap buffers
   └── Update statistics
```

### Sprite Batching Algorithm

```cpp
// Pseudo-code for batching
Begin():
    Clear vertex buffer
    Clear texture slots
    Reset sprite count

DrawSprite(sprite):
    texIndex = GetTextureIndex(sprite.texture)
    if texIndex < 0 OR sprite_count >= MAX_SPRITES:
        Flush()  // Current batch is full
        texIndex = GetTextureIndex(sprite.texture)

    AddQuad(sprite, texIndex)
    sprite_count++

End():
    if sprite_count > 0:
        Flush()

Flush():
    Upload vertices to GPU
    Bind all textures
    Set shader uniforms
    glDrawElements()
    Clear buffers
```

## Memory Management

### Vertex Buffer Layout

```cpp
struct SpriteVertex {
    vec3 position;    // 12 bytes
    vec2 texCoord;    // 8 bytes
    vec4 color;       // 16 bytes
    float texIndex;   // 4 bytes
    // Total: 40 bytes per vertex
    // Per sprite: 160 bytes (4 vertices)
};

// Default capacity: 10,000 sprites
// Memory usage: 10,000 * 160 = 1.6 MB
```

### Index Buffer

```cpp
// Indices for quad (2 triangles)
GLuint indices[6] = {0, 1, 2, 2, 3, 0};

// For batch of N sprites:
// Index count: N * 6
// Memory: N * 6 * 4 bytes = 24N bytes
```

### Texture Slots

```cpp
// Maximum texture units: 32
// Texture IDs cached per batch
// Minimizes glBindTexture() calls
```

## Performance Optimizations

### Sprite Batching

**Problem**: Each draw call has overhead (~100-1000 CPU cycles)

**Solution**: Batch multiple sprites into single draw call

**Benefits**:
- Reduced draw calls: 10,000 sprites → ~10-20 draw calls
- Better GPU utilization
- Reduced CPU overhead

### Multi-Texture Batching

**Problem**: Texture switching is expensive

**Solution**: Bind multiple textures simultaneously, use index in shader

**Implementation**:
```glsl
// In fragment shader
int texIndex = int(v_TexIndex);
vec4 color = texture(u_Textures[texIndex], v_TexCoord);
```

**Limitations**:
- Maximum 32 texture units (hardware dependent)
- Batch flushes when texture slots full

### Depth Testing

**Z-ordering**: Sprites sorted by depth value (back to front)

```cpp
sprite.depth = layerDepth + objectDepth;
// layerDepth: 0.0 (background) to 3.0 (UI)
// objectDepth: 0.0 to 1.0 within layer
```

### Particle System Optimization

1. **Object Pooling**: Pre-allocate particle array, reuse inactive particles
2. **GPU Upload**: Single buffer upload per emitter
3. **Culling**: Skip rendering for inactive particles
4. **Batch Rendering**: All particles in single draw call

## Shader System

### Shader Stages

```
Vertex Shader
    ↓
[Geometry Shader] (optional)
    ↓
Fragment Shader
    ↓
Framebuffer
```

### Uniform Caching

```cpp
// Cache uniform locations to avoid glGetUniformLocation() calls
std::unordered_map<std::string, GLint> m_UniformLocationCache;
```

### Shader Hot-Reloading (Future)

```cpp
// Watch shader files for changes
// Recompile and reload without restart
```

## Camera System

### Projection Matrix

Orthographic projection for 2D:

```cpp
projection = glm::ortho(
    -halfWidth, halfWidth,
    -halfHeight, halfHeight,
    -1.0f, 1.0f
);
```

### View Matrix

```cpp
view = translate(-camera.position) *
       rotate(camera.rotation) *
       scale(camera.zoom);
```

### Smooth Following

Exponential interpolation:

```cpp
position += (target - position) * followSpeed * deltaTime;
// followSpeed: 0.0 (no follow) to 1.0 (instant)
```

### Screen Shake

Perlin noise or random offset:

```cpp
shakeOffset = randomDirection * intensity * (1.0 - progress);
effectivePosition = position + shakeOffset;
```

## Animation System

### Frame Timing

```cpp
// Accumulator-based timing
frameTimer += deltaTime * speedMultiplier;
if (frameTimer >= currentFrame.duration) {
    frameTimer -= currentFrame.duration;
    AdvanceToNextFrame();
}
```

### UV Coordinate Calculation

For sprite sheet with grid:

```cpp
cellWidth = 1.0f / columns;
cellHeight = 1.0f / rows;

uvRect = vec4(
    (frameIndex % columns) * cellWidth,
    (frameIndex / columns) * cellHeight,
    cellWidth,
    cellHeight
);
```

## Particle System

### Particle Update

```cpp
// Euler integration
velocity += acceleration * dt;
position += velocity * dt;
color += colorDelta * dt;
size += sizeDelta * dt;
rotation += angularVelocity * dt;
age += dt;

if (age >= lifetime) {
    particle.active = false;
}
```

### Emission

```cpp
// Continuous emission
emissionAccumulator += dt * emissionRate;
int particlesToEmit = floor(emissionAccumulator);
emissionAccumulator -= particlesToEmit;

// Burst emission
Emit(count)
```

### Rendering

```cpp
// Build vertex buffer with active particles
for each active particle:
    Create quad with particle properties
    Add to vertex buffer

// Upload and draw
glBufferSubData(vertices)
glDrawArrays(GL_QUADS, ...)
```

## Post-Processing

### Render-to-Texture

```cpp
// 1. Create framebuffer
glGenFramebuffers(1, &fbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// 2. Attach texture
glFramebufferTexture2D(GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

// 3. Render to FBO
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
// ... render scene ...

// 4. Use texture for post-processing
glBindTexture(GL_TEXTURE_2D, texture);
```

### Bloom Effect

**Algorithm**:
1. Bright pass: Extract pixels above threshold
2. Gaussian blur: Blur bright pixels (multiple passes)
3. Combine: Add blurred result to original scene
4. Tone mapping: HDR to LDR conversion

**Performance**:
- Use half-resolution framebuffers
- Separable gaussian blur (horizontal then vertical)
- 5-10 blur passes typical

### Color Grading

**Operations**:
```glsl
// Brightness
color = color + brightness;

// Contrast
color = (color - 0.5) * contrast + 0.5;

// Saturation
float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
color = mix(vec3(luminance), color, saturation);

// Tint
color *= tint;
```

## Lighting System

### 2D Point Light

**Attenuation**:
```glsl
float distance = length(fragPos - lightPos);
float attenuation = intensity / (1.0 + distance² / radius²);
float falloff = smoothstep(radius, radius * 0.5, distance);
attenuation *= falloff;
```

**Color Blending**:
```glsl
vec3 lighting = ambientColor * ambientIntensity;
for (int i = 0; i < lightCount; i++) {
    lighting += lights[i].color * CalculateAttenuation(lights[i]);
}
finalColor = textureColor * lighting;
```

## OpenGL State Management

### State Changes

Minimize state changes for performance:

```cpp
// Expensive operations (minimize):
- glBindTexture()
- glUseProgram()
- glBindVertexArray()
- glEnable/Disable()

// Strategy:
- Batch by texture
- Batch by shader
- Set state once per frame
```

### Depth Testing

```cpp
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);  // Less or equal for 2D layering
```

### Alpha Blending

```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// For premultiplied alpha:
glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
```

## Error Handling

### OpenGL Error Checking

```cpp
void CheckGLError(const char* location) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error at " << location
                  << ": " << error << std::endl;
    }
}

// Use in debug builds
#ifdef DEBUG
    #define GL_CHECK(x) x; CheckGLError(#x)
#else
    #define GL_CHECK(x) x
#endif
```

### Shader Compilation

```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
    // Handle error
}
```

## Platform Considerations

### OpenGL Version

- **Minimum**: OpenGL 3.3 Core
- **Target**: OpenGL 4.5
- **Fallback**: OpenGL 3.0 ES for mobile

### SDL2 Configuration

```cpp
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
```

### VSync

```cpp
SDL_GL_SetSwapInterval(1);  // Enable VSync
SDL_GL_SetSwapInterval(0);  // Disable VSync
```

## Performance Benchmarks

### Target Performance

- **Sprites**: 10,000+ at 60 FPS
- **Particles**: 5,000+ at 60 FPS
- **Draw Calls**: < 50 per frame
- **Frame Time**: < 16.67ms (60 FPS)

### Profiling Points

1. Sprite batch flush count
2. Texture binding count
3. Shader switching count
4. Vertex upload size
5. GPU memory usage

### Optimization Checklist

- [ ] Batch sprites by texture
- [ ] Use texture atlases
- [ ] Cull off-screen objects
- [ ] Limit particle count
- [ ] Profile with GPU profiler
- [ ] Minimize state changes
- [ ] Use VBOs efficiently
- [ ] Cache uniform locations

## Future Enhancements

### Phase 2
- Instanced rendering for particles
- Compute shaders for particle updates
- Deferred rendering pipeline
- Normal mapping

### Phase 3
- GPU-driven rendering
- Indirect drawing
- Tile-based rendering
- Occlusion culling

### Phase 4
- Vulkan backend
- Multi-threaded rendering
- Ray-traced shadows
- Global illumination

## References

### OpenGL
- OpenGL 3.3 Core Specification
- OpenGL Shading Language 3.30 Specification

### Libraries
- SDL2: https://www.libsdl.org/
- GLEW: http://glew.sourceforge.net/
- GLM: https://glm.g-truc.net/
- stb_image: https://github.com/nothings/stb

### Best Practices
- "OpenGL SuperBible" (7th Edition)
- "Real-Time Rendering" (4th Edition)
- Khronos OpenGL Wiki

## Appendix

### Coordinate Systems

```
Screen Space: (0,0) top-left, (width, height) bottom-right
World Space:  Arbitrary units, camera-relative
Clip Space:   (-1,-1) to (1,1)
UV Space:     (0,0) bottom-left, (1,1) top-right
```

### Data Sizes

```
float: 4 bytes
vec2:  8 bytes
vec3:  12 bytes
vec4:  16 bytes
mat4:  64 bytes
```

### OpenGL Types

```cpp
GLuint   - unsigned int (IDs)
GLint    - signed int
GLfloat  - float
GLsizei  - size type
GLenum   - enumeration
```

---

**Version**: 1.0
**Last Updated**: 2025-11-13
**Author**: ARPG Rendering Team
