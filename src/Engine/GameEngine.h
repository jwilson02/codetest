#pragma once

#include "Time.h"
#include "EventSystem.h"
#include "SceneManager.h"
#include <SDL2/SDL.h>
#include <string>
#include <memory>

namespace Engine {

/**
 * @brief Configuration structure for engine initialization
 */
struct EngineConfig {
    std::string windowTitle = "SCUMM Style ARPG";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
    int targetFPS = 60;
    double fixedTimestep = 1.0 / 60.0; // 60 FPS physics
};

/**
 * @brief Main game engine class
 *
 * Manages the game loop, window, rendering context, and all engine systems.
 * Implements fixed timestep game loop for consistent physics simulation.
 */
class GameEngine {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the game engine instance
     */
    static GameEngine& GetInstance();

    /**
     * @brief Initialize the engine with configuration
     * @param config Engine configuration
     * @return True if initialization was successful, false otherwise
     */
    bool Initialize(const EngineConfig& config = EngineConfig());

    /**
     * @brief Start the main game loop
     *
     * This will run until the engine is stopped or the window is closed.
     */
    void Run();

    /**
     * @brief Stop the engine and exit the game loop
     */
    void Stop();

    /**
     * @brief Shutdown the engine and cleanup resources
     */
    void Shutdown();

    /**
     * @brief Check if the engine is running
     * @return True if running, false otherwise
     */
    bool IsRunning() const { return m_Running; }

    /**
     * @brief Get the SDL window
     * @return Pointer to the SDL window
     */
    SDL_Window* GetWindow() const { return m_Window; }

    /**
     * @brief Get the OpenGL context
     * @return SDL OpenGL context
     */
    SDL_GLContext GetGLContext() const { return m_GLContext; }

    /**
     * @brief Get the engine configuration
     * @return Reference to the engine configuration
     */
    const EngineConfig& GetConfig() const { return m_Config; }

    /**
     * @brief Set the window title
     * @param title New window title
     */
    void SetWindowTitle(const std::string& title);

    /**
     * @brief Set the window size
     * @param width New width in pixels
     * @param height New height in pixels
     */
    void SetWindowSize(int width, int height);

    /**
     * @brief Toggle fullscreen mode
     */
    void ToggleFullscreen();

    /**
     * @brief Get the window dimensions
     * @param width Output parameter for width
     * @param height Output parameter for height
     */
    void GetWindowSize(int& width, int& height) const;

private:
    GameEngine() = default;
    ~GameEngine() = default;

    // Prevent copying
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    /**
     * @brief Process SDL events
     */
    void ProcessEvents();

    /**
     * @brief Update game logic with variable timestep
     * @param deltaTime Time elapsed since last frame
     */
    void Update(double deltaTime);

    /**
     * @brief Fixed update for physics and game logic
     * @param fixedDeltaTime Fixed timestep
     */
    void FixedUpdate(double fixedDeltaTime);

    /**
     * @brief Render the current frame
     */
    void Render();

    /**
     * @brief Initialize SDL
     * @return True if successful, false otherwise
     */
    bool InitializeSDL();

    /**
     * @brief Initialize OpenGL
     * @return True if successful, false otherwise
     */
    bool InitializeOpenGL();

    /**
     * @brief Log engine information
     */
    void LogEngineInfo() const;

    // Engine state
    bool m_Initialized = false;
    bool m_Running = false;
    EngineConfig m_Config;

    // SDL components
    SDL_Window* m_Window = nullptr;
    SDL_GLContext m_GLContext = nullptr;

    // Event listener IDs
    EventListenerID m_WindowCloseListenerID = 0;
    EventListenerID m_WindowResizeListenerID = 0;
};

} // namespace Engine
