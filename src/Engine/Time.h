#pragma once

#include <chrono>
#include <cstdint>

namespace Engine {

/**
 * @brief Time management system for the game engine
 *
 * Provides high-precision timing for fixed timestep game loops,
 * delta time calculations, and frame rate management.
 */
class Time {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;

    /**
     * @brief Initialize the time system
     */
    static void Initialize();

    /**
     * @brief Update timing information - call once per frame
     */
    static void Update();

    /**
     * @brief Get delta time in seconds (variable timestep)
     * @return Time elapsed since last frame in seconds
     */
    static double GetDeltaTime();

    /**
     * @brief Get fixed delta time in seconds (for physics)
     * @return Fixed timestep in seconds
     */
    static double GetFixedDeltaTime();

    /**
     * @brief Get total time since engine start in seconds
     * @return Total elapsed time in seconds
     */
    static double GetTime();

    /**
     * @brief Get current frame count
     * @return Number of frames since engine start
     */
    static uint64_t GetFrameCount();

    /**
     * @brief Get current frames per second
     * @return Average FPS over the last second
     */
    static double GetFPS();

    /**
     * @brief Set the target fixed timestep for physics updates
     * @param timestep Target timestep in seconds (default: 1/60)
     */
    static void SetFixedTimestep(double timestep);

    /**
     * @brief Get the accumulator for fixed timestep updates
     * @return Time accumulated for fixed updates in seconds
     */
    static double GetAccumulator();

    /**
     * @brief Consume one fixed timestep from the accumulator
     */
    static void ConsumeFixedTimestep();

    /**
     * @brief Check if a fixed update should be performed
     * @return True if accumulator has enough time for a fixed update
     */
    static bool ShouldDoFixedUpdate();

    /**
     * @brief Reset the time system
     */
    static void Reset();

private:
    Time() = default;
    ~Time() = default;

    static TimePoint s_StartTime;
    static TimePoint s_LastFrameTime;
    static TimePoint s_CurrentFrameTime;
    static double s_DeltaTime;
    static double s_FixedDeltaTime;
    static double s_TimeScale;
    static double s_Accumulator;
    static uint64_t s_FrameCount;

    // FPS calculation
    static double s_FPSTimer;
    static uint32_t s_FPSFrameCount;
    static double s_CurrentFPS;
};

} // namespace Engine
