#include "GameLauncher.h"
#include "MainMenu.h"
#include "CharacterSelect.h"
#include "IntroSequence.h"
#include "FirstTimeSetup.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <algorithm>

namespace fs = std::filesystem;

namespace Launcher {

GameLauncher::GameLauncher()
    : currentState_(LauncherState::SplashScreen),
      previousState_(LauncherState::SplashScreen),
      shouldStartGame_(false),
      shouldExit_(false),
      isFirstLaunch_(false),
      gamepadActive_(false),
      selectedCharacter_("Warrior"),
      screenWidth_(1280),
      screenHeight_(720),
      renderer_(nullptr),
      splashTimer_(0.0f),
      logoTimer_(0.0f),
      transitionProgress_(0.0f),
      transitionDuration_(0.5f),
      isTransitioning_(false),
      currentLogoIndex_(0),
      currentLogoAlpha_(0.0f),
      particleSpawnTimer_(0.0f),
      particlesEnabled_(true),
      skipSplash_(false),
      skipLogos_(false),
      showIntro_(true),
      splashDuration_(3.0f) {
}

GameLauncher::~GameLauncher() {
}

bool GameLauncher::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Load configuration
    if (!LoadConfiguration()) {
        SDL_Log("Warning: Failed to load launcher configuration, using defaults");
    }

    // Load launch preferences
    LoadLaunchPreferences();

    // Check if this is first launch
    CheckFirstLaunch();

    // Load studio logos
    if (!LoadStudioLogos()) {
        SDL_Log("Warning: Failed to load studio logos");
    }

    // Detect installed DLCs
    DetectDLCs();

    // Initialize particle system
    InitializeParticles();

    // Create menu components
    mainMenu_ = std::make_unique<MainMenu>();
    mainMenu_->Initialize(renderer, screenWidth, screenHeight);
    mainMenu_->SetOnNewGame([this](const std::string& character) {
        selectedCharacter_ = character;
        TransitionToState(LauncherState::CharacterSelection);
    });
    mainMenu_->SetOnContinue([this]() {
        TransitionToState(LauncherState::LoadGame);
    });
    mainMenu_->SetOnSettings([this]() {
        TransitionToState(LauncherState::Settings);
    });
    mainMenu_->SetOnCredits([this]() {
        TransitionToState(LauncherState::Credits);
    });
    mainMenu_->SetOnExit([this]() {
        shouldExit_ = true;
    });

    // Create character selection
    characterSelect_ = std::make_unique<CharacterSelect>();
    characterSelect_->Initialize(renderer, screenWidth, screenHeight);
    characterSelect_->SetOnCharacterConfirmed([this](const std::string& character) {
        selectedCharacter_ = character;
        if (showIntro_) {
            TransitionToState(LauncherState::IntroSequence);
        } else {
            shouldStartGame_ = true;
            if (onGameStart_) {
                onGameStart_(selectedCharacter_);
            }
        }
    });
    characterSelect_->SetOnBack([this]() {
        TransitionToState(LauncherState::MainMenu);
    });

    // Create intro sequence
    introSequence_ = std::make_unique<IntroSequence>();
    introSequence_->Initialize(renderer, screenWidth, screenHeight);
    introSequence_->SetOnComplete([this]() {
        shouldStartGame_ = true;
        if (onGameStart_) {
            onGameStart_(selectedCharacter_);
        }
    });
    introSequence_->SetOnSkip([this]() {
        shouldStartGame_ = true;
        if (onGameStart_) {
            onGameStart_(selectedCharacter_);
        }
    });

    // Create first-time setup
    firstTimeSetup_ = std::make_unique<FirstTimeSetup>();
    firstTimeSetup_->Initialize(renderer, screenWidth, screenHeight);
    firstTimeSetup_->SetOnComplete([this]() {
        SaveLaunchPreferences();
        TransitionToState(LauncherState::MainMenu);
    });

    // Determine starting state
    if (isFirstLaunch_ && !skipSplash_) {
        currentState_ = LauncherState::SplashScreen;
    } else if (skipSplash_ && skipLogos_) {
        currentState_ = LauncherState::MainMenu;
    } else if (skipSplash_) {
        currentState_ = LauncherState::StudioLogos;
    }

