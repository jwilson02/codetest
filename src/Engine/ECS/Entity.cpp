#include "Entity.h"

namespace Engine {

Entity::Entity(EntityID id, const std::string& name)
    : m_ID(id), m_Name(name) {
}

Entity::~Entity() {
    // Call OnDetach for all components
    for (auto& component : m_Components) {
        if (component) {
            component->OnDetach();
        }
    }
    m_Components.clear();
    m_ComponentMap.clear();
}

const std::vector<ComponentPtr>& Entity::GetAllComponents() const {
    return m_Components;
}

void Entity::Update(double deltaTime) {
    if (!m_Active) {
        return;
    }

    for (auto& component : m_Components) {
        if (component && component->IsActive()) {
            component->Update(deltaTime);
        }
    }
}

void Entity::FixedUpdate(double fixedDeltaTime) {
    if (!m_Active) {
        return;
    }

    for (auto& component : m_Components) {
        if (component && component->IsActive()) {
            component->FixedUpdate(fixedDeltaTime);
        }
    }
}

} // namespace Engine
