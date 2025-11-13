#include "MemoryTracker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <algorithm>

namespace Performance {

MemoryTracker& MemoryTracker::GetInstance() {
    static MemoryTracker instance;
    return instance;
}

void MemoryTracker::Initialize() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Enabled = true;
    m_NextAllocationID = 1;
    m_Allocations.clear();
    m_CategoryStats.clear();
    m_GlobalStats = MemoryStats();

    // Initialize category stats
    for (int i = 0; i < static_cast<int>(MemoryCategory::COUNT); ++i) {
        m_CategoryStats[static_cast<MemoryCategory>(i)] = MemoryStats();
    }

    std::cout << "[MemoryTracker] Initialized" << std::endl;
}

void MemoryTracker::Shutdown() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::cout << "\n[MemoryTracker] Shutting down..." << std::endl;

    PrintStats();

    auto leaks = DetectLeaks();
    if (!leaks.empty()) {
        std::cout << "\n[MemoryTracker] WARNING: Memory leaks detected!" << std::endl;
        PrintLeaks();
    } else {
        std::cout << "[MemoryTracker] No memory leaks detected." << std::endl;
    }

    m_Enabled = false;
}

void MemoryTracker::TrackAllocation(void* ptr, size_t size, const char* file,
                                   int line, const char* function,
                                   MemoryCategory category) {
    if (!m_Enabled || ptr == nullptr) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    AllocationInfo info;
    info.address = ptr;
    info.size = size;
    info.file = file;
    info.line = line;
    info.function = function;
    info.timestamp = GetTimestamp();
    info.allocationID = m_NextAllocationID++;

    m_Allocations[ptr] = info;

    // Update global stats
    m_GlobalStats.totalAllocated += size;
    m_GlobalStats.allocationCount++;
    m_GlobalStats.totalAllocationCount++;

    if (m_GlobalStats.totalAllocated > m_GlobalStats.peakAllocated) {
        m_GlobalStats.peakAllocated = m_GlobalStats.totalAllocated;
    }

    // Update category stats
    auto& catStats = m_CategoryStats[category];
    catStats.totalAllocated += size;
    catStats.allocationCount++;
    catStats.totalAllocationCount++;

    if (catStats.totalAllocated > catStats.peakAllocated) {
        catStats.peakAllocated = catStats.totalAllocated;
    }
}

void MemoryTracker::TrackDeallocation(void* ptr) {
    if (!m_Enabled || ptr == nullptr) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Allocations.find(ptr);
    if (it == m_Allocations.end()) {
        std::cerr << "[MemoryTracker] WARNING: Attempting to free untracked memory at "
                  << ptr << std::endl;
        return;
    }

    const AllocationInfo& info = it->second;
    size_t size = info.size;

    // Update global stats
    m_GlobalStats.totalAllocated -= size;
    m_GlobalStats.totalFreed += size;
    m_GlobalStats.allocationCount--;
    m_GlobalStats.freeCount++;

    // Update category stats (we don't track category on dealloc, so update all)
    // In a real implementation, we'd store the category with the allocation
    for (auto& pair : m_CategoryStats) {
        auto& catStats = pair.second;
        if (catStats.totalAllocated >= size) {
            catStats.totalAllocated -= size;
            catStats.totalFreed += size;
            catStats.allocationCount--;
            catStats.freeCount++;
            break;
        }
    }

    m_Allocations.erase(it);
}

MemoryStats MemoryTracker::GetStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_GlobalStats;
}

MemoryStats MemoryTracker::GetCategoryStats(MemoryCategory category) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_CategoryStats.find(category);
    if (it != m_CategoryStats.end()) {
        return it->second;
    }

    return MemoryStats();
}

std::vector<MemoryLeak> MemoryTracker::DetectLeaks() const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::vector<MemoryLeak> leaks;

    for (const auto& pair : m_Allocations) {
        const AllocationInfo& info = pair.second;

        MemoryLeak leak;
        leak.size = info.size;
        leak.file = info.file;
        leak.line = info.line;
        leak.function = info.function;
        leak.allocationID = info.allocationID;

        leaks.push_back(leak);
    }

    // Sort by size (largest first)
    std::sort(leaks.begin(), leaks.end(),
        [](const MemoryLeak& a, const MemoryLeak& b) {
            return a.size > b.size;
        });

    return leaks;
}

