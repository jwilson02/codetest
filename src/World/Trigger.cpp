#include "Trigger.h"
#include "TileMap.h"
#include <algorithm>
#include <cmath>

namespace World {

// ==================== Trigger Implementation ====================

Trigger::Trigger(int id, const std::string& name, TriggerType type)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_x(0), m_y(0)
    , m_width(100), m_height(100)
    , m_radius(50)
    , m_shape(CollisionShape::Rectangle)
    , m_enabled(true)
    , m_active(false)
    , m_oneShot(false)
    , m_triggered(false)
    , m_cooldown(0.0f)
    , m_remainingCooldown(0.0f)
    , m_collisionObject(nullptr)
    , m_userData(nullptr)
{
}

Trigger::~Trigger() {
}

bool Trigger::checkConditions() const {
    if (m_conditions.empty()) {
        return true;
    }

    for (const auto& condition : m_conditions) {
        bool result = true;

        // Check condition based on type
        switch (condition.type) {
            case TriggerCondition::None:
                result = true;
                break;

            case TriggerCondition::HasItem:
                // TODO: Check if player has item
                result = false;
                break;

            case TriggerCondition::QuestComplete:
                // TODO: Check if quest is complete
                result = false;
                break;

            case TriggerCondition::QuestActive:
                // TODO: Check if quest is active
                result = false;
                break;

            case TriggerCondition::LevelMin:
                // TODO: Check player level
                result = false;
                break;

            default:
                result = true;
                break;
        }

        // Apply inversion if needed
        if (condition.inverted) {
            result = !result;
        }

        // All conditions must be true
        if (!result) {
            return false;
        }
    }

    return true;
}

bool Trigger::canActivate() const {
    if (!m_enabled) return false;
    if (m_oneShot && m_triggered) return false;
    if (m_remainingCooldown > 0) return false;
    if (!checkConditions()) return false;

    return true;
}

void Trigger::activate() {
    if (!canActivate()) return;

    m_active = true;
    m_triggered = true;
    m_remainingCooldown = m_cooldown;

    executeActions();

    if (m_onActivate) {
        m_onActivate(this);
    }
}

void Trigger::deactivate() {
    m_active = false;
}

void Trigger::reset() {
    m_active = false;
    m_triggered = false;
    m_remainingCooldown = 0.0f;
}

void Trigger::update(float deltaTime) {
    if (m_remainingCooldown > 0) {
        m_remainingCooldown -= deltaTime;
        if (m_remainingCooldown < 0) {
            m_remainingCooldown = 0;
        }
    }
}

void Trigger::updateCollisionObject() {
    if (!m_collisionObject) return;

    switch (m_shape) {
        case CollisionShape::Rectangle:
            m_collisionObject->setRectangle(m_x, m_y, m_width, m_height);
            break;

        case CollisionShape::Circle:
            m_collisionObject->setCircle(m_x, m_y, m_radius);
            break;

        default:
            break;
    }

    m_collisionObject->setTrigger(true);
}

void Trigger::executeActions() {
    for (const auto& action : m_actions) {
        switch (action.type) {
            case TriggerAction::LoadMap:
                // TODO: Load map
                break;

            case TriggerAction::Teleport:
                // TODO: Teleport player
                break;

            case TriggerAction::SpawnEnemy:
                // TODO: Spawn enemy
                break;

            case TriggerAction::PlaySound:
                // TODO: Play sound
                break;

            case TriggerAction::PlayMusic:
                // TODO: Play music
                break;

            case TriggerAction::ShowMessage:
                // TODO: Show message
                break;

            case TriggerAction::StartQuest:
                // TODO: Start quest
                break;

            default:
                break;
        }
    }
}

// ==================== EnterTrigger Implementation ====================

void EnterTrigger::onEntityEnter(void* entity) {
    if (canActivate()) {
        activate();

        if (m_onEnter) {
            m_onEnter(this);
        }
    }
}

// ==================== ExitTrigger Implementation ====================

void ExitTrigger::onEntityExit(void* entity) {
    if (canActivate()) {
        activate();

        if (m_onExit) {
            m_onExit(this);
        }
    }
}

// ==================== ProximityTrigger Implementation ====================

void ProximityTrigger::update(float deltaTime) {
    Trigger::update(deltaTime);

    // Calculate distance to target
    float dx = m_targetX - m_x;
    float dy = m_targetY - m_y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= m_checkRadius && canActivate()) {
        activate();
    }
}

