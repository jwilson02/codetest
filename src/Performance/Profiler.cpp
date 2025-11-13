#include "Profiler.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace Performance {

Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::Initialize(size_t historySize) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_HistorySize = historySize;
    m_FrameHistory.reserve(historySize);
    m_SampleHistory.reserve(historySize);
    m_StartTime = std::chrono::high_resolution_clock::now();
    m_FrameCount = 0;
    m_InFrame = false;

    std::cout << "[Profiler] Initialized with history size: " << historySize << std::endl;
}

void Profiler::Shutdown() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_FrameHistory.clear();
    m_SampleHistory.clear();
    m_CurrentSamples.clear();

    while (!m_ScopeStack.empty()) m_ScopeStack.pop();
    while (!m_TimeStack.empty()) m_TimeStack.pop();
    while (!m_DepthStack.empty()) m_DepthStack.pop();

    std::cout << "[Profiler] Shutdown" << std::endl;
}

void Profiler::BeginFrame() {
    if (!m_Enabled) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    m_InFrame = true;
    m_FrameStartTime = GetCurrentTimeMS();
    m_CurrentSamples.clear();
    m_CurrentDepth = 0;

    // Clear stacks (safety check)
    while (!m_ScopeStack.empty()) m_ScopeStack.pop();
    while (!m_TimeStack.empty()) m_TimeStack.pop();
    while (!m_DepthStack.empty()) m_DepthStack.pop();
}

void Profiler::EndFrame() {
    if (!m_Enabled || !m_InFrame) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    double frameEndTime = GetCurrentTimeMS();
    double frameTime = frameEndTime - m_FrameStartTime;

    // Update frame stats
    m_CurrentFrameStats.frameTimeMS = frameTime;
    m_CurrentFrameStats.fps = frameTime > 0.0 ? 1000.0 / frameTime : 0.0;

    // Add to history
    m_FrameHistory.push_back(m_CurrentFrameStats);
    m_SampleHistory.push_back(m_CurrentSamples);

    // Limit history size
    if (m_FrameHistory.size() > m_HistorySize) {
        m_FrameHistory.erase(m_FrameHistory.begin());
        m_SampleHistory.erase(m_SampleHistory.begin());
    }

    m_FrameCount++;
    m_InFrame = false;
}

void Profiler::BeginScope(const std::string& name) {
    if (!m_Enabled || !m_InFrame) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    double startTime = GetCurrentTimeMS();

    m_ScopeStack.push(name);
    m_TimeStack.push(startTime);
    m_DepthStack.push(m_CurrentDepth);
    m_CurrentDepth++;
}

void Profiler::EndScope() {
    if (!m_Enabled || !m_InFrame || m_ScopeStack.empty()) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    double endTime = GetCurrentTimeMS();

    std::string name = m_ScopeStack.top();
    double startTime = m_TimeStack.top();
    size_t depth = m_DepthStack.top();

    m_ScopeStack.pop();
    m_TimeStack.pop();
    m_DepthStack.pop();
    m_CurrentDepth--;

    ProfileSample sample;
    sample.name = name;
    sample.startTime = startTime;
    sample.endTime = endTime;
    sample.elapsedMS = endTime - startTime;
    sample.callCount = 1;
    sample.depth = depth;

    m_CurrentSamples.push_back(sample);
}

double Profiler::GetCurrentTimeMS() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_StartTime);
    return duration.count() / 1000.0;
}

FrameStats Profiler::GetAverageFrameStats(size_t frameCount) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_FrameHistory.empty()) {
        return FrameStats();
    }

    size_t count = frameCount == 0 ? m_FrameHistory.size()
                                   : std::min(frameCount, m_FrameHistory.size());

    FrameStats avg;
    size_t startIdx = m_FrameHistory.size() - count;

    for (size_t i = startIdx; i < m_FrameHistory.size(); ++i) {
        const FrameStats& stats = m_FrameHistory[i];
        avg.frameTimeMS += stats.frameTimeMS;
        avg.fps += stats.fps;
        avg.drawCalls += stats.drawCalls;
        avg.trianglesRendered += stats.trianglesRendered;
        avg.objectsRendered += stats.objectsRendered;
        avg.objectsCulled += stats.objectsCulled;
    }

    if (count > 0) {
        avg.frameTimeMS /= count;
        avg.fps /= count;
        avg.drawCalls /= count;
        avg.trianglesRendered /= count;
        avg.objectsRendered /= count;
        avg.objectsCulled /= count;
    }

    return avg;
}

