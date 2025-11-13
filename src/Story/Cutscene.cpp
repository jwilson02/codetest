#include "Cutscene.h"
#include <algorithm>
#include <sstream>
#include <ctime>

namespace Story {

// CutscenePlayer::Implementation
struct CutscenePlayer::Implementation {
    std::unordered_map<std::string, Cutscene> cutscenes;

    // Playback state
    CutsceneState state = CutsceneState::STOPPED;
    std::string currentCutsceneId;
    float currentTime = 0.0f;
    std::vector<CutsceneActor> activeActors;

    // Active commands
    std::vector<const CutsceneCommand*> activeCommands;

    // Choice state
    bool waitingForChoice = false;
    std::vector<std::string> currentChoices;
    int selectedChoice = -1;

    // Callbacks
    CutsceneCallbacks callbacks;
    CameraControlFunction cameraControl;
    ActorSpawnFunction actorSpawn;

    // Settings
    bool canSkip = true;
    bool canPause = false;
    bool autoPlayVoice = true;
    bool showSubtitles = true;
    bool debugMode = false;
};

CutscenePlayer::CutscenePlayer() : impl(std::make_unique<Implementation>()) {}

CutscenePlayer::~CutscenePlayer() = default;

bool CutscenePlayer::initialize() {
    impl->cutscenes.clear();
    impl->state = CutsceneState::STOPPED;
    impl->currentTime = 0.0f;
    return true;
}

void CutscenePlayer::shutdown() {
    stop();
    impl->cutscenes.clear();
}

bool CutscenePlayer::loadCutscene(const std::string& filePath) {
    // Would load from JSON file
    return true;
}

bool CutscenePlayer::loadCutsceneFromJson(const std::string& jsonData) {
    // Would parse JSON
    return true;
}

void CutscenePlayer::registerCutscene(const Cutscene& cutscene) {
    impl->cutscenes[cutscene.id] = cutscene;
}

void CutscenePlayer::unloadCutscene(const std::string& cutsceneId) {
    impl->cutscenes.erase(cutsceneId);
}

bool CutscenePlayer::play(const std::string& cutsceneId) {
    auto it = impl->cutscenes.find(cutsceneId);
    if (it == impl->cutscenes.end()) return false;

    if (impl->state != CutsceneState::STOPPED) {
        stop();
    }

    impl->currentCutsceneId = cutsceneId;
    impl->currentTime = 0.0f;
    impl->state = CutsceneState::PLAYING;
    impl->waitingForChoice = false;
    impl->selectedChoice = -1;

    const Cutscene& cutscene = it->second;

    // Initialize actors
    impl->activeActors = cutscene.actors;
    for (const auto& actor : impl->activeActors) {
        if (impl->actorSpawn) {
            impl->actorSpawn(actor);
        }
    }

    // Trigger callback
    if (impl->callbacks.onCutsceneStart) {
        impl->callbacks.onCutsceneStart(cutsceneId);
    }

    return true;
}

void CutscenePlayer::pause() {
    if (impl->state == CutsceneState::PLAYING && impl->canPause) {
        impl->state = CutsceneState::PAUSED;
        if (impl->callbacks.onPause) {
            impl->callbacks.onPause();
        }
    }
}

void CutscenePlayer::resume() {
    if (impl->state == CutsceneState::PAUSED) {
        impl->state = CutsceneState::PLAYING;
        if (impl->callbacks.onResume) {
            impl->callbacks.onResume();
        }
    }
}

void CutscenePlayer::stop() {
    if (impl->state == CutsceneState::STOPPED) return;

    std::string cutsceneId = impl->currentCutsceneId;

    impl->state = CutsceneState::STOPPED;
    impl->currentTime = 0.0f;
    impl->activeActors.clear();
    impl->activeCommands.clear();
    impl->waitingForChoice = false;
    impl->currentChoices.clear();

    if (impl->callbacks.onCutsceneEnd) {
        impl->callbacks.onCutsceneEnd(cutsceneId);
    }

    impl->currentCutsceneId.clear();
}

void CutscenePlayer::skip() {
    if (impl->state == CutsceneState::PLAYING && impl->canSkip) {
        impl->state = CutsceneState::SKIPPING;

        if (impl->callbacks.onSkip) {
            impl->callbacks.onSkip();
        }

        stop();
    }
}

bool CutscenePlayer::isPlaying() const {
    return impl->state == CutsceneState::PLAYING ||
           impl->state == CutsceneState::PAUSED ||
           impl->state == CutsceneState::SKIPPING;
}

bool CutscenePlayer::isPaused() const {
    return impl->state == CutsceneState::PAUSED;
}

void CutscenePlayer::update(float deltaTime) {
    if (impl->state != CutsceneState::PLAYING) return;
    if (impl->waitingForChoice) return;

    impl->currentTime += deltaTime;

    const Cutscene* cutscene = getCurrentCutscene();
    if (!cutscene) {
        stop();
        return;
    }

    // Execute commands at current time
    auto commands = getCommandsAtTime(impl->currentTime);
    for (const auto& cmd : commands) {
        executeCommand(cmd);
    }

    // Update actors
    updateActors(deltaTime);

    // Update camera
    updateCamera(deltaTime);

    // Check if cutscene is complete
    if (impl->currentTime >= cutscene->totalDuration) {
        stop();
    }
}

CutsceneState CutscenePlayer::getState() const {
    return impl->state;
}

float CutscenePlayer::getCurrentTime() const {
    return impl->currentTime;
}

float CutscenePlayer::getTotalDuration() const {
    const Cutscene* cutscene = getCurrentCutscene();
    return cutscene ? cutscene->totalDuration : 0.0f;
}

float CutscenePlayer::getProgress() const {
    float duration = getTotalDuration();
    return duration > 0.0f ? impl->currentTime / duration : 0.0f;
}

const Cutscene* CutscenePlayer::getCurrentCutscene() const {
    if (impl->currentCutsceneId.empty()) return nullptr;

    auto it = impl->cutscenes.find(impl->currentCutsceneId);
    return it != impl->cutscenes.end() ? &it->second : nullptr;
}

CutsceneActor* CutscenePlayer::getActor(const std::string& actorId) {
    for (auto& actor : impl->activeActors) {
        if (actor.id == actorId) {
            return &actor;
        }
    }
    return nullptr;
}

const CutsceneActor* CutscenePlayer::getActor(const std::string& actorId) const {
    for (const auto& actor : impl->activeActors) {
        if (actor.id == actorId) {
            return &actor;
        }
    }
    return nullptr;
}

void CutscenePlayer::setCallbacks(const CutsceneCallbacks& callbacks) {
    impl->callbacks = callbacks;
}

void CutscenePlayer::selectChoice(int choiceIndex) {
    if (!impl->waitingForChoice) return;
    if (choiceIndex < 0 || choiceIndex >= static_cast<int>(impl->currentChoices.size())) return;

    impl->selectedChoice = choiceIndex;
    impl->waitingForChoice = false;

    // Resume playback
    // Would handle branching based on choice
}

std::vector<std::string> CutscenePlayer::getCurrentChoices() const {
    return impl->currentChoices;
}

bool CutscenePlayer::isWaitingForChoice() const {
    return impl->waitingForChoice;
}

void CutscenePlayer::setCameraControl(CameraControlFunction cameraFunc) {
    impl->cameraControl = cameraFunc;
}

void CutscenePlayer::setActorSpawnFunction(ActorSpawnFunction spawnFunc) {
    impl->actorSpawn = spawnFunc;
}

void CutscenePlayer::setSkippable(bool skippable) {
    impl->canSkip = skippable;
}

void CutscenePlayer::setPausable(bool pausable) {
    impl->canPause = pausable;
}

void CutscenePlayer::setAutoPlayVoice(bool autoPlay) {
    impl->autoPlayVoice = autoPlay;
}

void CutscenePlayer::setShowSubtitles(bool show) {
    impl->showSubtitles = show;
}

std::string CutscenePlayer::serializeState() const {
    // Would serialize to JSON
    return "{}";
}

bool CutscenePlayer::deserializeState(const std::string& data) {
    // Would deserialize from JSON
    return true;
}

void CutscenePlayer::setDebugMode(bool enable) {
    impl->debugMode = enable;
}

void CutscenePlayer::jumpToTime(float time) {
    if (impl->debugMode) {
        impl->currentTime = time;
    }
}

std::vector<std::string> CutscenePlayer::getCommandTimeline() const {
    std::vector<std::string> timeline;
    const Cutscene* cutscene = getCurrentCutscene();
    if (!cutscene) return timeline;

    for (const auto& cmd : cutscene->commands) {
        std::stringstream ss;
        ss << cmd.timestamp << "s: " << static_cast<int>(cmd.type);
        timeline.push_back(ss.str());
    }

    return timeline;
}

void CutscenePlayer::executeCommand(const CutsceneCommand& cmd) {
    if (impl->callbacks.onCommandExecute) {
        impl->callbacks.onCommandExecute(cmd);
    }

    switch (cmd.type) {
        case CutsceneCommandType::DIALOGUE:
            // Trigger dialogue display
            break;

        case CutsceneCommandType::CAMERA_MOVE:
            if (impl->cameraControl) {
                impl->cameraControl(cmd.cameraCommand);
            }
            break;

        case CutsceneCommandType::SPAWN_ACTOR: {
            CutsceneActor* actor = getActor(cmd.actorId);
            if (actor) {
                actor->visible = true;
                if (impl->actorSpawn) {
                    impl->actorSpawn(*actor);
                }
            }
            break;
        }

        case CutsceneCommandType::REMOVE_ACTOR: {
            CutsceneActor* actor = getActor(cmd.actorId);
            if (actor) {
                actor->visible = false;
            }
            break;
        }

        case CutsceneCommandType::MOVE_ACTOR: {
            CutsceneActor* actor = getActor(cmd.actorId);
            if (actor) {
                // Would interpolate to target position
                actor->x = cmd.targetX;
                actor->y = cmd.targetY;
                actor->z = cmd.targetZ;
            }
            break;
        }

        case CutsceneCommandType::PLAY_ANIMATION:
            // Trigger animation
            break;

        case CutsceneCommandType::PLAY_SOUND:
            // Play sound effect
            break;

        case CutsceneCommandType::PLAY_MUSIC:
            // Play music
            break;

        case CutsceneCommandType::FADE_IN:
        case CutsceneCommandType::FADE_OUT:
            // Trigger fade
            break;

        case CutsceneCommandType::CHOICE:
            impl->waitingForChoice = true;
            impl->currentChoices = cmd.choices;
            if (impl->callbacks.onChoice) {
                impl->callbacks.onChoice(cmd.actorId, cmd.choices);
            }
            break;

        case CutsceneCommandType::END:
            stop();
            break;

        default:
            break;
    }
}

void CutscenePlayer::updateActors(float deltaTime) {
    // Update actor states, animations, etc.
}

void CutscenePlayer::updateCamera(float deltaTime) {
    // Update camera position, interpolation, etc.
}

std::vector<CutsceneCommand> CutscenePlayer::getCommandsAtTime(float time) const {
    std::vector<CutsceneCommand> commands;
    const Cutscene* cutscene = getCurrentCutscene();
    if (!cutscene) return commands;

    // Find commands that should execute at this time
    for (const auto& cmd : cutscene->commands) {
        if (cmd.timestamp <= time && cmd.timestamp > (time - 0.1f)) {
            commands.push_back(cmd);
        }
    }

    return commands;
}

// CutsceneBuilder implementation
CutsceneBuilder::CutsceneBuilder(const std::string& id) : currentTime_(0.0f) {
    cutscene_.id = id;
}

CutsceneBuilder& CutsceneBuilder::addActor(const std::string& actorId,
                                          const std::string& modelId,
                                          float x, float y, float z) {
    CutsceneActor actor;
    actor.id = actorId;
    actor.modelId = modelId;
    actor.x = x;
    actor.y = y;
    actor.z = z;
    cutscene_.actors.push_back(actor);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::removeActor(const std::string& actorId, float atTime) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::REMOVE_ACTOR;
    cmd.timestamp = atTime;
    cmd.actorId = actorId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::wait(float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::WAIT;
    cmd.timestamp = currentTime_;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::dialogue(const std::string& actorId,
                                          const std::string& text,
                                          float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::DIALOGUE;
    cmd.timestamp = currentTime_;
    cmd.actorId = actorId;
    cmd.text = text;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::moveCamera(float x, float y, float z, float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::CAMERA_MOVE;
    cmd.timestamp = currentTime_;
    cmd.cameraCommand.x = x;
    cmd.cameraCommand.y = y;
    cmd.cameraCommand.z = z;
    cmd.cameraCommand.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::focusOn(const std::string& actorId, float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::CAMERA_MOVE;
    cmd.timestamp = currentTime_;
    cmd.cameraCommand.targetId = actorId;
    cmd.cameraCommand.followTarget = true;
    cmd.cameraCommand.duration = duration;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::moveActor(const std::string& actorId,
                                           float x, float y, float z,
                                           float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::MOVE_ACTOR;
    cmd.timestamp = currentTime_;
    cmd.actorId = actorId;
    cmd.targetX = x;
    cmd.targetY = y;
    cmd.targetZ = z;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::playAnimation(const std::string& actorId,
                                               const std::string& animId) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::PLAY_ANIMATION;
    cmd.timestamp = currentTime_;
    cmd.actorId = actorId;
    cmd.animationId = animId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::fadeOut(float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::FADE_OUT;
    cmd.timestamp = currentTime_;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::fadeIn(float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::FADE_IN;
    cmd.timestamp = currentTime_;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::playMusic(const std::string& musicId) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::PLAY_MUSIC;
    cmd.timestamp = currentTime_;
    cmd.musicId = musicId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::playSound(const std::string& soundId) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::PLAY_SOUND;
    cmd.timestamp = currentTime_;
    cmd.soundId = soundId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::showText(const std::string& text, float duration) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::SHOW_TEXT;
    cmd.timestamp = currentTime_;
    cmd.text = text;
    cmd.duration = duration;
    cutscene_.commands.push_back(cmd);
    currentTime_ += duration;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::setFlag(const std::string& flagId) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::SET_FLAG;
    cmd.timestamp = currentTime_;
    cmd.flagId = flagId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::branch(const std::string& condition,
                                        const std::string& branchId) {
    CutsceneCommand cmd;
    cmd.type = CutsceneCommandType::BRANCH;
    cmd.timestamp = currentTime_;
    cmd.condition = condition;
    cmd.branchCommandId = branchId;
    cutscene_.commands.push_back(cmd);
    return *this;
}

CutsceneBuilder& CutsceneBuilder::setSkippable(bool skippable) {
    cutscene_.skippable = skippable;
    return *this;
}

CutsceneBuilder& CutsceneBuilder::setCinematicBars(const std::string& style) {
    cutscene_.cinematicBars = style;
    return *this;
}

Cutscene CutsceneBuilder::build() {
    cutscene_.totalDuration = currentTime_;
    return cutscene_;
}

std::string CutsceneBuilder::toJson() const {
    // Would generate JSON
    return "{}";
}

} // namespace Story
