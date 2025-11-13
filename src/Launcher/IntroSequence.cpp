#include "IntroSequence.h"
#include <cmath>
#include <algorithm>

namespace Launcher {

IntroSequence::IntroSequence()
    : screenWidth_(1280),
      screenHeight_(720),
      renderer_(nullptr),
      currentScene_(0),
      totalTime_(0.0f),
      currentSubtitle_(-1),
      paused_(false),
      allowSkip_(true),
      skipPromptAlpha_(0.0f),
      skipHoldTime_(0.0f),
      skipHoldRequired_(2.0f),
      letterboxHeight_(80),
      fadeAlpha_(0.0f),
      textScrollOffset_(0.0f),
      showProgress_(true),
      showSkipPrompt_(true),
      letterboxEnabled_(true) {
}

IntroSequence::~IntroSequence() {
}

bool IntroSequence::Initialize(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    renderer_ = renderer;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    // Load intro scenes
    LoadIntroScenes();

    // Load subtitles
    LoadSubtitles();

    // Load audio assets
    LoadAudioAssets();

    SDL_Log("IntroSequence initialized with %zu scenes", scenes_.size());
    return true;
}

void IntroSequence::Update(float deltaTime) {
    if (paused_) return;

    totalTime_ += deltaTime;

    // Update skip prompt
    if (showSkipPrompt_) {
        skipPromptAlpha_ = 0.5f + 0.5f * std::sin(totalTime_ * 3.0f);
    }

    // Update current scene
    if (currentScene_ < static_cast<int>(scenes_.size())) {
        UpdateCurrentScene(deltaTime);
        UpdateSubtitles(deltaTime);
        UpdateMusic(deltaTime);

        // Check if scene is complete
        auto& scene = scenes_[currentScene_];
        if (scene.currentTime >= scene.duration) {
            TransitionToNextScene();
        }
    } else {
        // Sequence complete
        if (onComplete_) {
            onComplete_();
        }
    }
}

void IntroSequence::Render(SDL_Renderer* renderer) {
    // Clear to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render current scene
    if (currentScene_ < static_cast<int>(scenes_.size())) {
        RenderCurrentScene(renderer);
    }

    // Render letterbox
    if (letterboxEnabled_) {
        RenderLetterbox(renderer);
    }

    // Render subtitles
    RenderSubtitles(renderer);

    // Render progress indicator
    if (showProgress_) {
        RenderProgress(renderer);
    }

    // Render skip prompt
    if (showSkipPrompt_ && allowSkip_) {
        RenderSkipPrompt(renderer);
    }
}

void IntroSequence::HandleInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_SPACE:
                Skip();
                break;

            case SDLK_RETURN:
                // Skip current scene
                if (currentScene_ < static_cast<int>(scenes_.size())) {
                    auto& scene = scenes_[currentScene_];
                    scene.currentTime = scene.duration;
                }
                break;

            case SDLK_p:
                // Toggle pause
                if (paused_) {
                    Resume();
                } else {
                    Pause();
                }
                break;
        }
    } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START ||
            event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
            Skip();
        }
    }
}

void IntroSequence::Skip() {
    if (!allowSkip_) return;

    SDL_Log("Intro sequence skipped");
    currentScene_ = static_cast<int>(scenes_.size());

    if (onSkip_) {
        onSkip_();
    }
}

// Private methods

