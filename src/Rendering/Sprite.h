#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Texture.h"
#include "Shader.h"

namespace Rendering {

/**
 * Sprite vertex data for batching
 */
struct SpriteVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec4 color;
    float texIndex;  // Texture unit index for multi-texture batching
};

/**
 * Sprite render data
 */
struct Sprite {
    std::shared_ptr<Texture> texture;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 origin;      // Pivot point (0-1 normalized)
    glm::vec4 texCoords;   // UV coordinates (x, y, width, height)
    glm::vec4 color;
    float rotation;        // Radians
    float depth;           // Z-order for sorting
    bool flipX;
    bool flipY;

    Sprite()
        : position(0.0f)
        , size(1.0f)
        , origin(0.5f, 0.5f)
        , texCoords(0.0f, 0.0f, 1.0f, 1.0f)
        , color(1.0f)
        , rotation(0.0f)
        , depth(0.0f)
        , flipX(false)
        , flipY(false) {
    }
};

/**
 * Sprite batch renderer for efficient rendering of many sprites
 */
class SpriteBatch {
public:
    SpriteBatch(size_t maxSprites = 10000);
    ~SpriteBatch();

    // Initialize the batch renderer
    bool Initialize();

    // Begin batch (must be called before drawing sprites)
    void Begin();

    // Draw a sprite (adds to batch)
    void DrawSprite(const Sprite& sprite);

    // Draw a simple sprite with minimal parameters
    void DrawSprite(std::shared_ptr<Texture> texture,
                    const glm::vec2& position,
                    const glm::vec2& size,
                    const glm::vec4& color = glm::vec4(1.0f),
                    float rotation = 0.0f,
                    float depth = 0.0f);

    // Draw sprite with UV coordinates (for sprite sheets)
    void DrawSprite(std::shared_ptr<Texture> texture,
                    const glm::vec2& position,
                    const glm::vec2& size,
                    const glm::vec4& uvRect,
                    const glm::vec4& color = glm::vec4(1.0f),
                    float rotation = 0.0f,
                    float depth = 0.0f);

    // End batch and flush to GPU
    void End();

    // Set shader to use for rendering
    void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }

    // Set view-projection matrix
    void SetViewProjectionMatrix(const glm::mat4& vp) { m_ViewProjectionMatrix = vp; }

    // Get statistics
    size_t GetSpriteCount() const { return m_SpriteCount; }
    size_t GetDrawCallCount() const { return m_DrawCallCount; }

private:
    size_t m_MaxSprites;
    size_t m_SpriteCount;
    size_t m_DrawCallCount;

    // OpenGL buffers
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;

    // Batch data
    std::vector<SpriteVertex> m_Vertices;
    std::vector<GLuint> m_Indices;

    // Current batch state
    std::vector<std::shared_ptr<Texture>> m_TextureSlots;
    size_t m_TextureSlotIndex;
    static const size_t MAX_TEXTURE_SLOTS = 32;

    // Shader and matrices
    std::shared_ptr<Shader> m_Shader;
    glm::mat4 m_ViewProjectionMatrix;

    // Helper functions
    void Flush();
    float GetTextureIndex(std::shared_ptr<Texture> texture);
    void AddQuad(const Sprite& sprite, float texIndex);
};

} // namespace Rendering
