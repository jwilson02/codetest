#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace AI {

// Node execution status
enum class NodeStatus {
    SUCCESS,
    FAILURE,
    RUNNING
};

// Forward declaration
class BehaviorTreeContext;

/**
 * Base class for all behavior tree nodes
 */
class BTNode {
public:
    virtual ~BTNode() = default;

    // Execute the node and return its status
    virtual NodeStatus Execute(BehaviorTreeContext& context) = 0;

    // Called when the node is first entered
    virtual void OnEnter(BehaviorTreeContext& context) {}

    // Called when the node is exited
    virtual void OnExit(BehaviorTreeContext& context) {}

    // Reset the node state
    virtual void Reset() {}

    // Set node name for debugging
    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

protected:
    std::string m_name;
};

/**
 * Context passed to behavior tree nodes containing shared state
 */
class BehaviorTreeContext {
public:
    BehaviorTreeContext(int entityId, int aiId)
        : m_entityId(entityId), m_aiId(aiId) {}

    // Entity identifiers
    int GetEntityId() const { return m_entityId; }
    int GetAIId() const { return m_aiId; }

    // Blackboard - shared data storage
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

    void ClearAll() {
        m_blackboard.clear();
    }

private:
    int m_entityId;
    int m_aiId;
    std::unordered_map<std::string, std::shared_ptr<void>> m_blackboard;
};

// ============================================================================
// COMPOSITE NODES
// ============================================================================

/**
 * Sequence Node - Executes children in order until one fails
 * Returns SUCCESS if all children succeed
 * Returns FAILURE if any child fails
 */
class BTSequence : public BTNode {
public:
    void AddChild(std::shared_ptr<BTNode> child) {
        m_children.push_back(child);
    }

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override;

private:
    std::vector<std::shared_ptr<BTNode>> m_children;
    size_t m_currentChild = 0;
};

/**
 * Selector Node - Executes children in order until one succeeds
 * Returns SUCCESS if any child succeeds
 * Returns FAILURE if all children fail
 */
class BTSelector : public BTNode {
public:
    void AddChild(std::shared_ptr<BTNode> child) {
        m_children.push_back(child);
    }

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override;

private:
    std::vector<std::shared_ptr<BTNode>> m_children;
    size_t m_currentChild = 0;
};

/**
 * Parallel Node - Executes all children simultaneously
 * Returns SUCCESS if required number of children succeed
 * Returns FAILURE if too many children fail
 */
class BTParallel : public BTNode {
public:
    BTParallel(int requiredSuccesses = -1) // -1 means all must succeed
        : m_requiredSuccesses(requiredSuccesses) {}

    void AddChild(std::shared_ptr<BTNode> child) {
        m_children.push_back(child);
    }

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override;

private:
    std::vector<std::shared_ptr<BTNode>> m_children;
    int m_requiredSuccesses;
};

// ============================================================================
// DECORATOR NODES
// ============================================================================

/**
 * Inverter - Inverts the result of its child
 */
class BTInverter : public BTNode {
public:
    BTInverter(std::shared_ptr<BTNode> child) : m_child(child) {}

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override { if (m_child) m_child->Reset(); }

private:
    std::shared_ptr<BTNode> m_child;
};

/**
 * Repeater - Repeats its child N times or until it fails
 */
class BTRepeater : public BTNode {
public:
    BTRepeater(std::shared_ptr<BTNode> child, int repeatCount = -1) // -1 means infinite
        : m_child(child), m_repeatCount(repeatCount), m_currentCount(0) {}

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override;

private:
    std::shared_ptr<BTNode> m_child;
    int m_repeatCount;
    int m_currentCount;
};

/**
 * UntilSuccess - Repeats its child until it succeeds
 */
class BTUntilSuccess : public BTNode {
public:
    BTUntilSuccess(std::shared_ptr<BTNode> child) : m_child(child) {}

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override { if (m_child) m_child->Reset(); }

private:
    std::shared_ptr<BTNode> m_child;
};

/**
 * Cooldown - Only executes child if cooldown has elapsed
 */
class BTCooldown : public BTNode {
public:
    BTCooldown(std::shared_ptr<BTNode> child, float cooldownTime)
        : m_child(child), m_cooldownTime(cooldownTime), m_lastExecuteTime(0.0f) {}

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void Reset() override;

private:
    std::shared_ptr<BTNode> m_child;
    float m_cooldownTime;
    float m_lastExecuteTime;
};