void IntroSequence::LoadIntroScenes() {
    scenes_.clear();

    // Scene 1: Fade in with title
    CinematicScene scene1;
    scene1.type = SceneType::FadeIn;
    scene1.backgroundPath = "assets/intro/background1.png";
    scene1.musicPath = "assets/audio/music/intro_theme.ogg";
    scene1.voicePath = "";
    scene1.textLines = {"In a world torn by darkness..."};
    scene1.duration = 3.0f;
    scene1.currentTime = 0.0f;
    scene1.textColor = {255, 255, 255, 255};
    scene1.skippable = true;
    scenes_.push_back(scene1);

    // Scene 2: Still image with narration
    CinematicScene scene2;
    scene2.type = SceneType::StillImage;
    scene2.backgroundPath = "assets/intro/background2.png";
    scene2.musicPath = "";
    scene2.voicePath = "assets/audio/voice/intro_narrator1.wav";
    scene2.textLines = {
        "The ancient kingdoms fell one by one,",
        "consumed by shadow and flame."
    };
    scene2.duration = 5.0f;
    scene2.currentTime = 0.0f;
    scene2.textColor = {255, 215, 0, 255};
    scene2.skippable = true;
    scenes_.push_back(scene2);

    // Scene 3: Text scroll (Star Wars style)
    CinematicScene scene3;
    scene3.type = SceneType::TextScroll;
    scene3.backgroundPath = "";
    scene3.musicPath = "";
    scene3.voicePath = "";
    scene3.textLines = {
        "In the age of legends,",
        "heroes rose from humble beginnings.",
        "",
        "Now, as the darkness returns,",
        "a new champion must emerge.",
        "",
        "Your journey begins..."
    };
    scene3.duration = 8.0f;
    scene3.currentTime = 0.0f;
    scene3.textColor = {255, 215, 0, 255};
    scene3.skippable = true;
    scenes_.push_back(scene3);

    // Scene 4: Dialogue scene
    CinematicScene scene4;
    scene4.type = SceneType::DialogueScene;
    scene4.backgroundPath = "assets/intro/tavern.png";
    scene4.musicPath = "";
    scene4.voicePath = "assets/audio/voice/intro_dialogue1.wav";
    scene4.textLines = {"The call to adventure awaits..."};
    scene4.duration = 4.0f;
    scene4.currentTime = 0.0f;
    scene4.textColor = {255, 255, 255, 255};
    scene4.skippable = true;
    scenes_.push_back(scene4);

    // Scene 5: Action sequence
    CinematicScene scene5;
    scene5.type = SceneType::ActionSequence;
    scene5.backgroundPath = "assets/intro/battle.png";
    scene5.musicPath = "assets/audio/music/battle_theme.ogg";
    scene5.voicePath = "";
    scene5.textLines = {};
    scene5.duration = 6.0f;
    scene5.currentTime = 0.0f;
    scene5.textColor = {255, 255, 255, 255};
    scene5.skippable = true;
    scenes_.push_back(scene5);

    // Scene 6: Fade out
    CinematicScene scene6;
    scene6.type = SceneType::FadeOut;
    scene6.backgroundPath = "";
    scene6.musicPath = "";
    scene6.voicePath = "";
    scene6.textLines = {"Your legend begins now..."};
    scene6.duration = 2.0f;
    scene6.currentTime = 0.0f;
    scene6.textColor = {255, 255, 255, 255};
    scene6.skippable = false;
    scenes_.push_back(scene6);
}

void IntroSequence::LoadSubtitles() {
    subtitles_.clear();

    // Subtitle for scene 1
    Subtitle sub1;
    sub1.text = "In a world torn by darkness...";
    sub1.startTime = 0.5f;
    sub1.endTime = 2.5f;
    sub1.color = {255, 255, 255, 255};
    sub1.visible = false;
    subtitles_.push_back(sub1);

    // Subtitle for scene 2
    Subtitle sub2;
    sub2.text = "The ancient kingdoms fell one by one";
    sub2.startTime = 3.5f;
    sub2.endTime = 6.0f;
    sub2.color = {255, 215, 0, 255};
    sub2.visible = false;
    subtitles_.push_back(sub2);

    // More subtitles would be added here
}

void IntroSequence::LoadAudioAssets() {
    // Load music and voice assets
    SDL_Log("Loading intro audio assets...");
}

void IntroSequence::RenderCurrentScene(SDL_Renderer* renderer) {
    if (currentScene_ >= static_cast<int>(scenes_.size())) return;

    const auto& scene = scenes_[currentScene_];

    switch (scene.type) {
        case SceneType::FadeIn:
            RenderFadeInScene(renderer, scene);
            break;

        case SceneType::StillImage:
            RenderStillImageScene(renderer, scene);
            break;

        case SceneType::TextScroll:
            RenderTextScrollScene(renderer, scene);
            break;

        case SceneType::DialogueScene:
            RenderDialogueScene(renderer, scene);
            break;

        case SceneType::ActionSequence:
            RenderActionScene(renderer, scene);
            break;

        case SceneType::FadeOut:
            RenderFadeOutScene(renderer, scene);
            break;

        default:
            break;
    }
}

