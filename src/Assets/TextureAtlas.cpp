#include "TextureAtlas.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

// For image loading/saving
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace Assets {

// MaxRects helper structures
struct MaxRectsNode {
    int x, y, width, height;
    bool used;

    MaxRectsNode() : x(0), y(0), width(0), height(0), used(false) {}
    MaxRectsNode(int x, int y, int w, int h)
        : x(x), y(y), width(w), height(h), used(false) {}
};

TextureAtlas::TextureAtlas()
    : m_TextureID(0)
    , m_Width(0)
    , m_Height(0)
    , m_AtlasData(nullptr) {
}

TextureAtlas::~TextureAtlas() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
    }
    if (m_AtlasData) {
        delete[] m_AtlasData;
    }
}

bool TextureAtlas::Generate(std::vector<AtlasInput>& inputs, const AtlasSettings& settings) {
    if (inputs.empty()) {
        std::cerr << "Cannot generate atlas: no input sprites" << std::endl;
        return false;
    }

    std::cout << "Generating texture atlas with " << inputs.size() << " sprites..." << std::endl;

    // Sort inputs by height (descending) for better packing
    std::sort(inputs.begin(), inputs.end(),
        [](const AtlasInput& a, const AtlasInput& b) {
            return a.height > b.height;
        });

    // Pack sprites based on selected algorithm
    bool success = false;
    switch (settings.algorithm) {
        case PackingAlgorithm::MaxRects:
            success = PackMaxRects(inputs, settings);
            break;
        case PackingAlgorithm::Simple:
            success = PackSimple(inputs, settings);
            break;
        default:
            success = PackMaxRects(inputs, settings); // Default to MaxRects
            break;
    }

    if (!success) {
        std::cerr << "Failed to pack sprites into atlas" << std::endl;
        return false;
    }

    std::cout << "Atlas generated successfully: " << m_Width << "x" << m_Height << std::endl;
    std::cout << "  Sprites: " << m_Sprites.size() << std::endl;
    std::cout << "  Memory: " << (GetMemorySize() / 1024) << " KB" << std::endl;

    return true;
}

bool TextureAtlas::Load(const std::string& atlasImagePath, const std::string& atlasDataPath) {
    // Load atlas texture
    int channels;
    unsigned char* data = stbi_load(atlasImagePath.c_str(), &m_Width, &m_Height, &channels, 4);

    if (!data) {
        std::cerr << "Failed to load atlas image: " << atlasImagePath << std::endl;
        return false;
    }

    // Create OpenGL texture
    bool success = CreateTexture(data, m_Width, m_Height, true);
    stbi_image_free(data);

    if (!success) {
        return false;
    }

    // TODO: Load sprite data from JSON/binary file
    std::cout << "Loaded atlas: " << atlasImagePath << " (" << m_Width << "x" << m_Height << ")" << std::endl;

    return true;
}