std::shared_ptr<ProfileData> Profiler::GetFrameProfile() const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_CurrentSamples.empty()) {
        return nullptr;
    }

    return BuildProfileTree(m_CurrentSamples);
}

std::shared_ptr<ProfileData> Profiler::GetAverageProfile(size_t frameCount) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_SampleHistory.empty()) {
        return nullptr;
    }

    size_t count = std::min(frameCount, m_SampleHistory.size());
    size_t startIdx = m_SampleHistory.size() - count;

    std::vector<std::shared_ptr<ProfileData>> trees;
    for (size_t i = startIdx; i < m_SampleHistory.size(); ++i) {
        auto tree = BuildProfileTree(m_SampleHistory[i]);
        if (tree) {
            trees.push_back(tree);
        }
    }

    if (trees.empty()) {
        return nullptr;
    }

    auto merged = MergeProfileTrees(trees);

    // Calculate averages and percentages
    if (merged) {
        merged->CalculateAverage(count);
        double frameTime = GetAverageFrameStats(count).frameTimeMS;
        merged->CalculatePercentOfFrame(frameTime);

        // Recursively calculate for children
        std::function<void(std::shared_ptr<ProfileData>&, size_t, double)> processChildren;
        processChildren = [&](std::shared_ptr<ProfileData>& node, size_t frames, double frameTimeMS) {
            node->CalculateAverage(frames);
            node->CalculatePercentOfFrame(frameTimeMS);
            for (auto& child : node->children) {
                processChildren(child, frames, frameTimeMS);
            }
        };

        for (auto& child : merged->children) {
            processChildren(child, count, frameTime);
        }
    }

    return merged;
}

std::shared_ptr<ProfileData> Profiler::BuildProfileTree(
    const std::vector<ProfileSample>& samples) const {

    if (samples.empty()) {
        return nullptr;
    }

    auto root = std::make_shared<ProfileData>();
    root->name = "Frame";
    root->depth = 0;

    std::vector<std::shared_ptr<ProfileData>> stack;
    stack.push_back(root);

    for (const auto& sample : samples) {
        // Pop stack to correct depth
        while (stack.size() > sample.depth + 1) {
            stack.pop_back();
        }

        // Create node
        auto node = std::make_shared<ProfileData>();
        node->name = sample.name;
        node->depth = sample.depth;
        node->Update(sample.elapsedMS);

        // Add to parent
        if (!stack.empty()) {
            stack.back()->children.push_back(node);
            stack.back()->Update(sample.elapsedMS);
        }

        stack.push_back(node);
    }

    return root;
}

std::shared_ptr<ProfileData> Profiler::MergeProfileTrees(
    const std::vector<std::shared_ptr<ProfileData>>& trees) const {

    if (trees.empty()) {
        return nullptr;
    }

    if (trees.size() == 1) {
        return trees[0];
    }

    // Create merged root
    auto merged = std::make_shared<ProfileData>();
    merged->name = trees[0]->name;
    merged->depth = trees[0]->depth;

    // Merge data
    for (const auto& tree : trees) {
        merged->totalTimeMS += tree->totalTimeMS;
        merged->minTimeMS = std::min(merged->minTimeMS, tree->minTimeMS);
        merged->maxTimeMS = std::max(merged->maxTimeMS, tree->maxTimeMS);
        merged->callCount += tree->callCount;
    }

    // Merge children recursively
    std::unordered_map<std::string, std::vector<std::shared_ptr<ProfileData>>> childrenByName;

    for (const auto& tree : trees) {
        for (const auto& child : tree->children) {
            childrenByName[child->name].push_back(child);
        }
    }

    for (const auto& pair : childrenByName) {
        auto mergedChild = MergeProfileTrees(pair.second);
        if (mergedChild) {
            merged->children.push_back(mergedChild);
        }
    }

    return merged;
}

