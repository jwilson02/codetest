#include "DialogueTree.h"
#include <algorithm>
#include <queue>
#include <set>
#include <sstream>
#include <fstream>

namespace Story {

// DialogueTree::Implementation
struct DialogueTree::Implementation {
    std::string rootId;
    std::unordered_map<std::string, TreeNode> nodes;
    std::unordered_map<std::string, BranchPoint> branchPoints;
    std::string currentBranch;
};

DialogueTree::DialogueTree() : impl(std::make_unique<Implementation>()) {}

DialogueTree::~DialogueTree() = default;

void DialogueTree::setRootNode(const std::string& nodeId) {
    impl->rootId = nodeId;
    if (impl->nodes.find(nodeId) != impl->nodes.end()) {
        impl->nodes[nodeId].depth = 0;
    }
}

void DialogueTree::addNode(const TreeNode& node) {
    impl->nodes[node.id] = node;
}

void DialogueTree::addBranch(const std::string& parentId, const std::string& childId) {
    auto parentIt = impl->nodes.find(parentId);
    auto childIt = impl->nodes.find(childId);

    if (parentIt != impl->nodes.end() && childIt != impl->nodes.end()) {
        // Add child to parent
        auto& childIds = parentIt->second.childIds;
        if (std::find(childIds.begin(), childIds.end(), childId) == childIds.end()) {
            childIds.push_back(childId);
        }

        // Set parent of child
        childIt->second.parentId = parentId;

        // Update depth
        childIt->second.depth = parentIt->second.depth + 1;

        // Update branching factor
        parentIt->second.branchingFactor = static_cast<int>(childIds.size());
    }
}

void DialogueTree::removeBranch(const std::string& parentId, const std::string& childId) {
    auto parentIt = impl->nodes.find(parentId);
    if (parentIt != impl->nodes.end()) {
        auto& childIds = parentIt->second.childIds;
        childIds.erase(std::remove(childIds.begin(), childIds.end(), childId), childIds.end());
        parentIt->second.branchingFactor = static_cast<int>(childIds.size());
    }

    auto childIt = impl->nodes.find(childId);
    if (childIt != impl->nodes.end()) {
        childIt->second.parentId.clear();
    }
}

void DialogueTree::addBranchPoint(const BranchPoint& branchPoint) {
    impl->branchPoints[branchPoint.id] = branchPoint;
}

const BranchPoint* DialogueTree::getBranchPoint(const std::string& id) const {
    auto it = impl->branchPoints.find(id);
    return it != impl->branchPoints.end() ? &it->second : nullptr;
}

std::vector<BranchPoint> DialogueTree::getAllBranchPoints() const {
    std::vector<BranchPoint> result;
    for (const auto& [id, bp] : impl->branchPoints) {
        result.push_back(bp);
    }
    return result;
}

const TreeNode* DialogueTree::getNode(const std::string& nodeId) const {
    auto it = impl->nodes.find(nodeId);
    return it != impl->nodes.end() ? &it->second : nullptr;
}

const TreeNode* DialogueTree::getRootNode() const {
    return getNode(impl->rootId);
}

std::vector<const TreeNode*> DialogueTree::getChildren(const std::string& nodeId) const {
    std::vector<const TreeNode*> children;
    const TreeNode* node = getNode(nodeId);
    if (!node) return children;

    for (const auto& childId : node->childIds) {
        const TreeNode* child = getNode(childId);
        if (child) children.push_back(child);
    }

    return children;
}

const TreeNode* DialogueTree::getParent(const std::string& nodeId) const {
    const TreeNode* node = getNode(nodeId);
    if (!node || node->parentId.empty()) return nullptr;
    return getNode(node->parentId);
}

std::vector<const TreeNode*> DialogueTree::getPath(const std::string& fromId, const std::string& toId) const {
    std::vector<const TreeNode*> path;
    std::unordered_map<std::string, std::string> parent;
    std::queue<std::string> queue;
    std::set<std::string> visited;

    queue.push(fromId);
    visited.insert(fromId);

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();

        if (current == toId) {
            // Reconstruct path
            std::vector<std::string> pathIds;
            std::string id = toId;
            while (id != fromId) {
                pathIds.push_back(id);
                id = parent[id];
            }
            pathIds.push_back(fromId);
            std::reverse(pathIds.begin(), pathIds.end());

            for (const auto& id : pathIds) {
                const TreeNode* node = getNode(id);
                if (node) path.push_back(node);
            }
            break;
        }

        const TreeNode* node = getNode(current);
        if (node) {
            for (const auto& childId : node->childIds) {
                if (visited.find(childId) == visited.end()) {
                    visited.insert(childId);
                    parent[childId] = current;
                    queue.push(childId);
                }
            }
        }
    }

    return path;
}

void DialogueTree::markVisited(const std::string& nodeId) {
    auto it = impl->nodes.find(nodeId);
    if (it != impl->nodes.end()) {
        it->second.visited = true;
        it->second.visitCount++;
    }
}

