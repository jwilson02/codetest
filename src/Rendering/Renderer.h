#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Sprite.h"
#include "Animation.h"
#include "ParticleSystem.h"

namespace Rendering {

/**
 * Render layer for organizing draw calls
 */
enum class RenderLayer {
    Background = 0,
    Game = 1,
    Foreground = 2,
    UI = 3,
    COUNT
};

/**
 * Framebuffer for render-to-texture
 */
class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    bool Create(int width, int height, bool useDepth = true);
    void Bind();
    void Unbind();

    std::shared_ptr<Texture> GetColorTexture() const { return m_ColorTexture; }
    std::shared_ptr<Texture> GetDepthTexture() const { return m_DepthTexture; }

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    GLuint m_FBO;
    GLuint m_RBO;
    std::shared_ptr<Texture> m_ColorTexture;
    std::shared_ptr<Texture> m_DepthTexture;
    int m_Width;
    int m_Height;
    bool m_UseDepth;
};

/**
 * Post-processing effect
 */
class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;
    virtual void Apply(std::shared_ptr<Texture> input, std::shared_ptr<Framebuffer> output) = 0;
    virtual void SetEnabled(bool enabled) { m_Enabled = enabled; }
    virtual bool IsEnabled() const { return m_Enabled; }

protected:
    bool m_Enabled = true;
};

/**
 * Bloom post-processing effect
 */
class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();
    bool Initialize(int width, int height);
    void Apply(std::shared_ptr<Texture> input, std::shared_ptr<Framebuffer> output) override;

    void SetThreshold(float threshold) { m_Threshold = threshold; }
    void SetIntensity(float intensity) { m_Intensity = intensity; }
    void SetBlurPasses(int passes) { m_BlurPasses = passes; }

private:
    std::shared_ptr<Shader> m_BrightPassShader;
    std::shared_ptr<Shader> m_BlurShader;
    std::shared_ptr<Shader> m_CombineShader;
    std::shared_ptr<Framebuffer> m_BrightPassFBO;
    std::shared_ptr<Framebuffer> m_BlurFBO1;
    std::shared_ptr<Framebuffer> m_BlurFBO2;

    float m_Threshold;
    float m_Intensity;
    int m_BlurPasses;
    int m_Width, m_Height;

    GLuint m_QuadVAO;
    GLuint m_QuadVBO;
};

/**
 * Color grading effect
 */
class ColorGradingEffect : public PostProcessEffect {
public:
    ColorGradingEffect();
    bool Initialize();
    void Apply(std::shared_ptr<Texture> input, std::shared_ptr<Framebuffer> output) override;

    void SetBrightness(float brightness) { m_Brightness = brightness; }
    void SetContrast(float contrast) { m_Contrast = contrast; }
    void SetSaturation(float saturation) { m_Saturation = saturation; }
    void SetTint(const glm::vec3& tint) { m_Tint = tint; }

private:
    std::shared_ptr<Shader> m_Shader;
    float m_Brightness;
    float m_Contrast;
    float m_Saturation;
    glm::vec3 m_Tint;

    GLuint m_QuadVAO;
    GLuint m_QuadVBO;
};

/**
 * Main renderer class
 */
class Renderer {
public:
    static Renderer& GetInstance();

    // Initialize renderer
    bool Initialize(SDL_Window* window);

    // Shutdown renderer
    void Shutdown();

    // Frame management
    void BeginFrame();
    void EndFrame();

    // Clear screen
    void Clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    // Set render layer
    void SetRenderLayer(RenderLayer layer) { m_CurrentLayer = layer; }
    RenderLayer GetRenderLayer() const { return m_CurrentLayer; }

    // Camera
    void SetCamera(std::shared_ptr<Camera> camera) { m_Camera = camera; }
    std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    // Sprite batch
    SpriteBatch& GetSpriteBatch() { return m_SpriteBatch; }

    // Particle system
    ParticleSystemManager& GetParticleSystem() { return m_ParticleSystem; }

    // Viewport
    void SetViewport(int x, int y, int width, int height);
    glm::ivec4 GetViewport() const { return m_Viewport; }

    // Post-processing
    void AddPostProcessEffect(std::shared_ptr<PostProcessEffect> effect);
    void ClearPostProcessEffects();
    void SetPostProcessingEnabled(bool enabled) { m_PostProcessingEnabled = enabled; }
    bool IsPostProcessingEnabled() const { return m_PostProcessingEnabled; }

    // Lighting (simple 2D lighting)
    struct Light {
        glm::vec2 position;
        glm::vec3 color;
        float intensity;
        float radius;

        Light() : position(0.0f), color(1.0f), intensity(1.0f), radius(100.0f) {}
    };

    void AddLight(const Light& light);
    void ClearLights();
    void SetAmbientLight(const glm::vec3& color, float intensity);

    // Render statistics
    struct RenderStats {
        size_t drawCalls;
        size_t spritesRendered;
        size_t particlesRendered;
        float frameTime;
    };

    const RenderStats& GetStats() const { return m_Stats; }

    // Utility - draw full screen quad
    void DrawFullScreenQuad();

    // Get window size
    glm::ivec2 GetWindowSize() const;

private:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    SDL_Window* m_Window;
    SDL_GLContext m_GLContext;

    // Core components
    std::shared_ptr<Camera> m_Camera;
    SpriteBatch m_SpriteBatch;
    ParticleSystemManager m_ParticleSystem;

    // Rendering state
    RenderLayer m_CurrentLayer;
    glm::ivec4 m_Viewport;

    // Post-processing
    bool m_PostProcessingEnabled;
    std::vector<std::shared_ptr<PostProcessEffect>> m_PostProcessEffects;
    std::shared_ptr<Framebuffer> m_MainFBO;
    std::shared_ptr<Framebuffer> m_TempFBO;

    // Lighting
    std::vector<Light> m_Lights;
    glm::vec3 m_AmbientColor;
    float m_AmbientIntensity;
    std::shared_ptr<Shader> m_LightingShader;

    // Full screen quad for post-processing
    GLuint m_FullScreenQuadVAO;
    GLuint m_FullScreenQuadVBO;

    // Statistics
    RenderStats m_Stats;
    float m_FrameStartTime;

    // Initialization helpers
    bool InitializeGL();
    bool InitializeShaders();
    bool InitializeBuffers();
    void InitializeFullScreenQuad();

    // Rendering helpers
    void RenderLighting();
    void ApplyPostProcessing();
};

} // namespace Rendering
