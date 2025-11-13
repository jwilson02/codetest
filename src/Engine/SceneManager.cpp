#include "SceneManager.h"
#include <iostream>

namespace Engine {

// Scene implementation

Scene::Scene(const std::string& name)
    : m_Name(name) {
}

Scene::~Scene() {
    DestroyAllEntities();
}

void Scene::Update(double deltaTime) {
    for (auto& entity : m_Entities) {
        if (entity && entity->IsActive()) {
            entity->Update(deltaTime);
        }
    }
}

void Scene::FixedUpdate(double fixedDeltaTime) {
    for (auto& entity : m_Entities) {
        if (entity && entity->IsActive()) {
            entity->FixedUpdate(fixedDeltaTime);
        }
    }
}

std::shared_ptr<Entity> Scene::CreateEntity(const std::string& name) {
    EntityID id = m_NextEntityID++;
    auto entity = std::make_shared<Entity>(id, name);
    entity->SetScene(this);

    m_Entities.push_back(entity);
    m_EntityMap[id] = entity;

    return entity;
}

std::shared_ptr<Entity> Scene::GetEntity(EntityID id) const {
    auto it = m_EntityMap.find(id);
    return (it != m_EntityMap.end()) ? it->second : nullptr;
}

std::shared_ptr<Entity> Scene::GetEntityByName(const std::string& name) const {
    auto it = std::find_if(m_Entities.begin(), m_Entities.end(),
        [&name](const std::shared_ptr<Entity>& entity) {
            return entity && entity->GetName() == name;
        });

    return (it != m_Entities.end()) ? *it : nullptr;
}

const std::vector<std::shared_ptr<Entity>>& Scene::GetAllEntities() const {
    return m_Entities;
}

bool Scene::DestroyEntity(EntityID id) {
    auto it = m_EntityMap.find(id);
    if (it == m_EntityMap.end()) {
        return false;
    }

    // Remove from vector
    m_Entities.erase(
        std::remove(m_Entities.begin(), m_Entities.end(), it->second),
        m_Entities.end()
    );

    // Remove from map
    m_EntityMap.erase(it);

    return true;
}

void Scene::DestroyAllEntities() {
    m_Entities.clear();
    m_EntityMap.clear();
}

// SceneManager implementation

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::AddScene(const std::string& name, std::unique_ptr<Scene> scene) {
    if (!scene) {
        std::cerr << "[SceneManager] Error: Cannot add null scene" << std::endl;
        return;
    }

    if (m_Scenes.find(name) != m_Scenes.end()) {
        std::cerr << "[SceneManager] Warning: Scene '" << name
                  << "' already exists. Replacing." << std::endl;
    }

    scene->SetName(name);
    m_Scenes[name] = std::move(scene);

    std::cout << "[SceneManager] Added scene: " << name << std::endl;
}

bool SceneManager::SwitchScene(const std::string& name) {
    auto it = m_Scenes.find(name);
    if (it == m_Scenes.end()) {
        std::cerr << "[SceneManager] Error: Scene '" << name
                  << "' not found" << std::endl;
        return false;
    }

    // Call OnExit on current scene
    if (m_ActiveScene) {
        std::cout << "[SceneManager] Exiting scene: " << m_ActiveSceneName << std::endl;
        m_ActiveScene->OnExit();
    }

    // Switch to new scene
    m_ActiveScene = it->second.get();
    m_ActiveSceneName = name;

    // Call OnEnter on new scene
    std::cout << "[SceneManager] Entering scene: " << name << std::endl;
    m_ActiveScene->OnEnter();

    return true;
}

Scene* SceneManager::GetActiveScene() const {
    return m_ActiveScene;
}

Scene* SceneManager::GetScene(const std::string& name) const {
    auto it = m_Scenes.find(name);
    return (it != m_Scenes.end()) ? it->second.get() : nullptr;
}

bool SceneManager::RemoveScene(const std::string& name) {
    auto it = m_Scenes.find(name);
    if (it == m_Scenes.end()) {
        return false;
    }

    // If this is the active scene, deactivate it
    if (m_ActiveScene == it->second.get()) {
        m_ActiveScene->OnExit();
        m_ActiveScene = nullptr;
        m_ActiveSceneName.clear();
    }

    m_Scenes.erase(it);
    std::cout << "[SceneManager] Removed scene: " << name << std::endl;

    return true;
}

bool SceneManager::HasScene(const std::string& name) const {
    return m_Scenes.find(name) != m_Scenes.end();
}

void SceneManager::Update(double deltaTime) {
    if (m_ActiveScene) {
        m_ActiveScene->Update(deltaTime);
    }
}

void SceneManager::FixedUpdate(double fixedDeltaTime) {
    if (m_ActiveScene) {
        m_ActiveScene->FixedUpdate(fixedDeltaTime);
    }
}

void SceneManager::Render() {
    if (m_ActiveScene) {
        m_ActiveScene->Render();
    }
}

std::vector<std::string> SceneManager::GetSceneNames() const {
    std::vector<std::string> names;
    names.reserve(m_Scenes.size());

    for (const auto& pair : m_Scenes) {
        names.push_back(pair.first);
    }

    return names;
}

void SceneManager::Clear() {
    if (m_ActiveScene) {
        m_ActiveScene->OnExit();
        m_ActiveScene = nullptr;
        m_ActiveSceneName.clear();
    }

    m_Scenes.clear();
    std::cout << "[SceneManager] Cleared all scenes" << std::endl;
}

} // namespace Engine