bool DialogueTree::isVisited(const std::string& nodeId) const {
    auto it = impl->nodes.find(nodeId);
    return it != impl->nodes.end() && it->second.visited;
}

void DialogueTree::resetVisited() {
    for (auto& [id, node] : impl->nodes) {
        node.visited = false;
    }
}

int DialogueTree::getTreeDepth() const {
    int maxDepth = 0;
    for (const auto& [id, node] : impl->nodes) {
        maxDepth = std::max(maxDepth, node.depth);
    }
    return maxDepth;
}

int DialogueTree::getBranchingFactor(const std::string& nodeId) const {
    const TreeNode* node = getNode(nodeId);
    return node ? node->branchingFactor : 0;
}

std::vector<std::string> DialogueTree::getLeafNodes() const {
    std::vector<std::string> leaves;
    for (const auto& [id, node] : impl->nodes) {
        if (node.childIds.empty()) {
            leaves.push_back(id);
        }
    }
    return leaves;
}

std::vector<std::string> DialogueTree::getCriticalPath() const {
    // Returns the main story path (nodes without branches or first branch)
    std::vector<std::string> path;
    if (impl->rootId.empty()) return path;

    std::string current = impl->rootId;
    while (!current.empty()) {
        path.push_back(current);
        const TreeNode* node = getNode(current);
        if (!node || node->childIds.empty()) break;

        // Follow first child (critical path)
        current = node->childIds[0];
    }

    return path;
}

void DialogueTree::addConsequence(const std::string& nodeId, const std::string& tag) {
    auto it = impl->nodes.find(nodeId);
    if (it != impl->nodes.end()) {
        it->second.consequenceTags.push_back(tag);
    }
}

std::vector<std::string> DialogueTree::getConsequences(const std::string& nodeId) const {
    const TreeNode* node = getNode(nodeId);
    return node ? node->consequenceTags : std::vector<std::string>();
}

std::vector<std::string> DialogueTree::getAllConsequences() const {
    std::vector<std::string> all;
    for (const auto& [id, node] : impl->nodes) {
        all.insert(all.end(), node.consequenceTags.begin(), node.consequenceTags.end());
    }
    return all;
}

void DialogueTree::setCurrentBranch(const std::string& branchId) {
    impl->currentBranch = branchId;
}

std::string DialogueTree::getCurrentBranch() const {
    return impl->currentBranch;
}

std::vector<std::string> DialogueTree::getPossibleBranches(const std::string& nodeId) const {
    std::vector<std::string> branches;
    for (const auto& [id, bp] : impl->branchPoints) {
        if (bp.nodeId == nodeId) {
            branches = bp.branchIds;
            break;
        }
    }
    return branches;
}

bool DialogueTree::loadFromJson(const std::string& jsonData) {
    // Would parse JSON
    return true;
}

std::string DialogueTree::toJson() const {
    // Would generate JSON
    return "{}";
}

bool DialogueTree::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    file << toJson();
    return true;
}

bool DialogueTree::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadFromJson(buffer.str());
}

std::string DialogueTree::generateGraphviz() const {
    std::stringstream ss;
    ss << "digraph DialogueTree {\n";
    ss << "  rankdir=TB;\n";

    for (const auto& [id, node] : impl->nodes) {
        ss << "  \"" << id << "\" [label=\"" << node.description << "\"];\n";
        for (const auto& childId : node.childIds) {
            ss << "  \"" << id << "\" -> \"" << childId << "\";\n";
        }
    }

    ss << "}\n";
    return ss.str();
}

void DialogueTree::exportStatistics(const std::string& filePath) const {
    // Would export detailed statistics
}

bool DialogueTree::validate(std::vector<std::string>& errors) const {
    errors.clear();

    // Check for root
    if (impl->rootId.empty()) {
        errors.push_back("No root node set");
    }

    // Check for orphaned nodes
    auto orphaned = findOrphanedNodes();
    for (const auto& id : orphaned) {
        errors.push_back("Orphaned node: " + id);
    }

    // Check for circular references
    if (hasCircularReferences()) {
        errors.push_back("Circular references detected");
    }

    return errors.empty();
}

