#include "ThreadPool.h"
#include <iostream>

namespace Performance {

ThreadPool::ThreadPool(size_t numThreads)
    : m_Stop(false), m_ActiveTasks(0) {

    // Use hardware concurrency if numThreads is 0
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4; // Fallback
    }

    m_Workers.reserve(numThreads);

    for (size_t i = 0; i < numThreads; ++i) {
        m_Workers.emplace_back([this] { WorkerThread(); });
    }

    std::cout << "[ThreadPool] Initialized with " << numThreads << " threads" << std::endl;
}

ThreadPool::~ThreadPool() {
    Shutdown(true);
}

void ThreadPool::WaitForAll() {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_CompletionCondition.wait(lock, [this] {
        return m_Tasks.empty() && m_ActiveTasks.load() == 0;
    });
}

size_t ThreadPool::GetPendingTaskCount() const {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    return m_Tasks.size();
}

bool ThreadPool::IsIdle() const {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    return m_Tasks.empty() && m_ActiveTasks.load() == 0;
}

void ThreadPool::Shutdown(bool waitForCompletion) {
    if (m_Stop.load()) return;

    if (waitForCompletion) {
        WaitForAll();
    }

    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Stop = true;
    }

    m_Condition.notify_all();

    for (std::thread& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "[ThreadPool] Shutdown" << std::endl;
}

void ThreadPool::WorkerThread() {
    while (true) {
        Task task(TaskPriority::Normal, nullptr);

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            m_Condition.wait(lock, [this] {
                return m_Stop.load() || !m_Tasks.empty();
            });

            if (m_Stop.load() && m_Tasks.empty()) {
                return;
            }

            if (!m_Tasks.empty()) {
                task = std::move(const_cast<Task&>(m_Tasks.top()));
                m_Tasks.pop();
                m_ActiveTasks++;
            }
        }

        if (task.function) {
            try {
                task.function();
            } catch (const std::exception& e) {
                std::cerr << "[ThreadPool] Task threw exception: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[ThreadPool] Task threw unknown exception" << std::endl;
            }

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_ActiveTasks--;

                if (m_Tasks.empty() && m_ActiveTasks.load() == 0) {
                    m_CompletionCondition.notify_all();
                }
            }
        }
    }
}

// ========== JobSystem Implementation ==========

JobSystem& JobSystem::GetInstance() {
    static JobSystem instance;
    return instance;
}

void JobSystem::Initialize(size_t numThreads) {
    if (m_ThreadPool) {
        std::cerr << "[JobSystem] Already initialized" << std::endl;
        return;
    }

    m_ThreadPool = std::make_unique<ThreadPool>(numThreads);
    std::cout << "[JobSystem] Initialized with " << m_ThreadPool->GetThreadCount()
              << " threads" << std::endl;
}

void JobSystem::Shutdown() {
    if (!m_ThreadPool) return;

    m_ThreadPool->Shutdown(true);
    m_ThreadPool.reset();

    std::cout << "[JobSystem] Shutdown" << std::endl;
}

void JobSystem::WaitForAll() {
    if (m_ThreadPool) {
        m_ThreadPool->WaitForAll();
    }
}

} // namespace Performance
