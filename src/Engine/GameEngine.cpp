#include "GameEngine.h"
#include <GL/gl.h>
#include <iostream>
#include <stdexcept>

namespace Engine {

GameEngine& GameEngine::GetInstance() {
    static GameEngine instance;
    return instance;
}

bool GameEngine::Initialize(const EngineConfig& config) {
    if (m_Initialized) {
        std::cerr << "[Engine] Warning: Engine already initialized" << std::endl;
        return true;
    }

    std::cout << "[Engine] Initializing game engine..." << std::endl;

    m_Config = config;

    // Initialize SDL
    if (!InitializeSDL()) {
        std::cerr << "[Engine] Failed to initialize SDL" << std::endl;
        return false;
    }

    // Initialize OpenGL
    if (!InitializeOpenGL()) {
        std::cerr << "[Engine] Failed to initialize OpenGL" << std::endl;
        Shutdown();
        return false;
    }

    // Initialize engine systems
    Time::Initialize();
    Time::SetFixedTimestep(m_Config.fixedTimestep);

    // Subscribe to window events
    auto& eventSystem = EventSystem::GetInstance();
    m_WindowCloseListenerID = eventSystem.Subscribe<WindowCloseEvent>(
        [this](const WindowCloseEvent&) {
            std::cout << "[Engine] Window close event received" << std::endl;
            Stop();
        }
    );

    m_WindowResizeListenerID = eventSystem.Subscribe<WindowResizeEvent>(
        [this](const WindowResizeEvent& event) {
            std::cout << "[Engine] Window resized to "
                      << event.GetWidth() << "x" << event.GetHeight() << std::endl;
            glViewport(0, 0, event.GetWidth(), event.GetHeight());
        }
    );

    m_Initialized = true;

    LogEngineInfo();

    std::cout << "[Engine] Initialization complete" << std::endl;

    return true;
}

void GameEngine::Run() {
    if (!m_Initialized) {
        std::cerr << "[Engine] Error: Engine not initialized. Call Initialize() first." << std::endl;
        return;
    }

    if (m_Running) {
        std::cerr << "[Engine] Warning: Engine already running" << std::endl;
        return;
    }

    std::cout << "[Engine] Starting game loop..." << std::endl;

    m_Running = true;
    Time::Initialize(); // Reset time for clean start

    // Main game loop with fixed timestep
    while (m_Running) {
        Time::Update();

        // Process input events
        ProcessEvents();

        // Fixed timestep updates (physics, etc.)
        while (Time::ShouldDoFixedUpdate()) {
            FixedUpdate(Time::GetFixedDeltaTime());
            Time::ConsumeFixedTimestep();
        }

        // Variable timestep update (rendering, animations, etc.)
        Update(Time::GetDeltaTime());

        // Render
        Render();

        // Cap frame rate if not using VSync
        if (!m_Config.vsync && m_Config.targetFPS > 0) {
            double targetFrameTime = 1.0 / m_Config.targetFPS;
            double frameTime = Time::GetDeltaTime();
            if (frameTime < targetFrameTime) {
                int delayMs = static_cast<int>((targetFrameTime - frameTime) * 1000.0);
                SDL_Delay(delayMs);
            }
        }
    }

    std::cout << "[Engine] Game loop ended" << std::endl;
}

void GameEngine::Stop() {
    std::cout << "[Engine] Stopping engine..." << std::endl;
    m_Running = false;
}

void GameEngine::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    std::cout << "[Engine] Shutting down engine..." << std::endl;

    // Unsubscribe from events
    auto& eventSystem = EventSystem::GetInstance();
    if (m_WindowCloseListenerID != 0) {
        eventSystem.Unsubscribe<WindowCloseEvent>(m_WindowCloseListenerID);
    }
    if (m_WindowResizeListenerID != 0) {
        eventSystem.Unsubscribe<WindowResizeEvent>(m_WindowResizeListenerID);
    }

    // Clear all systems
    SceneManager::GetInstance().Clear();
    EventSystem::GetInstance().Clear();