bool DialogueTree::hasCircularReferences() const {
    std::unordered_map<std::string, bool> visited;
    std::unordered_map<std::string, bool> recStack;

    for (const auto& [id, node] : impl->nodes) {
        visited[id] = false;
        recStack[id] = false;
    }

    // DFS to detect cycles
    for (const auto& [id, node] : impl->nodes) {
        if (!visited[id]) {
            std::unordered_map<std::string, bool> currentRecStack = recStack;
            if (hasPath(id, id, visited)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> DialogueTree::findOrphanedNodes() const {
    std::vector<std::string> orphaned;
    std::set<std::string> reachable;

    if (!impl->rootId.empty()) {
        std::queue<std::string> queue;
        queue.push(impl->rootId);
        reachable.insert(impl->rootId);

        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();

            const TreeNode* node = getNode(current);
            if (node) {
                for (const auto& childId : node->childIds) {
                    if (reachable.find(childId) == reachable.end()) {
                        reachable.insert(childId);
                        queue.push(childId);
                    }
                }
            }
        }
    }

    for (const auto& [id, node] : impl->nodes) {
        if (reachable.find(id) == reachable.end() && id != impl->rootId) {
            orphaned.push_back(id);
        }
    }

    return orphaned;
}

void DialogueTree::calculateDepth(const std::string& nodeId, int depth) {
    auto it = impl->nodes.find(nodeId);
    if (it != impl->nodes.end()) {
        it->second.depth = depth;
        for (const auto& childId : it->second.childIds) {
            calculateDepth(childId, depth + 1);
        }
    }
}

bool DialogueTree::hasPath(const std::string& fromId, const std::string& toId,
                          std::unordered_map<std::string, bool>& visited) const {
    if (fromId == toId && visited[fromId]) return true;

    visited[fromId] = true;

    const TreeNode* node = getNode(fromId);
    if (node) {
        for (const auto& childId : node->childIds) {
            if (hasPath(childId, toId, visited)) {
                return true;
            }
        }
    }

    return false;
}

// TreeBuilder implementation
TreeBuilder::TreeBuilder() : tree_(std::make_unique<DialogueTree>()) {}

TreeBuilder& TreeBuilder::startTree(const std::string& rootId) {
    TreeNode root;
    root.id = rootId;
    tree_->addNode(root);
    tree_->setRootNode(rootId);
    currentId_ = rootId;
    return *this;
}

TreeBuilder& TreeBuilder::addLinearSequence(const std::vector<std::string>& nodeIds) {
    for (size_t i = 0; i < nodeIds.size(); ++i) {
        TreeNode node;
        node.id = nodeIds[i];
        tree_->addNode(node);

        if (i > 0) {
            tree_->addBranch(nodeIds[i - 1], nodeIds[i]);
        }
    }

    if (!nodeIds.empty()) {
        currentId_ = nodeIds.back();
    }

    return *this;
}

TreeBuilder& TreeBuilder::addBranch(const std::string& fromId,
                                   const std::vector<std::string>& branchIds) {
    for (const auto& branchId : branchIds) {
        TreeNode node;
        node.id = branchId;
        tree_->addNode(node);
        tree_->addBranch(fromId, branchId);
    }

    currentId_ = fromId;
    return *this;
}

TreeBuilder& TreeBuilder::mergeBranches(const std::vector<std::string>& branchIds,
                                       const std::string& mergeId) {
    TreeNode mergeNode;
    mergeNode.id = mergeId;
    tree_->addNode(mergeNode);

    for (const auto& branchId : branchIds) {
        tree_->addBranch(branchId, mergeId);
    }

    currentId_ = mergeId;
    return *this;
}

TreeBuilder& TreeBuilder::markCriticalChoice(const std::string& nodeId,
                                            BranchPoint::ImpactLevel impact) {
    BranchPoint bp;
    bp.id = "branch_" + nodeId;
    bp.nodeId = nodeId;
    bp.impact = impact;
    tree_->addBranchPoint(bp);

    return *this;
}

std::unique_ptr<DialogueTree> TreeBuilder::build() {
    return std::move(tree_);
}

// NarrativeFlowAnalyzer implementation
NarrativeFlowAnalyzer::FlowStatistics NarrativeFlowAnalyzer::analyze(const DialogueTree& tree) {
    FlowStatistics stats;
    stats.totalNodes = 0;
    stats.totalBranchPoints = static_cast<int>(tree.getAllBranchPoints().size());
    stats.maxDepth = tree.getTreeDepth();
    stats.avgBranchingFactor = 0.0f;
    stats.criticalChoices = 0;
    stats.possibleEndings = static_cast<int>(tree.getLeafNodes().size());

    // Count nodes and calculate average branching factor
    int totalBranching = 0;
    int nodesWithChildren = 0;

    // Would iterate through all nodes
    // Simplified for this implementation

    if (nodesWithChildren > 0) {
        stats.avgBranchingFactor = static_cast<float>(totalBranching) / nodesWithChildren;
    }

    return stats;
}

std::vector<std::string> NarrativeFlowAnalyzer::findOptimalPath(const DialogueTree& tree,
                                                                const std::string& goalNodeId) {
    // Would implement A* or similar pathfinding
    return tree.getCriticalPath();
}

float NarrativeFlowAnalyzer::calculateReplayValue(const DialogueTree& tree) {
    auto stats = analyze(tree);

    // Higher branching factor and more branch points = higher replay value
    float replayValue = 0.0f;
    replayValue += stats.avgBranchingFactor * 10.0f;
    replayValue += stats.totalBranchPoints * 5.0f;
    replayValue += stats.criticalChoices * 20.0f;

    return std::min(100.0f, replayValue);
}

} // namespace Story
