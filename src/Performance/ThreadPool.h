#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

namespace Performance {

/**
 * @brief Task priority for thread pool
 */
enum class TaskPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/**
 * @brief Thread pool for parallel task execution
 *
 * Manages a pool of worker threads for executing tasks asynchronously.
 * Supports task priorities, futures for result retrieval, and proper
 * shutdown handling.
 */
class ThreadPool {
public:
    /**
     * @brief Construct thread pool
     * @param numThreads Number of worker threads (0 = hardware concurrency)
     */
    explicit ThreadPool(size_t numThreads = 0);

    /**
     * @brief Destructor - waits for all tasks to complete
     */
    ~ThreadPool();

    /**
     * @brief Enqueue a task for execution
     * @tparam F Function type
     * @tparam Args Argument types
     * @param priority Task priority
     * @param f Function to execute
     * @param args Function arguments
     * @return Future for the task result
     */
    template<typename F, typename... Args>
    auto Enqueue(TaskPriority priority, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Enqueue a task with normal priority
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Function arguments
     * @return Future for the task result
     */
    template<typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        return Enqueue(TaskPriority::Normal, std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * @brief Wait for all tasks to complete
     */
    void WaitForAll();

    /**
     * @brief Get number of worker threads
     * @return Thread count
     */
    size_t GetThreadCount() const { return m_Workers.size(); }

    /**
     * @brief Get number of pending tasks
     * @return Pending task count
     */
    size_t GetPendingTaskCount() const;

    /**
     * @brief Get number of active tasks
     * @return Active task count
     */
    size_t GetActiveTaskCount() const { return m_ActiveTasks.load(); }

    /**
     * @brief Check if thread pool is idle
     * @return True if no tasks are pending or active
     */
    bool IsIdle() const;

    /**
     * @brief Shutdown the thread pool
     * @param waitForCompletion If true, waits for all tasks to complete
     */
    void Shutdown(bool waitForCompletion = true);

private:
    /**
     * @brief Task wrapper with priority
     */
    struct Task {
        TaskPriority priority;
        std::function<void()> function;

        Task(TaskPriority p, std::function<void()> f)
            : priority(p), function(std::move(f)) {}

        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    /**
     * @brief Worker thread function
     */
    void WorkerThread();

    std::vector<std::thread> m_Workers;
    std::priority_queue<Task> m_Tasks;

    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    std::condition_variable m_CompletionCondition;

    std::atomic<bool> m_Stop;
    std::atomic<size_t> m_ActiveTasks;
};

// Template implementation
template<typename F, typename... Args>
auto ThreadPool::Enqueue(TaskPriority priority, F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {

    using ReturnType = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        if (m_Stop) {
            throw std::runtime_error("Cannot enqueue task on stopped ThreadPool");
        }

        m_Tasks.emplace(priority, [task]() { (*task)(); });
    }

    m_Condition.notify_one();
    return result;
}

/**
 * @brief Parallel for loop using thread pool
 *
 * Divides iterations across worker threads for parallel execution.
 *
 * @param threadPool Thread pool to use
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @param func Function to execute for each index
 */
template<typename Func>
void ParallelFor(ThreadPool& threadPool, size_t start, size_t end, Func func) {
    if (start >= end) return;

    size_t numThreads = threadPool.GetThreadCount();
    size_t range = end - start;
    size_t chunkSize = std::max(size_t(1), range / numThreads);

    std::vector<std::future<void>> futures;

    for (size_t i = start; i < end; i += chunkSize) {
        size_t chunkEnd = std::min(i + chunkSize, end);

        auto future = threadPool.Enqueue([i, chunkEnd, &func]() {
            for (size_t j = i; j < chunkEnd; ++j) {
                func(j);
            }
        });

        futures.push_back(std::move(future));
    }

    // Wait for all chunks to complete
    for (auto& future : futures) {
        future.wait();
    }
}

/**
 * @brief Parallel for each using thread pool
 *
 * Executes a function for each element in parallel.
 *
 * @tparam Iterator Iterator type
 * @tparam Func Function type
 * @param threadPool Thread pool to use
 * @param begin Begin iterator
 * @param end End iterator
 * @param func Function to execute for each element
 */
template<typename Iterator, typename Func>
void ParallelForEach(ThreadPool& threadPool, Iterator begin, Iterator end, Func func) {
    size_t distance = std::distance(begin, end);
    if (distance == 0) return;

    size_t numThreads = threadPool.GetThreadCount();
    size_t chunkSize = std::max(size_t(1), distance / numThreads);

    std::vector<std::future<void>> futures;
    Iterator current = begin;

    while (current != end) {
        Iterator chunkEnd = current;
        std::advance(chunkEnd, std::min(chunkSize, static_cast<size_t>(std::distance(current, end))));

        auto future = threadPool.Enqueue([current, chunkEnd, &func]() {
            for (auto it = current; it != chunkEnd; ++it) {
                func(*it);
            }
        });

        futures.push_back(std::move(future));
        current = chunkEnd;
    }

    // Wait for all chunks to complete
    for (auto& future : futures) {
        future.wait();
    }
}

/**
 * @brief Job system for game tasks
 *
 * Higher-level abstraction over thread pool for common game tasks.
 */
class JobSystem {
public:
    /**
     * @brief Initialize job system
     * @param numThreads Number of worker threads
     */
    void Initialize(size_t numThreads = 0);

    /**
     * @brief Shutdown job system
     */
    void Shutdown();

    /**
     * @brief Get the singleton instance
     * @return Reference to job system
     */
    static JobSystem& GetInstance();

    /**
     * @brief Schedule a job
     * @tparam F Function type
     * @tparam Args Argument types
     * @param priority Job priority
     * @param f Function to execute
     * @param args Function arguments
     * @return Future for the job result
     */
    template<typename F, typename... Args>
    auto Schedule(TaskPriority priority, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        if (!m_ThreadPool) {
            throw std::runtime_error("JobSystem not initialized");
        }
        return m_ThreadPool->Enqueue(priority, std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * @brief Schedule a job with normal priority
     */
    template<typename F, typename... Args>
    auto Schedule(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        return Schedule(TaskPriority::Normal, std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * @brief Wait for all jobs to complete
     */
    void WaitForAll();

    /**
     * @brief Get the thread pool
     * @return Pointer to thread pool
     */
    ThreadPool* GetThreadPool() { return m_ThreadPool.get(); }

private:
    JobSystem() = default;
    ~JobSystem() = default;
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    std::unique_ptr<ThreadPool> m_ThreadPool;
};

} // namespace Performance
