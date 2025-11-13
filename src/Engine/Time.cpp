#include "Time.h"
#include <algorithm>

namespace Engine {

// Static member initialization
Time::TimePoint Time::s_StartTime;
Time::TimePoint Time::s_LastFrameTime;
Time::TimePoint Time::s_CurrentFrameTime;
double Time::s_DeltaTime = 0.0;
double Time::s_FixedDeltaTime = 1.0 / 60.0; // 60 FPS default
double Time::s_TimeScale = 1.0;
double Time::s_Accumulator = 0.0;
uint64_t Time::s_FrameCount = 0;
double Time::s_FPSTimer = 0.0;
uint32_t Time::s_FPSFrameCount = 0;
double Time::s_CurrentFPS = 0.0;

void Time::Initialize() {
    s_StartTime = Clock::now();
    s_LastFrameTime = s_StartTime;
    s_CurrentFrameTime = s_StartTime;
    s_DeltaTime = 0.0;
    s_Accumulator = 0.0;
    s_FrameCount = 0;
    s_FPSTimer = 0.0;
    s_FPSFrameCount = 0;
    s_CurrentFPS = 0.0;
}

void Time::Update() {
    s_CurrentFrameTime = Clock::now();
    Duration frameDuration = s_CurrentFrameTime - s_LastFrameTime;
    s_DeltaTime = frameDuration.count() * s_TimeScale;

    // Clamp delta time to prevent spiral of death
    // Maximum 250ms per frame (4 FPS minimum)
    s_DeltaTime = std::min(s_DeltaTime, 0.25);

    // Update accumulator for fixed timestep
    s_Accumulator += s_DeltaTime;

    // Update frame count
    s_FrameCount++;

    // Calculate FPS
    s_FPSTimer += s_DeltaTime;
    s_FPSFrameCount++;

    if (s_FPSTimer >= 1.0) {
        s_CurrentFPS = static_cast<double>(s_FPSFrameCount) / s_FPSTimer;
        s_FPSTimer = 0.0;
        s_FPSFrameCount = 0;
    }

    s_LastFrameTime = s_CurrentFrameTime;
}

double Time::GetDeltaTime() {
    return s_DeltaTime;
}

double Time::GetFixedDeltaTime() {
    return s_FixedDeltaTime;
}

double Time::GetTime() {
    Duration elapsed = s_CurrentFrameTime - s_StartTime;
    return elapsed.count();
}

uint64_t Time::GetFrameCount() {
    return s_FrameCount;
}

double Time::GetFPS() {
    return s_CurrentFPS;
}

void Time::SetFixedTimestep(double timestep) {
    if (timestep > 0.0) {
        s_FixedDeltaTime = timestep;
    }
}

double Time::GetAccumulator() {
    return s_Accumulator;
}

void Time::ConsumeFixedTimestep() {
    s_Accumulator -= s_FixedDeltaTime;
}

bool Time::ShouldDoFixedUpdate() {
    return s_Accumulator >= s_FixedDeltaTime;
}

void Time::Reset() {
    Initialize();
}

} // namespace Engine