void IntroSequence::RenderFadeInScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    // Calculate fade alpha
    float progress = scene.currentTime / scene.duration;
    Uint8 alpha = static_cast<Uint8>(progress * 255);

    // Render background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, alpha);

    SDL_Rect bg = {0, letterboxHeight_, screenWidth_, screenHeight_ - 2 * letterboxHeight_};
    SDL_RenderFillRect(renderer, &bg);

    // Render text
    if (!scene.textLines.empty() && progress > 0.3f) {
        SDL_Rect textRect = {
            screenWidth_ / 2 - 400,
            screenHeight_ / 2 - 50,
            800,
            100
        };

        Uint8 textAlpha = static_cast<Uint8>((progress - 0.3f) * 255 / 0.7f);
        SDL_SetRenderDrawColor(renderer,
            scene.textColor.r,
            scene.textColor.g,
            scene.textColor.b,
            textAlpha);

        SDL_RenderDrawRect(renderer, &textRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::RenderStillImageScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    // Render background image (placeholder)
    SDL_Rect imageRect = {
        screenWidth_ / 4,
        letterboxHeight_ + 50,
        screenWidth_ / 2,
        screenHeight_ - 2 * letterboxHeight_ - 100
    };

    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
    SDL_RenderFillRect(renderer, &imageRect);

    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &imageRect);

    // Render text lines at bottom
    int textY = screenHeight_ - letterboxHeight_ - 100;
    for (const auto& line : scene.textLines) {
        SDL_Rect textRect = {
            screenWidth_ / 2 - 300,
            textY,
            600,
            30
        };

        SDL_SetRenderDrawColor(renderer,
            scene.textColor.r,
            scene.textColor.g,
            scene.textColor.b,
            255);
        SDL_RenderDrawRect(renderer, &textRect);

        textY += 35;
    }
}

void IntroSequence::RenderTextScrollScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    // Star Wars style text scroll
    float progress = scene.currentTime / scene.duration;
    float scrollY = screenHeight_ - progress * (screenHeight_ + 500);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int lineY = static_cast<int>(scrollY);
    for (const auto& line : scene.textLines) {
        if (lineY > letterboxHeight_ && lineY < screenHeight_ - letterboxHeight_) {
            // Calculate perspective scale
            float centerDist = std::abs(lineY - screenHeight_ / 2);
            float scale = 1.0f - (centerDist / screenHeight_) * 0.5f;
            scale = std::max(0.3f, scale);

            int textWidth = static_cast<int>(600 * scale);
            SDL_Rect textRect = {
                screenWidth_ / 2 - textWidth / 2,
                lineY,
                textWidth,
                static_cast<int>(40 * scale)
            };

            // Fade based on distance from center
            Uint8 alpha = static_cast<Uint8>(255 * scale);

            SDL_SetRenderDrawColor(renderer,
                scene.textColor.r,
                scene.textColor.g,
                scene.textColor.b,
                alpha);
            SDL_RenderDrawRect(renderer, &textRect);
        }

        lineY += 80;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::RenderDialogueScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    // Render background
    SDL_Rect bgRect = {
        0,
        letterboxHeight_,
        screenWidth_,
        screenHeight_ - 2 * letterboxHeight_
    };

    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // Render dialogue box at bottom
    SDL_Rect dialogueBox = {
        50,
        screenHeight_ - letterboxHeight_ - 150,
        screenWidth_ - 100,
        120
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 230);
    SDL_RenderFillRect(renderer, &dialogueBox);

    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawRect(renderer, &dialogueBox);
        dialogueBox.x++; dialogueBox.y++;
        dialogueBox.w -= 2; dialogueBox.h -= 2;
    }

    // Render text
    if (!scene.textLines.empty()) {
        SDL_Rect textRect = {
            70,
            screenHeight_ - letterboxHeight_ - 120,
            screenWidth_ - 140,
            80
        };

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &textRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::RenderActionScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    // Render action sequence with camera shake
    float shake = std::sin(scene.currentTime * 10.0f) * 5.0f;

    SDL_Rect actionRect = {
        static_cast<int>(screenWidth_ / 4 + shake),
        letterboxHeight_ + static_cast<int>(shake),
        screenWidth_ / 2,
        screenHeight_ - 2 * letterboxHeight_
    };

    SDL_SetRenderDrawColor(renderer, 60, 40, 40, 255);
    SDL_RenderFillRect(renderer, &actionRect);

    // Flash effects
    if (static_cast<int>(scene.currentTime * 5) % 2 == 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 100);
        SDL_RenderFillRect(renderer, &actionRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &actionRect);
}

