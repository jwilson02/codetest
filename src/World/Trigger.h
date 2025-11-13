#pragma once

#include "Collision.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace World {

// Forward declarations
class TileMap;

// Trigger types
enum class TriggerType {
    Enter,          // Triggered when entering the zone
    Exit,           // Triggered when exiting the zone
    Stay,           // Triggered while inside the zone
    Interact,       // Triggered by player interaction
    Proximity,      // Triggered when within range
    Collision,      // Triggered by collision
    Timed,          // Triggered after a delay
    Script,         // Custom script trigger
    Quest,          // Quest-related trigger
    Cutscene        // Starts a cutscene
};

// Trigger activation conditions
struct TriggerCondition {
    enum Type {
        None,
        HasItem,
        QuestComplete,
        QuestActive,
        LevelMin,
        LevelMax,
        TimeOfDay,
        WeatherType,
        PlayerClass,
        Custom
    };

    Type type;
    std::string parameter;
    std::string value;
    bool inverted; // NOT condition

    TriggerCondition() : type(None), inverted(false) {}
};

// Trigger actions
struct TriggerAction {
    enum Type {
        LoadMap,
        Teleport,
        SpawnEnemy,
        SpawnItem,
        PlaySound,
        PlayMusic,
        ShowMessage,
        StartQuest,
        CompleteQuest,
        OpenDoor,
        ActivateObject,
        ChangeWeather,
        ChangeTime,
        GiveItem,
        RemoveItem,
        Script,
        Cutscene,
        SaveGame,
        Checkpoint
    };

    Type type;
    std::string parameter;
    std::string target;
    float value;
    std::string script;

    TriggerAction() : type(LoadMap), value(0.0f) {}
};

// Base trigger zone
class Trigger {
public:
    Trigger(int id, const std::string& name, TriggerType type);
    virtual ~Trigger();

    // Basic properties
    int getID() const { return m_id; }
    const std::string& getName() const { return m_name; }
    TriggerType getType() const { return m_type; }

    // Position and shape
    void setPosition(float x, float y) { m_x = x; m_y = y; updateCollisionObject(); }
    float getX() const { return m_x; }
    float getY() const { return m_y; }

    void setSize(float width, float height) { m_width = width; m_height = height; updateCollisionObject(); }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

    void setRadius(float radius) { m_radius = radius; updateCollisionObject(); }
    float getRadius() const { return m_radius; }

    void setShape(CollisionShape shape) { m_shape = shape; updateCollisionObject(); }
    CollisionShape getShape() const { return m_shape; }

    // State
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }

    void setOneShot(bool oneShot) { m_oneShot = oneShot; }
    bool isOneShot() const { return m_oneShot; }

    void setTriggered(bool triggered) { m_triggered = triggered; }
    bool isTriggered() const { return m_triggered; }

    // Cooldown
    void setCooldown(float cooldown) { m_cooldown = cooldown; }
    float getCooldown() const { return m_cooldown; }
    float getRemainingCooldown() const { return m_remainingCooldown; }

    // Conditions
    void addCondition(const TriggerCondition& condition) { m_conditions.push_back(condition); }
    void clearConditions() { m_conditions.clear(); }
    const std::vector<TriggerCondition>& getConditions() const { return m_conditions; }
    bool checkConditions() const;

    // Actions
    void addAction(const TriggerAction& action) { m_actions.push_back(action); }
    void clearActions() { m_actions.clear(); }
    const std::vector<TriggerAction>& getActions() const { return m_actions; }

    // Activation
    virtual bool canActivate() const;
    virtual void activate();
    virtual void deactivate();
    virtual void reset();

    // Update
    virtual void update(float deltaTime);

    // Collision handling
    void setCollisionObject(CollisionObject* obj) { m_collisionObject = obj; }
    CollisionObject* getCollisionObject() const { return m_collisionObject; }

    // Callbacks
    using TriggerCallback = std::function<void(Trigger*)>;
    void setOnEnter(TriggerCallback callback) { m_onEnter = callback; }
    void setOnExit(TriggerCallback callback) { m_onExit = callback; }
    void setOnStay(TriggerCallback callback) { m_onStay = callback; }
    void setOnActivate(TriggerCallback callback) { m_onActivate = callback; }

    // User data
    void setUserData(void* data) { m_userData = data; }
    void* getUserData() const { return m_userData; }

protected:
    int m_id;
    std::string m_name;
    TriggerType m_type;

    float m_x, m_y;
    float m_width, m_height;
    float m_radius;
    CollisionShape m_shape;

    bool m_enabled;
    bool m_active;
    bool m_oneShot;
    bool m_triggered;

    float m_cooldown;
    float m_remainingCooldown;

    std::vector<TriggerCondition> m_conditions;
    std::vector<TriggerAction> m_actions;

    CollisionObject* m_collisionObject;

    TriggerCallback m_onEnter;
    TriggerCallback m_onExit;
    TriggerCallback m_onStay;
    TriggerCallback m_onActivate;

    void* m_userData;

    virtual void updateCollisionObject();
    void executeActions();
};

// Specific trigger types

class EnterTrigger : public Trigger {
public:
    EnterTrigger(int id, const std::string& name)
        : Trigger(id, name, TriggerType::Enter) {}

    void onEntityEnter(void* entity);
};

class ExitTrigger : public Trigger {
public:
    ExitTrigger(int id, const std::string& name)
        : Trigger(id, name, TriggerType::Exit) {}

    void onEntityExit(void* entity);
};

