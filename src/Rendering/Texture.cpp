#include "Texture.h"
#include <iostream>
#include <unordered_map>

// Using stb_image for texture loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Rendering {

Texture::Texture()
    : m_TextureID(0), m_Width(0), m_Height(0), m_Format(GL_RGBA) {
}

Texture::~Texture() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
    }
}

bool Texture::LoadFromFile(const std::string& filepath, bool generateMipmaps) {
    // Load image using stb_image
    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filepath.c_str(), &m_Width, &m_Height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Determine format based on channels
    GLenum format, internalFormat;
    switch (channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            std::cerr << "Unsupported texture format with " << channels << " channels" << std::endl;
            stbi_image_free(data);
            return false;
    }

    m_Format = format;

    // Create OpenGL texture
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);

    // Generate mipmaps if requested
    if (generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Free image data
    stbi_image_free(data);

    std::cout << "Loaded texture: " << filepath << " (" << m_Width << "x" << m_Height << ")" << std::endl;
    return true;
}

bool Texture::Create(int width, int height, GLenum format, GLenum internalFormat) {
    m_Width = width;
    m_Height = height;
    m_Format = format;

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool Texture::LoadFromMemory(const unsigned char* data, int width, int height,
                              GLenum format, GLenum internalFormat) {
    if (!data) {
        return false;
    }

    m_Width = width;
    m_Height = height;
    m_Format = format;

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Texture::Bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFilterMode(GLenum minFilter, GLenum magFilter) {
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetWrapMode(GLenum wrapS, GLenum wrapT) {
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetBorderColor(float r, float g, float b, float a) {
    float borderColor[] = { r, g, b, a };
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TextureManager implementation
TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::Load(const std::string& filepath, bool generateMipmaps) {
    // Check if already loaded
    auto it = m_Textures.find(filepath);
    if (it != m_Textures.end()) {
        return it->second;
    }

    // Load new texture
    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(filepath, generateMipmaps)) {
        m_Textures[filepath] = texture;
        return texture;
    }

    return nullptr;
}

std::shared_ptr<Texture> TextureManager::Get(const std::string& name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureManager::Add(const std::string& name, std::shared_ptr<Texture> texture) {
    m_Textures[name] = texture;
}

void TextureManager::Remove(const std::string& name) {
    m_Textures.erase(name);
}

void TextureManager::Clear() {
    m_Textures.clear();
}

} // namespace Rendering
