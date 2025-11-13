#pragma once

#include "ECS/Entity.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

namespace Engine {

/**
 * @brief Scene class representing a game scene/level
 *
 * Scenes contain entities and manage their lifecycle.
 * Only one scene can be active at a time.
 */
class Scene {
public:
    /**
     * @brief Construct a scene with a name
     * @param name Scene name for identification
     */
    explicit Scene(const std::string& name);

    /**
     * @brief Destructor - cleans up all entities
     */
    virtual ~Scene();

    // Prevent copying
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Allow moving
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    /**
     * @brief Called when the scene becomes active
     */
    virtual void OnEnter() {}

    /**
     * @brief Called when the scene becomes inactive
     */
    virtual void OnExit() {}

    /**
     * @brief Update all entities in the scene
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void Update(double deltaTime);

    /**
     * @brief Fixed update for all entities in the scene
     * @param fixedDeltaTime Fixed timestep in seconds
     */
    virtual void FixedUpdate(double fixedDeltaTime);

    /**
     * @brief Render the scene (to be implemented by derived classes)
     */
    virtual void Render() {}

    /**
     * @brief Create a new entity in the scene
     * @param name Entity name for debugging
     * @return Shared pointer to the created entity
     */
    std::shared_ptr<Entity> CreateEntity(const std::string& name = "Entity");

    /**
     * @brief Get an entity by ID
     * @param id Entity ID
     * @return Shared pointer to the entity, or nullptr if not found
     */
    std::shared_ptr<Entity> GetEntity(EntityID id) const;

    /**
     * @brief Get an entity by name (returns first match)
     * @param name Entity name
     * @return Shared pointer to the entity, or nullptr if not found
     */
    std::shared_ptr<Entity> GetEntityByName(const std::string& name) const;

    /**
     * @brief Get all entities in the scene
     * @return Vector of entity pointers
     */
    const std::vector<std::shared_ptr<Entity>>& GetAllEntities() const;

    /**
     * @brief Destroy an entity by ID
     * @param id Entity ID
     * @return True if entity was destroyed, false if not found
     */
    bool DestroyEntity(EntityID id);

    /**
     * @brief Destroy all entities in the scene
     */
    void DestroyAllEntities();

    /**
     * @brief Get the scene name
     * @return Scene name
     */
    const std::string& GetName() const { return m_Name; }

    /**
     * @brief Set the scene name
     * @param name New scene name
     */
    void SetName(const std::string& name) { m_Name = name; }

    /**
     * @brief Get the number of entities in the scene
     * @return Entity count
     */
    size_t GetEntityCount() const { return m_Entities.size(); }

private:
    std::string m_Name;
    std::vector<std::shared_ptr<Entity>> m_Entities;
    std::unordered_map<EntityID, std::shared_ptr<Entity>> m_EntityMap;
    EntityID m_NextEntityID = 1;
};

/**
 * @brief Scene manager for handling multiple scenes
 *
 * Manages scene loading, switching, and lifecycle.
 * Implements singleton pattern for global access.
 */
class SceneManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the scene manager instance
     */
    static SceneManager& GetInstance();

    /**
     * @brief Add a scene to the manager
     * @param name Scene name/identifier
     * @param scene Unique pointer to the scene
     */
    void AddScene(const std::string& name, std::unique_ptr<Scene> scene);

    /**
     * @brief Create and add a scene of a specific type
     * @tparam T Scene type (must derive from Scene)
     * @tparam Args Constructor argument types
     * @param name Scene name/identifier
     * @param args Constructor arguments
     */
    template<typename T, typename... Args>
    void CreateScene(const std::string& name, Args&&... args);

    /**
     * @brief Switch to a different scene
     * @param name Scene name to switch to
     * @return True if switch was successful, false if scene not found
     */
    bool SwitchScene(const std::string& name);

    /**
     * @brief Get the currently active scene
     * @return Pointer to the active scene, or nullptr if none
     */
    Scene* GetActiveScene() const;

    /**
     * @brief Get a scene by name
     * @param name Scene name
     * @return Pointer to the scene, or nullptr if not found
     */
    Scene* GetScene(const std::string& name) const;

    /**
     * @brief Remove a scene from the manager
     * @param name Scene name to remove
     * @return True if scene was removed, false if not found
     */
    bool RemoveScene(const std::string& name);

    /**
     * @brief Check if a scene exists
     * @param name Scene name
     * @return True if scene exists, false otherwise
     */
    bool HasScene(const std::string& name) const;

    /**
     * @brief Update the active scene
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void Update(double deltaTime);

    /**
     * @brief Fixed update for the active scene
     * @param fixedDeltaTime Fixed timestep in seconds
     */
    void FixedUpdate(double fixedDeltaTime);

    /**
     * @brief Render the active scene
     */
    void Render();

    /**
     * @brief Get all scene names
     * @return Vector of scene names
     */
    std::vector<std::string> GetSceneNames() const;

    /**
     * @brief Clear all scenes
     */
    void Clear();

private:
    SceneManager() = default;
    ~SceneManager() = default;

    // Prevent copying
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Scene>> m_Scenes;
    Scene* m_ActiveScene = nullptr;
    std::string m_ActiveSceneName;
};

// Template implementations

template<typename T, typename... Args>
void SceneManager::CreateScene(const std::string& name, Args&&... args) {
    static_assert(std::is_base_of<Scene, T>::value,
                  "T must derive from Scene");

    auto scene = std::make_unique<T>(std::forward<Args>(args)...);
    AddScene(name, std::move(scene));
}

} // namespace Engine
