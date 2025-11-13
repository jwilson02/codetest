#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Story {

// Cutscene command types
enum class CutsceneCommandType {
    DIALOGUE,           // Show dialogue
    CAMERA_MOVE,        // Move camera
    CAMERA_SHAKE,       // Shake camera
    FADE_IN,           // Fade in from black
    FADE_OUT,          // Fade out to black
    PLAY_ANIMATION,    // Play character animation
    SPAWN_ACTOR,       // Spawn character/object
    REMOVE_ACTOR,      // Remove character/object
    MOVE_ACTOR,        // Move character/object
    PLAY_SOUND,        // Play sound effect
    PLAY_MUSIC,        // Play music
    STOP_MUSIC,        // Stop music
    SHOW_TEXT,         // Show text overlay
    HIDE_TEXT,         // Hide text overlay
    WAIT,              // Wait for duration
    SET_FLAG,          // Set story flag
    TRIGGER_EVENT,     // Trigger game event
    CHOICE,            // Player choice in cutscene
    BRANCH,            // Branch based on condition
    END                // End cutscene
};

// Easing functions for smooth transitions
enum class EasingType {
    LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,
    BOUNCE,
    ELASTIC
};

// Camera command
struct CameraCommand {
    std::string targetId;       // Actor or position to focus on
    float x, y, z;              // Target position
    float duration;             // Transition duration
    float zoom;                 // Zoom level
    EasingType easing;
    bool followTarget;          // Continue following target

    CameraCommand() : x(0), y(0), z(0), duration(0), zoom(1.0f),
                     easing(EasingType::LINEAR), followTarget(false) {}
};

// Actor (character/object in cutscene)
struct CutsceneActor {
    std::string id;
    std::string modelId;
    std::string name;
    float x, y, z;              // Position
    float rotation;             // Y-axis rotation
    float scale;
    bool visible;

    CutsceneActor() : x(0), y(0), z(0), rotation(0), scale(1.0f), visible(true) {}
};

// Cutscene command
struct CutsceneCommand {
    CutsceneCommandType type;
    float timestamp;            // When to execute (in seconds from start)
    float duration;             // How long it lasts

    // Command-specific data
    std::string actorId;
    std::string targetId;
    std::string text;
    std::string animationId;
    std::string soundId;
    std::string musicId;
    std::string dialogueId;
    std::string flagId;
    std::string eventId;

    // Movement
    float targetX, targetY, targetZ;
    float moveSpeed;
    EasingType easing;

    // Camera
    CameraCommand cameraCommand;

    // Visual effects
    float fadeAlpha;            // For fades (0 = transparent, 1 = opaque)
    std::string color;          // Hex color

    // Conditions (for branching)
    std::string condition;
    std::string branchCommandId;

    // Choices
    std::vector<std::string> choices;
    std::vector<std::string> choiceBranches;

    CutsceneCommand() : type(CutsceneCommandType::WAIT), timestamp(0), duration(0),
                       targetX(0), targetY(0), targetZ(0), moveSpeed(1.0f),
                       easing(EasingType::LINEAR), fadeAlpha(0) {}
};

// Cutscene definition
struct Cutscene {
    std::string id;
    std::string title;
    std::string description;

    // Actors
    std::vector<CutsceneActor> actors;

    // Commands (timeline)
    std::vector<CutsceneCommand> commands;

    // Settings
    bool skippable;
    bool pausable;
    float totalDuration;        // Calculated from commands
    std::string cinematicBars;   // "letterbox", "pillarbox", "none"

    // Audio
    std::string backgroundMusicId;
    float musicVolume;

    // Triggers
    std::string onStartScript;
    std::string onEndScript;
    std::vector<std::string> flagsToSet;

    Cutscene() : skippable(true), pausable(false), totalDuration(0),
                cinematicBars("letterbox"), musicVolume(0.7f) {}
};

// Cutscene player state
enum class CutsceneState {
    STOPPED,
    PLAYING,
    PAUSED,
    SKIPPING,
    COMPLETED
};

// Main cutscene system
class CutscenePlayer {
public:
    CutscenePlayer();
    ~CutscenePlayer();

    // Initialization
    bool initialize();
    void shutdown();

