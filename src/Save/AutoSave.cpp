#include "AutoSave.h"
#include <algorithm>

namespace Save {

AutoSave::AutoSave()
    : m_Initialized(false)
    , m_TimeSinceLastSave(0.0f)
    , m_IsAutosaving(false)
    , m_CurrentAutoSaveSlot(0)
    , m_InCombat(false)
    , m_InCutscene(false)
    , m_InDialogue(false)
    , m_TotalAutoSaves(0)
    , m_FailedAutoSaves(0)
    , m_TotalSaveTime(0.0f)
{
}

AutoSave::~AutoSave() {
    shutdown();
}

bool AutoSave::initialize(const Config& config) {
    if (m_Initialized) {
        return true;
    }

    m_Config = config;
    m_TimeSinceLastSave = 0.0f;
    m_LastSaveTime = std::chrono::system_clock::now();
    m_CurrentAutoSaveSlot = 0;

    m_Initialized = true;
    return true;
}

void AutoSave::shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Wait for any pending autosave
    while (m_IsAutosaving) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_Initialized = false;
}

void AutoSave::update(float deltaTime) {
    if (!m_Initialized || !m_Config.enabled || m_IsAutosaving) {
        return;
    }

    m_TimeSinceLastSave += deltaTime;

    // Check if it's time for a time-based autosave
    if (m_TimeSinceLastSave >= m_Config.intervalSeconds) {
        triggerAutoSave(TriggerType::TIME_BASED, "Interval");
    }
}

void AutoSave::setEnabled(bool enabled) {
    m_Config.enabled = enabled;

    if (enabled) {
        notify("Autosave enabled");
    } else {
        notify("Autosave disabled");
    }
}

bool AutoSave::triggerAutoSave(TriggerType trigger, const std::string& triggerName) {
    if (!m_Initialized || !m_Config.enabled) {
        return false;
    }

    if (m_IsAutosaving) {
        notify("Autosave already in progress", true);
        return false;
    }

    if (!isSafeToAutoSave()) {
        // Try again later
        return false;
    }

    return performAutoSave(trigger, triggerName);
}

