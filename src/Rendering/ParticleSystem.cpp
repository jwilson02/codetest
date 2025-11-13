#include "ParticleSystem.h"
#include <algorithm>
#include <random>
#include <iostream>

namespace Rendering {

// Random number generation
static std::random_device s_RandomDevice;
static std::mt19937 s_RandomEngine(s_RandomDevice());

// ParticleEmitter implementation
ParticleEmitter::ParticleEmitter(const ParticleEmitterConfig& config)
    : m_Config(config)
    , m_VAO(0)
    , m_VBO(0)
    , m_IsPlaying(false)
    , m_IsPaused(false)
    , m_EmitterAge(0.0f)
    , m_EmissionAccumulator(0.0f)
    , m_ActiveParticleCount(0) {

    m_Particles.resize(config.maxParticles);
    m_VertexBuffer.reserve(config.maxParticles * 4); // 4 vertices per particle

    InitializeBuffers();
}

void ParticleEmitter::InitializeBuffers() {
    // Generate VAO and VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // Allocate buffer
    glBufferData(GL_ARRAY_BUFFER, m_Config.maxParticles * 4 * sizeof(ParticleVertex), nullptr, GL_DYNAMIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                         (void*)offsetof(ParticleVertex, position));

    // TexCoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                         (void*)offsetof(ParticleVertex, texCoord));

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                         (void*)offsetof(ParticleVertex, color));

    // Size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                         (void*)offsetof(ParticleVertex, size));

    // Rotation
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                         (void*)offsetof(ParticleVertex, rotation));

    glBindVertexArray(0);
}

void ParticleEmitter::Update(float deltaTime) {
    if (!m_IsPlaying || m_IsPaused) {
        return;
    }

    m_EmitterAge += deltaTime;

    // Check if emitter has expired
    if (m_Config.duration > 0.0f && m_EmitterAge >= m_Config.duration) {
        if (m_Config.looping) {
            m_EmitterAge = 0.0f;
        } else {
            m_IsPlaying = false;
        }
    }

    // Emit new particles
    if (m_IsPlaying && (m_Config.looping || m_EmitterAge < m_Config.duration)) {
        m_EmissionAccumulator += deltaTime * m_Config.emissionRate;
        int particlesToEmit = static_cast<int>(m_EmissionAccumulator);
        m_EmissionAccumulator -= particlesToEmit;

        for (int i = 0; i < particlesToEmit; i++) {
            EmitParticle();
        }
    }

    // Update particles
    m_ActiveParticleCount = 0;
    for (auto& particle : m_Particles) {
        if (particle.active) {
            UpdateParticle(particle, deltaTime);
            if (particle.active) {
                m_ActiveParticleCount++;
            }
        }
    }
}

void ParticleEmitter::Render(std::shared_ptr<Shader> shader, const glm::mat4& viewProjection) {
    if (m_ActiveParticleCount == 0) {
        return;
    }

    // Build vertex buffer
    m_VertexBuffer.clear();

    for (const auto& particle : m_Particles) {
        if (!particle.active) {
            continue;
        }

        // Calculate corner offsets
        float halfSize = particle.size * 0.5f;
        glm::vec2 corners[4] = {
            glm::vec2(-halfSize, -halfSize),
            glm::vec2(halfSize, -halfSize),
            glm::vec2(halfSize, halfSize),
            glm::vec2(-halfSize, halfSize)
        };

        // Apply rotation
        if (particle.rotation != 0.0f) {
            float cosR = std::cos(particle.rotation);
            float sinR = std::sin(particle.rotation);
            for (int i = 0; i < 4; i++) {
                float x = corners[i].x;
                float y = corners[i].y;
                corners[i].x = x * cosR - y * sinR;
                corners[i].y = x * sinR + y * cosR;
            }
        }

        // Create vertices
        glm::vec2 uvs[4] = {
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        };

        for (int i = 0; i < 4; i++) {
            ParticleVertex vertex;
            vertex.position = glm::vec3(particle.position + corners[i], 0.0f);
            vertex.texCoord = uvs[i];
            vertex.color = particle.color;
            vertex.size = particle.size;
            vertex.rotation = particle.rotation;
            m_VertexBuffer.push_back(vertex);
        }
    }

    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffer.size() * sizeof(ParticleVertex), m_VertexBuffer.data());

    // Render
    if (shader && shader->IsValid()) {
        shader->Use();
        shader->SetMat4("u_ViewProjection", viewProjection);

        if (m_Texture) {
            m_Texture->Bind(0);
            shader->SetInt("u_Texture", 0);
        }
    }

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_QUADS, 0, m_VertexBuffer.size());
    glBindVertexArray(0);
}

void ParticleEmitter::Play() {
    m_IsPlaying = true;
    m_IsPaused = false;
}

void ParticleEmitter::Stop() {
    m_IsPlaying = false;
    Reset();
}

void ParticleEmitter::Pause() {
    m_IsPaused = true;
}

void ParticleEmitter::Reset() {
    for (auto& particle : m_Particles) {
        particle.active = false;
    }
    m_EmitterAge = 0.0f;
    m_EmissionAccumulator = 0.0f;
    m_ActiveParticleCount = 0;
}