std::vector<BottleneckInfo> Profiler::DetectBottlenecks(double threshold) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::vector<BottleneckInfo> bottlenecks;

    auto profile = GetAverageProfile(60);
    if (!profile) {
        return bottlenecks;
    }

    // Check each system against its budget
    std::unordered_map<std::string, double> budgets = {
        {"Physics", PerformanceBudget::PHYSICS_BUDGET_MS},
        {"Rendering", PerformanceBudget::RENDERING_BUDGET_MS},
        {"AI", PerformanceBudget::AI_BUDGET_MS},
        {"Gameplay", PerformanceBudget::GAMEPLAY_BUDGET_MS},
        {"UI", PerformanceBudget::UI_BUDGET_MS},
        {"Audio", PerformanceBudget::AUDIO_BUDGET_MS}
    };

    // Search for systems in profile tree
    std::function<void(const std::shared_ptr<ProfileData>&)> searchTree;
    searchTree = [&](const std::shared_ptr<ProfileData>& node) {
        for (const auto& pair : budgets) {
            if (node->name.find(pair.first) != std::string::npos) {
                auto bottleneck = CheckBudget(pair.first, node->avgTimeMS, pair.second);
                if (bottleneck && bottleneck->overBudgetPercent > threshold) {
                    bottlenecks.push_back(*bottleneck);
                }
            }
        }

        for (const auto& child : node->children) {
            searchTree(child);
        }
    };

    searchTree(profile);

    // Sort by severity
    std::sort(bottlenecks.begin(), bottlenecks.end(),
        [](const BottleneckInfo& a, const BottleneckInfo& b) {
            return a.overBudgetPercent > b.overBudgetPercent;
        });

    return bottlenecks;
}

std::shared_ptr<BottleneckInfo> Profiler::CheckBudget(
    const std::string& systemName, double timeMS, double budgetMS) const {

    if (timeMS <= budgetMS) {
        return nullptr;
    }

    auto info = std::make_shared<BottleneckInfo>();
    info->systemName = systemName;
    info->timeMS = timeMS;
    info->budgetMS = budgetMS;
    info->overBudgetPercent = ((timeMS - budgetMS) / budgetMS) * 100.0;
    info->isCritical = info->overBudgetPercent > 50.0; // Over budget by 50%+

    return info;
}

void Profiler::PrintResults(bool detailed) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::cout << "\n========== Performance Profile ==========\n" << std::endl;

    // Frame stats
    auto avgStats = GetAverageFrameStats(60);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Frame Time: " << avgStats.frameTimeMS << " ms" << std::endl;
    std::cout << "FPS: " << avgStats.fps << std::endl;
    std::cout << "Draw Calls: " << avgStats.drawCalls << std::endl;
    std::cout << "Triangles: " << avgStats.trianglesRendered << std::endl;
    std::cout << "Objects Rendered: " << avgStats.objectsRendered << std::endl;
    std::cout << "Objects Culled: " << avgStats.objectsCulled << std::endl;

    if (detailed) {
        std::cout << "\n--- Detailed Profile ---\n" << std::endl;
        auto profile = GetAverageProfile(60);
        if (profile) {
            PrintProfileTree(profile);
        }
    }

    // Bottlenecks
    std::cout << "\n--- Bottleneck Detection ---\n" << std::endl;
    auto bottlenecks = DetectBottlenecks(10.0);
    if (bottlenecks.empty()) {
        std::cout << "No bottlenecks detected!" << std::endl;
    } else {
        for (const auto& bn : bottlenecks) {
            std::cout << (bn.isCritical ? "[CRITICAL] " : "[WARNING] ");
            std::cout << bn.systemName << ": " << bn.timeMS << " ms ";
            std::cout << "(budget: " << bn.budgetMS << " ms, ";
            std::cout << "over by " << bn.overBudgetPercent << "%)" << std::endl;
        }
    }

    std::cout << "\n========================================\n" << std::endl;
}