bool TextureAtlas::Save(const std::string& atlasImagePath, const std::string& atlasDataPath) const {
    if (!m_AtlasData) {
        std::cerr << "No atlas data to save" << std::endl;
        return false;
    }

    // Save atlas texture as PNG
    int result = stbi_write_png(atlasImagePath.c_str(), m_Width, m_Height, 4, m_AtlasData, m_Width * 4);

    if (!result) {
        std::cerr << "Failed to save atlas image: " << atlasImagePath << std::endl;
        return false;
    }

    // Save sprite data as JSON
    std::ofstream file(atlasDataPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open atlas data file: " << atlasDataPath << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"width\": " << m_Width << ",\n";
    file << "  \"height\": " << m_Height << ",\n";
    file << "  \"sprites\": [\n";

    size_t count = 0;
    for (const auto& pair : m_Sprites) {
        const auto& sprite = pair.second;
        file << "    {\n";
        file << "      \"name\": \"" << sprite.name << "\",\n";
        file << "      \"x\": " << sprite.rect.x << ",\n";
        file << "      \"y\": " << sprite.rect.y << ",\n";
        file << "      \"width\": " << sprite.rect.width << ",\n";
        file << "      \"height\": " << sprite.rect.height << ",\n";
        file << "      \"u0\": " << sprite.uv.u0 << ",\n";
        file << "      \"v0\": " << sprite.uv.v0 << ",\n";
        file << "      \"u1\": " << sprite.uv.u1 << ",\n";
        file << "      \"v1\": " << sprite.uv.v1 << ",\n";
        file << "      \"rotated\": " << (sprite.rotated ? "true" : "false") << "\n";
        file << "    }";

        if (++count < m_Sprites.size()) {
            file << ",";
        }
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();

    std::cout << "Saved atlas to: " << atlasImagePath << std::endl;
    std::cout << "Saved atlas data to: " << atlasDataPath << std::endl;

    return true;
}

const AtlasSprite* TextureAtlas::GetSprite(const std::string& name) const {
    auto it = m_Sprites.find(name);
    if (it != m_Sprites.end()) {
        return &it->second;
    }
    return nullptr;
}

void TextureAtlas::Bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void TextureAtlas::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

size_t TextureAtlas::GetMemorySize() const {
    return m_Width * m_Height * 4; // RGBA
}

AtlasUV TextureAtlas::CalculateUV(const AtlasRect& rect) const {
    AtlasUV uv;

    if (m_Width > 0 && m_Height > 0) {
        uv.u0 = static_cast<float>(rect.x) / m_Width;
        uv.v0 = static_cast<float>(rect.y) / m_Height;
        uv.u1 = static_cast<float>(rect.x + rect.width) / m_Width;
        uv.v1 = static_cast<float>(rect.y + rect.height) / m_Height;
    }

    return uv;
}

// Private methods

bool TextureAtlas::PackMaxRects(std::vector<AtlasInput>& inputs, const AtlasSettings& settings) {
    // Start with a reasonably sized atlas
    int atlasWidth = 256;
    int atlasHeight = 256;

    // Try progressively larger sizes until everything fits
    while (atlasWidth <= settings.maxWidth && atlasHeight <= settings.maxHeight) {
        std::vector<MaxRectsNode> freeRects;
        freeRects.push_back(MaxRectsNode(0, 0, atlasWidth, atlasHeight));

        m_Sprites.clear();
        bool allFit = true;

        for (auto& input : inputs) {
            int spriteWidth = input.width + settings.padding * 2;
            int spriteHeight = input.height + settings.padding * 2;

            // Find best free rectangle
            int bestScore = INT_MAX;
            size_t bestRect = SIZE_MAX;
            bool shouldRotate = false;

            for (size_t i = 0; i < freeRects.size(); ++i) {
                auto& rect = freeRects[i];
                if (rect.used) continue;

                // Try normal orientation
                if (rect.width >= spriteWidth && rect.height >= spriteHeight) {
                    int score = std::min(rect.width - spriteWidth, rect.height - spriteHeight);
                    if (score < bestScore) {
                        bestScore = score;
                        bestRect = i;
                        shouldRotate = false;
                    }
                }

                // Try rotated orientation
                if (settings.allowRotation && rect.width >= spriteHeight && rect.height >= spriteWidth) {
                    int score = std::min(rect.width - spriteHeight, rect.height - spriteWidth);
                    if (score < bestScore) {
                        bestScore = score;
                        bestRect = i;
                        shouldRotate = true;
                    }
                }
            }

            if (bestRect == SIZE_MAX) {
                allFit = false;
                break;
            }

            // Place sprite
            auto& rect = freeRects[bestRect];
            AtlasSprite sprite;
            sprite.name = input.name;
            sprite.rotated = shouldRotate;

            if (shouldRotate) {
                sprite.rect = AtlasRect(rect.x + settings.padding, rect.y + settings.padding,
                                       input.height, input.width);
            } else {
                sprite.rect = AtlasRect(rect.x + settings.padding, rect.y + settings.padding,
                                       input.width, input.height);
            }

            sprite.uv = CalculateUV(sprite.rect);
            m_Sprites[sprite.name] = sprite;

            // Split free rectangle
            int usedWidth = shouldRotate ? spriteHeight : spriteWidth;
            int usedHeight = shouldRotate ? spriteWidth : spriteHeight;

            // Add new free rectangles
            if (rect.width > usedWidth) {
                freeRects.push_back(MaxRectsNode(rect.x + usedWidth, rect.y,
                                                rect.width - usedWidth, usedHeight));
            }
            if (rect.height > usedHeight) {
                freeRects.push_back(MaxRectsNode(rect.x, rect.y + usedHeight,
                                                rect.width, rect.height - usedHeight));
            }

            rect.used = true;
        }

        if (allFit) {
            m_Width = settings.powerOfTwo ? atlasWidth : atlasWidth;
            m_Height = settings.powerOfTwo ? atlasHeight : atlasHeight;
            break;
        }

        // Try larger size
        if (atlasWidth == atlasHeight) {
            atlasWidth *= 2;
        } else {
            atlasHeight *= 2;
        }
    }

    if (m_Sprites.size() != inputs.size()) {
        return false;
    }

    // Create atlas texture
    m_AtlasData = new unsigned char[m_Width * m_Height * 4];
    std::memset(m_AtlasData, 0, m_Width * m_Height * 4);

    // Copy sprites to atlas
    for (auto& input : inputs) {
        const auto* sprite = GetSprite(input.name);
        if (sprite) {
            CopySprite(m_AtlasData, input.data, sprite->rect,
                      input.width, input.height, m_Width, sprite->rotated);
        }
    }

    return CreateTexture(m_AtlasData, m_Width, m_Height, settings.generateMipmaps);
}

bool TextureAtlas::PackSimple(std::vector<AtlasInput>& inputs, const AtlasSettings& settings) {
    // Simple row-based packing
    int currentX = 0;
    int currentY = 0;
    int rowHeight = 0;
    int maxWidth = 0;

    m_Sprites.clear();

    for (auto& input : inputs) {
        int spriteWidth = input.width + settings.padding * 2;
        int spriteHeight = input.height + settings.padding * 2;

        // Check if we need a new row
        if (currentX + spriteWidth > settings.maxWidth) {
            currentX = 0;
            currentY += rowHeight;
            rowHeight = 0;
        }

        // Check if we exceed max height
        if (currentY + spriteHeight > settings.maxHeight) {
            std::cerr << "Sprites don't fit in atlas with simple packing" << std::endl;
            return false;
        }

        // Place sprite
        AtlasSprite sprite;
        sprite.name = input.name;
        sprite.rect = AtlasRect(currentX + settings.padding, currentY + settings.padding,
                               input.width, input.height);
        sprite.rotated = false;

        m_Sprites[sprite.name] = sprite;

        currentX += spriteWidth;
        rowHeight = std::max(rowHeight, spriteHeight);
        maxWidth = std::max(maxWidth, currentX);
    }

    m_Width = settings.powerOfTwo ? NextPowerOfTwo(maxWidth) : maxWidth;
    m_Height = settings.powerOfTwo ? NextPowerOfTwo(currentY + rowHeight) : currentY + rowHeight;

    // Update UVs with actual atlas size
    for (auto& pair : m_Sprites) {
        pair.second.uv = CalculateUV(pair.second.rect);
    }

    // Create atlas texture
    m_AtlasData = new unsigned char[m_Width * m_Height * 4];
    std::memset(m_AtlasData, 0, m_Width * m_Height * 4);

    // Copy sprites to atlas
    for (auto& input : inputs) {
        const auto* sprite = GetSprite(input.name);
        if (sprite) {
            CopySprite(m_AtlasData, input.data, sprite->rect,
                      input.width, input.height, m_Width, sprite->rotated);
        }
    }

    return CreateTexture(m_AtlasData, m_Width, m_Height, settings.generateMipmaps);
}

AtlasRect TextureAtlas::TrimAlpha(const unsigned char* data, int width, int height) {
    // Find the bounding box of non-transparent pixels
    int minX = width, minY = height, maxX = 0, maxY = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            if (data[index + 3] > 0) { // Check alpha channel
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }

    if (minX > maxX || minY > maxY) {
        // Completely transparent
        return AtlasRect(0, 0, 0, 0);
    }

    AtlasRect rect;
    rect.x = minX;
    rect.y = minY;
    rect.width = maxX - minX + 1;
    rect.height = maxY - minY + 1;
    rect.originalWidth = width;
    rect.originalHeight = height;
    rect.offsetX = minX;
    rect.offsetY = minY;

    return rect;
}

bool TextureAtlas::CreateTexture(const unsigned char* data, int width, int height, bool generateMipmaps) {
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

int TextureAtlas::NextPowerOfTwo(int value) const {
    int power = 1;
    while (power < value) {
        power *= 2;
    }
    return power;
}

void TextureAtlas::CopySprite(unsigned char* atlasData, const unsigned char* spriteData,
                               const AtlasRect& rect, int spriteWidth, int spriteHeight,
                               int atlasWidth, bool rotated) {
    if (rotated) {
        // Copy with 90-degree rotation
        for (int y = 0; y < spriteHeight; ++y) {
            for (int x = 0; x < spriteWidth; ++x) {
                int srcIndex = (y * spriteWidth + x) * 4;
                int dstX = rect.x + y;
                int dstY = rect.y + (spriteWidth - 1 - x);
                int dstIndex = (dstY * atlasWidth + dstX) * 4;

                atlasData[dstIndex + 0] = spriteData[srcIndex + 0];
                atlasData[dstIndex + 1] = spriteData[srcIndex + 1];
                atlasData[dstIndex + 2] = spriteData[srcIndex + 2];
                atlasData[dstIndex + 3] = spriteData[srcIndex + 3];
            }
        }
    } else {
        // Copy normally
        for (int y = 0; y < spriteHeight; ++y) {
            int srcIndex = y * spriteWidth * 4;
            int dstIndex = ((rect.y + y) * atlasWidth + rect.x) * 4;
            std::memcpy(&atlasData[dstIndex], &spriteData[srcIndex], spriteWidth * 4);
        }
    }
}

// AtlasManager implementation

AtlasManager& AtlasManager::GetInstance() {
    static AtlasManager instance;
    return instance;
}

std::shared_ptr<TextureAtlas> AtlasManager::GetAtlas(const std::string& name) {
    auto it = m_Atlases.find(name);
    if (it != m_Atlases.end()) {
        return it->second;
    }
    return nullptr;
}

void AtlasManager::AddAtlas(const std::string& name, std::shared_ptr<TextureAtlas> atlas) {
    m_Atlases[name] = atlas;
}

void AtlasManager::RemoveAtlas(const std::string& name) {
    m_Atlases.erase(name);
}

void AtlasManager::Clear() {
    m_Atlases.clear();
}

} // namespace Assets
