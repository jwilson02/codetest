#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Story {

// Tree node for branching narratives
struct TreeNode {
    std::string id;
    std::string parentId;
    std::vector<std::string> childIds;

    // Content
    std::string dialogueId;
    std::string description;

    // Branching info
    int branchingFactor = 0;    // Number of choices that led here
    int depth = 0;               // Depth in tree

    // State tracking
    bool visited = false;
    int visitCount = 0;
    float firstVisitTime = 0.0f;

    // Consequence tracking
    std::vector<std::string> consequenceTags;
    std::string narrativeBranch;  // Which major branch this belongs to

    TreeNode() = default;
};

// Branching point in narrative
struct BranchPoint {
    std::string id;
    std::string nodeId;
    std::string description;
    std::vector<std::string> branchIds;

    // Impact
    enum class ImpactLevel {
        MINOR,      // Affects single conversation
        MODERATE,   // Affects quest line
        MAJOR,      // Affects chapter outcome
        CRITICAL    // Affects ending
    };
    ImpactLevel impact;

    // Merge info
    bool canMerge = false;
    std::string mergeNodeId;

    BranchPoint() : impact(ImpactLevel::MINOR) {}
};

// Dialogue tree structure
class DialogueTree {
public:
    DialogueTree();
    ~DialogueTree();

    // Tree construction
    void setRootNode(const std::string& nodeId);
    void addNode(const TreeNode& node);
    void addBranch(const std::string& parentId, const std::string& childId);
    void removeBranch(const std::string& parentId, const std::string& childId);

    // Branch points
    void addBranchPoint(const BranchPoint& branchPoint);
    const BranchPoint* getBranchPoint(const std::string& id) const;
    std::vector<BranchPoint> getAllBranchPoints() const;

    // Navigation
    const TreeNode* getNode(const std::string& nodeId) const;
    const TreeNode* getRootNode() const;
    std::vector<const TreeNode*> getChildren(const std::string& nodeId) const;
    const TreeNode* getParent(const std::string& nodeId) const;
    std::vector<const TreeNode*> getPath(const std::string& fromId, const std::string& toId) const;

    // State management
    void markVisited(const std::string& nodeId);
    bool isVisited(const std::string& nodeId) const;
    void resetVisited();

    // Analysis
    int getTreeDepth() const;
    int getBranchingFactor(const std::string& nodeId) const;
    std::vector<std::string> getLeafNodes() const;
    std::vector<std::string> getCriticalPath() const; // Main story path

    // Consequence tracking
    void addConsequence(const std::string& nodeId, const std::string& tag);
    std::vector<std::string> getConsequences(const std::string& nodeId) const;
    std::vector<std::string> getAllConsequences() const;

    // Branch tracking
    void setCurrentBranch(const std::string& branchId);
    std::string getCurrentBranch() const;
    std::vector<std::string> getPossibleBranches(const std::string& nodeId) const;

    // Serialization
    bool loadFromJson(const std::string& jsonData);
    std::string toJson() const;
    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);

    // Visualization
    std::string generateGraphviz() const; // For visualization
    void exportStatistics(const std::string& filePath) const;

    // Validation
    bool validate(std::vector<std::string>& errors) const;
    bool hasCircularReferences() const;
    std::vector<std::string> findOrphanedNodes() const;

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal helpers
    void calculateDepth(const std::string& nodeId, int depth);
    bool hasPath(const std::string& fromId, const std::string& toId,
                 std::unordered_map<std::string, bool>& visited) const;
};

// Tree builder for creating complex narratives
class TreeBuilder {
public:
    TreeBuilder();

    TreeBuilder& startTree(const std::string& rootId);
    TreeBuilder& addLinearSequence(const std::vector<std::string>& nodeIds);
    TreeBuilder& addBranch(const std::string& fromId, const std::vector<std::string>& branchIds);
    TreeBuilder& mergeBranches(const std::vector<std::string>& branchIds, const std::string& mergeId);
    TreeBuilder& markCriticalChoice(const std::string& nodeId, BranchPoint::ImpactLevel impact);

    std::unique_ptr<DialogueTree> build();

private:
    std::unique_ptr<DialogueTree> tree_;
    std::string currentId_;
};

// Narrative flow analyzer
class NarrativeFlowAnalyzer {
public:
    struct FlowStatistics {
        int totalNodes;
        int totalBranchPoints;
        int maxDepth;
        float avgBranchingFactor;
        int criticalChoices;
        int possibleEndings;
        std::vector<std::string> deadEnds;
        std::unordered_map<std::string, int> branchPopularity; // How often branches are taken
    };

    static FlowStatistics analyze(const DialogueTree& tree);
    static std::vector<std::string> findOptimalPath(const DialogueTree& tree,
                                                     const std::string& goalNodeId);
    static float calculateReplayValue(const DialogueTree& tree); // Based on branching
};

} // namespace Story
