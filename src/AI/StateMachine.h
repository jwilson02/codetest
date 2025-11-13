#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

namespace AI {

// Forward declarations
class StateMachine;
class State;

/**
 * Transition - Defines a transition from one state to another
 */
class Transition {
public:
    using ConditionFunction = std::function<bool(StateMachine&)>;

    Transition(const std::string& targetState, ConditionFunction condition)
        : m_targetState(targetState)
        , m_condition(condition)
    {}

    const std::string& GetTargetState() const { return m_targetState; }

    bool CheckCondition(StateMachine& sm) const {
        return m_condition ? m_condition(sm) : false;
    }

private:
    std::string m_targetState;
    ConditionFunction m_condition;
};

/**
 * State - Represents a single state in the state machine
 */
class State {
public:
    using StateFunction = std::function<void(StateMachine&, float)>;
    using CallbackFunction = std::function<void(StateMachine&)>;

    State(const std::string& name)
        : m_name(name)
        , m_onEnter(nullptr)
        , m_onUpdate(nullptr)
        , m_onExit(nullptr)
    {}

    // Set callbacks
    void SetOnEnter(CallbackFunction callback) { m_onEnter = callback; }
    void SetOnUpdate(StateFunction callback) { m_onUpdate = callback; }
    void SetOnExit(CallbackFunction callback) { m_onExit = callback; }

    // Execute callbacks
    void OnEnter(StateMachine& sm) const {
        if (m_onEnter) m_onEnter(sm);
    }

    void OnUpdate(StateMachine& sm, float deltaTime) const {
        if (m_onUpdate) m_onUpdate(sm, deltaTime);
    }

    void OnExit(StateMachine& sm) const {
        if (m_onExit) m_onExit(sm);
    }

    // Add transition
    void AddTransition(const std::string& targetState, Transition::ConditionFunction condition) {
        m_transitions.emplace_back(targetState, condition);
    }

    // Get transitions
    const std::vector<Transition>& GetTransitions() const { return m_transitions; }

    // Get name
    const std::string& GetName() const { return m_name; }

private:
    std::string m_name;
    CallbackFunction m_onEnter;
    StateFunction m_onUpdate;
    CallbackFunction m_onExit;
    std::vector<Transition> m_transitions;
};

/**
 * StateMachine - Finite state machine for AI behaviors
 */
class StateMachine {
public:
    StateMachine();
    ~StateMachine() = default;

    // State management
    void AddState(const std::string& name);
    State* GetState(const std::string& name);
    bool HasState(const std::string& name) const;

    // Set callbacks for a state
    void SetStateOnEnter(const std::string& stateName, State::CallbackFunction callback);
    void SetStateOnUpdate(const std::string& stateName, State::StateFunction callback);
    void SetStateOnExit(const std::string& stateName, State::CallbackFunction callback);

    // Add transition between states
    void AddTransition(const std::string& fromState, const std::string& toState,
                      Transition::ConditionFunction condition);

    // Initial state
    void SetInitialState(const std::string& stateName);

    // Force a state change
    void ChangeState(const std::string& newState);

    // Update the state machine
    void Update(float deltaTime);

    // Get current state
    const std::string& GetCurrentStateName() const { return m_currentStateName; }
    const State* GetCurrentState() const;

    // Reset to initial state
    void Reset();

    // Enable/disable
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    // Blackboard - shared data storage between states
    template<typename T>
    void SetValue(const std::string& key, const T& value) {
        m_blackboard[key] = std::make_shared<T>(value);
    }

    template<typename T>
    T GetValue(const std::string& key, const T& defaultValue = T()) const {
        auto it = m_blackboard.find(key);
        if (it != m_blackboard.end()) {
            auto ptr = std::static_pointer_cast<T>(it->second);
            return ptr ? *ptr : defaultValue;
        }
        return defaultValue;
    }

    bool HasValue(const std::string& key) const {
        return m_blackboard.find(key) != m_blackboard.end();
    }

    void ClearValue(const std::string& key) {
        m_blackboard.erase(key);
    }

    void ClearAllValues() {
        m_blackboard.clear();
    }

    // Time tracking
    float GetTimeInCurrentState() const { return m_timeInCurrentState; }

    // History tracking
    const std::string& GetPreviousStateName() const { return m_previousStateName; }

    // Debug
    void SetDebugName(const std::string& name) { m_debugName = name; }
    const std::string& GetDebugName() const { return m_debugName; }

private:
    std::unordered_map<std::string, std::unique_ptr<State>> m_states;
    std::string m_currentStateName;
    std::string m_previousStateName;
    std::string m_initialStateName;
    float m_timeInCurrentState;
    bool m_enabled;
    std::string m_debugName;

    // Blackboard for shared data
    std::unordered_map<std::string, std::shared_ptr<void>> m_blackboard;

    // Helper methods
    void CheckTransitions();
    void EnterState(const std::string& stateName);
    void ExitState();
};

/**
 * StateMachineBuilder - Fluent API for building state machines
 */
class StateMachineBuilder {
public:
    StateMachineBuilder();

    // Add a state
    StateMachineBuilder& AddState(const std::string& name);

    // Set callbacks for the last added state
    StateMachineBuilder& OnEnter(State::CallbackFunction callback);
    StateMachineBuilder& OnUpdate(State::StateFunction callback);
    StateMachineBuilder& OnExit(State::CallbackFunction callback);

    // Add transition from current state to target state
    StateMachineBuilder& TransitionTo(const std::string& targetState,
                                     Transition::ConditionFunction condition);

    // Set initial state
    StateMachineBuilder& SetInitialState(const std::string& stateName);

    // Build the state machine
    std::shared_ptr<StateMachine> Build();

private:
    std::shared_ptr<StateMachine> m_stateMachine;
    std::string m_currentState;
};

// ============================================================================
// COMMON STATE MACHINE TEMPLATES
// ============================================================================

/**
 * Creates a basic patrol state machine
 */
std::shared_ptr<StateMachine> CreatePatrolStateMachine();

/**
 * Creates a basic combat state machine (Idle -> Alert -> Chase -> Attack -> Retreat)
 */
std::shared_ptr<StateMachine> CreateCombatStateMachine();

/**
 * Creates an NPC daily routine state machine
 */
std::shared_ptr<StateMachine> CreateNPCRoutineStateMachine();

} // namespace AI
