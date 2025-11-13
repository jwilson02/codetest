#include "StateMachine.h"

namespace AI {

// ============================================================================
// STATE MACHINE IMPLEMENTATION
// ============================================================================

StateMachine::StateMachine()
    : m_timeInCurrentState(0.0f)
    , m_enabled(true)
    , m_debugName("StateMachine")
{
}

void StateMachine::AddState(const std::string& name) {
    if (!HasState(name)) {
        m_states[name] = std::make_unique<State>(name);
    }
}

State* StateMachine::GetState(const std::string& name) {
    auto it = m_states.find(name);
    return (it != m_states.end()) ? it->second.get() : nullptr;
}

bool StateMachine::HasState(const std::string& name) const {
    return m_states.find(name) != m_states.end();
}

void StateMachine::SetStateOnEnter(const std::string& stateName, State::CallbackFunction callback) {
    State* state = GetState(stateName);
    if (state) {
        state->SetOnEnter(callback);
    }
}

void StateMachine::SetStateOnUpdate(const std::string& stateName, State::StateFunction callback) {
    State* state = GetState(stateName);
    if (state) {
        state->SetOnUpdate(callback);
    }
}

void StateMachine::SetStateOnExit(const std::string& stateName, State::CallbackFunction callback) {
    State* state = GetState(stateName);
    if (state) {
        state->SetOnExit(callback);
    }
}

void StateMachine::AddTransition(const std::string& fromState, const std::string& toState,
                                 Transition::ConditionFunction condition) {
    State* state = GetState(fromState);
    if (state) {
        state->AddTransition(toState, condition);
    }
}

void StateMachine::SetInitialState(const std::string& stateName) {
    m_initialStateName = stateName;
    if (m_currentStateName.empty()) {
        EnterState(stateName);
    }
}

void StateMachine::ChangeState(const std::string& newState) {
    if (newState == m_currentStateName) {
        return; // Already in this state
    }

    if (!HasState(newState)) {
        return; // State doesn't exist
    }

    ExitState();
    EnterState(newState);
}

void StateMachine::Update(float deltaTime) {
    if (!m_enabled || m_currentStateName.empty()) {
        return;
    }

    m_timeInCurrentState += deltaTime;

    // Update current state
    const State* currentState = GetCurrentState();
    if (currentState) {
        currentState->OnUpdate(*this, deltaTime);
    }

    // Check for transitions
    CheckTransitions();
}

const State* StateMachine::GetCurrentState() const {
    auto it = m_states.find(m_currentStateName);
    return (it != m_states.end()) ? it->second.get() : nullptr;
}

void StateMachine::Reset() {
    if (!m_currentStateName.empty()) {
        ExitState();
    }

    m_currentStateName.clear();
    m_previousStateName.clear();
    m_timeInCurrentState = 0.0f;

    if (!m_initialStateName.empty()) {
        EnterState(m_initialStateName);
    }
}

void StateMachine::CheckTransitions() {
    const State* currentState = GetCurrentState();
    if (!currentState) {
        return;
    }

    const auto& transitions = currentState->GetTransitions();
    for (const auto& transition : transitions) {
        if (transition.CheckCondition(*this)) {
            ChangeState(transition.GetTargetState());
            break; // Only take the first valid transition
        }
    }
}

void StateMachine::EnterState(const std::string& stateName) {
    m_previousStateName = m_currentStateName;
    m_currentStateName = stateName;
    m_timeInCurrentState = 0.0f;

    State* state = GetState(stateName);
    if (state) {
        state->OnEnter(*this);
    }
}

void StateMachine::ExitState() {
    State* state = GetState(m_currentStateName);
    if (state) {
        state->OnExit(*this);
    }
}

// ============================================================================
// STATE MACHINE BUILDER IMPLEMENTATION
// ============================================================================

StateMachineBuilder::StateMachineBuilder()
    : m_stateMachine(std::make_shared<StateMachine>())
{
}

StateMachineBuilder& StateMachineBuilder::AddState(const std::string& name) {
    m_stateMachine->AddState(name);
    m_currentState = name;
    return *this;
}

StateMachineBuilder& StateMachineBuilder::OnEnter(State::CallbackFunction callback) {
    if (!m_currentState.empty()) {
        m_stateMachine->SetStateOnEnter(m_currentState, callback);
    }
    return *this;
}

StateMachineBuilder& StateMachineBuilder::OnUpdate(State::StateFunction callback) {
    if (!m_currentState.empty()) {
        m_stateMachine->SetStateOnUpdate(m_currentState, callback);
    }
    return *this;
}

StateMachineBuilder& StateMachineBuilder::OnExit(State::CallbackFunction callback) {
    if (!m_currentState.empty()) {
        m_stateMachine->SetStateOnExit(m_currentState, callback);
    }
    return *this;
}

StateMachineBuilder& StateMachineBuilder::TransitionTo(const std::string& targetState,
                                                       Transition::ConditionFunction condition) {
    if (!m_currentState.empty()) {
        m_stateMachine->AddTransition(m_currentState, targetState, condition);
    }
    return *this;
}

StateMachineBuilder& StateMachineBuilder::SetInitialState(const std::string& stateName) {
    m_stateMachine->SetInitialState(stateName);
    return *this;
}

std::shared_ptr<StateMachine> StateMachineBuilder::Build() {
    return m_stateMachine;
}

// ============================================================================
// COMMON STATE MACHINE TEMPLATES
// ============================================================================

std::shared_ptr<StateMachine> CreatePatrolStateMachine() {
    StateMachineBuilder builder;

    builder.AddState("Patrol")
        .OnEnter([](StateMachine& sm) {
            sm.SetValue("patrolIndex", 0);
        })
        .OnUpdate([](StateMachine& sm, float dt) {
            // Patrol logic would go here
            // Move to next waypoint, etc.
        })
        .TransitionTo("Idle", [](StateMachine& sm) {
            return sm.GetValue<bool>("shouldIdle", false);
        });

    builder.AddState("Idle")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Idle logic
        })
        .TransitionTo("Patrol", [](StateMachine& sm) {
            return sm.GetTimeInCurrentState() > 3.0f; // Idle for 3 seconds
        });

    builder.SetInitialState("Patrol");

    return builder.Build();
}