// ==================== TimedTrigger Implementation ====================

void TimedTrigger::update(float deltaTime) {
    Trigger::update(deltaTime);

    if (!m_enabled || (m_oneShot && m_triggered)) {
        return;
    }

    m_elapsed += deltaTime;

    if (m_elapsed >= m_delay && canActivate()) {
        activate();
        m_elapsed = 0.0f;
    }
}

void TimedTrigger::reset() {
    Trigger::reset();
    m_elapsed = 0.0f;
}

// ==================== ScriptTrigger Implementation ====================

void ScriptTrigger::activate() {
    if (!canActivate()) return;

    // TODO: Execute script
    // This would integrate with a scripting system

    Trigger::activate();
}

// ==================== TriggerManager Implementation ====================

TriggerManager::TriggerManager()
    : m_collisionSystem(nullptr)
    , m_nextTriggerID(1)
    , m_debugDraw(false)
{
}

TriggerManager::~TriggerManager() {
    shutdown();
}

bool TriggerManager::initialize(CollisionSystem* collisionSystem) {
    m_collisionSystem = collisionSystem;
    return true;
}

void TriggerManager::shutdown() {
    m_triggers.clear();
    m_triggerNameToID.clear();
    m_entities.clear();
}

Trigger* TriggerManager::createTrigger(TriggerType type, const std::string& name) {
    int id = m_nextTriggerID++;
    std::unique_ptr<Trigger> trigger;

    switch (type) {
        case TriggerType::Enter:
            trigger = std::make_unique<EnterTrigger>(id, name);
            break;

        case TriggerType::Exit:
            trigger = std::make_unique<ExitTrigger>(id, name);
            break;

        case TriggerType::Proximity:
            trigger = std::make_unique<ProximityTrigger>(id, name);
            break;

        case TriggerType::Timed:
            trigger = std::make_unique<TimedTrigger>(id, name);
            break;

        case TriggerType::Script:
            trigger = std::make_unique<ScriptTrigger>(id, name);
            break;

        default:
            trigger = std::make_unique<Trigger>(id, name, type);
            break;
    }

    // Create collision object
    if (m_collisionSystem) {
        CollisionObject* collisionObj = m_collisionSystem->createObject(CollisionShape::Rectangle);
        collisionObj->setTrigger(true);
        collisionObj->setLayer(CollisionLayers::LAYER_TRIGGER);
        collisionObj->setUserData(trigger.get());
        trigger->setCollisionObject(collisionObj);
    }

    Trigger* triggerPtr = trigger.get();
    m_triggers[id] = std::move(trigger);
    m_triggerNameToID[name] = id;

    return triggerPtr;
}

EnterTrigger* TriggerManager::createEnterTrigger(const std::string& name) {
    return static_cast<EnterTrigger*>(createTrigger(TriggerType::Enter, name));
}

ExitTrigger* TriggerManager::createExitTrigger(const std::string& name) {
    return static_cast<ExitTrigger*>(createTrigger(TriggerType::Exit, name));
}

ProximityTrigger* TriggerManager::createProximityTrigger(const std::string& name) {
    return static_cast<ProximityTrigger*>(createTrigger(TriggerType::Proximity, name));
}

TimedTrigger* TriggerManager::createTimedTrigger(const std::string& name) {
    return static_cast<TimedTrigger*>(createTrigger(TriggerType::Timed, name));
}

