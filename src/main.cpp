#include "Engine/GameEngine.h"
#include "Engine/SceneManager.h"
#include "Engine/EventSystem.h"
#include "Engine/Time.h"
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/Component.h"
#include <iostream>
#include <memory>
#include <SDL2/SDL.h>

using namespace Engine;

/**
 * @brief Example component for demonstration
 */
class TransformComponent : public ComponentBase<TransformComponent> {
public:
    TransformComponent(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : m_Position{x, y, z} {}

    void Update(double deltaTime) override {
        // Example: Rotate the position slightly each frame
        m_Rotation += deltaTime * 0.5;
    }

    float m_Position[3];
    float m_Rotation = 0.0f;
    float m_Scale[3] = {1.0f, 1.0f, 1.0f};
};

/**
 * @brief Example component for player movement
 */
class PlayerControllerComponent : public ComponentBase<PlayerControllerComponent> {
public:
    PlayerControllerComponent() {
        // Subscribe to key events
        auto& eventSystem = EventSystem::GetInstance();
        m_KeyPressedListener = eventSystem.Subscribe<KeyPressedEvent>(
            [this](const KeyPressedEvent& event) {
                OnKeyPressed(event);
            }
        );
    }

    ~PlayerControllerComponent() {
        // Unsubscribe from events
        auto& eventSystem = EventSystem::GetInstance();
        eventSystem.Unsubscribe<KeyPressedEvent>(m_KeyPressedListener);
    }

    void Update(double deltaTime) override {
        // Example: Simple movement based on key state
        auto transform = GetEntity()->GetComponent<TransformComponent>();
        if (transform) {
            if (m_MoveLeft) {
                transform->m_Position[0] -= m_Speed * deltaTime;
            }
            if (m_MoveRight) {
                transform->m_Position[0] += m_Speed * deltaTime;
            }
            if (m_MoveUp) {
                transform->m_Position[1] += m_Speed * deltaTime;
            }
            if (m_MoveDown) {
                transform->m_Position[1] -= m_Speed * deltaTime;
            }
        }
    }

private:
    void OnKeyPressed(const KeyPressedEvent& event) {
        if (event.IsRepeat()) return;

        switch (event.GetKeyCode()) {
            case SDLK_LEFT:
            case SDLK_a:
                m_MoveLeft = true;
                std::cout << "[Player] Moving left" << std::endl;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                m_MoveRight = true;
                std::cout << "[Player] Moving right" << std::endl;
                break;
            case SDLK_UP:
            case SDLK_w:
                m_MoveUp = true;
                std::cout << "[Player] Moving up" << std::endl;
                break;
            case SDLK_DOWN:
            case SDLK_s:
                m_MoveDown = true;
                std::cout << "[Player] Moving down" << std::endl;
                break;
            case SDLK_SPACE:
                std::cout << "[Player] Action!" << std::endl;
                break;
            case SDLK_ESCAPE:
                std::cout << "[Player] Escape pressed - stopping engine" << std::endl;
                GameEngine::GetInstance().Stop();
                break;
        }
    }

    EventListenerID m_KeyPressedListener = 0;
    float m_Speed = 2.0f;
    bool m_MoveLeft = false;
    bool m_MoveRight = false;
    bool m_MoveUp = false;
    bool m_MoveDown = false;
};

/**
 * @brief Example game scene
 */
class GameScene : public Scene {
public:
    GameScene() : Scene("GameScene") {}

    void OnEnter() override {
        std::cout << "[GameScene] Creating game world..." << std::endl;

        // Create player entity
        auto player = CreateEntity("Player");
        player->AddComponent<TransformComponent>(0.0f, 0.0f, 0.0f);
        player->AddComponent<PlayerControllerComponent>();

        std::cout << "[GameScene] Player entity created with ID: " << player->GetID() << std::endl;

        // Create some example entities
        for (int i = 0; i < 3; ++i) {
            auto entity = CreateEntity("Entity_" + std::to_string(i));
            entity->AddComponent<TransformComponent>(
                static_cast<float>(i * 2),
                static_cast<float>(i),
                0.0f
            );
        }

        std::cout << "[GameScene] Created " << GetEntityCount() << " entities" << std::endl;
    }

    void OnExit() override {
        std::cout << "[GameScene] Cleaning up game world..." << std::endl;
    }

    void Update(double deltaTime) override {
        Scene::Update(deltaTime);

        // Display FPS every second
        static double fpsTimer = 0.0;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0) {
            std::cout << "[GameScene] FPS: " << Time::GetFPS()
                      << " | Frame: " << Time::GetFrameCount()
                      << " | Time: " << Time::GetTime() << "s" << std::endl;
            fpsTimer = 0.0;
        }
    }

    void Render() override {
        // In a real game, this would render sprites, tilemaps, etc.
        // For now, it's just a placeholder
    }
};

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    try {
        std::cout << "===========================================\n";
        std::cout << "  SCUMM VM-Style ARPG Game Engine\n";
        std::cout << "  Modern C++17 Architecture Demo\n";
        std::cout << "===========================================\n\n";

        // Configure the engine
        EngineConfig config;
        config.windowTitle = "SCUMM Style ARPG - Demo";
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.fullscreen = false;
        config.vsync = true;
        config.targetFPS = 60;
        config.fixedTimestep = 1.0 / 60.0;

        // Get engine instance
        auto& engine = GameEngine::GetInstance();

        // Initialize the engine
        if (!engine.Initialize(config)) {
            std::cerr << "Failed to initialize game engine!" << std::endl;
            return 1;
        }

        // Create and set up scenes
        auto& sceneManager = SceneManager::GetInstance();
        sceneManager.CreateScene<GameScene>("GameScene");

        // Switch to the game scene
        if (!sceneManager.SwitchScene("GameScene")) {
            std::cerr << "Failed to switch to GameScene!" << std::endl;
            engine.Shutdown();
            return 1;
        }

        std::cout << "\nControls:\n";
        std::cout << "  WASD / Arrow Keys - Move\n";
        std::cout << "  SPACE - Action\n";
        std::cout << "  ESC - Quit\n\n";

        // Run the game loop
        engine.Run();

        // Shutdown the engine
        engine.Shutdown();

        std::cout << "\n===========================================\n";
        std::cout << "  Thank you for playing!\n";
        std::cout << "===========================================\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred!" << std::endl;
        return 1;
    }
}
