#pragma once

#include <GL/glew.h>
#include <string>
#include <memory>

namespace Rendering {

/**
 * Texture class for loading and managing OpenGL textures
 */
class Texture {
public:
    Texture();
    ~Texture();

    // Load texture from file
    bool LoadFromFile(const std::string& filepath, bool generateMipmaps = true);

    // Create empty texture
    bool Create(int width, int height, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8);

    // Create from raw data
    bool LoadFromMemory(const unsigned char* data, int width, int height,
                        GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8);

    // Bind texture to a specific texture unit
    void Bind(GLuint unit = 0) const;

    // Unbind texture
    void Unbind() const;

    // Getters
    GLuint GetID() const { return m_TextureID; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    bool IsValid() const { return m_TextureID != 0; }

    // Texture parameters
    void SetFilterMode(GLenum minFilter, GLenum magFilter);
    void SetWrapMode(GLenum wrapS, GLenum wrapT);
    void SetBorderColor(float r, float g, float b, float a);

    // Generate mipmaps
    void GenerateMipmaps();

private:
    GLuint m_TextureID;
    int m_Width;
    int m_Height;
    GLenum m_Format;
};

/**
 * Texture Manager for caching and reusing textures
 */
class TextureManager {
public:
    static TextureManager& GetInstance();

    // Load or get cached texture
    std::shared_ptr<Texture> Load(const std::string& filepath, bool generateMipmaps = true);

    // Get texture by name
    std::shared_ptr<Texture> Get(const std::string& name);

    // Add texture with custom name
    void Add(const std::string& name, std::shared_ptr<Texture> texture);

    // Remove texture from cache
    void Remove(const std::string& name);

    // Clear all cached textures
    void Clear();

private:
    TextureManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
};

} // namespace Rendering