ScriptTrigger* TriggerManager::createScriptTrigger(const std::string& name) {
    return static_cast<ScriptTrigger*>(createTrigger(TriggerType::Script, name));
}

void TriggerManager::destroyTrigger(int triggerID) {
    auto it = m_triggers.find(triggerID);
    if (it != m_triggers.end()) {
        Trigger* trigger = it->second.get();

        // Remove from name map
        m_triggerNameToID.erase(trigger->getName());

        // Destroy collision object
        if (m_collisionSystem && trigger->getCollisionObject()) {
            m_collisionSystem->destroyObject(trigger->getCollisionObject());
        }

        m_triggers.erase(it);
    }
}

void TriggerManager::destroyTrigger(Trigger* trigger) {
    if (trigger) {
        destroyTrigger(trigger->getID());
    }
}

Trigger* TriggerManager::getTrigger(int triggerID) {
    auto it = m_triggers.find(triggerID);
    if (it != m_triggers.end()) {
        return it->second.get();
    }
    return nullptr;
}

Trigger* TriggerManager::getTrigger(const std::string& name) {
    auto it = m_triggerNameToID.find(name);
    if (it != m_triggerNameToID.end()) {
        return getTrigger(it->second);
    }
    return nullptr;
}

std::vector<Trigger*> TriggerManager::getTriggersInArea(float x, float y,
                                                         float width, float height) {
    std::vector<Trigger*> results;

    for (auto& pair : m_triggers) {
        Trigger* trigger = pair.second.get();
        if (!trigger->isEnabled()) continue;

        float tx = trigger->getX();
        float ty = trigger->getY();

        if (tx >= x && tx <= x + width && ty >= y && ty <= y + height) {
            results.push_back(trigger);
        }
    }

    return results;
}

std::vector<Trigger*> TriggerManager::getTriggersInRadius(float x, float y, float radius) {
    std::vector<Trigger*> results;
    float radiusSq = radius * radius;

    for (auto& pair : m_triggers) {
        Trigger* trigger = pair.second.get();
        if (!trigger->isEnabled()) continue;

        float dx = trigger->getX() - x;
        float dy = trigger->getY() - y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= radiusSq) {
            results.push_back(trigger);
        }
    }

    return results;
}

std::vector<Trigger*> TriggerManager::getTriggersByType(TriggerType type) {
    std::vector<Trigger*> results;

    for (auto& pair : m_triggers) {
        if (pair.second->getType() == type) {
            results.push_back(pair.second.get());
        }
    }

    return results;
}

void TriggerManager::registerEntity(void* entity, float x, float y) {
    EntityInfo info;
    info.x = x;
    info.y = y;
    m_entities[entity] = info;
}

void TriggerManager::updateEntityPosition(void* entity, float x, float y) {
    auto it = m_entities.find(entity);
    if (it == m_entities.end()) {
        registerEntity(entity, x, y);
        return;
    }

    float oldX = it->second.x;
    float oldY = it->second.y;

    it->second.x = x;
    it->second.y = y;

    handleEntityMovement(entity, oldX, oldY, x, y);
}

void TriggerManager::unregisterEntity(void* entity) {
    m_entities.erase(entity);
}

void TriggerManager::update(float deltaTime) {
    // Update all triggers
    for (auto& pair : m_triggers) {
        pair.second->update(deltaTime);
    }

    checkTriggers();
}

bool TriggerManager::loadTriggersFromMap(const std::string& mapFilename) {
    // TODO: Load triggers from map file
    return true;
}

bool TriggerManager::saveTriggersToMap(const std::string& mapFilename) const {
    // TODO: Save triggers to map file
    return true;
}

bool TriggerManager::loadTriggerFromJSON(const std::string& json) {
    // TODO: Parse JSON and create trigger
    return true;
}

void TriggerManager::renderDebug() {
    if (!m_debugDraw) return;

    // Render trigger zones
    // This would integrate with your rendering system
}

