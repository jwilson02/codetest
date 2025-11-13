#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <GL/glew.h>

namespace Assets {

/**
 * @brief Rectangle representing a sprite in the atlas
 */
struct AtlasRect {
    int x, y;           // Position in atlas
    int width, height;  // Size of sprite
    int originalWidth, originalHeight; // Original size before packing
    int offsetX, offsetY; // Offset from original position (for trimming)

    AtlasRect()
        : x(0), y(0), width(0), height(0)
        , originalWidth(0), originalHeight(0)
        , offsetX(0), offsetY(0) {}

    AtlasRect(int x, int y, int w, int h)
        : x(x), y(y), width(w), height(h)
        , originalWidth(w), originalHeight(h)
        , offsetX(0), offsetY(0) {}
};

/**
 * @brief UV coordinates for texture sampling
 */
struct AtlasUV {
    float u0, v0; // Top-left
    float u1, v1; // Bottom-right

    AtlasUV() : u0(0), v0(0), u1(1), v1(1) {}
};

/**
 * @brief Sprite information in the atlas
 */
struct AtlasSprite {
    std::string name;
    AtlasRect rect;
    AtlasUV uv;
    bool rotated; // If sprite was rotated 90° during packing

    AtlasSprite() : rotated(false) {}
};

/**
 * @brief Packing algorithm options
 */
enum class PackingAlgorithm {
    Simple,         // Simple row-based packing
    MaxRects,       // MaxRects algorithm (efficient)
    Skyline,        // Skyline algorithm
    BinaryTree      // Binary tree packing
};

/**
 * @brief Atlas generation settings
 */
struct AtlasSettings {
    int maxWidth;           // Maximum atlas width
    int maxHeight;          // Maximum atlas height
    int padding;            // Padding between sprites
    bool powerOfTwo;        // Force power-of-2 dimensions
    bool trimAlpha;         // Trim transparent pixels
    bool allowRotation;     // Allow 90° rotation for better packing
    PackingAlgorithm algorithm; // Packing algorithm to use
    bool generateMipmaps;   // Generate mipmaps for atlas

    AtlasSettings()
        : maxWidth(2048)
        , maxHeight(2048)
        , padding(2)
        , powerOfTwo(true)
        , trimAlpha(true)
        , allowRotation(true)
        , algorithm(PackingAlgorithm::MaxRects)
        , generateMipmaps(true) {}
};

/**
 * @brief Input sprite for atlas generation
 */
struct AtlasInput {
    std::string name;
    unsigned char* data;  // RGBA pixel data
    int width;
    int height;
    bool ownsData;        // If true, will free data on destruction

    AtlasInput()
        : data(nullptr), width(0), height(0), ownsData(false) {}

    ~AtlasInput() {
        if (ownsData && data) {
            delete[] data;
        }
    }

    // Prevent copying
    AtlasInput(const AtlasInput&) = delete;
    AtlasInput& operator=(const AtlasInput&) = delete;

    // Allow moving
    AtlasInput(AtlasInput&& other) noexcept
        : name(std::move(other.name))
        , data(other.data)
        , width(other.width)
        , height(other.height)
        , ownsData(other.ownsData) {
        other.data = nullptr;
        other.ownsData = false;
    }
};

/**
 * @brief Texture atlas for efficient sprite rendering
 *
 * Combines multiple small textures into a single large texture
 * to reduce draw calls and improve performance.
 */
class TextureAtlas {
public:
    TextureAtlas();
    ~TextureAtlas();

    /**
     * @brief Generate atlas from a list of sprites
     * @param inputs List of input sprites
     * @param settings Atlas generation settings
     * @return True if successful
     */
    bool Generate(std::vector<AtlasInput>& inputs, const AtlasSettings& settings = AtlasSettings());

    /**
     * @brief Load atlas from files
     * @param atlasImagePath Path to the atlas texture
     * @param atlasDataPath Path to the atlas data (JSON/binary)
     * @return True if successful
     */
    bool Load(const std::string& atlasImagePath, const std::string& atlasDataPath);