void Profiler::PrintProfileTree(const std::shared_ptr<ProfileData>& data, size_t indent) const {
    if (!data) return;

    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << data->name << ": ";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << data->avgTimeMS << " ms ";
    std::cout << "(" << data->percentOfFrame << "%) ";
    std::cout << "[min: " << data->minTimeMS << ", max: " << data->maxTimeMS << "]";
    std::cout << std::endl;

    for (const auto& child : data->children) {
        PrintProfileTree(child, indent + 1);
    }
}

void Profiler::ExportToJSON(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Profiler] Failed to open file: " << filepath << std::endl;
        return;
    }

    auto profile = GetAverageProfile(60);
    auto stats = GetAverageFrameStats(60);

    file << "{\n";
    file << "  \"frameStats\": {\n";
    file << "    \"frameTimeMS\": " << stats.frameTimeMS << ",\n";
    file << "    \"fps\": " << stats.fps << ",\n";
    file << "    \"drawCalls\": " << stats.drawCalls << ",\n";
    file << "    \"trianglesRendered\": " << stats.trianglesRendered << ",\n";
    file << "    \"objectsRendered\": " << stats.objectsRendered << ",\n";
    file << "    \"objectsCulled\": " << stats.objectsCulled << "\n";
    file << "  },\n";

    file << "  \"profile\": {\n";
    if (profile) {
        // Simplified profile output
        file << "    \"name\": \"" << profile->name << "\",\n";
        file << "    \"avgTimeMS\": " << profile->avgTimeMS << ",\n";
        file << "    \"percentOfFrame\": " << profile->percentOfFrame << "\n";
    }
    file << "  },\n";

    auto bottlenecks = DetectBottlenecks(10.0);
    file << "  \"bottlenecks\": [\n";
    for (size_t i = 0; i < bottlenecks.size(); ++i) {
        const auto& bn = bottlenecks[i];
        file << "    {\n";
        file << "      \"system\": \"" << bn.systemName << "\",\n";
        file << "      \"timeMS\": " << bn.timeMS << ",\n";
        file << "      \"budgetMS\": " << bn.budgetMS << ",\n";
        file << "      \"overBudgetPercent\": " << bn.overBudgetPercent << ",\n";
        file << "      \"isCritical\": " << (bn.isCritical ? "true" : "false") << "\n";
        file << "    }" << (i < bottlenecks.size() - 1 ? "," : "") << "\n";
    }
    file << "  ]\n";
    file << "}\n";

    file.close();
    std::cout << "[Profiler] Exported to JSON: " << filepath << std::endl;
}

void Profiler::ExportToCSV(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Profiler] Failed to open file: " << filepath << std::endl;
        return;
    }

    file << "Frame,FrameTimeMS,FPS,DrawCalls,Triangles,ObjectsRendered,ObjectsCulled\n";

    for (size_t i = 0; i < m_FrameHistory.size(); ++i) {
        const auto& stats = m_FrameHistory[i];
        file << i << ",";
        file << stats.frameTimeMS << ",";
        file << stats.fps << ",";
        file << stats.drawCalls << ",";
        file << stats.trianglesRendered << ",";
        file << stats.objectsRendered << ",";
        file << stats.objectsCulled << "\n";
    }

    file.close();
    std::cout << "[Profiler] Exported to CSV: " << filepath << std::endl;
}

void Profiler::Reset() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_FrameHistory.clear();
    m_SampleHistory.clear();
    m_CurrentSamples.clear();
    m_FrameCount = 0;

    while (!m_ScopeStack.empty()) m_ScopeStack.pop();
    while (!m_TimeStack.empty()) m_TimeStack.pop();
    while (!m_DepthStack.empty()) m_DepthStack.pop();

    m_CurrentFrameStats = FrameStats();

    std::cout << "[Profiler] Reset" << std::endl;
}

void Profiler::UpdateRenderStats(size_t drawCalls, size_t triangles,
                                 size_t objectsRendered, size_t objectsCulled) {
    if (!m_Enabled || !m_InFrame) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    m_CurrentFrameStats.drawCalls = drawCalls;
    m_CurrentFrameStats.trianglesRendered = triangles;
    m_CurrentFrameStats.objectsRendered = objectsRendered;
    m_CurrentFrameStats.objectsCulled = objectsCulled;
}

} // namespace Performance