TriggerManager::Stats TriggerManager::getStats() const {
    Stats stats;
    stats.totalTriggers = m_triggers.size();
    stats.activeTriggers = 0;
    stats.triggeredCount = 0;

    for (const auto& pair : m_triggers) {
        if (pair.second->isActive()) {
            stats.activeTriggers++;
        }
        if (pair.second->isTriggered()) {
            stats.triggeredCount++;
        }
    }

    return stats;
}

void TriggerManager::checkTriggers() {
    // Check triggers against entities
    for (auto& entityPair : m_entities) {
        void* entity = entityPair.first;
        EntityInfo& info = entityPair.second;

        for (auto& triggerPair : m_triggers) {
            Trigger* trigger = triggerPair.second.get();
            if (!trigger->isEnabled()) continue;

            // Check if entity is inside trigger
            bool inside = false;

            if (trigger->getShape() == CollisionShape::Rectangle) {
                float tx = trigger->getX();
                float ty = trigger->getY();
                float tw = trigger->getWidth();
                float th = trigger->getHeight();

                inside = (info.x >= tx && info.x <= tx + tw &&
                         info.y >= ty && info.y <= ty + th);
            } else if (trigger->getShape() == CollisionShape::Circle) {
                float dx = info.x - trigger->getX();
                float dy = info.y - trigger->getY();
                float distSq = dx * dx + dy * dy;
                float radiusSq = trigger->getRadius() * trigger->getRadius();

                inside = distSq <= radiusSq;
            }

            // Handle trigger state changes
            bool wasInside = std::find(info.currentTriggers.begin(),
                                      info.currentTriggers.end(),
                                      trigger->getID()) != info.currentTriggers.end();

            if (inside && !wasInside) {
                // Entity entered trigger
                info.currentTriggers.push_back(trigger->getID());

                if (trigger->getType() == TriggerType::Enter) {
                    EnterTrigger* enterTrigger = static_cast<EnterTrigger*>(trigger);
                    enterTrigger->onEntityEnter(entity);
                }
            } else if (!inside && wasInside) {
                // Entity exited trigger
                info.currentTriggers.erase(
                    std::remove(info.currentTriggers.begin(),
                               info.currentTriggers.end(),
                               trigger->getID()),
                    info.currentTriggers.end());

                if (trigger->getType() == TriggerType::Exit) {
                    ExitTrigger* exitTrigger = static_cast<ExitTrigger*>(trigger);
                    exitTrigger->onEntityExit(entity);
                }
            } else if (inside && wasInside) {
                // Entity staying in trigger
                if (trigger->getType() == TriggerType::Stay) {
                    if (trigger->canActivate()) {
                        trigger->activate();
                    }
                }
            }
        }
    }
}

void TriggerManager::handleEntityMovement(void* entity, float oldX, float oldY,
                                         float newX, float newY) {
    // Movement handling is done in checkTriggers
}

// ==================== InteractiveObject Implementation ====================

InteractiveObject::InteractiveObject(const std::string& name, float x, float y)
    : m_name(name)
    , m_x(x)
    , m_y(y)
    , m_interactionDistance(50.0f)
    , m_enabled(true)
    , m_interactable(true)
    , m_trigger(nullptr)
{
}

InteractiveObject::~InteractiveObject() {
}

void InteractiveObject::setPosition(float x, float y) {
    m_x = x;
    m_y = y;

    if (m_trigger) {
        m_trigger->setPosition(x, y);
    }
}

bool InteractiveObject::interact(void* player) {
    if (!m_enabled || !m_interactable) {
        return false;
    }

    if (m_onInteract) {
        m_onInteract(this, player);
        return true;
    }

    if (m_trigger) {
        m_trigger->activate();
        return true;
    }

    return false;
}

void InteractiveObject::update(float deltaTime) {
    // Update logic
}

void InteractiveObject::render() {
    // Render sprite
    // This would integrate with your rendering system
}

} // namespace World
