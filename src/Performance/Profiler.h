#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <stack>
#include <memory>
#include <mutex>

namespace Performance {

/**
 * @brief Performance budget configuration for different systems
 *
 * Target: 60 FPS = 16.67ms per frame
 */
struct PerformanceBudget {
    static constexpr double FRAME_TIME_TARGET_MS = 16.67; // 60 FPS
    static constexpr double PHYSICS_BUDGET_MS = 2.0;      // 12% of frame
    static constexpr double RENDERING_BUDGET_MS = 8.0;     // 48% of frame
    static constexpr double AI_BUDGET_MS = 2.0;           // 12% of frame
    static constexpr double GAMEPLAY_BUDGET_MS = 2.0;      // 12% of frame
    static constexpr double UI_BUDGET_MS = 1.0;           // 6% of frame
    static constexpr double AUDIO_BUDGET_MS = 0.5;        // 3% of frame
    static constexpr double OVERHEAD_BUDGET_MS = 1.17;     // 7% of frame
};

/**
 * @brief Profile sample containing timing information
 */
struct ProfileSample {
    std::string name;
    double startTime;
    double endTime;
    double elapsedMS;
    size_t callCount;
    size_t depth;

    ProfileSample()
        : name(""), startTime(0.0), endTime(0.0), elapsedMS(0.0),
          callCount(0), depth(0) {}
};

/**
 * @brief Aggregated profile data over multiple frames
 */
struct ProfileData {
    std::string name;
    double totalTimeMS;
    double minTimeMS;
    double maxTimeMS;
    double avgTimeMS;
    size_t callCount;
    size_t depth;
    double percentOfFrame;

    std::vector<std::shared_ptr<ProfileData>> children;

    ProfileData()
        : name(""), totalTimeMS(0.0), minTimeMS(999999.0), maxTimeMS(0.0),
          avgTimeMS(0.0), callCount(0), depth(0), percentOfFrame(0.0) {}

    void Update(double elapsedMS) {
        totalTimeMS += elapsedMS;
        minTimeMS = std::min(minTimeMS, elapsedMS);
        maxTimeMS = std::max(maxTimeMS, elapsedMS);
        callCount++;
    }

    void CalculateAverage(size_t frameCount) {
        if (frameCount > 0) {
            avgTimeMS = totalTimeMS / frameCount;
        }
    }

    void CalculatePercentOfFrame(double frameTimeMS) {
        if (frameTimeMS > 0.0) {
            percentOfFrame = (avgTimeMS / frameTimeMS) * 100.0;
        }
    }
};

/**
 * @brief Frame statistics
 */
struct FrameStats {
    double frameTimeMS;
    double fps;
    size_t drawCalls;
    size_t trianglesRendered;
    size_t objectsRendered;
    size_t objectsCulled;

    FrameStats()
        : frameTimeMS(0.0), fps(0.0), drawCalls(0),
          trianglesRendered(0), objectsRendered(0), objectsCulled(0) {}
};

/**
 * @brief Bottleneck detection result
 */
struct BottleneckInfo {
    std::string systemName;
    double timeMS;
    double budgetMS;
    double overBudgetPercent;
    bool isCritical;

    BottleneckInfo()
        : systemName(""), timeMS(0.0), budgetMS(0.0),
          overBudgetPercent(0.0), isCritical(false) {}
};

/**
 * @brief Main performance profiler
 *
 * Provides hierarchical CPU profiling with frame time tracking,
 * performance budget monitoring, and bottleneck detection.
 * Thread-safe for multi-threaded profiling.
 */
class Profiler {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the profiler instance
     */
    static Profiler& GetInstance();

    /**
     * @brief Initialize the profiler
     * @param historySize Number of frames to keep in history
     */
    void Initialize(size_t historySize = 120);

    /**
     * @brief Shutdown the profiler
     */
    void Shutdown();

    /**
     * @brief Begin a new frame
     */
    void BeginFrame();

    /**
     * @brief End the current frame
     */
    void EndFrame();

    /**
     * @brief Begin a profiling scope
     * @param name Name of the scope
     */
    void BeginScope(const std::string& name);

    /**
     * @brief End the current profiling scope
     */
    void EndScope();

    /**
     * @brief Enable or disable profiling
     * @param enabled True to enable, false to disable
     */
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

    /**
     * @brief Check if profiling is enabled
     * @return True if enabled, false otherwise
     */
    bool IsEnabled() const { return m_Enabled; }

    /**
     * @brief Get current frame statistics
     * @return Reference to frame stats
     */
    const FrameStats& GetFrameStats() const { return m_CurrentFrameStats; }