    // Cleanup OpenGL
    if (m_GLContext) {
        SDL_GL_DeleteContext(m_GLContext);
        m_GLContext = nullptr;
    }

    // Cleanup SDL
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }

    SDL_Quit();

    m_Initialized = false;
    m_Running = false;

    std::cout << "[Engine] Shutdown complete" << std::endl;
}

void GameEngine::SetWindowTitle(const std::string& title) {
    if (m_Window) {
        SDL_SetWindowTitle(m_Window, title.c_str());
        m_Config.windowTitle = title;
    }
}

void GameEngine::SetWindowSize(int width, int height) {
    if (m_Window) {
        SDL_SetWindowSize(m_Window, width, height);
        m_Config.windowWidth = width;
        m_Config.windowHeight = height;

        // Publish resize event
        EventSystem::GetInstance().Publish(WindowResizeEvent(width, height));
    }
}

void GameEngine::ToggleFullscreen() {
    if (!m_Window) {
        return;
    }

    m_Config.fullscreen = !m_Config.fullscreen;

    Uint32 flags = m_Config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(m_Window, flags);

    std::cout << "[Engine] Fullscreen: " << (m_Config.fullscreen ? "ON" : "OFF") << std::endl;
}

void GameEngine::GetWindowSize(int& width, int& height) const {
    if (m_Window) {
        SDL_GetWindowSize(m_Window, &width, &height);
    } else {
        width = m_Config.windowWidth;
        height = m_Config.windowHeight;
    }
}

void GameEngine::ProcessEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                EventSystem::GetInstance().Publish(WindowCloseEvent());
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    EventSystem::GetInstance().Publish(
                        WindowResizeEvent(event.window.data1, event.window.data2)
                    );
                }
                break;

            case SDL_KEYDOWN:
                if (!event.key.repeat) {
                    EventSystem::GetInstance().Publish(
                        KeyPressedEvent(event.key.keysym.sym, false)
                    );
                } else {
                    EventSystem::GetInstance().Publish(
                        KeyPressedEvent(event.key.keysym.sym, true)
                    );
                }
                break;

            case SDL_KEYUP:
                EventSystem::GetInstance().Publish(
                    KeyReleasedEvent(event.key.keysym.sym)
                );
                break;

            default:
                break;
        }
    }
}

void GameEngine::Update(double deltaTime) {
    // Update the active scene
    SceneManager::GetInstance().Update(deltaTime);
}

void GameEngine::FixedUpdate(double fixedDeltaTime) {
    // Fixed update for the active scene
    SceneManager::GetInstance().FixedUpdate(fixedDeltaTime);
}

void GameEngine::Render() {
    // Clear the screen
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the active scene
    SceneManager::GetInstance().Render();

    // Swap buffers
    SDL_GL_SwapWindow(m_Window);
}

bool GameEngine::InitializeSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Engine] SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create window
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (m_Config.fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    m_Window = SDL_CreateWindow(
        m_Config.windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Config.windowWidth,
        m_Config.windowHeight,
        windowFlags
    );

    if (!m_Window) {
        std::cerr << "[Engine] SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool GameEngine::InitializeOpenGL() {
    // Create OpenGL context
    m_GLContext = SDL_GL_CreateContext(m_Window);
    if (!m_GLContext) {
        std::cerr << "[Engine] SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set VSync
    if (SDL_GL_SetSwapInterval(m_Config.vsync ? 1 : 0) != 0) {
        std::cerr << "[Engine] Warning: Unable to set VSync: " << SDL_GetError() << std::endl;
    }

    // Initialize viewport
    glViewport(0, 0, m_Config.windowWidth, m_Config.windowHeight);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void GameEngine::LogEngineInfo() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  SCUMM Style ARPG Engine" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Window Size:     " << m_Config.windowWidth << "x" << m_Config.windowHeight << std::endl;
    std::cout << "VSync:           " << (m_Config.vsync ? "ON" : "OFF") << std::endl;
    std::cout << "Fixed Timestep:  " << m_Config.fixedTimestep << "s" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

} // namespace Engine
