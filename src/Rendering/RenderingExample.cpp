/**
 * Example usage of the 2D Rendering System for ARPG
 *
 * This file demonstrates how to use all the rendering components:
 * - Sprite rendering with batching
 * - Animation system
 * - Particle effects
 * - Camera with smooth following
 * - Lighting system
 * - Post-processing effects
 */

#include "Renderer.h"
#include "Sprite.h"
#include "Animation.h"
#include "ParticleSystem.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

using namespace Rendering;

class Game {
public:
    Game() : m_Running(false), m_Window(nullptr) {}

    bool Initialize() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create window
        m_Window = SDL_CreateWindow(
            "ARPG Rendering Example",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
        );

        if (!m_Window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }

        // Initialize renderer
        if (!Renderer::GetInstance().Initialize(m_Window)) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // Load shaders
        LoadShaders();

        // Setup camera
        SetupCamera();

        // Load textures and setup sprites
        LoadAssets();

        // Setup animations
        SetupAnimations();

        // Create particle effects
        SetupParticles();

        // Setup lighting
        SetupLighting();

        // Setup post-processing
        SetupPostProcessing();

        m_Running = true;
        return true;
    }

    void Run() {
        Uint32 lastTime = SDL_GetTicks();
        float deltaTime = 0.0f;

        while (m_Running) {
            // Calculate delta time
            Uint32 currentTime = SDL_GetTicks();
            deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            // Process events
            ProcessEvents();

            // Update game logic
            Update(deltaTime);

            // Render
            Render();
        }
    }

    void Shutdown() {
        Renderer::GetInstance().Shutdown();

        if (m_Window) {
            SDL_DestroyWindow(m_Window);
            m_Window = nullptr;
        }

        SDL_Quit();
    }