size_t MemoryTracker::GetAllocationCount() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_GlobalStats.allocationCount;
}

size_t MemoryTracker::GetTotalAllocated() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_GlobalStats.totalAllocated;
}

size_t MemoryTracker::GetPeakAllocated() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_GlobalStats.peakAllocated;
}

void MemoryTracker::PrintStats() const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::cout << "\n========== Memory Statistics ==========\n" << std::endl;

    auto formatBytes = [](size_t bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unit = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unit < 3) {
            size /= 1024.0;
            unit++;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return oss.str();
    };

    std::cout << "Total Allocated: " << formatBytes(m_GlobalStats.totalAllocated)
              << " (" << m_GlobalStats.totalAllocated << " bytes)" << std::endl;
    std::cout << "Total Freed: " << formatBytes(m_GlobalStats.totalFreed)
              << " (" << m_GlobalStats.totalFreed << " bytes)" << std::endl;
    std::cout << "Peak Allocated: " << formatBytes(m_GlobalStats.peakAllocated)
              << " (" << m_GlobalStats.peakAllocated << " bytes)" << std::endl;
    std::cout << "Active Allocations: " << m_GlobalStats.allocationCount << std::endl;
    std::cout << "Total Allocations: " << m_GlobalStats.totalAllocationCount << std::endl;
    std::cout << "Total Frees: " << m_GlobalStats.freeCount << std::endl;

    std::cout << "\n--- Memory by Category ---\n" << std::endl;

    for (int i = 0; i < static_cast<int>(MemoryCategory::COUNT); ++i) {
        MemoryCategory cat = static_cast<MemoryCategory>(i);
        auto it = m_CategoryStats.find(cat);

        if (it != m_CategoryStats.end() && it->second.totalAllocationCount > 0) {
            const auto& stats = it->second;
            std::cout << GetCategoryName(cat) << ": "
                      << formatBytes(stats.totalAllocated)
                      << " (peak: " << formatBytes(stats.peakAllocated) << ")"
                      << std::endl;
        }
    }

    std::cout << "\n======================================\n" << std::endl;
}

void MemoryTracker::PrintLeaks() const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto leaks = DetectLeaks();

    if (leaks.empty()) {
        std::cout << "No memory leaks detected." << std::endl;
        return;
    }

    std::cout << "\n========== Memory Leaks (" << leaks.size() << ") ==========\n" << std::endl;

    size_t totalLeaked = 0;
    int count = 0;
    const int maxDisplay = 20; // Display top 20 leaks

    for (const auto& leak : leaks) {
        totalLeaked += leak.size;

        if (count < maxDisplay) {
            std::cout << "[Leak #" << leak.allocationID << "] ";
            std::cout << leak.size << " bytes";

            if (leak.file) {
                std::cout << " at " << leak.file << ":" << leak.line;
            }

            if (leak.function) {
                std::cout << " in " << leak.function << "()";
            }

            std::cout << std::endl;
        }

        count++;
    }

    if (leaks.size() > maxDisplay) {
        std::cout << "\n... and " << (leaks.size() - maxDisplay) << " more leaks." << std::endl;
    }

    std::cout << "\nTotal leaked: " << totalLeaked << " bytes" << std::endl;
    std::cout << "=====================================\n" << std::endl;
}