bool AutoSave::saveAndQuit() {
    if (!m_Config.saveOnQuitEnabled) {
        return true;
    }

    // Force autosave regardless of blocks (except if already autosaving)
    if (m_IsAutosaving) {
        // Wait for current autosave to complete
        while (m_IsAutosaving) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Clear blocks temporarily
    auto tempBlocks = m_BlockReasons;
    m_BlockReasons.clear();

    bool result = performAutoSave(TriggerType::EVENT_BASED, "Quit");

    // Restore blocks (though we're quitting anyway)
    m_BlockReasons = tempBlocks;

    return result;
}

void AutoSave::setInterval(float seconds) {
    m_Config.intervalSeconds = std::max(seconds, 30.0f); // Minimum 30 seconds
}

float AutoSave::getTimeUntilNextSave() const {
    float remaining = m_Config.intervalSeconds - m_TimeSinceLastSave;
    return std::max(remaining, 0.0f);
}

void AutoSave::blockAutoSave(const std::string& reason) {
    if (std::find(m_BlockReasons.begin(), m_BlockReasons.end(), reason) == m_BlockReasons.end()) {
        m_BlockReasons.push_back(reason);
    }
}

void AutoSave::unblockAutoSave(const std::string& reason) {
    m_BlockReasons.erase(
        std::remove(m_BlockReasons.begin(), m_BlockReasons.end(), reason),
        m_BlockReasons.end()
    );
}

void AutoSave::clearAllBlocks() {
    m_BlockReasons.clear();
}

int AutoSave::getAutoSaveSlot() const {
    // Autosave slots are after regular save slots
    // They rotate: autosave_0, autosave_1, autosave_2, etc.
    int offset = getAutoSaveSlotOffset();
    return offset + m_CurrentAutoSaveSlot;
}

LoadResult AutoSave::loadMostRecentAutoSave() {
    SaveSystem& saveSystem = SaveSystem::getInstance();

    // Find the most recent autosave
    SaveSlot mostRecentSlot;
    mostRecentSlot.isEmpty = true;

    int offset = getAutoSaveSlotOffset();

    for (int i = 0; i < m_Config.maxAutoSaveSlots; ++i) {
        int slotNumber = offset + i;
        SaveSlot slot = saveSystem.getSlotInfo(slotNumber);

        if (!slot.isEmpty && !slot.isCorrupted) {
            if (mostRecentSlot.isEmpty ||
                slot.metadata.timestamp > mostRecentSlot.metadata.timestamp) {
                mostRecentSlot = slot;
            }
        }
    }

    if (mostRecentSlot.isEmpty) {
        LoadResult result;
        result.error = "No valid autosave found";
        return result;
    }

    return saveSystem.loadFromSlot(mostRecentSlot.slotNumber);
}

std::vector<SaveSlot> AutoSave::getAutoSaveSlots() const {
    std::vector<SaveSlot> slots;
    SaveSystem& saveSystem = SaveSystem::getInstance();

    int offset = getAutoSaveSlotOffset();

    for (int i = 0; i < m_Config.maxAutoSaveSlots; ++i) {
        int slotNumber = offset + i;
        slots.push_back(saveSystem.getSlotInfo(slotNumber));
    }

    // Sort by timestamp (newest first)
    std::sort(slots.begin(), slots.end(),
              [](const SaveSlot& a, const SaveSlot& b) {
                  if (a.isEmpty && !b.isEmpty) return false;
                  if (!a.isEmpty && b.isEmpty) return true;
                  if (a.isEmpty && b.isEmpty) return false;
                  return a.metadata.timestamp > b.metadata.timestamp;
              });

    return slots;
}

void AutoSave::onLevelUp() {
    if (m_Config.saveOnLevelUpEnabled) {
        triggerAutoSave(TriggerType::PROGRESS_BASED, "Level Up");
    }
}

void AutoSave::onQuestComplete(const std::string& questName) {
    if (m_Config.saveOnQuestCompleteEnabled) {
        triggerAutoSave(TriggerType::PROGRESS_BASED, "Quest: " + questName);
    }
}

void AutoSave::onEnterSafeZone(const std::string& zoneName) {
    triggerAutoSave(TriggerType::LOCATION_BASED, "Safe Zone: " + zoneName);
}

void AutoSave::onCheckpointReached(const std::string& checkpointName) {
    triggerAutoSave(TriggerType::CHECKPOINT_BASED, "Checkpoint: " + checkpointName);
}

void AutoSave::setInCombat(bool inCombat) {
    m_InCombat = inCombat;

    if (inCombat && m_Config.preventDuringCombat) {
        blockAutoSave("Combat");
    } else {
        unblockAutoSave("Combat");
    }
}

void AutoSave::setInCutscene(bool inCutscene) {
    m_InCutscene = inCutscene;

    if (inCutscene && m_Config.preventDuringCutscene) {
        blockAutoSave("Cutscene");
    } else {
        unblockAutoSave("Cutscene");
    }
}

void AutoSave::setInDialogue(bool inDialogue) {
    m_InDialogue = inDialogue;

    if (inDialogue && m_Config.preventDuringDialogue) {
        blockAutoSave("Dialogue");
    } else {
        unblockAutoSave("Dialogue");
    }
}

AutoSave::Stats AutoSave::getStats() const {
    Stats stats;
    stats.totalAutoSaves = m_TotalAutoSaves;
    stats.failedAutoSaves = m_FailedAutoSaves;
    stats.averageSaveTime = (m_TotalAutoSaves > 0) ?
                            (m_TotalSaveTime / m_TotalAutoSaves) : 0.0f;
    stats.lastAutoSave = m_LastSaveTime;

    if (!m_SaveHistory.empty()) {
        stats.lastTriggerType = m_SaveHistory.back().triggerType;
    } else {
        stats.lastTriggerType = TriggerType::TIME_BASED;
    }

    return stats;
}

void AutoSave::resetTimer() {
    m_TimeSinceLastSave = 0.0f;
}

bool AutoSave::performAutoSave(TriggerType trigger, const std::string& triggerName) {
    m_IsAutosaving = true;

    if (m_StateCallback) {
        m_StateCallback(true);
    }

    if (m_Config.notifyPlayer) {
        notify("Autosaving...");
    }

    SaveSystem& saveSystem = SaveSystem::getInstance();

    // Determine autosave slot
    int slotNumber = getAutoSaveSlot();

    // Create autosave name
    std::string saveName = "AutoSave_" + std::to_string(m_CurrentAutoSaveSlot);

    // Perform save
    auto startTime = std::chrono::high_resolution_clock::now();
    SaveResult result = saveSystem.saveToSlot(slotNumber, saveName);
    auto endTime = std::chrono::high_resolution_clock::now();

    float saveTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    // Record event
    AutoSaveEvent event;
    event.triggerType = trigger;
    event.triggerName = triggerName;
    event.timestamp = std::chrono::system_clock::now();
    event.wasSuccessful = result.success;
    event.error = result.error;
    recordSaveEvent(event);

    // Update statistics
    m_TotalAutoSaves++;
    m_TotalSaveTime += saveTime;

    if (result.success) {
        m_LastSaveTime = std::chrono::system_clock::now();
        m_TimeSinceLastSave = 0.0f;

        // Move to next autosave slot (rotate)
        m_CurrentAutoSaveSlot = (m_CurrentAutoSaveSlot + 1) % m_Config.maxAutoSaveSlots;

        if (m_Config.notifyPlayer) {
            notify("Autosave complete");
        }
    } else {
        m_FailedAutoSaves++;

        if (m_Config.notifyPlayer) {
            notify("Autosave failed: " + result.error, true);
        }
    }

    m_IsAutosaving = false;

    if (m_StateCallback) {
        m_StateCallback(false);
    }

    return result.success;
}

bool AutoSave::isSafeToAutoSave() const {
    // Check if any blocks are active
    if (!m_BlockReasons.empty()) {
        return false;
    }

    // Check game state
    if (m_Config.preventDuringCombat && m_InCombat) {
        return false;
    }

    if (m_Config.preventDuringCutscene && m_InCutscene) {
        return false;
    }

    if (m_Config.preventDuringDialogue && m_InDialogue) {
        return false;
    }

    return true;
}

void AutoSave::notify(const std::string& message, bool isError) {
    if (m_NotificationCallback) {
        m_NotificationCallback(message, isError);
    }
}

void AutoSave::recordSaveEvent(const AutoSaveEvent& event) {
    m_SaveHistory.push_back(event);

    // Limit history size
    while (m_SaveHistory.size() > MAX_HISTORY_SIZE) {
        m_SaveHistory.erase(m_SaveHistory.begin());
    }
}

int AutoSave::getAutoSaveSlotOffset() const {
    // Autosave slots come after regular save slots
    // Example: If there are 10 regular slots (0-9), autosaves start at 10
    return SaveSystem::getInstance().getConfig().maxSaveSlots;
}

} // namespace Save
