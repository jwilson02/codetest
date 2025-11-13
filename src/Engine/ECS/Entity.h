#pragma once

#include "Component.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <algorithm>
#include <stdexcept>
#include <string>

namespace Engine {

// Forward declaration
class Scene;

/**
 * @brief Unique identifier for entities
 */
using EntityID = uint64_t;

/**
 * @brief Entity class for the ECS system
 *
 * Entities are containers for components. They represent game objects
 * and manage the lifecycle of their components.
 */
class Entity {
public:
    /**
     * @brief Construct an entity with a unique ID
     * @param id Unique entity identifier
     * @param name Optional entity name for debugging
     */
    explicit Entity(EntityID id, const std::string& name = "Entity");

    /**
     * @brief Destructor - cleans up all components
     */
    ~Entity();

    // Prevent copying
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Allow moving
    Entity(Entity&&) noexcept = default;
    Entity& operator=(Entity&&) noexcept = default;

    /**
     * @brief Add a component to the entity
     * @tparam T Component type (must derive from Component)
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Shared pointer to the created component
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args);

    /**
     * @brief Get a component from the entity
     * @tparam T Component type
     * @return Shared pointer to the component, or nullptr if not found
     */
    template<typename T>
    std::shared_ptr<T> GetComponent() const;

    /**
     * @brief Check if the entity has a specific component
     * @tparam T Component type
     * @return True if component exists, false otherwise
     */
    template<typename T>
    bool HasComponent() const;

    /**
     * @brief Remove a component from the entity
     * @tparam T Component type
     * @return True if component was removed, false if not found
     */
    template<typename T>
    bool RemoveComponent();

    /**
     * @brief Get all components attached to this entity
     * @return Vector of component pointers
     */
    const std::vector<ComponentPtr>& GetAllComponents() const;

    /**
     * @brief Update all active components
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void Update(double deltaTime);

    /**
     * @brief Fixed update for all active components
     * @param fixedDeltaTime Fixed timestep in seconds
     */
    void FixedUpdate(double fixedDeltaTime);

    /**
     * @brief Get the entity's unique ID
     * @return Entity ID
     */
    EntityID GetID() const { return m_ID; }

    /**
     * @brief Get the entity's name
     * @return Entity name
     */
    const std::string& GetName() const { return m_Name; }

    /**
     * @brief Set the entity's name
     * @param name New entity name
     */
    void SetName(const std::string& name) { m_Name = name; }

    /**
     * @brief Check if the entity is active
     * @return True if active, false otherwise
     */
    bool IsActive() const { return m_Active; }

    /**
     * @brief Set the active state of the entity
     * @param active New active state
     */
    void SetActive(bool active) { m_Active = active; }

    /**
     * @brief Get the scene this entity belongs to
     * @return Pointer to the scene
     */
    Scene* GetScene() const { return m_Scene; }

    /**
     * @brief Set the scene this entity belongs to (called by Scene)
     * @param scene Pointer to the scene
     */
    void SetScene(Scene* scene) { m_Scene = scene; }

private:
    EntityID m_ID;
    std::string m_Name;
    bool m_Active = true;
    Scene* m_Scene = nullptr;

    std::vector<ComponentPtr> m_Components;
    std::unordered_map<std::type_index, ComponentPtr> m_ComponentMap;
};

// Template implementations

template<typename T, typename... Args>
std::shared_ptr<T> Entity::AddComponent(Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    std::type_index typeIndex = std::type_index(typeid(T));

    // Check if component already exists
    if (m_ComponentMap.find(typeIndex) != m_ComponentMap.end()) {
        throw std::runtime_error("Entity already has component of type " +
                                 std::string(typeid(T).name()));
    }

    // Create the component
    auto component = std::make_shared<T>(std::forward<Args>(args)...);
    component->SetEntity(this);

    // Store in both containers
    m_Components.push_back(component);
    m_ComponentMap[typeIndex] = component;

    // Call OnAttach
    component->OnAttach();

    return component;
}

template<typename T>
std::shared_ptr<T> Entity::GetComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = m_ComponentMap.find(typeIndex);

    if (it != m_ComponentMap.end()) {
        return std::static_pointer_cast<T>(it->second);
    }

    return nullptr;
}

template<typename T>
bool Entity::HasComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    std::type_index typeIndex = std::type_index(typeid(T));
    return m_ComponentMap.find(typeIndex) != m_ComponentMap.end();
}

template<typename T>
bool Entity::RemoveComponent() {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = m_ComponentMap.find(typeIndex);

    if (it == m_ComponentMap.end()) {
        return false;
    }

    // Call OnDetach
    it->second->OnDetach();

    // Remove from vector
    m_Components.erase(
        std::remove(m_Components.begin(), m_Components.end(), it->second),
        m_Components.end()
    );

    // Remove from map
    m_ComponentMap.erase(it);

    return true;
}

} // namespace Engine