void IntroSequence::RenderFadeOutScene(SDL_Renderer* renderer, const CinematicScene& scene) {
    float progress = scene.currentTime / scene.duration;
    Uint8 alpha = static_cast<Uint8>((1.0f - progress) * 255);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Render final text
    if (!scene.textLines.empty()) {
        SDL_Rect textRect = {
            screenWidth_ / 2 - 300,
            screenHeight_ / 2 - 25,
            600,
            50
        };

        SDL_SetRenderDrawColor(renderer,
            scene.textColor.r,
            scene.textColor.g,
            scene.textColor.b,
            alpha);
        SDL_RenderDrawRect(renderer, &textRect);
    }

    // Fade to black overlay
    Uint8 fadeAlpha = static_cast<Uint8>(progress * 255);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, fadeAlpha);
    SDL_Rect fullScreen = {0, 0, screenWidth_, screenHeight_};
    SDL_RenderFillRect(renderer, &fullScreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::RenderLetterbox(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Top bar
    SDL_Rect topBar = {0, 0, screenWidth_, letterboxHeight_};
    SDL_RenderFillRect(renderer, &topBar);

    // Bottom bar
    SDL_Rect bottomBar = {0, screenHeight_ - letterboxHeight_, screenWidth_, letterboxHeight_};
    SDL_RenderFillRect(renderer, &bottomBar);
}

void IntroSequence::RenderSubtitles(SDL_Renderer* renderer) {
    Subtitle* activeSub = GetActiveSubtitle();
    if (!activeSub || !activeSub->visible) return;

    SDL_Rect subRect = {
        screenWidth_ / 2 - 400,
        screenHeight_ - letterboxHeight_ - 80,
        800,
        60
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &subRect);

    SDL_SetRenderDrawColor(renderer,
        activeSub->color.r,
        activeSub->color.g,
        activeSub->color.b,
        255);
    SDL_RenderDrawRect(renderer, &subRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::RenderProgress(SDL_Renderer* renderer) {
    if (scenes_.empty()) return;

    // Progress bar
    int barWidth = 300;
    int barHeight = 5;
    int barX = screenWidth_ / 2 - barWidth / 2;
    int barY = screenHeight_ - letterboxHeight_ + 10;

    SDL_Rect bgBar = {barX, barY, barWidth, barHeight};
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 200);
    SDL_RenderFillRect(renderer, &bgBar);

    // Progress fill
    float progress = static_cast<float>(currentScene_) / scenes_.size();
    int fillWidth = static_cast<int>(barWidth * progress);

    SDL_Rect fillBar = {barX, barY, fillWidth, barHeight};
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderFillRect(renderer, &fillBar);

    // Scene markers
    for (size_t i = 0; i < scenes_.size(); ++i) {
        int markerX = barX + static_cast<int>((barWidth * i) / scenes_.size());
        SDL_Rect marker = {markerX - 1, barY - 2, 2, barHeight + 4};
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &marker);
    }
}

void IntroSequence::RenderSkipPrompt(SDL_Renderer* renderer) {
    SDL_Rect promptRect = {
        screenWidth_ - 250,
        letterboxHeight_ + 20,
        230,
        40
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    Uint8 alpha = static_cast<Uint8>(skipPromptAlpha_ * 180);

    SDL_SetRenderDrawColor(renderer, 40, 40, 50, alpha);
    SDL_RenderFillRect(renderer, &promptRect);

    SDL_SetRenderDrawColor(renderer, 255, 215, 0, alpha);
    SDL_RenderDrawRect(renderer, &promptRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void IntroSequence::UpdateCurrentScene(float deltaTime) {
    if (currentScene_ >= static_cast<int>(scenes_.size())) return;

    auto& scene = scenes_[currentScene_];
    scene.currentTime += deltaTime;
}

void IntroSequence::UpdateSubtitles(float deltaTime) {
    for (auto& subtitle : subtitles_) {
        subtitle.visible = (totalTime_ >= subtitle.startTime &&
                          totalTime_ <= subtitle.endTime);
    }
}

void IntroSequence::UpdateMusic(float deltaTime) {
    // Update music playback
    // In a real implementation, this would handle audio crossfading
}

void IntroSequence::TransitionToNextScene() {
    currentScene_++;

    if (currentScene_ < static_cast<int>(scenes_.size())) {
        SDL_Log("Transitioning to scene %d", currentScene_);
        PlaySceneMusic(scenes_[currentScene_]);
    } else {
        SDL_Log("Intro sequence complete");
        StopMusic();
    }
}

void IntroSequence::PlaySceneMusic(const CinematicScene& scene) {
    if (!scene.musicPath.empty()) {
        SDL_Log("Playing music: %s", scene.musicPath.c_str());
        // In a real implementation, load and play the music
    }
}

void IntroSequence::StopMusic() {
    SDL_Log("Stopping intro music");
    // In a real implementation, stop all audio
}

void IntroSequence::PlayVoiceLine(const std::string& voicePath) {
    if (!voicePath.empty()) {
        SDL_Log("Playing voice: %s", voicePath.c_str());
        // In a real implementation, play the voice line
    }
}

float IntroSequence::GetSceneProgress() const {
    if (currentScene_ >= static_cast<int>(scenes_.size())) return 1.0f;

    const auto& scene = scenes_[currentScene_];
    return scene.currentTime / scene.duration;
}

Subtitle* IntroSequence::GetActiveSubtitle() {
    for (auto& subtitle : subtitles_) {
        if (subtitle.visible) {
            return &subtitle;
        }
    }
    return nullptr;
}

} // namespace Launcher
