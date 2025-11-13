#pragma once

#include "SaveSystem.h"
#include <functional>
#include <chrono>
#include <string>
#include <vector>

namespace Save {

/**
 * @brief Autosave system for automatic game saving
 *
 * Manages automatic saves at intervals, on events, and based on triggers.
 * Prevents autosaves during critical moments (combat, cutscenes, etc.)
 */
class AutoSave {
public:
    /**
     * @brief Autosave trigger types
     */
    enum class TriggerType {
        TIME_BASED,           // Save every X seconds
        PROGRESS_BASED,       // Save on significant progress (level up, quest complete)
        LOCATION_BASED,       // Save when entering safe zones
        EVENT_BASED,          // Save on specific game events
        CHECKPOINT_BASED      // Save at designated checkpoint locations
    };

    /**
     * @brief Autosave configuration
     */
    struct Config {
        bool enabled;
        float intervalSeconds;        // Time between autosaves
        int maxAutoSaveSlots;         // Number of rotating autosave slots
        bool saveOnQuitEnabled;       // Save when quitting game
        bool saveOnLevelUpEnabled;    // Save when leveling up
        bool saveOnQuestCompleteEnabled; // Save when completing quests
        bool preventDuringCombat;     // Don't autosave during combat
        bool preventDuringCutscene;   // Don't autosave during cutscenes
        bool preventDuringDialogue;   // Don't autosave during dialogue
        bool notifyPlayer;            // Show notification when autosaving
        bool pauseGameDuringSave;     // Pause game during autosave (safer but interrupts gameplay)

        Config()
            : enabled(true)
            , intervalSeconds(300.0f)  // 5 minutes
            , maxAutoSaveSlots(3)
            , saveOnQuitEnabled(true)
            , saveOnLevelUpEnabled(true)
            , saveOnQuestCompleteEnabled(true)
            , preventDuringCombat(true)
            , preventDuringCutscene(true)
            , preventDuringDialogue(true)
            , notifyPlayer(true)
            , pauseGameDuringSave(false)
        {}
    };

    /**
     * @brief Autosave event
     */
    struct AutoSaveEvent {
        TriggerType triggerType;
        std::string triggerName;
        std::chrono::system_clock::time_point timestamp;
        bool wasSuccessful;
        std::string error;
    };

    /**
     * @brief Autosave notification callback
     */
    using NotificationCallback = std::function<void(const std::string& message, bool isError)>;

    /**
     * @brief Autosave state callback
     */
    using StateCallback = std::function<void(bool isAutosaving)>;

    AutoSave();
    ~AutoSave();

    /**
     * @brief Initialize autosave system
     */
    bool initialize(const Config& config = Config());

    /**
     * @brief Shutdown autosave system
     */
    void shutdown();

    /**
     * @brief Update autosave system (call every frame)
     * @param deltaTime Time elapsed since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Enable/disable autosave
     */
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_Config.enabled; }

    /**
     * @brief Trigger manual autosave
     */
    bool triggerAutoSave(TriggerType trigger = TriggerType::EVENT_BASED,
                         const std::string& triggerName = "Manual");

    /**
     * @brief Save and quit
     */
    bool saveAndQuit();

    /**
     * @brief Set autosave interval
     */
    void setInterval(float seconds);
    float getInterval() const { return m_Config.intervalSeconds; }

    /**
     * @brief Get time until next autosave
     */
    float getTimeUntilNextSave() const;

    /**
     * @brief Get last autosave time
     */
    std::chrono::system_clock::time_point getLastSaveTime() const { return m_LastSaveTime; }

    /**
     * @brief Block autosaving (e.g., during combat)
     */
    void blockAutoSave(const std::string& reason);

    /**
     * @brief Unblock autosaving
     */
    void unblockAutoSave(const std::string& reason);

    /**
     * @brief Check if autosaving is blocked
     */
    bool isBlocked() const { return !m_BlockReasons.empty(); }

