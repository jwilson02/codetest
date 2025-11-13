#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Texture.h"

namespace Rendering {

/**
 * Animation frame data
 */
struct AnimationFrame {
    glm::vec4 uvRect;  // UV coordinates (x, y, width, height)
    float duration;    // Duration in seconds

    AnimationFrame(const glm::vec4& uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), float dur = 0.1f)
        : uvRect(uv), duration(dur) {
    }
};

/**
 * Animation clip containing multiple frames
 */
class AnimationClip {
public:
    AnimationClip(const std::string& name = "");

    // Add frame to animation
    void AddFrame(const AnimationFrame& frame);
    void AddFrame(const glm::vec4& uvRect, float duration = 0.1f);

    // Create animation from sprite sheet grid
    void CreateFromGrid(int frameCount, int columns, int rows,
                       float frameDuration = 0.1f,
                       int startFrame = 0);

    // Getters
    const std::string& GetName() const { return m_Name; }
    size_t GetFrameCount() const { return m_Frames.size(); }
    const AnimationFrame& GetFrame(size_t index) const { return m_Frames[index]; }
    float GetTotalDuration() const;

    // Properties
    void SetLooping(bool loop) { m_Looping = loop; }
    bool IsLooping() const { return m_Looping; }

    void SetSpeed(float speed) { m_Speed = speed; }
    float GetSpeed() const { return m_Speed; }

private:
    std::string m_Name;
    std::vector<AnimationFrame> m_Frames;
    bool m_Looping;
    float m_Speed;
};

/**
 * Animation state machine for controlling animations
 */
class AnimationController {
public:
    AnimationController();

    // Update animation (call every frame)
    void Update(float deltaTime);

    // Add animation clip
    void AddClip(const std::string& name, const AnimationClip& clip);
    void AddClip(std::shared_ptr<AnimationClip> clip);

    // Play animation
    void Play(const std::string& name, bool forceRestart = false);
    void Stop();
    void Pause();
    void Resume();

    // Animation control
    void SetSpeed(float speed) { m_Speed = speed; }
    float GetSpeed() const { return m_Speed; }

    void SetFrame(size_t frameIndex);
    void SetTime(float time);

    // Getters
    const std::string& GetCurrentAnimation() const { return m_CurrentAnimationName; }
    size_t GetCurrentFrame() const { return m_CurrentFrame; }
    float GetCurrentTime() const { return m_CurrentTime; }
    bool IsPlaying() const { return m_IsPlaying; }
    bool IsPaused() const { return m_IsPaused; }

    // Get current frame UV coordinates
    glm::vec4 GetCurrentFrameUV() const;

    // Check if animation exists
    bool HasClip(const std::string& name) const;

    // Get clip
    std::shared_ptr<AnimationClip> GetClip(const std::string& name);

    // Events
    using AnimationEventCallback = std::function<void(const std::string&)>;
    void SetOnAnimationComplete(AnimationEventCallback callback) { m_OnComplete = callback; }
    void SetOnFrameChange(AnimationEventCallback callback) { m_OnFrameChange = callback; }

private:
    std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_Clips;
    std::string m_CurrentAnimationName;
    std::shared_ptr<AnimationClip> m_CurrentClip;

    size_t m_CurrentFrame;
    float m_CurrentTime;
    float m_FrameTimer;

    bool m_IsPlaying;
    bool m_IsPaused;
    float m_Speed;

    // Event callbacks
    AnimationEventCallback m_OnComplete;
    AnimationEventCallback m_OnFrameChange;

    void AdvanceFrame();
};

/**
 * Sprite sheet helper for easy sprite sheet management
 */
class SpriteSheet {
public:
    SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows);

    // Get UV rect for a specific cell
    glm::vec4 GetCellUV(int index) const;
    glm::vec4 GetCellUV(int column, int row) const;

    // Create animation from range
    AnimationClip CreateAnimation(const std::string& name,
                                 int startFrame, int endFrame,
                                 float frameDuration = 0.1f,
                                 bool looping = true);

    // Getters
    std::shared_ptr<Texture> GetTexture() const { return m_Texture; }
    int GetColumns() const { return m_Columns; }
    int GetRows() const { return m_Rows; }
    glm::vec2 GetCellSize() const { return m_CellSize; }

private:
    std::shared_ptr<Texture> m_Texture;
    int m_Columns;
    int m_Rows;
    glm::vec2 m_CellSize;
};

} // namespace Rendering