    SDL_Log("GameLauncher initialized successfully");
    return true;
}

void GameLauncher::Update(float deltaTime) {
    // Update particles
    if (particlesEnabled_) {
        UpdateParticles(deltaTime);
    }

    // Update transition
    if (isTransitioning_) {
        transitionProgress_ += deltaTime / transitionDuration_;
        if (transitionProgress_ >= 1.0f) {
            transitionProgress_ = 1.0f;
            isTransitioning_ = false;
        }
    }

    // Update based on current state
    switch (currentState_) {
        case LauncherState::SplashScreen:
            UpdateSplashScreen(deltaTime);
            break;

        case LauncherState::StudioLogos:
            UpdateStudioLogos(deltaTime);
            break;

        case LauncherState::FirstTimeSetup:
            UpdateFirstTimeSetup(deltaTime);
            break;

        case LauncherState::MainMenu:
            UpdateMainMenu(deltaTime);
            break;

        case LauncherState::CharacterSelection:
            UpdateCharacterSelection(deltaTime);
            break;

        case LauncherState::IntroSequence:
            UpdateIntroSequence(deltaTime);
            break;

        case LauncherState::Settings:
        case LauncherState::Credits:
        case LauncherState::LoadGame:
            // These would be handled by their respective systems
            if (mainMenu_) {
                mainMenu_->Update(deltaTime);
            }
            break;

        default:
            break;
    }
}

void GameLauncher::Render(SDL_Renderer* renderer) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render based on current state
    switch (currentState_) {
        case LauncherState::SplashScreen:
            RenderSplashScreen(renderer);
            break;

        case LauncherState::StudioLogos:
            RenderStudioLogos(renderer);
            break;

        case LauncherState::FirstTimeSetup:
            if (firstTimeSetup_) {
                firstTimeSetup_->Render(renderer);
            }
            break;

        case LauncherState::MainMenu:
        case LauncherState::Settings:
        case LauncherState::Credits:
        case LauncherState::LoadGame:
            if (mainMenu_) {
                mainMenu_->Render(renderer);
            }
            break;

        case LauncherState::CharacterSelection:
            if (characterSelect_) {
                characterSelect_->Render(renderer);
            }
            break;

        case LauncherState::IntroSequence:
            if (introSequence_) {
                introSequence_->Render(renderer);
            }
            break;

        default:
            break;
    }

    // Render particles on top
    if (particlesEnabled_ && currentState_ == LauncherState::MainMenu) {
        RenderParticles(renderer);
    }

    // Render transition effect
    if (isTransitioning_) {
        RenderTransition(renderer);
    }
}

void GameLauncher::HandleInput(const SDL_Event& event) {
    // Detect gamepad activation
    if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERAXISMOTION) {
        gamepadActive_ = true;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEMOTION) {
        gamepadActive_ = false;
    }

    // Handle skip actions
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_SPACE) {
            SkipCurrentScreen();
        }
    } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START ||
            event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
            SkipCurrentScreen();
        }
    }

    // Delegate to current state handler
    switch (currentState_) {
        case LauncherState::FirstTimeSetup:
            if (firstTimeSetup_) {
                firstTimeSetup_->HandleInput(event);
            }
            break;

        case LauncherState::MainMenu:
        case LauncherState::Settings:
        case LauncherState::Credits:
        case LauncherState::LoadGame:
            if (mainMenu_) {
                mainMenu_->HandleInput(event);
            }
            break;

        case LauncherState::CharacterSelection:
            if (characterSelect_) {
                characterSelect_->HandleInput(event);
            }
            break;

        case LauncherState::IntroSequence:
            if (introSequence_) {
                introSequence_->HandleInput(event);
            }
            break;

        default:
            break;
    }
}

bool GameLauncher::IsFirstLaunch() const {
    return isFirstLaunch_;
}

void GameLauncher::SetDLCEnabled(const std::string& dlcId, bool enabled) {
    for (auto& dlc : detectedDLCs_) {
        if (dlc.id == dlcId) {
            dlc.enabled = enabled;
            SDL_Log("DLC '%s' %s", dlc.name.c_str(), enabled ? "enabled" : "disabled");
            break;
        }
    }
}