private:
    bool m_Running;
    SDL_Window* m_Window;

    // Rendering components
    std::shared_ptr<Camera> m_Camera;
    std::shared_ptr<Shader> m_SpriteShader;
    std::shared_ptr<Shader> m_ParticleShader;

    // Textures
    std::shared_ptr<Texture> m_PlayerTexture;
    std::shared_ptr<Texture> m_EnemyTexture;
    std::shared_ptr<Texture> m_BackgroundTexture;

    // Animation
    std::shared_ptr<SpriteSheet> m_PlayerSpriteSheet;
    AnimationController m_PlayerAnimation;

    // Particles
    std::shared_ptr<ParticleEmitter> m_FireEmitter;

    // Game objects
    glm::vec2 m_PlayerPos;
    float m_PlayerRotation;

    void LoadShaders() {
        // Load sprite shader
        m_SpriteShader = std::make_shared<Shader>();
        if (!m_SpriteShader->LoadFromFiles(
            "assets/shaders/sprite.vert",
            "assets/shaders/sprite.frag")) {
            std::cerr << "Failed to load sprite shader" << std::endl;
        }

        // Load particle shader
        m_ParticleShader = std::make_shared<Shader>();
        if (!m_ParticleShader->LoadFromFiles(
            "assets/shaders/particle.vert",
            "assets/shaders/particle.frag")) {
            std::cerr << "Failed to load particle shader" << std::endl;
        }

        // Set shader for sprite batch
        Renderer::GetInstance().GetSpriteBatch().SetShader(m_SpriteShader);
        Renderer::GetInstance().GetParticleSystem().SetShader(m_ParticleShader);
    }

    void SetupCamera() {
        auto& renderer = Renderer::GetInstance();
        m_Camera = renderer.GetCamera();

        // Configure camera
        m_Camera->SetZoom(1.0f);
        m_Camera->SetSmoothFollow(true);
        m_Camera->SetFollowSpeed(0.1f);

        // Set camera bounds (optional)
        // m_Camera->SetBounds(glm::vec4(-1000, -1000, 1000, 1000));
    }

    void LoadAssets() {
        // Load textures using texture manager
        auto& texManager = TextureManager::GetInstance();

        m_PlayerTexture = texManager.Load("assets/textures/player.png");
        m_EnemyTexture = texManager.Load("assets/textures/enemy.png");
        m_BackgroundTexture = texManager.Load("assets/textures/background.png");

        // Initialize game object positions
        m_PlayerPos = glm::vec2(0.0f, 0.0f);
        m_PlayerRotation = 0.0f;
    }

    void SetupAnimations() {
        if (!m_PlayerTexture) return;

        // Create sprite sheet (assuming 8x4 grid)
        m_PlayerSpriteSheet = std::make_shared<SpriteSheet>(m_PlayerTexture, 8, 4);

        // Create animations
        auto idleAnim = m_PlayerSpriteSheet->CreateAnimation("idle", 0, 7, 0.1f, true);
        auto runAnim = m_PlayerSpriteSheet->CreateAnimation("run", 8, 15, 0.08f, true);
        auto attackAnim = m_PlayerSpriteSheet->CreateAnimation("attack", 16, 23, 0.06f, false);

        // Add animations to controller
        m_PlayerAnimation.AddClip("idle", idleAnim);
        m_PlayerAnimation.AddClip("run", runAnim);
        m_PlayerAnimation.AddClip("attack", attackAnim);

        // Play initial animation
        m_PlayerAnimation.Play("idle");

        // Set callbacks
        m_PlayerAnimation.SetOnAnimationComplete([](const std::string& name) {
            std::cout << "Animation completed: " << name << std::endl;
        });
    }

    void SetupParticles() {
        // Create fire particle emitter config
        ParticleEmitterConfig fireConfig;
        fireConfig.emissionRate = 50.0f;
        fireConfig.maxParticles = 500;
        fireConfig.lifetime = 1.0f;
        fireConfig.lifetimeVariation = 0.3f;
        fireConfig.position = glm::vec2(0.0f, 0.0f);
        fireConfig.positionVariation = glm::vec2(10.0f, 5.0f);
        fireConfig.velocity = glm::vec2(0.0f, -50.0f);
        fireConfig.velocityVariation = glm::vec2(20.0f, 10.0f);
        fireConfig.acceleration = glm::vec2(0.0f, -20.0f); // Rising effect
        fireConfig.startColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); // Orange
        fireConfig.endColor = glm::vec4(1.0f, 0.2f, 0.0f, 0.0f);   // Red fade out
        fireConfig.startSize = 15.0f;
        fireConfig.endSize = 5.0f;
        fireConfig.sizeVariation = 5.0f;
        fireConfig.angularVelocity = 2.0f;

        // Create emitter
        m_FireEmitter = Renderer::GetInstance().GetParticleSystem().CreateEmitter(fireConfig);
        m_FireEmitter->Play();
    }

    void SetupLighting() {
        auto& renderer = Renderer::GetInstance();

        // Set ambient light
        renderer.SetAmbientLight(glm::vec3(0.3f, 0.3f, 0.4f), 0.5f);

        // Add some lights
        Renderer::Light playerLight;
        playerLight.position = m_PlayerPos;
        playerLight.color = glm::vec3(1.0f, 0.9f, 0.7f);
        playerLight.intensity = 2.0f;
        playerLight.radius = 200.0f;
        renderer.AddLight(playerLight);

        // Add a colored light
        Renderer::Light coloredLight;
        coloredLight.position = glm::vec2(300.0f, 200.0f);
        coloredLight.color = glm::vec3(0.2f, 0.8f, 1.0f);
        coloredLight.intensity = 1.5f;
        coloredLight.radius = 150.0f;
        renderer.AddLight(coloredLight);
    }

    void SetupPostProcessing() {
        auto& renderer = Renderer::GetInstance();

        // Create bloom effect
        auto bloom = std::make_shared<BloomEffect>();
        bloom->Initialize(1280, 720);
        bloom->SetThreshold(0.8f);
        bloom->SetIntensity(0.5f);
        bloom->SetBlurPasses(5);
        renderer.AddPostProcessEffect(bloom);

        // Create color grading effect
        auto colorGrading = std::make_shared<ColorGradingEffect>();
        colorGrading->Initialize();
        colorGrading->SetBrightness(0.05f);
        colorGrading->SetContrast(1.1f);
        colorGrading->SetSaturation(1.2f);
        renderer.AddPostProcessEffect(colorGrading);

        // Enable post-processing
        renderer.SetPostProcessingEnabled(true);
    }

    void ProcessEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    m_Running = false;
                    break;

                case SDL_KEYDOWN:
                    HandleKeyPress(event.key.keysym.sym);
                    break;

                case SDL_MOUSEWHEEL:
                    // Zoom with mouse wheel
                    if (m_Camera) {
                        float currentZoom = m_Camera->GetZoom();
                        float newZoom = currentZoom + event.wheel.y * 0.1f;
                        m_Camera->SetTargetZoom(glm::clamp(newZoom, 0.5f, 3.0f));
                    }
                    break;
            }
        }
    }

    void HandleKeyPress(SDL_Keycode key) {
        switch (key) {
            case SDLK_ESCAPE:
                m_Running = false;
                break;

            case SDLK_SPACE:
                // Trigger attack animation
                m_PlayerAnimation.Play("attack");
                break;

            case SDLK_1:
                // Camera shake
                if (m_Camera) {
                    m_Camera->Shake(20.0f, 0.5f);
                }
                break;

            case SDLK_p:
                // Toggle post-processing
                {
                    auto& renderer = Renderer::GetInstance();
                    bool enabled = renderer.IsPostProcessingEnabled();
                    renderer.SetPostProcessingEnabled(!enabled);
                    std::cout << "Post-processing: " << (!enabled ? "ON" : "OFF") << std::endl;
                }
                break;
        }
    }

    void Update(float deltaTime) {
        // Update player movement (WASD)
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        glm::vec2 movement(0.0f);

        if (keyState[SDL_SCANCODE_W]) movement.y -= 1.0f;
        if (keyState[SDL_SCANCODE_S]) movement.y += 1.0f;
        if (keyState[SDL_SCANCODE_A]) movement.x -= 1.0f;
        if (keyState[SDL_SCANCODE_D]) movement.x += 1.0f;

        // Normalize movement
        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);
            m_PlayerPos += movement * 200.0f * deltaTime;

            // Play run animation if not attacking
            if (m_PlayerAnimation.GetCurrentAnimation() != "attack" ||
                !m_PlayerAnimation.IsPlaying()) {
                m_PlayerAnimation.Play("run");
            }

            // Update player rotation
            m_PlayerRotation = std::atan2(movement.y, movement.x);
        } else {
            // Play idle animation if not attacking
            if (m_PlayerAnimation.GetCurrentAnimation() != "attack" ||
                !m_PlayerAnimation.IsPlaying()) {
                m_PlayerAnimation.Play("idle");
            }
        }

        // Update camera to follow player
        if (m_Camera) {
            m_Camera->SetTarget(m_PlayerPos);
            m_Camera->Update(deltaTime);
        }

        // Update animations
        m_PlayerAnimation.Update(deltaTime);

        // Update particles
        if (m_FireEmitter) {
            m_FireEmitter->SetPosition(m_PlayerPos + glm::vec2(0.0f, 20.0f));
        }
        Renderer::GetInstance().GetParticleSystem().Update(deltaTime);
    }

    void Render() {
        auto& renderer = Renderer::GetInstance();
        auto& spriteBatch = renderer.GetSpriteBatch();

        // Begin frame
        renderer.BeginFrame();
        renderer.Clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

        // Set camera for sprite batch
        if (m_Camera) {
            spriteBatch.SetViewProjectionMatrix(m_Camera->GetViewProjectionMatrix());
        }

        // === BACKGROUND LAYER ===
        renderer.SetRenderLayer(RenderLayer::Background);
        spriteBatch.Begin();

        if (m_BackgroundTexture) {
            // Draw tiled background
            for (int y = -5; y <= 5; y++) {
                for (int x = -5; x <= 5; x++) {
                    spriteBatch.DrawSprite(
                        m_BackgroundTexture,
                        glm::vec2(x * 128.0f, y * 128.0f),
                        glm::vec2(128.0f, 128.0f)
                    );
                }
            }
        }

        spriteBatch.End();

        // === GAME LAYER ===
        renderer.SetRenderLayer(RenderLayer::Game);
        spriteBatch.Begin();

        // Draw player with animation
        if (m_PlayerTexture) {
            glm::vec4 currentFrame = m_PlayerAnimation.GetCurrentFrameUV();
            spriteBatch.DrawSprite(
                m_PlayerTexture,
                m_PlayerPos,
                glm::vec2(64.0f, 64.0f),
                currentFrame,
                glm::vec4(1.0f),
                m_PlayerRotation,
                0.0f
            );
        }

        // Draw some enemies
        if (m_EnemyTexture) {
            for (int i = 0; i < 10; i++) {
                float angle = (i / 10.0f) * 2.0f * 3.14159f;
                glm::vec2 pos = m_PlayerPos + glm::vec2(
                    std::cos(angle) * 200.0f,
                    std::sin(angle) * 200.0f
                );

                spriteBatch.DrawSprite(
                    m_EnemyTexture,
                    pos,
                    glm::vec2(48.0f, 48.0f),
                    glm::vec4(1.0f, 0.8f, 0.8f, 1.0f)
                );
            }
        }

        spriteBatch.End();

        // Draw particles
        if (m_Camera) {
            renderer.GetParticleSystem().Render(m_Camera->GetViewProjectionMatrix());
        }

        // === UI LAYER ===
        renderer.SetRenderLayer(RenderLayer::UI);
        // Draw UI elements here (health bars, minimap, etc.)

        // End frame and present
        renderer.EndFrame();

        // Print stats
        const auto& stats = renderer.GetStats();
        static int frameCount = 0;
        if (++frameCount % 60 == 0) {
            std::cout << "FPS: " << (1.0f / stats.frameTime)
                      << " | Draw Calls: " << stats.drawCalls
                      << " | Sprites: " << stats.spritesRendered << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;

    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game" << std::endl;
        return 1;
    }

    std::cout << "=== ARPG Rendering System Example ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move player" << std::endl;
    std::cout << "  Space - Attack" << std::endl;
    std::cout << "  Mouse Wheel - Zoom" << std::endl;
    std::cout << "  1 - Camera shake" << std::endl;
    std::cout << "  P - Toggle post-processing" << std::endl;
    std::cout << "  ESC - Quit" << std::endl;
    std::cout << "======================================" << std::endl;

    game.Run();
    game.Shutdown();

    return 0;
}
