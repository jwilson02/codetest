#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Rendering {

/**
 * 2D Camera system with smooth following, zoom, and shake effects
 */
class Camera {
public:
    Camera(float width, float height);

    // Update camera (call every frame)
    void Update(float deltaTime);

    // Get view-projection matrix
    glm::mat4 GetViewProjectionMatrix() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Camera position
    void SetPosition(const glm::vec2& position);
    void SetPosition(float x, float y);
    glm::vec2 GetPosition() const { return m_Position; }

    // Camera target (for smooth following)
    void SetTarget(const glm::vec2& target);
    void SetTarget(float x, float y);
    glm::vec2 GetTarget() const { return m_Target; }

    // Follow speed (0.0 = no following, 1.0 = instant)
    void SetFollowSpeed(float speed) { m_FollowSpeed = glm::clamp(speed, 0.0f, 1.0f); }
    float GetFollowSpeed() const { return m_FollowSpeed; }

    // Enable/disable smooth following
    void SetSmoothFollow(bool enable) { m_SmoothFollow = enable; }
    bool IsSmoothFollowEnabled() const { return m_SmoothFollow; }

    // Zoom
    void SetZoom(float zoom);
    float GetZoom() const { return m_Zoom; }
    void SetTargetZoom(float zoom);
    void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }

    // Screen shake
    void Shake(float intensity, float duration);
    void StopShake();

    // Viewport
    void SetViewportSize(float width, float height);
    glm::vec2 GetViewportSize() const { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }

    // Bounds constraints
    void SetBounds(const glm::vec4& bounds); // (minX, minY, maxX, maxY)
    void ClearBounds();
    bool HasBounds() const { return m_UseBounds; }

    // Convert screen to world coordinates
    glm::vec2 ScreenToWorld(const glm::vec2& screenPos) const;
    glm::vec2 WorldToScreen(const glm::vec2& worldPos) const;

    // Camera offset (for split-screen or UI)
    void SetOffset(const glm::vec2& offset) { m_Offset = offset; }
    glm::vec2 GetOffset() const { return m_Offset; }

    // Rotation (in radians)
    void SetRotation(float rotation) { m_Rotation = rotation; }
    float GetRotation() const { return m_Rotation; }

private:
    // Core properties
    glm::vec2 m_Position;
    glm::vec2 m_Target;
    float m_Zoom;
    float m_TargetZoom;
    float m_Rotation;

    // Viewport
    float m_ViewportWidth;
    float m_ViewportHeight;
    glm::vec2 m_Offset;

    // Smooth following
    bool m_SmoothFollow;
    float m_FollowSpeed;
    float m_ZoomSpeed;

    // Screen shake
    bool m_IsShaking;
    float m_ShakeIntensity;
    float m_ShakeDuration;
    float m_ShakeTimer;
    glm::vec2 m_ShakeOffset;

    // Bounds
    bool m_UseBounds;
    glm::vec4 m_Bounds; // (minX, minY, maxX, maxY)

    // Helper functions
    void UpdateShake(float deltaTime);
    void ApplyBounds();
    void UpdateMatrices();

    // Cached matrices
    mutable glm::mat4 m_ViewMatrix;
    mutable glm::mat4 m_ProjectionMatrix;
    mutable glm::mat4 m_ViewProjectionMatrix;
    mutable bool m_MatricesDirty;
};

} // namespace Rendering