// Private methods

void GameLauncher::TransitionToState(LauncherState newState) {
    if (currentState_ == newState) return;

    previousState_ = currentState_;
    currentState_ = newState;
    isTransitioning_ = true;
    transitionProgress_ = 0.0f;

    SDL_Log("Transitioning from state %d to state %d",
            static_cast<int>(previousState_),
            static_cast<int>(currentState_));
}

void GameLauncher::UpdateSplashScreen(float deltaTime) {
    splashTimer_ += deltaTime;

    if (splashTimer_ >= splashDuration_) {
        if (!studioLogos_.empty() && !skipLogos_) {
            TransitionToState(LauncherState::StudioLogos);
        } else if (isFirstLaunch_) {
            TransitionToState(LauncherState::FirstTimeSetup);
        } else {
            TransitionToState(LauncherState::MainMenu);
        }
        splashTimer_ = 0.0f;
    }
}

void GameLauncher::UpdateStudioLogos(float deltaTime) {
    if (currentLogoIndex_ >= static_cast<int>(studioLogos_.size())) {
        if (isFirstLaunch_) {
            TransitionToState(LauncherState::FirstTimeSetup);
        } else {
            TransitionToState(LauncherState::MainMenu);
        }
        return;
    }

    const auto& logo = studioLogos_[currentLogoIndex_];
    logoTimer_ += deltaTime;

    // Calculate alpha based on fade in/out
    if (logoTimer_ < logo.fadeInDuration) {
        currentLogoAlpha_ = logoTimer_ / logo.fadeInDuration;
    } else if (logoTimer_ > (logo.displayDuration - logo.fadeOutDuration)) {
        float fadeOutTime = logoTimer_ - (logo.displayDuration - logo.fadeOutDuration);
        currentLogoAlpha_ = 1.0f - (fadeOutTime / logo.fadeOutDuration);
    } else {
        currentLogoAlpha_ = 1.0f;
    }

    // Move to next logo
    if (logoTimer_ >= logo.displayDuration) {
        currentLogoIndex_++;
        logoTimer_ = 0.0f;
        currentLogoAlpha_ = 0.0f;
    }
}

void GameLauncher::UpdateFirstTimeSetup(float deltaTime) {
    if (firstTimeSetup_) {
        firstTimeSetup_->Update(deltaTime);
    }
}

void GameLauncher::UpdateMainMenu(float deltaTime) {
    if (mainMenu_) {
        mainMenu_->Update(deltaTime);
    }
}

void GameLauncher::UpdateCharacterSelection(float deltaTime) {
    if (characterSelect_) {
        characterSelect_->Update(deltaTime);
    }
}

void GameLauncher::UpdateIntroSequence(float deltaTime) {
    if (introSequence_) {
        introSequence_->Update(deltaTime);
    }
}

void GameLauncher::RenderSplashScreen(SDL_Renderer* renderer) {
    // Render splash screen with animated elements
    float pulse = 0.5f + 0.5f * std::sin(splashTimer_ * 2.0f);

    // Draw centered splash text
    SDL_Rect splashRect = {
        screenWidth_ / 2 - 300,
        screenHeight_ / 2 - 100,
        600,
        200
    };

    // Animated border with glow effect
    Uint8 glowAlpha = static_cast<Uint8>(pulse * 255);
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, glowAlpha);

    for (int i = 0; i < 5; i++) {
        SDL_Rect borderRect = {
            splashRect.x - i * 2,
            splashRect.y - i * 2,
            splashRect.w + i * 4,
            splashRect.h + i * 4
        };
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    // Main splash rectangle
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderFillRect(renderer, &splashRect);

    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &splashRect);

    // Progress indicator
    float progress = splashTimer_ / splashDuration_;
    SDL_Rect progressRect = {
        screenWidth_ / 2 - 200,
        screenHeight_ - 100,
        static_cast<int>(400 * progress),
        10
    };
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 200);
    SDL_RenderFillRect(renderer, &progressRect);
}