    /**
     * @brief Get averaged frame statistics
     * @param frameCount Number of frames to average (0 = all history)
     * @return Averaged frame stats
     */
    FrameStats GetAverageFrameStats(size_t frameCount = 60) const;

    /**
     * @brief Get profile data for current frame
     * @return Hierarchical profile data
     */
    std::shared_ptr<ProfileData> GetFrameProfile() const;

    /**
     * @brief Get averaged profile data
     * @param frameCount Number of frames to average
     * @return Averaged hierarchical profile data
     */
    std::shared_ptr<ProfileData> GetAverageProfile(size_t frameCount = 60) const;

    /**
     * @brief Detect performance bottlenecks
     * @param threshold Percentage over budget to be considered a bottleneck (default 10%)
     * @return List of detected bottlenecks
     */
    std::vector<BottleneckInfo> DetectBottlenecks(double threshold = 10.0) const;

    /**
     * @brief Print profiling results to console
     * @param detailed Include detailed breakdown
     */
    void PrintResults(bool detailed = true) const;

    /**
     * @brief Export profiling data to JSON
     * @param filepath Path to output file
     */
    void ExportToJSON(const std::string& filepath) const;

    /**
     * @brief Export profiling data to CSV
     * @param filepath Path to output file
     */
    void ExportToCSV(const std::string& filepath) const;

    /**
     * @brief Reset all profiling data
     */
    void Reset();

    /**
     * @brief Update frame stats (called by renderer/engine)
     * @param drawCalls Number of draw calls this frame
     * @param triangles Number of triangles rendered
     * @param objectsRendered Number of objects rendered
     * @param objectsCulled Number of objects culled
     */
    void UpdateRenderStats(size_t drawCalls, size_t triangles,
                          size_t objectsRendered, size_t objectsCulled);

private:
    Profiler() = default;
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    /**
     * @brief Get current time in milliseconds
     * @return Current time
     */
    double GetCurrentTimeMS() const;

    /**
     * @brief Build profile tree from flat samples
     * @param samples Flat list of samples
     * @return Root of profile tree
     */
    std::shared_ptr<ProfileData> BuildProfileTree(
        const std::vector<ProfileSample>& samples) const;

    /**
     * @brief Merge profile trees for averaging
     * @param trees List of profile trees to merge
     * @return Merged and averaged tree
     */
    std::shared_ptr<ProfileData> MergeProfileTrees(
        const std::vector<std::shared_ptr<ProfileData>>& trees) const;

    /**
     * @brief Print profile tree recursively
     * @param data Profile data node
     * @param indent Indentation level
     */
    void PrintProfileTree(const std::shared_ptr<ProfileData>& data,
                         size_t indent = 0) const;

    /**
     * @brief Check if a system is over budget
     * @param systemName Name of the system
     * @param timeMS Time taken in milliseconds
     * @param budgetMS Budget in milliseconds
     * @return Bottleneck info if over budget, nullptr otherwise
     */
    std::shared_ptr<BottleneckInfo> CheckBudget(
        const std::string& systemName, double timeMS, double budgetMS) const;

    bool m_Enabled = true;
    bool m_InFrame = false;
    size_t m_HistorySize;
    size_t m_FrameCount = 0;

    // Timing
    std::chrono::high_resolution_clock::time_point m_StartTime;
    double m_FrameStartTime = 0.0;

    // Current frame data
    std::vector<ProfileSample> m_CurrentSamples;
    std::stack<std::string> m_ScopeStack;
    std::stack<double> m_TimeStack;
    std::stack<size_t> m_DepthStack;
    size_t m_CurrentDepth = 0;

    FrameStats m_CurrentFrameStats;

    // Historical data
    std::vector<FrameStats> m_FrameHistory;
    std::vector<std::vector<ProfileSample>> m_SampleHistory;

    // Thread safety
    mutable std::mutex m_Mutex;
};

/**
 * @brief RAII helper for profiling scopes
 *
 * Usage:
 *   PROFILE_SCOPE("MyFunction");
 */
class ProfileScope {
public:
    explicit ProfileScope(const std::string& name) {
        Profiler::GetInstance().BeginScope(name);
    }

    ~ProfileScope() {
        Profiler::GetInstance().EndScope();
    }

private:
    ProfileScope(const ProfileScope&) = delete;
    ProfileScope& operator=(const ProfileScope&) = delete;
};

// Convenience macros
#ifdef PROFILING_ENABLED
    #define PROFILE_FUNCTION() Performance::ProfileScope __profileScope(__FUNCTION__)
    #define PROFILE_SCOPE(name) Performance::ProfileScope __profileScope##__LINE__(name)
#else
    #define PROFILE_FUNCTION() ((void)0)
    #define PROFILE_SCOPE(name) ((void)0)
#endif

} // namespace Performance
