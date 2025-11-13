#pragma once

#include "SaveFile.h"
#include "Serializer.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

namespace Save {

/**
 * @brief Save operation result
 */
struct SaveResult {
    bool success;
    std::string error;
    std::string filePath;
    float saveTimeMs;

    SaveResult() : success(false), saveTimeMs(0.0f) {}
};

/**
 * @brief Load operation result
 */
struct LoadResult {
    bool success;
    std::string error;
    SaveMetadata metadata;
    float loadTimeMs;
    bool wasMigrated;

    LoadResult() : success(false), loadTimeMs(0.0f), wasMigrated(false) {}
};

/**
 * @brief Checkpoint data for quick saves
 */
struct Checkpoint {
    std::string id;
    std::string name;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> data;
    bool isTemporary;

    Checkpoint() : isTemporary(true) {}
};

/**
 * @brief Main save system manager
 *
 * Manages save slots, autosaves, checkpoints, and backup/recovery.
 * Thread-safe operations for background saving.
 */
class SaveSystem {
public:
    /**
     * @brief Save slot configuration
     */
    struct Config {
        int maxSaveSlots;
        int maxBackupsPerSlot;
        int maxCheckpoints;
        std::string saveDirectory;
        std::string backupDirectory;
        bool enableAutomaticBackup;
        bool enableCorruptionRecovery;
        SaveFile::EncryptionLevel encryptionLevel;
        SaveFile::CompressionLevel compressionLevel;

        Config()
            : maxSaveSlots(10)
            , maxBackupsPerSlot(3)
            , maxCheckpoints(5)
            , saveDirectory("./saves")
            , backupDirectory("./saves/backups")
            , enableAutomaticBackup(true)
            , enableCorruptionRecovery(true)
            , encryptionLevel(SaveFile::EncryptionLevel::LIGHT)
            , compressionLevel(SaveFile::CompressionLevel::BALANCED)
        {}
    };

    /**
     * @brief Save operation callbacks
     */
    using SaveCallback = std::function<void(const SaveResult& result)>;
    using LoadCallback = std::function<void(const LoadResult& result)>;
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;

    /**
     * @brief Get singleton instance
     */
    static SaveSystem& getInstance();

    /**
     * @brief Initialize save system
     */
    bool initialize(const Config& config = Config());

    /**
     * @brief Shutdown save system
     */
    void shutdown();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return m_Initialized; }

    // === Save Slot Management ===

    /**
     * @brief Save game to slot
     */
    SaveResult saveToSlot(int slotNumber, const std::string& saveName = "");

    /**
     * @brief Load game from slot
     */
    LoadResult loadFromSlot(int slotNumber);

    /**
     * @brief Delete save slot
     */
    bool deleteSlot(int slotNumber);

    /**
     * @brief Get save slot information
     */
    SaveSlot getSlotInfo(int slotNumber) const;

    /**
     * @brief Get all save slots
     */
    std::vector<SaveSlot> getAllSlots() const;

    /**
     * @brief Check if slot exists and is valid
     */
    bool isSlotValid(int slotNumber) const;

    /**
     * @brief Get first empty slot
     */
    int getFirstEmptySlot() const;

    /**
     * @brief Copy save slot
     */
    bool copySlot(int sourceSlot, int destSlot);

    // === Quick Save/Load ===

    /**
     * @brief Quick save to dedicated slot
     */
    SaveResult quickSave();

    /**
     * @brief Quick load from dedicated slot
     */
    LoadResult quickLoad();

    /**
     * @brief Check if quick save exists
     */
    bool hasQuickSave() const;

    // === Checkpoint System ===

    /**
     * @brief Create checkpoint
     */
    bool createCheckpoint(const std::string& checkpointId, const std::string& name = "");

    /**
     * @brief Load checkpoint
     */
    bool loadCheckpoint(const std::string& checkpointId);

    /**
     * @brief Delete checkpoint
     */
    bool deleteCheckpoint(const std::string& checkpointId);

    /**
     * @brief Get all checkpoints
     */
    std::vector<Checkpoint> getAllCheckpoints() const;

    /**
     * @brief Clear old checkpoints (keeps only recent ones)
     */
    void clearOldCheckpoints();

    // === Data Management ===

    /**
     * @brief Set data section for next save
     */
    void setData(const std::string& section, const std::string& data);

    /**
     * @brief Get data section from last load
     */
    std::string getData(const std::string& section) const;

    /**
     * @brief Set binary data section
     */
    void setBinaryData(const std::string& section, const std::vector<uint8_t>& data);

    /**
     * @brief Get binary data section
     */
    std::vector<uint8_t> getBinaryData(const std::string& section) const;