    /**
     * @brief Get block reasons
     */
    std::vector<std::string> getBlockReasons() const { return m_BlockReasons; }

    /**
     * @brief Clear all block reasons
     */
    void clearAllBlocks();

    /**
     * @brief Get autosave slot (cycles through available slots)
     */
    int getAutoSaveSlot() const;

    /**
     * @brief Load most recent autosave
     */
    LoadResult loadMostRecentAutoSave();

    /**
     * @brief Get all autosave slots
     */
    std::vector<SaveSlot> getAutoSaveSlots() const;

    /**
     * @brief Get autosave history
     */
    std::vector<AutoSaveEvent> getAutoSaveHistory() const { return m_SaveHistory; }

    /**
     * @brief Clear autosave history
     */
    void clearHistory() { m_SaveHistory.clear(); }

    /**
     * @brief Set callbacks
     */
    void setNotificationCallback(NotificationCallback callback) { m_NotificationCallback = callback; }
    void setStateCallback(StateCallback callback) { m_StateCallback = callback; }

    /**
     * @brief Configuration
     */
    void setConfig(const Config& config) { m_Config = config; }
    const Config& getConfig() const { return m_Config; }

    /**
     * @brief Register event triggers
     */
    void onLevelUp();
    void onQuestComplete(const std::string& questName);
    void onEnterSafeZone(const std::string& zoneName);
    void onCheckpointReached(const std::string& checkpointName);

    /**
     * @brief Game state setters (to control blocking)
     */
    void setInCombat(bool inCombat);
    void setInCutscene(bool inCutscene);
    void setInDialogue(bool inDialogue);

    /**
     * @brief Check if currently autosaving
     */
    bool isAutosaving() const { return m_IsAutosaving; }

    /**
     * @brief Get statistics
     */
    struct Stats {
        int totalAutoSaves;
        int failedAutoSaves;
        float averageSaveTime;
        std::chrono::system_clock::time_point lastAutoSave;
        TriggerType lastTriggerType;
    };
    Stats getStats() const;

    /**
     * @brief Reset autosave timer
     */
    void resetTimer();

private:
    /**
     * @brief Perform autosave
     */
    bool performAutoSave(TriggerType trigger, const std::string& triggerName);

    /**
     * @brief Check if safe to autosave
     */
    bool isSafeToAutoSave() const;

    /**
     * @brief Show notification
     */
    void notify(const std::string& message, bool isError = false);

    /**
     * @brief Record save event
     */
    void recordSaveEvent(const AutoSaveEvent& event);

    /**
     * @brief Get autosave slot offset
     */
    int getAutoSaveSlotOffset() const;

    Config m_Config;
    bool m_Initialized;

    // Timing
    float m_TimeSinceLastSave;
    std::chrono::system_clock::time_point m_LastSaveTime;

    // State
    bool m_IsAutosaving;
    int m_CurrentAutoSaveSlot;
    std::vector<std::string> m_BlockReasons;

    // Game state flags
    bool m_InCombat;
    bool m_InCutscene;
    bool m_InDialogue;

    // History
    std::vector<AutoSaveEvent> m_SaveHistory;
    static constexpr size_t MAX_HISTORY_SIZE = 50;

    // Callbacks
    NotificationCallback m_NotificationCallback;
    StateCallback m_StateCallback;

    // Statistics
    int m_TotalAutoSaves;
    int m_FailedAutoSaves;
    float m_TotalSaveTime;
};

/**
 * @brief RAII class to temporarily block autosaving
 */
class AutoSaveBlocker {
public:
    AutoSaveBlocker(AutoSave& autoSave, const std::string& reason)
        : m_AutoSave(autoSave), m_Reason(reason)
    {
        m_AutoSave.blockAutoSave(m_Reason);
    }

    ~AutoSaveBlocker() {
        m_AutoSave.unblockAutoSave(m_Reason);
    }

private:
    AutoSave& m_AutoSave;
    std::string m_Reason;
};

} // namespace Save