    /**
     * @brief Save atlas to files
     * @param atlasImagePath Output path for atlas texture
     * @param atlasDataPath Output path for atlas data
     * @return True if successful
     */
    bool Save(const std::string& atlasImagePath, const std::string& atlasDataPath) const;

    /**
     * @brief Get sprite by name
     * @param name Sprite name
     * @return Pointer to sprite or nullptr if not found
     */
    const AtlasSprite* GetSprite(const std::string& name) const;

    /**
     * @brief Get all sprites in the atlas
     * @return Map of sprite names to sprite data
     */
    const std::unordered_map<std::string, AtlasSprite>& GetSprites() const { return m_Sprites; }

    /**
     * @brief Bind atlas texture
     * @param unit Texture unit to bind to
     */
    void Bind(GLuint unit = 0) const;

    /**
     * @brief Unbind atlas texture
     */
    void Unbind() const;

    /**
     * @brief Get OpenGL texture ID
     * @return Texture ID
     */
    GLuint GetTextureID() const { return m_TextureID; }

    /**
     * @brief Get atlas dimensions
     */
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    /**
     * @brief Check if atlas is valid
     * @return True if atlas has been generated/loaded
     */
    bool IsValid() const { return m_TextureID != 0; }

    /**
     * @brief Get memory size
     * @return Size in bytes
     */
    size_t GetMemorySize() const;

    /**
     * @brief Calculate UV coordinates for a sprite
     * @param rect Sprite rectangle in atlas
     * @return UV coordinates
     */
    AtlasUV CalculateUV(const AtlasRect& rect) const;

private:
    /**
     * @brief Pack sprites into atlas
     */
    bool PackSprites(std::vector<AtlasInput>& inputs, const AtlasSettings& settings);

    /**
     * @brief Pack using MaxRects algorithm
     */
    bool PackMaxRects(std::vector<AtlasInput>& inputs, const AtlasSettings& settings);

    /**
     * @brief Pack using simple row-based algorithm
     */
    bool PackSimple(std::vector<AtlasInput>& inputs, const AtlasSettings& settings);

    /**
     * @brief Trim transparent pixels from sprite
     */
    AtlasRect TrimAlpha(const unsigned char* data, int width, int height);

    /**
     * @brief Create OpenGL texture from packed data
     */
    bool CreateTexture(const unsigned char* data, int width, int height, bool generateMipmaps);

    /**
     * @brief Find next power of two
     */
    int NextPowerOfTwo(int value) const;

    /**
     * @brief Copy sprite data to atlas buffer
     */
    void CopySprite(unsigned char* atlasData, const unsigned char* spriteData,
                    const AtlasRect& rect, int spriteWidth, int spriteHeight,
                    int atlasWidth, bool rotated);

    GLuint m_TextureID;
    int m_Width;
    int m_Height;
    std::unordered_map<std::string, AtlasSprite> m_Sprites;
    unsigned char* m_AtlasData; // Keep atlas data in memory for saving
};

/**
 * @brief Atlas manager for managing multiple atlases
 */
class AtlasManager {
public:
    static AtlasManager& GetInstance();

    /**
     * @brief Load or create an atlas
     * @param name Atlas name
     * @return Pointer to atlas
     */
    std::shared_ptr<TextureAtlas> GetAtlas(const std::string& name);

    /**
     * @brief Add an atlas
     * @param name Atlas name
     * @param atlas Atlas instance
     */
    void AddAtlas(const std::string& name, std::shared_ptr<TextureAtlas> atlas);

    /**
     * @brief Remove an atlas
     * @param name Atlas name
     */
    void RemoveAtlas(const std::string& name);

    /**
     * @brief Clear all atlases
     */
    void Clear();

private:
    AtlasManager() = default;
    std::unordered_map<std::string, std::shared_ptr<TextureAtlas>> m_Atlases;
};

} // namespace Assets