    /**
     * @brief Get current save file
     */
    SaveFile& getCurrentSaveFile() { return m_CurrentSaveFile; }
    const SaveFile& getCurrentSaveFile() const { return m_CurrentSaveFile; }

    /**
     * @brief Update metadata
     */
    void updateMetadata(const SaveMetadata& metadata);
    SaveMetadata& getMetadata() { return m_CurrentSaveFile.getMetadata(); }

    // === Backup and Recovery ===

    /**
     * @brief Create backup of save slot
     */
    bool createBackup(int slotNumber);

    /**
     * @brief Restore from backup
     */
    bool restoreFromBackup(int slotNumber, int backupIndex = 0);

    /**
     * @brief Get available backups for slot
     */
    std::vector<std::string> getBackups(int slotNumber) const;

    /**
     * @brief Verify save file integrity
     */
    bool verifySaveIntegrity(int slotNumber) const;

    /**
     * @brief Attempt to recover corrupted save
     */
    bool recoverCorruptedSave(int slotNumber);

    /**
     * @brief Clean up old backups
     */
    void cleanupOldBackups();

    // === Async Operations ===

    /**
     * @brief Save asynchronously
     */
    void saveAsync(int slotNumber, SaveCallback callback = nullptr);

    /**
     * @brief Load asynchronously
     */
    void loadAsync(int slotNumber, LoadCallback callback = nullptr);

    /**
     * @brief Check if save/load operation is in progress
     */
    bool isBusy() const { return m_IsBusy; }

    /**
     * @brief Wait for async operations to complete
     */
    void waitForCompletion();

    // === Callbacks ===

    /**
     * @brief Set save callback
     */
    void setSaveCallback(SaveCallback callback) { m_SaveCallback = callback; }

    /**
     * @brief Set load callback
     */
    void setLoadCallback(LoadCallback callback) { m_LoadCallback = callback; }

    /**
     * @brief Set progress callback
     */
    void setProgressCallback(ProgressCallback callback) { m_ProgressCallback = callback; }

    // === Utilities ===

    /**
     * @brief Get save directory
     */
    std::string getSaveDirectory() const { return m_Config.saveDirectory; }

    /**
     * @brief Get slot file path
     */
    std::string getSlotFilePath(int slotNumber) const;

    /**
     * @brief Get total save size
     */
    size_t getTotalSaveSize() const;

    /**
     * @brief Export save to JSON (for debugging)
     */
    std::string exportSlotToJson(int slotNumber) const;

    /**
     * @brief Import save from JSON
     */
    bool importSlotFromJson(int slotNumber, const std::string& json);

    /**
     * @brief Get save statistics
     */
    struct SaveStats {
        int totalSaves;
        int corruptedSaves;
        size_t totalSize;
        std::chrono::system_clock::time_point oldestSave;
        std::chrono::system_clock::time_point newestSave;
    };
    SaveStats getSaveStats() const;

    /**
     * @brief Enable/disable automatic backup
     */
    void setAutomaticBackup(bool enable) { m_Config.enableAutomaticBackup = enable; }

    /**
     * @brief Get configuration
     */
    const Config& getConfig() const { return m_Config; }

private:
    SaveSystem();
    ~SaveSystem();

    // Prevent copying
    SaveSystem(const SaveSystem&) = delete;
    SaveSystem& operator=(const SaveSystem&) = delete;

    /**
     * @brief Create save directory if it doesn't exist
     */
    bool createSaveDirectory();

    /**
     * @brief Scan save directory for existing saves
     */
    void scanSaveDirectory();

    /**
     * @brief Get backup file path
     */
    std::string getBackupFilePath(int slotNumber, int backupIndex) const;

    /**
     * @brief Rotate backups (keep only recent ones)
     */
    void rotateBackups(int slotNumber);

    /**
     * @brief Report progress
     */
    void reportProgress(float progress, const std::string& status);

    /**
     * @brief Verify platform compatibility
     */
    bool verifyPlatformCompatibility(const SaveMetadata& metadata) const;

    // Configuration
    Config m_Config;
    bool m_Initialized;

    // Current save data
    SaveFile m_CurrentSaveFile;
    std::map<int, SaveSlot> m_SaveSlots;

    // Checkpoints
    std::map<std::string, Checkpoint> m_Checkpoints;

    // Quick save slot (special slot -1)
    static constexpr int QUICK_SAVE_SLOT = -1;

    // Async operations
    bool m_IsBusy;
    std::mutex m_Mutex;

    // Callbacks
    SaveCallback m_SaveCallback;
    LoadCallback m_LoadCallback;
    ProgressCallback m_ProgressCallback;
};

} // namespace Save