    // Cutscene loading
    bool loadCutscene(const std::string& filePath);
    bool loadCutsceneFromJson(const std::string& jsonData);
    void registerCutscene(const Cutscene& cutscene);
    void unloadCutscene(const std::string& cutsceneId);

    // Playback control
    bool play(const std::string& cutsceneId);
    void pause();
    void resume();
    void stop();
    void skip();
    bool isPlaying() const;
    bool isPaused() const;

    // Update (call every frame)
    void update(float deltaTime);

    // State queries
    CutsceneState getState() const;
    float getCurrentTime() const;
    float getTotalDuration() const;
    float getProgress() const;          // 0.0 to 1.0
    const Cutscene* getCurrentCutscene() const;

    // Actor management
    CutsceneActor* getActor(const std::string& actorId);
    const CutsceneActor* getActor(const std::string& actorId) const;

    // Callbacks
    struct CutsceneCallbacks {
        std::function<void(const std::string&)> onCutsceneStart;
        std::function<void(const std::string&)> onCutsceneEnd;
        std::function<void(const CutsceneCommand&)> onCommandExecute;
        std::function<void(const std::string&, const std::vector<std::string>&)> onChoice;
        std::function<void()> onPause;
        std::function<void()> onResume;
        std::function<void()> onSkip;
    };
    void setCallbacks(const CutsceneCallbacks& callbacks);

    // Choice handling
    void selectChoice(int choiceIndex);
    std::vector<std::string> getCurrentChoices() const;
    bool isWaitingForChoice() const;

    // Camera control integration
    using CameraControlFunction = std::function<void(const CameraCommand&)>;
    void setCameraControl(CameraControlFunction cameraFunc);

    // Actor spawning integration
    using ActorSpawnFunction = std::function<void(const CutsceneActor&)>;
    void setActorSpawnFunction(ActorSpawnFunction spawnFunc);

    // Settings
    void setSkippable(bool skippable);
    void setPausable(bool pausable);
    void setAutoPlayVoice(bool autoPlay);
    void setShowSubtitles(bool show);

    // Save/load (for mid-cutscene saves)
    std::string serializeState() const;
    bool deserializeState(const std::string& data);

    // Debug
    void setDebugMode(bool enable);
    void jumpToTime(float time);
    std::vector<std::string> getCommandTimeline() const;

private:
    struct Implementation;
    std::unique_ptr<Implementation> impl;

    // Internal command execution
    void executeCommand(const CutsceneCommand& cmd);
    void updateActors(float deltaTime);
    void updateCamera(float deltaTime);
    std::vector<CutsceneCommand> getCommandsAtTime(float time) const;
};

// Cutscene builder helper
class CutsceneBuilder {
public:
    CutsceneBuilder(const std::string& id);

    // Actor management
    CutsceneBuilder& addActor(const std::string& actorId, const std::string& modelId,
                             float x, float y, float z);
    CutsceneBuilder& removeActor(const std::string& actorId, float atTime);

    // Commands
    CutsceneBuilder& wait(float duration);
    CutsceneBuilder& dialogue(const std::string& actorId, const std::string& text, float duration);
    CutsceneBuilder& moveCamera(float x, float y, float z, float duration);
    CutsceneBuilder& focusOn(const std::string& actorId, float duration);
    CutsceneBuilder& moveActor(const std::string& actorId, float x, float y, float z, float duration);
    CutsceneBuilder& playAnimation(const std::string& actorId, const std::string& animId);
    CutsceneBuilder& fadeOut(float duration);
    CutsceneBuilder& fadeIn(float duration);
    CutsceneBuilder& playMusic(const std::string& musicId);
    CutsceneBuilder& playSound(const std::string& soundId);
    CutsceneBuilder& showText(const std::string& text, float duration);
    CutsceneBuilder& setFlag(const std::string& flagId);
    CutsceneBuilder& branch(const std::string& condition, const std::string& branchId);

    // Settings
    CutsceneBuilder& setSkippable(bool skippable);
    CutsceneBuilder& setCinematicBars(const std::string& style);

    // Build
    Cutscene build();
    std::string toJson() const;

private:
    Cutscene cutscene_;
    float currentTime_;
};

} // namespace Story