// ============================================================================
// LEAF NODES (ACTION & CONDITION)
// ============================================================================

/**
 * Action Node - Executes a custom action function
 */
class BTAction : public BTNode {
public:
    using ActionFunction = std::function<NodeStatus(BehaviorTreeContext&)>;

    BTAction(ActionFunction action) : m_action(action) {}

    NodeStatus Execute(BehaviorTreeContext& context) override {
        return m_action ? m_action(context) : NodeStatus::FAILURE;
    }

private:
    ActionFunction m_action;
};

/**
 * Condition Node - Tests a condition and returns SUCCESS or FAILURE
 */
class BTCondition : public BTNode {
public:
    using ConditionFunction = std::function<bool(BehaviorTreeContext&)>;

    BTCondition(ConditionFunction condition) : m_condition(condition) {}

    NodeStatus Execute(BehaviorTreeContext& context) override {
        return (m_condition && m_condition(context)) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

private:
    ConditionFunction m_condition;
};

/**
 * Wait Node - Waits for a specified duration
 */
class BTWait : public BTNode {
public:
    BTWait(float duration) : m_duration(duration), m_elapsed(0.0f) {}

    NodeStatus Execute(BehaviorTreeContext& context) override;
    void OnEnter(BehaviorTreeContext& context) override { m_elapsed = 0.0f; }
    void Reset() override { m_elapsed = 0.0f; }

private:
    float m_duration;
    float m_elapsed;
};

// ============================================================================
// BEHAVIOR TREE
// ============================================================================

/**
 * BehaviorTree - Main behavior tree class
 */
class BehaviorTree {
public:
    BehaviorTree(int entityId, int aiId);
    ~BehaviorTree() = default;

    // Set the root node of the tree
    void SetRoot(std::shared_ptr<BTNode> root) { m_root = root; }
    std::shared_ptr<BTNode> GetRoot() { return m_root; }

    // Execute the behavior tree
    NodeStatus Execute();

    // Reset the entire tree
    void Reset();

    // Get context
    BehaviorTreeContext& GetContext() { return m_context; }
    const BehaviorTreeContext& GetContext() const { return m_context; }

    // Enable/disable the tree
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    // Debug
    void SetDebugName(const std::string& name) { m_debugName = name; }
    const std::string& GetDebugName() const { return m_debugName; }

private:
    std::shared_ptr<BTNode> m_root;
    BehaviorTreeContext m_context;
    bool m_enabled;
    std::string m_debugName;
};

// ============================================================================
// BUILDER - Fluent API for constructing behavior trees
// ============================================================================

class BehaviorTreeBuilder {
public:
    BehaviorTreeBuilder(int entityId, int aiId);

    // Create nodes
    BehaviorTreeBuilder& Sequence(const std::string& name = "");
    BehaviorTreeBuilder& Selector(const std::string& name = "");
    BehaviorTreeBuilder& Parallel(int requiredSuccesses = -1, const std::string& name = "");
    BehaviorTreeBuilder& Inverter(const std::string& name = "");
    BehaviorTreeBuilder& Repeater(int count = -1, const std::string& name = "");
    BehaviorTreeBuilder& UntilSuccess(const std::string& name = "");
    BehaviorTreeBuilder& Cooldown(float cooldownTime, const std::string& name = "");

    BehaviorTreeBuilder& Action(BTAction::ActionFunction action, const std::string& name = "");
    BehaviorTreeBuilder& Condition(BTCondition::ConditionFunction condition, const std::string& name = "");
    BehaviorTreeBuilder& Wait(float duration, const std::string& name = "");

    // Tree structure control
    BehaviorTreeBuilder& End(); // End current composite/decorator

    // Build the final tree
    std::shared_ptr<BehaviorTree> Build();

private:
    struct BuilderNode {
        std::shared_ptr<BTNode> node;
        bool canHaveChildren;
        bool needsChild;
        int maxChildren;
        int currentChildren;
    };

    std::vector<BuilderNode> m_nodeStack;
    int m_entityId;
    int m_aiId;

    void PushNode(std::shared_ptr<BTNode> node, bool canHaveChildren, bool needsChild, int maxChildren = -1);
    void AttachNode(std::shared_ptr<BTNode> node);
};

} // namespace AI
