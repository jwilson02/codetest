#include "Sprite.h"
#include <algorithm>
#include <iostream>

namespace Rendering {

SpriteBatch::SpriteBatch(size_t maxSprites)
    : m_MaxSprites(maxSprites)
    , m_SpriteCount(0)
    , m_DrawCallCount(0)
    , m_VAO(0)
    , m_VBO(0)
    , m_EBO(0)
    , m_TextureSlotIndex(0)
    , m_ViewProjectionMatrix(1.0f) {

    // Reserve space for vertices and indices
    m_Vertices.reserve(maxSprites * 4);  // 4 vertices per sprite
    m_Indices.reserve(maxSprites * 6);   // 6 indices per sprite (2 triangles)
    m_TextureSlots.reserve(MAX_TEXTURE_SLOTS);
}

SpriteBatch::~SpriteBatch() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }
}

bool SpriteBatch::Initialize() {
    // Generate and bind VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Generate VBO
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_MaxSprites * 4 * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                         (void*)offsetof(SpriteVertex, position));

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                         (void*)offsetof(SpriteVertex, texCoord));

    // Color attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                         (void*)offsetof(SpriteVertex, color));

    // TexIndex attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                         (void*)offsetof(SpriteVertex, texIndex));

    // Generate EBO
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MaxSprites * 6 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::cout << "SpriteBatch initialized with capacity for " << m_MaxSprites << " sprites" << std::endl;
    return true;
}

void SpriteBatch::Begin() {
    m_Vertices.clear();
    m_Indices.clear();
    m_TextureSlots.clear();
    m_TextureSlotIndex = 0;
    m_SpriteCount = 0;
    m_DrawCallCount = 0;
}

void SpriteBatch::DrawSprite(const Sprite& sprite) {
    // Get or add texture to batch
    float texIndex = GetTextureIndex(sprite.texture);

    // If we've run out of texture slots or sprites, flush the batch
    if (texIndex < 0 || m_SpriteCount >= m_MaxSprites) {
        Flush();
        texIndex = GetTextureIndex(sprite.texture);
    }

    AddQuad(sprite, texIndex);
    m_SpriteCount++;
}

void SpriteBatch::DrawSprite(std::shared_ptr<Texture> texture,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              const glm::vec4& color,
                              float rotation,
                              float depth) {
    Sprite sprite;
    sprite.texture = texture;
    sprite.position = position;
    sprite.size = size;
    sprite.color = color;
    sprite.rotation = rotation;
    sprite.depth = depth;
    DrawSprite(sprite);
}

void SpriteBatch::DrawSprite(std::shared_ptr<Texture> texture,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              const glm::vec4& uvRect,
                              const glm::vec4& color,
                              float rotation,
                              float depth) {
    Sprite sprite;
    sprite.texture = texture;
    sprite.position = position;
    sprite.size = size;
    sprite.texCoords = uvRect;
    sprite.color = color;
    sprite.rotation = rotation;
    sprite.depth = depth;
    DrawSprite(sprite);
}

void SpriteBatch::End() {
    if (m_SpriteCount > 0) {
        Flush();
    }
}

void SpriteBatch::Flush() {
    if (m_Vertices.empty()) {
        return;
    }

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(SpriteVertex), m_Vertices.data());

    // Update EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Indices.size() * sizeof(GLuint), m_Indices.data());

    // Bind textures
    for (size_t i = 0; i < m_TextureSlots.size(); i++) {
        m_TextureSlots[i]->Bind(i);
    }

    // Use shader and set uniforms
    if (m_Shader && m_Shader->IsValid()) {
        m_Shader->Use();
        m_Shader->SetMat4("u_ViewProjection", m_ViewProjectionMatrix);

        // Set texture samplers
        int samplers[MAX_TEXTURE_SLOTS];
        for (int i = 0; i < MAX_TEXTURE_SLOTS; i++) {
            samplers[i] = i;
        }
        glUniform1iv(glGetUniformLocation(m_Shader->GetProgramID(), "u_Textures"), MAX_TEXTURE_SLOTS, samplers);
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Draw
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_DrawCallCount++;

    // Clear for next batch
    m_Vertices.clear();
    m_Indices.clear();
    m_TextureSlots.clear();
    m_TextureSlotIndex = 0;
    m_SpriteCount = 0;
}

float SpriteBatch::GetTextureIndex(std::shared_ptr<Texture> texture) {
    if (!texture || !texture->IsValid()) {
        return 0.0f;
    }

    // Check if texture is already in the batch
    for (size_t i = 0; i < m_TextureSlots.size(); i++) {
        if (m_TextureSlots[i]->GetID() == texture->GetID()) {
            return static_cast<float>(i);
        }
    }

    // Add texture to batch if there's space
    if (m_TextureSlots.size() < MAX_TEXTURE_SLOTS) {
        m_TextureSlots.push_back(texture);
        return static_cast<float>(m_TextureSlots.size() - 1);
    }

    // No space left
    return -1.0f;
}

void SpriteBatch::AddQuad(const Sprite& sprite, float texIndex) {
    GLuint indexOffset = m_Vertices.size();

    // Calculate sprite transform
    glm::vec2 origin = sprite.origin * sprite.size;

    // Define quad corners relative to origin
    glm::vec2 corners[4] = {
        glm::vec2(-origin.x, -origin.y),                          // Bottom-left
        glm::vec2(sprite.size.x - origin.x, -origin.y),           // Bottom-right
        glm::vec2(sprite.size.x - origin.x, sprite.size.y - origin.y), // Top-right
        glm::vec2(-origin.x, sprite.size.y - origin.y)            // Top-left
    };

    // Apply rotation if needed
    if (sprite.rotation != 0.0f) {
        float cosR = std::cos(sprite.rotation);
        float sinR = std::sin(sprite.rotation);
        for (int i = 0; i < 4; i++) {
            float x = corners[i].x;
            float y = corners[i].y;
            corners[i].x = x * cosR - y * sinR;
            corners[i].y = x * sinR + y * cosR;
        }
    }

    // Translate to world position
    for (int i = 0; i < 4; i++) {
        corners[i] += sprite.position;
    }

    // UV coordinates
    float u0 = sprite.texCoords.x;
    float v0 = sprite.texCoords.y;
    float u1 = sprite.texCoords.x + sprite.texCoords.z;
    float v1 = sprite.texCoords.y + sprite.texCoords.w;

    // Apply flip
    if (sprite.flipX) {
        std::swap(u0, u1);
    }
    if (sprite.flipY) {
        std::swap(v0, v1);
    }

    glm::vec2 uvs[4] = {
        glm::vec2(u0, v1),  // Bottom-left
        glm::vec2(u1, v1),  // Bottom-right
        glm::vec2(u1, v0),  // Top-right
        glm::vec2(u0, v0)   // Top-left
    };

    // Add vertices
    for (int i = 0; i < 4; i++) {
        SpriteVertex vertex;
        vertex.position = glm::vec3(corners[i], sprite.depth);
        vertex.texCoord = uvs[i];
        vertex.color = sprite.color;
        vertex.texIndex = texIndex;
        m_Vertices.push_back(vertex);
    }

    // Add indices (two triangles)
    m_Indices.push_back(indexOffset + 0);
    m_Indices.push_back(indexOffset + 1);
    m_Indices.push_back(indexOffset + 2);
    m_Indices.push_back(indexOffset + 2);
    m_Indices.push_back(indexOffset + 3);
    m_Indices.push_back(indexOffset + 0);
}

} // namespace Rendering
