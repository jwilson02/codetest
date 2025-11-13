#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "Texture.h"
#include "Shader.h"

namespace Rendering {

/**
 * Single particle data
 */
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    glm::vec4 color;
    glm::vec4 colorDelta;  // Color change per second
    float rotation;
    float angularVelocity;
    float size;
    float sizeDelta;       // Size change per second
    float lifetime;        // Total lifetime
    float age;             // Current age
    bool active;

    Particle()
        : position(0.0f)
        , velocity(0.0f)
        , acceleration(0.0f)
        , color(1.0f)
        , colorDelta(0.0f)
        , rotation(0.0f)
        , angularVelocity(0.0f)
        , size(1.0f)
        , sizeDelta(0.0f)
        , lifetime(1.0f)
        , age(0.0f)
        , active(false) {
    }
};

/**
 * Particle vertex for rendering
 */
struct ParticleVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec4 color;
    float size;
    float rotation;
};

/**
 * Particle emitter configuration
 */
struct ParticleEmitterConfig {
    // Emission
    float emissionRate;         // Particles per second
    int maxParticles;
    bool looping;
    float duration;             // Emitter lifetime (-1 for infinite)

    // Particle properties
    float lifetime;             // Particle lifetime
    float lifetimeVariation;

    // Position
    glm::vec2 position;
    glm::vec2 positionVariation;

    // Velocity
    glm::vec2 velocity;
    glm::vec2 velocityVariation;
    float speed;
    float speedVariation;

    // Acceleration
    glm::vec2 acceleration;     // Gravity, wind, etc.

    // Appearance
    glm::vec4 startColor;
    glm::vec4 endColor;
    float startSize;
    float endSize;
    float sizeVariation;

    // Rotation
    float startRotation;
    float startRotationVariation;
    float angularVelocity;
    float angularVelocityVariation;

    // Angle (emission direction)
    float angle;                // Radians
    float angleVariation;

    ParticleEmitterConfig()
        : emissionRate(10.0f)
        , maxParticles(100)
        , looping(true)
        , duration(-1.0f)
        , lifetime(2.0f)
        , lifetimeVariation(0.5f)
        , position(0.0f)
        , positionVariation(0.0f)
        , velocity(0.0f)
        , velocityVariation(0.0f)
        , speed(100.0f)
        , speedVariation(20.0f)
        , acceleration(0.0f, 0.0f)
        , startColor(1.0f)
        , endColor(1.0f, 1.0f, 1.0f, 0.0f)
        , startSize(10.0f)
        , endSize(0.0f)
        , sizeVariation(2.0f)
        , startRotation(0.0f)
        , startRotationVariation(0.0f)
        , angularVelocity(0.0f)
        , angularVelocityVariation(0.0f)
        , angle(0.0f)
        , angleVariation(3.14159f) {
    }
};

/**
 * Particle emitter
 */
class ParticleEmitter {
public:
    ParticleEmitter(const ParticleEmitterConfig& config = ParticleEmitterConfig());

    // Update particles
    void Update(float deltaTime);

    // Render particles
    void Render(std::shared_ptr<Shader> shader, const glm::mat4& viewProjection);

    // Control
    void Play();
    void Stop();
    void Pause();
    void Reset();
    void Emit(int count = 1);

    // Configuration
    void SetConfig(const ParticleEmitterConfig& config) { m_Config = config; }
    const ParticleEmitterConfig& GetConfig() const { return m_Config; }

    // Position
    void SetPosition(const glm::vec2& position) { m_Config.position = position; }
    glm::vec2 GetPosition() const { return m_Config.position; }

    // Texture
    void SetTexture(std::shared_ptr<Texture> texture) { m_Texture = texture; }
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }

    // State
    bool IsPlaying() const { return m_IsPlaying && !m_IsPaused; }
    bool IsPaused() const { return m_IsPaused; }
    int GetActiveParticleCount() const { return m_ActiveParticleCount; }

    // Burst emission
    void Burst(int count);

    // Custom update function
    using ParticleUpdateFunc = std::function<void(Particle&, float)>;
    void SetCustomUpdate(ParticleUpdateFunc func) { m_CustomUpdate = func; }

private:
    ParticleEmitterConfig m_Config;
    std::vector<Particle> m_Particles;
    std::shared_ptr<Texture> m_Texture;

    // OpenGL buffers
    GLuint m_VAO;
    GLuint m_VBO;
    std::vector<ParticleVertex> m_VertexBuffer;

    // State
    bool m_IsPlaying;
    bool m_IsPaused;
    float m_EmitterAge;
    float m_EmissionAccumulator;
    int m_ActiveParticleCount;

    // Custom update
    ParticleUpdateFunc m_CustomUpdate;

    // Helper functions
    void InitializeBuffers();
    void EmitParticle();
    void UpdateParticle(Particle& particle, float deltaTime);
    float RandomRange(float min, float max);
    glm::vec2 RandomVec2(const glm::vec2& min, const glm::vec2& max);
};

/**
 * Particle system manager
 */
class ParticleSystemManager {
public:
    ParticleSystemManager();
    ~ParticleSystemManager();

    // Initialize
    bool Initialize();

    // Update all emitters
    void Update(float deltaTime);

    // Render all emitters
    void Render(const glm::mat4& viewProjection);

    // Create emitter
    std::shared_ptr<ParticleEmitter> CreateEmitter(const ParticleEmitterConfig& config);

    // Add/remove emitters
    void AddEmitter(std::shared_ptr<ParticleEmitter> emitter);
    void RemoveEmitter(std::shared_ptr<ParticleEmitter> emitter);
    void Clear();

    // Get emitter count
    size_t GetEmitterCount() const { return m_Emitters.size(); }

    // Set shader
    void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }

private:
    std::vector<std::shared_ptr<ParticleEmitter>> m_Emitters;
    std::shared_ptr<Shader> m_Shader;
};

} // namespace Rendering