void ParticleEmitter::Emit(int count) {
    for (int i = 0; i < count; i++) {
        EmitParticle();
    }
}

void ParticleEmitter::Burst(int count) {
    Emit(count);
}

void ParticleEmitter::EmitParticle() {
    // Find inactive particle
    Particle* particle = nullptr;
    for (auto& p : m_Particles) {
        if (!p.active) {
            particle = &p;
            break;
        }
    }

    if (!particle) {
        return; // No available particles
    }

    // Initialize particle
    particle->active = true;
    particle->age = 0.0f;
    particle->lifetime = m_Config.lifetime + RandomRange(-m_Config.lifetimeVariation, m_Config.lifetimeVariation);

    // Position
    particle->position = m_Config.position + RandomVec2(
        -m_Config.positionVariation,
        m_Config.positionVariation
    );

    // Velocity
    float angle = m_Config.angle + RandomRange(-m_Config.angleVariation, m_Config.angleVariation);
    float speed = m_Config.speed + RandomRange(-m_Config.speedVariation, m_Config.speedVariation);

    glm::vec2 direction(std::cos(angle), std::sin(angle));
    particle->velocity = m_Config.velocity + direction * speed + RandomVec2(
        -m_Config.velocityVariation,
        m_Config.velocityVariation
    );

    // Acceleration
    particle->acceleration = m_Config.acceleration;

    // Color
    particle->color = m_Config.startColor;
    particle->colorDelta = (m_Config.endColor - m_Config.startColor) / particle->lifetime;

    // Size
    particle->size = m_Config.startSize + RandomRange(-m_Config.sizeVariation, m_Config.sizeVariation);
    particle->sizeDelta = (m_Config.endSize - particle->size) / particle->lifetime;

    // Rotation
    particle->rotation = m_Config.startRotation + RandomRange(
        -m_Config.startRotationVariation,
        m_Config.startRotationVariation
    );
    particle->angularVelocity = m_Config.angularVelocity + RandomRange(
        -m_Config.angularVelocityVariation,
        m_Config.angularVelocityVariation
    );
}

void ParticleEmitter::UpdateParticle(Particle& particle, float deltaTime) {
    particle.age += deltaTime;

    // Check if particle has expired
    if (particle.age >= particle.lifetime) {
        particle.active = false;
        return;
    }

    // Update velocity
    particle.velocity += particle.acceleration * deltaTime;

    // Update position
    particle.position += particle.velocity * deltaTime;

    // Update color
    particle.color += particle.colorDelta * deltaTime;

    // Update size
    particle.size += particle.sizeDelta * deltaTime;

    // Update rotation
    particle.rotation += particle.angularVelocity * deltaTime;

    // Custom update
    if (m_CustomUpdate) {
        m_CustomUpdate(particle, deltaTime);
    }
}

float ParticleEmitter::RandomRange(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_RandomEngine);
}

glm::vec2 ParticleEmitter::RandomVec2(const glm::vec2& min, const glm::vec2& max) {
    return glm::vec2(
        RandomRange(min.x, max.x),
        RandomRange(min.y, max.y)
    );
}

// ParticleSystemManager implementation
ParticleSystemManager::ParticleSystemManager() {
}

ParticleSystemManager::~ParticleSystemManager() {
    Clear();
}

bool ParticleSystemManager::Initialize() {
    // Nothing to initialize for now
    return true;
}

void ParticleSystemManager::Update(float deltaTime) {
    // Remove finished non-looping emitters
    m_Emitters.erase(
        std::remove_if(m_Emitters.begin(), m_Emitters.end(),
            [](const std::shared_ptr<ParticleEmitter>& emitter) {
                return !emitter->IsPlaying() && emitter->GetActiveParticleCount() == 0;
            }),
        m_Emitters.end()
    );

    // Update all emitters
    for (auto& emitter : m_Emitters) {
        emitter->Update(deltaTime);
    }
}

void ParticleSystemManager::Render(const glm::mat4& viewProjection) {
    if (!m_Shader) {
        return;
    }

    for (auto& emitter : m_Emitters) {
        emitter->Render(m_Shader, viewProjection);
    }
}

std::shared_ptr<ParticleEmitter> ParticleSystemManager::CreateEmitter(const ParticleEmitterConfig& config) {
    auto emitter = std::make_shared<ParticleEmitter>(config);
    m_Emitters.push_back(emitter);
    return emitter;
}

void ParticleSystemManager::AddEmitter(std::shared_ptr<ParticleEmitter> emitter) {
    if (emitter) {
        m_Emitters.push_back(emitter);
    }
}

void ParticleSystemManager::RemoveEmitter(std::shared_ptr<ParticleEmitter> emitter) {
    m_Emitters.erase(
        std::remove(m_Emitters.begin(), m_Emitters.end(), emitter),
        m_Emitters.end()
    );
}

void ParticleSystemManager::Clear() {
    m_Emitters.clear();
}

} // namespace Rendering