void GameLauncher::RenderStudioLogos(SDL_Renderer* renderer) {
    if (currentLogoIndex_ >= static_cast<int>(studioLogos_.size())) return;

    // Render current logo with fade effect
    SDL_Rect logoRect = {
        screenWidth_ / 2 - 256,
        screenHeight_ / 2 - 256,
        512,
        512
    };

    // Draw background gradient
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    // Draw logo placeholder (actual texture loading would happen in a real implementation)
    Uint8 alpha = static_cast<Uint8>(currentLogoAlpha_ * 255);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    SDL_RenderDrawRect(renderer, &logoRect);

    // Draw "Press any key to skip" text
    SDL_Rect skipTextRect = {
        screenWidth_ / 2 - 150,
        screenHeight_ - 50,
        300,
        30
    };
    Uint8 textAlpha = static_cast<Uint8>((0.5f + 0.5f * std::sin(logoTimer_ * 3.0f)) * 128);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, textAlpha);
    SDL_RenderDrawRect(renderer, &skipTextRect);
}

void GameLauncher::RenderParticles(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const auto& particle : particles_) {
        if (particle.lifetime <= 0.0f) continue;

        // Calculate alpha based on lifetime
        float lifeRatio = particle.lifetime / particle.maxLifetime;
        Uint8 alpha = static_cast<Uint8>(lifeRatio * particle.alpha * 255);

        SDL_SetRenderDrawColor(renderer,
                              particle.color.r,
                              particle.color.g,
                              particle.color.b,
                              alpha);

        // Draw particle as a small rectangle
        SDL_Rect particleRect = {
            static_cast<int>(particle.x - particle.size / 2),
            static_cast<int>(particle.y - particle.size / 2),
            static_cast<int>(particle.size),
            static_cast<int>(particle.size)
        };

        SDL_RenderFillRect(renderer, &particleRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void GameLauncher::RenderTransition(SDL_Renderer* renderer) {
    // Fade to black transition
    float alpha = std::sin(transitionProgress_ * M_PI);
    Uint8 overlayAlpha = static_cast<Uint8>(alpha * 200);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, overlayAlpha);

    SDL_Rect fullScreen = { 0, 0, screenWidth_, screenHeight_ };
    SDL_RenderFillRect(renderer, &fullScreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

bool GameLauncher::LoadConfiguration() {
    // Load launcher configuration from JSON
    std::string configPath = "assets/launcher/menu_config.json";

    if (!fs::exists(configPath)) {
        SDL_Log("Launcher config not found, using defaults");
        return false;
    }

    // In a real implementation, parse JSON here
    SDL_Log("Loaded launcher configuration from %s", configPath.c_str());
    return true;
}

bool GameLauncher::LoadStudioLogos() {
    // Configure studio logos
    StudioLogo logo1;
    logo1.texturePath = "assets/launcher/logo_studio1.png";
    logo1.displayDuration = 3.0f;
    logo1.fadeInDuration = 0.5f;
    logo1.fadeOutDuration = 0.5f;
    logo1.soundPath = "assets/audio/ui/logo_sound.wav";
    studioLogos_.push_back(logo1);

    StudioLogo logo2;
    logo2.texturePath = "assets/launcher/logo_publisher.png";
    logo2.displayDuration = 3.0f;
    logo2.fadeInDuration = 0.5f;
    logo2.fadeOutDuration = 0.5f;
    logo2.soundPath = "";
    studioLogos_.push_back(logo2);

    return !studioLogos_.empty();
}

void GameLauncher::DetectDLCs() {
    detectedDLCs_.clear();

    // Scan DLC directory
    std::string dlcPath = "data/dlc";

    if (!fs::exists(dlcPath)) {
        SDL_Log("No DLC directory found");
        return;
    }

    // Example DLC detection
    try {
        for (const auto& entry : fs::directory_iterator(dlcPath)) {
            if (entry.is_directory()) {
                DLCInfo dlc;
                dlc.id = entry.path().filename().string();
                dlc.name = dlc.id; // Would load from manifest
                dlc.version = "1.0.0";
                dlc.description = "Additional game content";
                dlc.iconPath = entry.path().string() + "/icon.png";
                dlc.installed = true;
                dlc.enabled = true;
                dlc.fileSize = 0; // Would calculate actual size
                dlc.releaseDate = "2025-01-01";

                detectedDLCs_.push_back(dlc);
                SDL_Log("Detected DLC: %s", dlc.name.c_str());
            }
        }
    } catch (const std::exception& e) {
        SDL_Log("Error scanning DLC directory: %s", e.what());
    }
}

void GameLauncher::CheckFirstLaunch() {
    std::string prefPath = "config/launcher_prefs.dat";
    isFirstLaunch_ = !fs::exists(prefPath);

    if (isFirstLaunch_) {
        SDL_Log("First launch detected");
    }
}

void GameLauncher::InitializeParticles() {
    particles_.reserve(200);
    particleSpawnTimer_ = 0.0f;
}

void GameLauncher::UpdateParticles(float deltaTime) {
    // Update existing particles
    for (auto& particle : particles_) {
        if (particle.lifetime <= 0.0f) continue;

        particle.x += particle.vx * deltaTime;
        particle.y += particle.vy * deltaTime;
        particle.lifetime -= deltaTime;

        // Apply gravity
        particle.vy += 50.0f * deltaTime;
    }

    // Remove dead particles
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const MenuParticle& p) { return p.lifetime <= 0.0f; }),
        particles_.end()
    );

    // Spawn new particles
    particleSpawnTimer_ += deltaTime;
    if (particleSpawnTimer_ >= 0.1f && particles_.size() < 200) {
        float x = static_cast<float>(rand() % screenWidth_);
        float y = -10.0f;
        SpawnParticle(x, y);
        particleSpawnTimer_ = 0.0f;
    }
}

