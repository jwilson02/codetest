#include "Renderer.h"
#include <iostream>
#include <SDL2/SDL.h>

namespace Rendering {

// Framebuffer implementation
Framebuffer::Framebuffer()
    : m_FBO(0), m_RBO(0), m_Width(0), m_Height(0), m_UseDepth(false) {
}

Framebuffer::~Framebuffer() {
    if (m_FBO != 0) {
        glDeleteFramebuffers(1, &m_FBO);
    }
    if (m_RBO != 0) {
        glDeleteRenderbuffers(1, &m_RBO);
    }
}

bool Framebuffer::Create(int width, int height, bool useDepth) {
    m_Width = width;
    m_Height = height;
    m_UseDepth = useDepth;

    // Create framebuffer
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // Create color texture
    m_ColorTexture = std::make_shared<Texture>();
    m_ColorTexture->Create(width, height, GL_RGBA, GL_RGBA8);
    m_ColorTexture->SetFilterMode(GL_LINEAR, GL_LINEAR);
    m_ColorTexture->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    // Attach color texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          m_ColorTexture->GetID(), 0);

    if (useDepth) {
        // Create depth texture
        m_DepthTexture = std::make_shared<Texture>();
        m_DepthTexture->Create(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24);
        m_DepthTexture->SetFilterMode(GL_NEAREST, GL_NEAREST);
        m_DepthTexture->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        // Attach depth texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                              m_DepthTexture->GetID(), 0);
    }

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// BloomEffect implementation
BloomEffect::BloomEffect()
    : m_Threshold(1.0f)
    , m_Intensity(1.0f)
    , m_BlurPasses(5)
    , m_Width(0)
    , m_Height(0)
    , m_QuadVAO(0)
    , m_QuadVBO(0) {
}

bool BloomEffect::Initialize(int width, int height) {
    m_Width = width;
    m_Height = height;

    // Create framebuffers
    m_BrightPassFBO = std::make_shared<Framebuffer>();
    m_BrightPassFBO->Create(width / 2, height / 2, false);

    m_BlurFBO1 = std::make_shared<Framebuffer>();
    m_BlurFBO1->Create(width / 2, height / 2, false);

    m_BlurFBO2 = std::make_shared<Framebuffer>();
    m_BlurFBO2->Create(width / 2, height / 2, false);

    // Initialize shaders (in a real implementation, load from files)
    // For now, we'll leave shader creation to the user

    // Create quad for rendering
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

void BloomEffect::Apply(std::shared_ptr<Texture> input, std::shared_ptr<Framebuffer> output) {
    if (!m_Enabled || !m_BrightPassShader || !m_BlurShader || !m_CombineShader) {
        return;
    }

    // Bright pass
    m_BrightPassFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    m_BrightPassShader->Use();
    m_BrightPassShader->SetFloat("u_Threshold", m_Threshold);
    input->Bind(0);
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Blur passes
    bool horizontal = true;
    for (int i = 0; i < m_BlurPasses * 2; i++) {
        auto targetFBO = horizontal ? m_BlurFBO1 : m_BlurFBO2;
        auto sourceTex = horizontal ? m_BrightPassFBO->GetColorTexture() : m_BlurFBO1->GetColorTexture();
        if (i > 0) {
            sourceTex = horizontal ? m_BlurFBO2->GetColorTexture() : m_BlurFBO1->GetColorTexture();
        }

        targetFBO->Bind();
        m_BlurShader->Use();
        m_BlurShader->SetBool("u_Horizontal", horizontal);
        sourceTex->Bind(0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        horizontal = !horizontal;
    }

    // Combine
    output->Bind();
    m_CombineShader->Use();
    m_CombineShader->SetFloat("u_Intensity", m_Intensity);
    input->Bind(0);
    m_BlurFBO2->GetColorTexture()->Bind(1);
    m_CombineShader->SetInt("u_Scene", 0);
    m_CombineShader->SetInt("u_BloomBlur", 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// ColorGradingEffect implementation
ColorGradingEffect::ColorGradingEffect()
    : m_Brightness(0.0f)
    , m_Contrast(1.0f)
    , m_Saturation(1.0f)
    , m_Tint(1.0f)
    , m_QuadVAO(0)
    , m_QuadVBO(0) {
}

bool ColorGradingEffect::Initialize() {
    // Create quad for rendering
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

void ColorGradingEffect::Apply(std::shared_ptr<Texture> input, std::shared_ptr<Framebuffer> output) {
    if (!m_Enabled || !m_Shader) {
        return;
    }

    output->Bind();
    m_Shader->Use();
    m_Shader->SetFloat("u_Brightness", m_Brightness);
    m_Shader->SetFloat("u_Contrast", m_Contrast);
    m_Shader->SetFloat("u_Saturation", m_Saturation);
    m_Shader->SetVec3("u_Tint", m_Tint);
    input->Bind(0);
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Renderer implementation
Renderer& Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

Renderer::Renderer()
    : m_Window(nullptr)
    , m_GLContext(nullptr)
    , m_SpriteBatch(10000)
    , m_CurrentLayer(RenderLayer::Game)
    , m_Viewport(0, 0, 800, 600)
    , m_PostProcessingEnabled(false)
    , m_AmbientColor(1.0f)
    , m_AmbientIntensity(0.3f)
    , m_FullScreenQuadVAO(0)
    , m_FullScreenQuadVBO(0)
    , m_FrameStartTime(0.0f) {

    m_Stats = {};
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(SDL_Window* window) {
    m_Window = window;

    if (!InitializeGL()) {
        return false;
    }

    if (!InitializeShaders()) {
        std::cerr << "Warning: Some shaders failed to initialize" << std::endl;
    }

    if (!InitializeBuffers()) {
        return false;
    }

    // Initialize sprite batch
    if (!m_SpriteBatch.Initialize()) {
        std::cerr << "Failed to initialize sprite batch" << std::endl;
        return false;
    }

    // Initialize particle system
    if (!m_ParticleSystem.Initialize()) {
        std::cerr << "Failed to initialize particle system" << std::endl;
        return false;
    }

    // Get window size
    int width, height;
    SDL_GetWindowSize(m_Window, &width, &height);
    m_Viewport = glm::ivec4(0, 0, width, height);

    // Create default camera
    m_Camera = std::make_shared<Camera>(static_cast<float>(width), static_cast<float>(height));

    // Create framebuffers for post-processing
    m_MainFBO = std::make_shared<Framebuffer>();
    m_MainFBO->Create(width, height, true);

    m_TempFBO = std::make_shared<Framebuffer>();
    m_TempFBO->Create(width, height, true);

    InitializeFullScreenQuad();

    std::cout << "Renderer initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

void Renderer::Shutdown() {
    if (m_FullScreenQuadVAO != 0) {
        glDeleteVertexArrays(1, &m_FullScreenQuadVAO);
        glDeleteBuffers(1, &m_FullScreenQuadVBO);
    }

    if (m_GLContext) {
        SDL_GL_DeleteContext(m_GLContext);
        m_GLContext = nullptr;
    }
}

void Renderer::BeginFrame() {
    m_FrameStartTime = SDL_GetTicks() / 1000.0f;
    m_Stats = {};

    if (m_PostProcessingEnabled && m_MainFBO) {
        m_MainFBO->Bind();
    }
}

void Renderer::EndFrame() {
    // Apply post-processing if enabled
    if (m_PostProcessingEnabled && !m_PostProcessEffects.empty()) {
        ApplyPostProcessing();
    }

    // Present
    SDL_GL_SwapWindow(m_Window);

    // Update statistics
    float frameEndTime = SDL_GetTicks() / 1000.0f;
    m_Stats.frameTime = frameEndTime - m_FrameStartTime;
    m_Stats.drawCalls = m_SpriteBatch.GetDrawCallCount();
    m_Stats.spritesRendered = m_SpriteBatch.GetSpriteCount();
}

void Renderer::Clear(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    m_Viewport = glm::ivec4(x, y, width, height);
    glViewport(x, y, width, height);
}

void Renderer::AddPostProcessEffect(std::shared_ptr<PostProcessEffect> effect) {
    if (effect) {
        m_PostProcessEffects.push_back(effect);
    }
}

void Renderer::ClearPostProcessEffects() {
    m_PostProcessEffects.clear();
}

void Renderer::AddLight(const Light& light) {
    m_Lights.push_back(light);
}

void Renderer::ClearLights() {
    m_Lights.clear();
}

void Renderer::SetAmbientLight(const glm::vec3& color, float intensity) {
    m_AmbientColor = color;
    m_AmbientIntensity = intensity;
}

void Renderer::DrawFullScreenQuad() {
    glBindVertexArray(m_FullScreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

glm::ivec2 Renderer::GetWindowSize() const {
    int width, height;
    SDL_GetWindowSize(m_Window, &width, &height);
    return glm::ivec2(width, height);
}

bool Renderer::InitializeGL() {
    // Create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_GLContext = SDL_GL_CreateContext(m_Window);
    if (!m_GLContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        return false;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    // Set OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    return true;
}

bool Renderer::InitializeShaders() {
    // Shaders will be loaded by the application
    // This is just a placeholder for any default shaders
    return true;
}

bool Renderer::InitializeBuffers() {
    // Initialize any global buffers here
    return true;
}

void Renderer::InitializeFullScreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_FullScreenQuadVAO);
    glGenBuffers(1, &m_FullScreenQuadVBO);
    glBindVertexArray(m_FullScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_FullScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::RenderLighting() {
    // Simple 2D lighting implementation
    // This would be more complex in a full implementation
    if (!m_LightingShader || m_Lights.empty()) {
        return;
    }

    m_LightingShader->Use();
    m_LightingShader->SetVec3("u_AmbientColor", m_AmbientColor);
    m_LightingShader->SetFloat("u_AmbientIntensity", m_AmbientIntensity);
    m_LightingShader->SetInt("u_LightCount", std::min(static_cast<int>(m_Lights.size()), 32));

    // Upload light data
    for (size_t i = 0; i < std::min(m_Lights.size(), size_t(32)); i++) {
        std::string base = "u_Lights[" + std::to_string(i) + "]";
        m_LightingShader->SetVec2(base + ".position", m_Lights[i].position);
        m_LightingShader->SetVec3(base + ".color", m_Lights[i].color);
        m_LightingShader->SetFloat(base + ".intensity", m_Lights[i].intensity);
        m_LightingShader->SetFloat(base + ".radius", m_Lights[i].radius);
    }
}

void Renderer::ApplyPostProcessing() {
    if (m_PostProcessEffects.empty()) {
        return;
    }

    auto source = m_MainFBO;
    auto target = m_TempFBO;

    for (size_t i = 0; i < m_PostProcessEffects.size(); i++) {
        auto& effect = m_PostProcessEffects[i];
        if (!effect->IsEnabled()) {
            continue;
        }

        // Last effect renders to screen
        if (i == m_PostProcessEffects.size() - 1) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(m_Viewport.x, m_Viewport.y, m_Viewport.z, m_Viewport.w);
        }

        effect->Apply(source->GetColorTexture(), target);

        // Swap buffers
        std::swap(source, target);
    }
}

} // namespace Rendering