class ProximityTrigger : public Trigger {
public:
    ProximityTrigger(int id, const std::string& name)
        : Trigger(id, name, TriggerType::Proximity)
        , m_targetX(0), m_targetY(0), m_checkRadius(100.0f) {}

    void setTarget(float x, float y) { m_targetX = x; m_targetY = y; }
    void setCheckRadius(float radius) { m_checkRadius = radius; }

    void update(float deltaTime) override;

private:
    float m_targetX, m_targetY;
    float m_checkRadius;
};

class TimedTrigger : public Trigger {
public:
    TimedTrigger(int id, const std::string& name)
        : Trigger(id, name, TriggerType::Timed)
        , m_delay(0.0f), m_elapsed(0.0f) {}

    void setDelay(float delay) { m_delay = delay; }
    float getDelay() const { return m_delay; }

    void update(float deltaTime) override;
    void reset() override;

private:
    float m_delay;
    float m_elapsed;
};

class ScriptTrigger : public Trigger {
public:
    ScriptTrigger(int id, const std::string& name)
        : Trigger(id, name, TriggerType::Script) {}

    void setScript(const std::string& script) { m_script = script; }
    const std::string& getScript() const { return m_script; }

    void activate() override;

private:
    std::string m_script;
};

// Trigger manager
class TriggerManager {
public:
    TriggerManager();
    ~TriggerManager();

    // Initialization
    bool initialize(CollisionSystem* collisionSystem);
    void shutdown();

    // Trigger creation
    Trigger* createTrigger(TriggerType type, const std::string& name);
    EnterTrigger* createEnterTrigger(const std::string& name);
    ExitTrigger* createExitTrigger(const std::string& name);
    ProximityTrigger* createProximityTrigger(const std::string& name);
    TimedTrigger* createTimedTrigger(const std::string& name);
    ScriptTrigger* createScriptTrigger(const std::string& name);

    // Trigger management
    void destroyTrigger(int triggerID);
    void destroyTrigger(Trigger* trigger);
    Trigger* getTrigger(int triggerID);
    Trigger* getTrigger(const std::string& name);

    // Trigger queries
    std::vector<Trigger*> getTriggersInArea(float x, float y, float width, float height);
    std::vector<Trigger*> getTriggersInRadius(float x, float y, float radius);
    std::vector<Trigger*> getTriggersByType(TriggerType type);

    // Entity tracking
    void registerEntity(void* entity, float x, float y);
    void updateEntityPosition(void* entity, float x, float y);
    void unregisterEntity(void* entity);

    // Update
    void update(float deltaTime);

    // Map association
    void setCurrentMap(const std::string& mapName) { m_currentMapName = mapName; }
    const std::string& getCurrentMap() const { return m_currentMapName; }

    // Loading and saving
    bool loadTriggersFromMap(const std::string& mapFilename);
    bool saveTriggersToMap(const std::string& mapFilename) const;
    bool loadTriggerFromJSON(const std::string& json);

    // Debug
    void setDebugDraw(bool enable) { m_debugDraw = enable; }
    bool isDebugDrawEnabled() const { return m_debugDraw; }
    void renderDebug();

    // Statistics
    struct Stats {
        int totalTriggers;
        int activeTriggers;
        int triggeredCount;
    };
    Stats getStats() const;

private:
    CollisionSystem* m_collisionSystem;
    std::unordered_map<int, std::unique_ptr<Trigger>> m_triggers;
    std::unordered_map<std::string, int> m_triggerNameToID;
    int m_nextTriggerID;

    std::string m_currentMapName;

    struct EntityInfo {
        float x, y;
        std::vector<int> currentTriggers; // Triggers entity is currently in
    };
    std::unordered_map<void*, EntityInfo> m_entities;

    bool m_debugDraw;

    void checkTriggers();
    void handleEntityMovement(void* entity, float oldX, float oldY, float newX, float newY);
};

// Interactive object (combines trigger with visual representation)
class InteractiveObject {
public:
    InteractiveObject(const std::string& name, float x, float y);
    ~InteractiveObject();

    // Basic properties
    const std::string& getName() const { return m_name; }
    void setPosition(float x, float y);
    float getX() const { return m_x; }
    float getY() const { return m_y; }

    // Visual
    void setSprite(const std::string& spriteName) { m_spriteName = spriteName; }
    const std::string& getSprite() const { return m_spriteName; }

    // Interaction
    void setInteractionPrompt(const std::string& prompt) { m_interactionPrompt = prompt; }
    const std::string& getInteractionPrompt() const { return m_interactionPrompt; }
    void setInteractionDistance(float distance) { m_interactionDistance = distance; }
    float getInteractionDistance() const { return m_interactionDistance; }

    // State
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void setInteractable(bool interactable) { m_interactable = interactable; }
    bool isInteractable() const { return m_interactable; }

    // Trigger
    void setTrigger(Trigger* trigger) { m_trigger = trigger; }
    Trigger* getTrigger() const { return m_trigger; }

    // Interaction callback
    using InteractionCallback = std::function<void(InteractiveObject*, void* player)>;
    void setOnInteract(InteractionCallback callback) { m_onInteract = callback; }

    bool interact(void* player);

    // Update and render
    void update(float deltaTime);
    void render();

private:
    std::string m_name;
    float m_x, m_y;
    std::string m_spriteName;
    std::string m_interactionPrompt;
    float m_interactionDistance;
    bool m_enabled;
    bool m_interactable;

    Trigger* m_trigger;
    InteractionCallback m_onInteract;
};

} // namespace World