void GameLauncher::SpawnParticle(float x, float y) {
    MenuParticle particle;
    particle.x = x;
    particle.y = y;
    particle.vx = (rand() % 40 - 20) * 0.5f;
    particle.vy = rand() % 50 + 20.0f;
    particle.lifetime = 3.0f + (rand() % 200) * 0.01f;
    particle.maxLifetime = particle.lifetime;
    particle.size = 2.0f + (rand() % 4);

    // Random golden/blue colors
    if (rand() % 2 == 0) {
        particle.color = { 255, 215, 0, 255 }; // Gold
    } else {
        particle.color = { 75, 144, 226, 255 }; // Blue
    }

    particle.alpha = 0.6f + (rand() % 40) * 0.01f;

    particles_.push_back(particle);
}

void GameLauncher::SkipCurrentScreen() {
    switch (currentState_) {
        case LauncherState::SplashScreen:
            splashTimer_ = splashDuration_;
            break;

        case LauncherState::StudioLogos:
            currentLogoIndex_ = static_cast<int>(studioLogos_.size());
            break;

        case LauncherState::IntroSequence:
            if (introSequence_) {
                introSequence_->Skip();
            }
            break;

        default:
            break;
    }
}

void GameLauncher::SaveLaunchPreferences() {
    std::string prefPath = "config/launcher_prefs.dat";

    // Create directory if needed
    fs::create_directories(fs::path(prefPath).parent_path());

    std::ofstream file(prefPath, std::ios::binary);
    if (file.is_open()) {
        // Write preferences (simplified)
        file << "launched=true\n";
        file << "skip_splash=" << (skipSplash_ ? "true" : "false") << "\n";
        file << "skip_logos=" << (skipLogos_ ? "true" : "false") << "\n";
        file << "show_intro=" << (showIntro_ ? "true" : "false") << "\n";
        file.close();
        SDL_Log("Saved launcher preferences");
    }
}

void GameLauncher::LoadLaunchPreferences() {
    std::string prefPath = "config/launcher_prefs.dat";

    if (!fs::exists(prefPath)) {
        return;
    }

    std::ifstream file(prefPath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("skip_splash=true") != std::string::npos) {
                skipSplash_ = true;
            } else if (line.find("skip_logos=true") != std::string::npos) {
                skipLogos_ = true;
            } else if (line.find("show_intro=false") != std::string::npos) {
                showIntro_ = false;
            }
        }
        file.close();
        SDL_Log("Loaded launcher preferences");
    }
}

} // namespace Launcher
