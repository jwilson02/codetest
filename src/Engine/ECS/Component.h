#pragma once

#include <memory>
#include <string>
#include <typeindex>

namespace Engine {

// Forward declaration
class Entity;

/**
 * @brief Base class for all components in the ECS system
 *
 * Components are pure data containers that can be attached to entities.
 * They should not contain game logic - that belongs in systems.
 */
class Component {
public:
    virtual ~Component() = default;

    /**
     * @brief Called when the component is added to an entity
     */
    virtual void OnAttach() {}

    /**
     * @brief Called when the component is removed from an entity
     */
    virtual void OnDetach() {}

    /**
     * @brief Called every frame to update the component
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void Update(double deltaTime) { (void)deltaTime; }

    /**
     * @brief Called at fixed intervals for physics updates
     * @param fixedDeltaTime Fixed timestep in seconds
     */
    virtual void FixedUpdate(double fixedDeltaTime) { (void)fixedDeltaTime; }

    /**
     * @brief Get the type name of the component
     * @return Type name as string
     */
    virtual std::string GetTypeName() const = 0;

    /**
     * @brief Check if the component is active
     * @return True if active, false otherwise
     */
    bool IsActive() const { return m_Active; }

    /**
     * @brief Set the active state of the component
     * @param active New active state
     */
    void SetActive(bool active) { m_Active = active; }

    /**
     * @brief Get the owning entity
     * @return Pointer to the owning entity
     */
    Entity* GetEntity() const { return m_Owner; }

protected:
    friend class Entity;

    /**
     * @brief Set the owning entity (called by Entity class)
     * @param owner Pointer to the owning entity
     */
    void SetEntity(Entity* owner) { m_Owner = owner; }

private:
    Entity* m_Owner = nullptr;
    bool m_Active = true;
};

/**
 * @brief Helper template to automatically implement GetTypeName
 */
template<typename T>
class ComponentBase : public Component {
public:
    static std::type_index GetStaticType() {
        return std::type_index(typeid(T));
    }

    std::string GetTypeName() const override {
        return typeid(T).name();
    }
};

// Common component type aliases for convenience
using ComponentPtr = std::shared_ptr<Component>;
using ComponentUniquePtr = std::unique_ptr<Component>;

} // namespace Engine
