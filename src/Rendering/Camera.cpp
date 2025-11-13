#include "Camera.h"
#include <cmath>
#include <algorithm>

namespace Rendering {

Camera::Camera(float width, float height)
    : m_Position(0.0f, 0.0f)
    , m_Target(0.0f, 0.0f)
    , m_Zoom(1.0f)
    , m_TargetZoom(1.0f)
    , m_Rotation(0.0f)
    , m_ViewportWidth(width)
    , m_ViewportHeight(height)
    , m_Offset(0.0f, 0.0f)
    , m_SmoothFollow(true)
    , m_FollowSpeed(0.1f)
    , m_ZoomSpeed(5.0f)
    , m_IsShaking(false)
    , m_ShakeIntensity(0.0f)
    , m_ShakeDuration(0.0f)
    , m_ShakeTimer(0.0f)
    , m_ShakeOffset(0.0f, 0.0f)
    , m_UseBounds(false)
    , m_Bounds(0.0f, 0.0f, 0.0f, 0.0f)
    , m_MatricesDirty(true) {
}

void Camera::Update(float deltaTime) {
    // Update smooth following
    if (m_SmoothFollow) {
        glm::vec2 direction = m_Target - m_Position;
        m_Position += direction * m_FollowSpeed;
    } else {
        m_Position = m_Target;
    }

    // Update zoom
    if (std::abs(m_Zoom - m_TargetZoom) > 0.001f) {
        float zoomDelta = (m_TargetZoom - m_Zoom) * m_ZoomSpeed * deltaTime;
        m_Zoom += zoomDelta;
        m_MatricesDirty = true;
    }

    // Update screen shake
    if (m_IsShaking) {
        UpdateShake(deltaTime);
    }

    // Apply bounds constraints
    if (m_UseBounds) {
        ApplyBounds();
    }

    m_MatricesDirty = true;
}

glm::mat4 Camera::GetViewProjectionMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera*>(this)->UpdateMatrices();
    }
    return m_ViewProjectionMatrix;
}

glm::mat4 Camera::GetViewMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera*>(this)->UpdateMatrices();
    }
    return m_ViewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const {
    if (m_MatricesDirty) {
        const_cast<Camera*>(this)->UpdateMatrices();
    }
    return m_ProjectionMatrix;
}

void Camera::SetPosition(const glm::vec2& position) {
    m_Position = position;
    m_Target = position;
    m_MatricesDirty = true;
}

void Camera::SetPosition(float x, float y) {
    SetPosition(glm::vec2(x, y));
}

void Camera::SetTarget(const glm::vec2& target) {
    m_Target = target;
    if (!m_SmoothFollow) {
        m_Position = target;
        m_MatricesDirty = true;
    }
}

void Camera::SetTarget(float x, float y) {
    SetTarget(glm::vec2(x, y));
}

void Camera::SetZoom(float zoom) {
    m_Zoom = std::max(0.1f, zoom);
    m_TargetZoom = m_Zoom;
    m_MatricesDirty = true;
}

void Camera::SetTargetZoom(float zoom) {
    m_TargetZoom = std::max(0.1f, zoom);
}

void Camera::Shake(float intensity, float duration) {
    m_IsShaking = true;
    m_ShakeIntensity = intensity;
    m_ShakeDuration = duration;
    m_ShakeTimer = 0.0f;
}

void Camera::StopShake() {
    m_IsShaking = false;
    m_ShakeOffset = glm::vec2(0.0f, 0.0f);
}

void Camera::SetViewportSize(float width, float height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_MatricesDirty = true;
}

void Camera::SetBounds(const glm::vec4& bounds) {
    m_Bounds = bounds;
    m_UseBounds = true;
    ApplyBounds();
}

void Camera::ClearBounds() {
    m_UseBounds = false;
}

glm::vec2 Camera::ScreenToWorld(const glm::vec2& screenPos) const {
    // Normalize screen coordinates to [-1, 1]
    glm::vec2 normalized;
    normalized.x = (screenPos.x / m_ViewportWidth) * 2.0f - 1.0f;
    normalized.y = 1.0f - (screenPos.y / m_ViewportHeight) * 2.0f;

    // Get inverse view-projection matrix
    glm::mat4 invViewProj = glm::inverse(GetViewProjectionMatrix());

    // Transform to world space
    glm::vec4 worldPos = invViewProj * glm::vec4(normalized, 0.0f, 1.0f);
    return glm::vec2(worldPos.x, worldPos.y);
}

glm::vec2 Camera::WorldToScreen(const glm::vec2& worldPos) const {
    // Transform to clip space
    glm::vec4 clipPos = GetViewProjectionMatrix() * glm::vec4(worldPos, 0.0f, 1.0f);

    // Normalize to [0, 1]
    glm::vec2 normalized;
    normalized.x = (clipPos.x + 1.0f) * 0.5f;
    normalized.y = (1.0f - clipPos.y) * 0.5f;

    // Convert to screen coordinates
    return glm::vec2(normalized.x * m_ViewportWidth, normalized.y * m_ViewportHeight);
}

void Camera::UpdateShake(float deltaTime) {
    m_ShakeTimer += deltaTime;

    if (m_ShakeTimer >= m_ShakeDuration) {
        StopShake();
        return;
    }

    // Calculate shake progress (0 to 1)
    float progress = m_ShakeTimer / m_ShakeDuration;
    float intensity = m_ShakeIntensity * (1.0f - progress); // Decay over time

    // Generate random offset
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    m_ShakeOffset.x = std::cos(angle) * intensity;
    m_ShakeOffset.y = std::sin(angle) * intensity;
}

void Camera::ApplyBounds() {
    float halfWidth = (m_ViewportWidth / m_Zoom) * 0.5f;
    float halfHeight = (m_ViewportHeight / m_Zoom) * 0.5f;

    m_Position.x = glm::clamp(m_Position.x, m_Bounds.x + halfWidth, m_Bounds.z - halfWidth);
    m_Position.y = glm::clamp(m_Position.y, m_Bounds.y + halfHeight, m_Bounds.w - halfHeight);
}

void Camera::UpdateMatrices() {
    // Calculate effective position with shake offset
    glm::vec2 effectivePosition = m_Position + m_ShakeOffset + m_Offset;

    // Create view matrix
    m_ViewMatrix = glm::mat4(1.0f);
    m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(-effectivePosition.x, -effectivePosition.y, 0.0f));

    // Apply rotation if needed
    if (m_Rotation != 0.0f) {
        m_ViewMatrix = glm::rotate(m_ViewMatrix, m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Apply zoom
    m_ViewMatrix = glm::scale(m_ViewMatrix, glm::vec3(m_Zoom, m_Zoom, 1.0f));

    // Create orthographic projection matrix
    float halfWidth = m_ViewportWidth * 0.5f;
    float halfHeight = m_ViewportHeight * 0.5f;
    m_ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);

    // Combine matrices
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

    m_MatricesDirty = false;
}

} // namespace Rendering
