#include "Animation.h"
#include <algorithm>

namespace Rendering {

// AnimationClip implementation
AnimationClip::AnimationClip(const std::string& name)
    : m_Name(name)
    , m_Looping(true)
    , m_Speed(1.0f) {
}

void AnimationClip::AddFrame(const AnimationFrame& frame) {
    m_Frames.push_back(frame);
}

void AnimationClip::AddFrame(const glm::vec4& uvRect, float duration) {
    m_Frames.emplace_back(uvRect, duration);
}

void AnimationClip::CreateFromGrid(int frameCount, int columns, int rows,
                                  float frameDuration, int startFrame) {
    m_Frames.clear();

    float cellWidth = 1.0f / columns;
    float cellHeight = 1.0f / rows;

    for (int i = 0; i < frameCount; i++) {
        int frameIndex = startFrame + i;
        int col = frameIndex % columns;
        int row = frameIndex / columns;

        glm::vec4 uvRect(
            col * cellWidth,
            row * cellHeight,
            cellWidth,
            cellHeight
        );

        AddFrame(uvRect, frameDuration);
    }
}

float AnimationClip::GetTotalDuration() const {
    float total = 0.0f;
    for (const auto& frame : m_Frames) {
        total += frame.duration;
    }
    return total / m_Speed;
}

// AnimationController implementation
AnimationController::AnimationController()
    : m_CurrentFrame(0)
    , m_CurrentTime(0.0f)
    , m_FrameTimer(0.0f)
    , m_IsPlaying(false)
    , m_IsPaused(false)
    , m_Speed(1.0f) {
}

void AnimationController::Update(float deltaTime) {
    if (!m_IsPlaying || m_IsPaused || !m_CurrentClip) {
        return;
    }

    if (m_CurrentClip->GetFrameCount() == 0) {
        return;
    }

    // Apply speed multiplier
    deltaTime *= m_Speed * m_CurrentClip->GetSpeed();

    m_CurrentTime += deltaTime;
    m_FrameTimer += deltaTime;

    // Get current frame duration
    const AnimationFrame& currentFrame = m_CurrentClip->GetFrame(m_CurrentFrame);

    // Check if it's time to advance to next frame
    if (m_FrameTimer >= currentFrame.duration) {
        m_FrameTimer -= currentFrame.duration;
        AdvanceFrame();
    }
}

void AnimationController::AddClip(const std::string& name, const AnimationClip& clip) {
    auto clipPtr = std::make_shared<AnimationClip>(clip);
    m_Clips[name] = clipPtr;
}

void AnimationController::AddClip(std::shared_ptr<AnimationClip> clip) {
    if (clip) {
        m_Clips[clip->GetName()] = clip;
    }
}

void AnimationController::Play(const std::string& name, bool forceRestart) {
    auto it = m_Clips.find(name);
    if (it == m_Clips.end()) {
        return;
    }

    // If already playing this animation and not forcing restart, continue
    if (m_CurrentAnimationName == name && m_IsPlaying && !forceRestart) {
        return;
    }

    m_CurrentAnimationName = name;
    m_CurrentClip = it->second;
    m_CurrentFrame = 0;
    m_CurrentTime = 0.0f;
    m_FrameTimer = 0.0f;
    m_IsPlaying = true;
    m_IsPaused = false;
}

void AnimationController::Stop() {
    m_IsPlaying = false;
    m_CurrentFrame = 0;
    m_CurrentTime = 0.0f;
    m_FrameTimer = 0.0f;
}

void AnimationController::Pause() {
    m_IsPaused = true;
}

void AnimationController::Resume() {
    m_IsPaused = false;
}

void AnimationController::SetFrame(size_t frameIndex) {
    if (!m_CurrentClip || frameIndex >= m_CurrentClip->GetFrameCount()) {
        return;
    }

    m_CurrentFrame = frameIndex;
    m_FrameTimer = 0.0f;

    // Calculate time to this frame
    m_CurrentTime = 0.0f;
    for (size_t i = 0; i < frameIndex; i++) {
        m_CurrentTime += m_CurrentClip->GetFrame(i).duration;
    }
}

void AnimationController::SetTime(float time) {
    if (!m_CurrentClip || m_CurrentClip->GetFrameCount() == 0) {
        return;
    }

    m_CurrentTime = time;
    m_CurrentFrame = 0;
    m_FrameTimer = 0.0f;

    // Find the frame at this time
    float accumulatedTime = 0.0f;
    for (size_t i = 0; i < m_CurrentClip->GetFrameCount(); i++) {
        float frameDuration = m_CurrentClip->GetFrame(i).duration;
        if (accumulatedTime + frameDuration > time) {
            m_CurrentFrame = i;
            m_FrameTimer = time - accumulatedTime;
            break;
        }
        accumulatedTime += frameDuration;
    }
}

glm::vec4 AnimationController::GetCurrentFrameUV() const {
    if (!m_CurrentClip || m_CurrentClip->GetFrameCount() == 0) {
        return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    return m_CurrentClip->GetFrame(m_CurrentFrame).uvRect;
}

bool AnimationController::HasClip(const std::string& name) const {
    return m_Clips.find(name) != m_Clips.end();
}

std::shared_ptr<AnimationClip> AnimationController::GetClip(const std::string& name) {
    auto it = m_Clips.find(name);
    if (it != m_Clips.end()) {
        return it->second;
    }
    return nullptr;
}

void AnimationController::AdvanceFrame() {
    if (!m_CurrentClip) {
        return;
    }

    size_t oldFrame = m_CurrentFrame;
    m_CurrentFrame++;

    // Check if we've reached the end
    if (m_CurrentFrame >= m_CurrentClip->GetFrameCount()) {
        if (m_CurrentClip->IsLooping()) {
            m_CurrentFrame = 0;
            m_CurrentTime = 0.0f;
        } else {
            m_CurrentFrame = m_CurrentClip->GetFrameCount() - 1;
            m_IsPlaying = false;

            // Trigger completion event
            if (m_OnComplete) {
                m_OnComplete(m_CurrentAnimationName);
            }
        }
    }

    // Trigger frame change event
    if (oldFrame != m_CurrentFrame && m_OnFrameChange) {
        m_OnFrameChange(m_CurrentAnimationName);
    }
}

// SpriteSheet implementation
SpriteSheet::SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows)
    : m_Texture(texture)
    , m_Columns(columns)
    , m_Rows(rows) {

    m_CellSize = glm::vec2(1.0f / columns, 1.0f / rows);
}

glm::vec4 SpriteSheet::GetCellUV(int index) const {
    int col = index % m_Columns;
    int row = index / m_Columns;
    return GetCellUV(col, row);
}

glm::vec4 SpriteSheet::GetCellUV(int column, int row) const {
    return glm::vec4(
        column * m_CellSize.x,
        row * m_CellSize.y,
        m_CellSize.x,
        m_CellSize.y
    );
}

AnimationClip SpriteSheet::CreateAnimation(const std::string& name,
                                          int startFrame, int endFrame,
                                          float frameDuration,
                                          bool looping) {
    AnimationClip clip(name);
    clip.SetLooping(looping);

    for (int i = startFrame; i <= endFrame; i++) {
        clip.AddFrame(GetCellUV(i), frameDuration);
    }

    return clip;
}

} // namespace Rendering