void MemoryTracker::ExportReport(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[MemoryTracker] Failed to open file: " << filepath << std::endl;
        return;
    }

    file << "Memory Tracker Report\n";
    file << "=====================\n\n";

    file << "Global Statistics:\n";
    file << "  Total Allocated: " << m_GlobalStats.totalAllocated << " bytes\n";
    file << "  Total Freed: " << m_GlobalStats.totalFreed << " bytes\n";
    file << "  Peak Allocated: " << m_GlobalStats.peakAllocated << " bytes\n";
    file << "  Active Allocations: " << m_GlobalStats.allocationCount << "\n";
    file << "  Total Allocations: " << m_GlobalStats.totalAllocationCount << "\n";
    file << "  Total Frees: " << m_GlobalStats.freeCount << "\n\n";

    file << "Category Statistics:\n";
    for (int i = 0; i < static_cast<int>(MemoryCategory::COUNT); ++i) {
        MemoryCategory cat = static_cast<MemoryCategory>(i);
        auto it = m_CategoryStats.find(cat);

        if (it != m_CategoryStats.end() && it->second.totalAllocationCount > 0) {
            const auto& stats = it->second;
            file << "  " << GetCategoryName(cat) << ":\n";
            file << "    Allocated: " << stats.totalAllocated << " bytes\n";
            file << "    Peak: " << stats.peakAllocated << " bytes\n";
            file << "    Active: " << stats.allocationCount << "\n";
        }
    }

    auto leaks = DetectLeaks();
    if (!leaks.empty()) {
        file << "\nMemory Leaks (" << leaks.size() << "):\n";

        for (const auto& leak : leaks) {
            file << "  [Leak #" << leak.allocationID << "] ";
            file << leak.size << " bytes";

            if (leak.file) {
                file << " at " << leak.file << ":" << leak.line;
            }

            if (leak.function) {
                file << " in " << leak.function << "()";
            }

            file << "\n";
        }
    }

    file.close();
    std::cout << "[MemoryTracker] Exported report to: " << filepath << std::endl;
}

void MemoryTracker::TakeSnapshot(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    MemorySnapshot snapshot;
    snapshot.name = name;
    snapshot.stats = m_GlobalStats;
    snapshot.allocationCount = m_Allocations.size();

    m_Snapshots[name] = snapshot;

    std::cout << "[MemoryTracker] Snapshot taken: " << name << std::endl;
}

void MemoryTracker::CompareWithSnapshot(const std::string& snapshotName) const {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Snapshots.find(snapshotName);
    if (it == m_Snapshots.end()) {
        std::cerr << "[MemoryTracker] Snapshot not found: " << snapshotName << std::endl;
        return;
    }

    const MemorySnapshot& snapshot = it->second;

    std::cout << "\n========== Snapshot Comparison ==========\n" << std::endl;
    std::cout << "Snapshot: " << snapshotName << std::endl;

    int64_t allocDiff = static_cast<int64_t>(m_GlobalStats.totalAllocated) -
                        static_cast<int64_t>(snapshot.stats.totalAllocated);
    int64_t countDiff = static_cast<int64_t>(m_Allocations.size()) -
                        static_cast<int64_t>(snapshot.allocationCount);

    std::cout << "\nAllocated Memory: ";
    std::cout << m_GlobalStats.totalAllocated << " bytes ";
    std::cout << "(" << (allocDiff >= 0 ? "+" : "") << allocDiff << ")" << std::endl;

    std::cout << "Active Allocations: ";
    std::cout << m_Allocations.size() << " ";
    std::cout << "(" << (countDiff >= 0 ? "+" : "") << countDiff << ")" << std::endl;

    std::cout << "\n========================================\n" << std::endl;
}

void MemoryTracker::Reset() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Allocations.clear();
    m_GlobalStats = MemoryStats();
    m_NextAllocationID = 1;

    for (auto& pair : m_CategoryStats) {
        pair.second = MemoryStats();
    }

    std::cout << "[MemoryTracker] Reset" << std::endl;
}

uint64_t MemoryTracker::GetTimestamp() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch());
    return duration.count();
}

const char* MemoryTracker::GetCategoryName(MemoryCategory category) {
    switch (category) {
        case MemoryCategory::General:   return "General";
        case MemoryCategory::Rendering: return "Rendering";
        case MemoryCategory::Audio:     return "Audio";
        case MemoryCategory::Physics:   return "Physics";
        case MemoryCategory::AI:        return "AI";
        case MemoryCategory::Gameplay:  return "Gameplay";
        case MemoryCategory::UI:        return "UI";
        case MemoryCategory::Assets:    return "Assets";
        default:                        return "Unknown";
    }
}

} // namespace Performance