std::shared_ptr<StateMachine> CreateCombatStateMachine() {
    StateMachineBuilder builder;

    builder.AddState("Idle")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Look for enemies in range
        })
        .TransitionTo("Alert", [](StateMachine& sm) {
            return sm.GetValue<bool>("enemyDetected", false);
        });

    builder.AddState("Alert")
        .OnEnter([](StateMachine& sm) {
            sm.SetValue("alertTime", 0.0f);
        })
        .OnUpdate([](StateMachine& sm, float dt) {
            float alertTime = sm.GetValue<float>("alertTime", 0.0f);
            sm.SetValue("alertTime", alertTime + dt);
        })
        .TransitionTo("Chase", [](StateMachine& sm) {
            return sm.GetValue<float>("alertTime", 0.0f) > 0.5f; // Alert for 0.5 seconds
        })
        .TransitionTo("Idle", [](StateMachine& sm) {
            return !sm.GetValue<bool>("enemyDetected", false);
        });

    builder.AddState("Chase")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Chase the enemy
        })
        .TransitionTo("Attack", [](StateMachine& sm) {
            return sm.GetValue<bool>("inAttackRange", false);
        })
        .TransitionTo("Idle", [](StateMachine& sm) {
            return !sm.GetValue<bool>("enemyDetected", false);
        });

    builder.AddState("Attack")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Attack the enemy
        })
        .TransitionTo("Chase", [](StateMachine& sm) {
            return !sm.GetValue<bool>("inAttackRange", false) &&
                   sm.GetValue<bool>("enemyDetected", false);
        })
        .TransitionTo("Retreat", [](StateMachine& sm) {
            float health = sm.GetValue<float>("healthPercent", 1.0f);
            return health < 0.3f; // Retreat if health below 30%
        })
        .TransitionTo("Idle", [](StateMachine& sm) {
            return !sm.GetValue<bool>("enemyDetected", false);
        });

    builder.AddState("Retreat")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Run away from enemy
        })
        .TransitionTo("Idle", [](StateMachine& sm) {
            return sm.GetValue<bool>("isSafe", false);
        })
        .TransitionTo("Chase", [](StateMachine& sm) {
            float health = sm.GetValue<float>("healthPercent", 1.0f);
            return health > 0.6f; // Return to fight if health recovered
        });

    builder.SetInitialState("Idle");

    return builder.Build();
}

std::shared_ptr<StateMachine> CreateNPCRoutineStateMachine() {
    StateMachineBuilder builder;

    builder.AddState("Sleep")
        .OnEnter([](StateMachine& sm) {
            // Go to bed
        })
        .OnUpdate([](StateMachine& sm, float dt) {
            // Sleep animation
        })
        .TransitionTo("Morning", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 6.0f && hour < 8.0f; // Wake at 6-8 AM
        });

    builder.AddState("Morning")
        .OnEnter([](StateMachine& sm) {
            // Get up, eat breakfast
        })
        .TransitionTo("Work", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 8.0f; // Start work at 8 AM
        });

    builder.AddState("Work")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Work activities
        })
        .TransitionTo("Lunch", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 12.0f && hour < 13.0f; // Lunch at noon
        })
        .TransitionTo("Evening", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 17.0f; // Finish work at 5 PM
        });

    builder.AddState("Lunch")
        .OnEnter([](StateMachine& sm) {
            // Go to eat
        })
        .TransitionTo("Work", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 13.0f; // Return to work at 1 PM
        });

    builder.AddState("Evening")
        .OnUpdate([](StateMachine& sm, float dt) {
            // Leisure activities
        })
        .TransitionTo("Sleep", [](StateMachine& sm) {
            float hour = sm.GetValue<float>("timeOfDay", 0.0f);
            return hour >= 22.0f || hour < 6.0f; // Sleep at 10 PM
        });

    builder.SetInitialState("Sleep");

    return builder.Build();
}

} // namespace AI
