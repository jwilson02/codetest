#pragma once

#include "SaveFile.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <memory>
#include <mutex>

namespace Save {

/**
 * @brief Cloud save provider
 */
enum class CloudProvider {
    NONE,
    STEAM_CLOUD,
    EPIC_CLOUD,
    GOG_GALAXY,
    XBOX_LIVE,
    PLAYSTATION_NETWORK,
    CUSTOM
};

/**
 * @brief Cloud sync status
 */
enum class SyncStatus {
    IDLE,
    UPLOADING,
    DOWNLOADING,
    SYNCING,
    ERROR,
    CONFLICT
};

/**
 * @brief Cloud file information
 */
struct CloudFileInfo {
    std::string fileName;
    std::string cloudPath;
    std::string localPath;
    size_t fileSize;
    std::chrono::system_clock::time_point lastModified;
    bool existsLocally;
    bool existsInCloud;
    bool needsUpload;
    bool needsDownload;
    bool hasConflict;

    CloudFileInfo()
        : fileSize(0)
        , existsLocally(false)
        , existsInCloud(false)
        , needsUpload(false)
        , needsDownload(false)
        , hasConflict(false)
    {}
};

/**
 * @brief Sync conflict resolution strategy
 */
enum class ConflictResolution {
    USE_LOCAL,      // Always use local version
    USE_CLOUD,      // Always use cloud version
    USE_NEWEST,     // Use whichever is newer
    USE_LARGEST,    // Use whichever is larger
    KEEP_BOTH,      // Keep both versions
    ASK_USER        // Prompt user to choose
};

/**
 * @brief Cloud sync result
 */
struct CloudSyncResult {
    bool success;
    std::string error;
    int filesUploaded;
    int filesDownloaded;
    int conflictsResolved;
    float syncTimeMs;

    CloudSyncResult()
        : success(false)
        , filesUploaded(0)
        , filesDownloaded(0)
        , conflictsResolved(0)
        , syncTimeMs(0.0f)
    {}
};

/**
 * @brief Cloud save synchronization manager
 *
 * Handles cloud save upload/download and conflict resolution.
 * Ready for Steam Cloud, Epic Games Store, and other platforms.
 */
class CloudSync {
public:
    /**
     * @brief Cloud sync configuration
     */
    struct Config {
        CloudProvider provider;
        bool enableAutoSync;
        float syncIntervalSeconds;
        bool syncOnStartup;
        bool syncOnQuit;
        bool syncAfterSave;
        ConflictResolution conflictResolution;
        size_t maxCloudStorageBytes;
        bool compressBeforeUpload;
        bool verifyAfterSync;

        Config()
            : provider(CloudProvider::STEAM_CLOUD)
            , enableAutoSync(true)
            , syncIntervalSeconds(600.0f)  // 10 minutes
            , syncOnStartup(true)
            , syncOnQuit(true)
            , syncAfterSave(true)
            , conflictResolution(ConflictResolution::USE_NEWEST)
            , maxCloudStorageBytes(100 * 1024 * 1024)  // 100 MB
            , compressBeforeUpload(true)
            , verifyAfterSync(true)
        {}
    };

    /**
     * @brief Sync callbacks
     */
    using SyncCallback = std::function<void(const CloudSyncResult& result)>;
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using ConflictCallback = std::function<ConflictResolution(const CloudFileInfo& fileInfo)>;

    CloudSync();
    ~CloudSync();

    /**
     * @brief Initialize cloud sync
     */
    bool initialize(const Config& config = Config());

    /**
     * @brief Shutdown cloud sync
     */
    void shutdown();

    /**
     * @brief Check if cloud sync is available
     */
    bool isAvailable() const;

    /**
     * @brief Check if cloud sync is enabled
     */
    bool isEnabled() const { return m_Config.enableAutoSync; }

    /**
     * @brief Enable/disable cloud sync
     */
    void setEnabled(bool enabled);

    /**
     * @brief Update cloud sync (call every frame)
     */
    void update(float deltaTime);

    // === Sync Operations ===

    /**
     * @brief Sync all saves with cloud
     */
    CloudSyncResult syncAll();

    /**
     * @brief Upload specific save to cloud
     */
    bool uploadSave(const std::string& filePath);

    /**
     * @brief Download specific save from cloud
     */
    bool downloadSave(const std::string& filePath);

    /**
     * @brief Upload all local saves to cloud
     */
    CloudSyncResult uploadAll();

    /**
     * @brief Download all cloud saves to local
     */
    CloudSyncResult downloadAll();

    /**
     * @brief Delete cloud save
     */
    bool deleteCloudSave(const std::string& fileName);

    /**
     * @brief Sync asynchronously
     */
    void syncAsync(SyncCallback callback = nullptr);

    // === Cloud File Management ===

    /**
     * @brief Get list of cloud files
     */
    std::vector<CloudFileInfo> getCloudFiles() const;

    /**
     * @brief Get cloud file info
     */
    CloudFileInfo getCloudFileInfo(const std::string& fileName) const;

    /**
     * @brief Check if file exists in cloud
     */
    bool existsInCloud(const std::string& fileName) const;

    /**
     * @brief Get cloud storage usage
     */
    size_t getCloudStorageUsed() const;

    /**
     * @brief Get available cloud storage
     */
    size_t getCloudStorageAvailable() const;

    /**
     * @brief Get cloud storage quota
     */
    size_t getCloudStorageQuota() const { return m_Config.maxCloudStorageBytes; }

    // === Conflict Management ===

    /**
     * @brief Get list of files with conflicts
     */
    std::vector<CloudFileInfo> getConflicts() const;

    /**
     * @brief Resolve conflict for specific file
     */
    bool resolveConflict(const std::string& fileName, ConflictResolution resolution);

    /**
     * @brief Resolve all conflicts
     */
    int resolveAllConflicts();

    /**
     * @brief Check if there are any conflicts
     */
    bool hasConflicts() const;

    // === Status ===

    /**
     * @brief Get current sync status
     */
    SyncStatus getStatus() const { return m_Status; }

    /**
     * @brief Check if sync is in progress
     */
    bool isSyncing() const {
        return m_Status == SyncStatus::UPLOADING ||
               m_Status == SyncStatus::DOWNLOADING ||
               m_Status == SyncStatus::SYNCING;
    }

    /**
     * @brief Get last sync time
     */
    std::chrono::system_clock::time_point getLastSyncTime() const { return m_LastSyncTime; }

    /**
     * @brief Get time until next sync
     */
    float getTimeUntilNextSync() const;

    /**
     * @brief Get last sync result
     */
    CloudSyncResult getLastSyncResult() const { return m_LastSyncResult; }

    // === Callbacks ===

    /**
     * @brief Set sync callback
     */
    void setSyncCallback(SyncCallback callback) { m_SyncCallback = callback; }

    /**
     * @brief Set progress callback
     */
    void setProgressCallback(ProgressCallback callback) { m_ProgressCallback = callback; }

    /**
     * @brief Set conflict callback
     */
    void setConflictCallback(ConflictCallback callback) { m_ConflictCallback = callback; }

    // === Configuration ===

    /**
     * @brief Get configuration
     */
    const Config& getConfig() const { return m_Config; }

    /**
     * @brief Set configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief Set conflict resolution strategy
     */
    void setConflictResolution(ConflictResolution resolution) {
        m_Config.conflictResolution = resolution;
    }

    // === Statistics ===

    /**
     * @brief Sync statistics
     */
    struct Stats {
        int totalSyncs;
        int successfulSyncs;
        int failedSyncs;
        int totalUploads;
        int totalDownloads;
        int totalConflicts;
        size_t totalBytesUploaded;
        size_t totalBytesDownloaded;
        float averageSyncTime;
    };

    Stats getStats() const;

    /**
     * @brief Reset statistics
     */
    void resetStats();

    // === Platform Specific ===

    /**
     * @brief Initialize Steam Cloud (if using Steam)
     */
    bool initializeSteamCloud();

    /**
     * @brief Check if Steam Cloud is enabled
     */
    bool isSteamCloudEnabled() const;

    /**
     * @brief Get Steam App ID
     */
    uint32_t getSteamAppId() const { return m_SteamAppId; }

    /**
     * @brief Set Steam App ID
     */
    void setSteamAppId(uint32_t appId) { m_SteamAppId = appId; }

private:
    /**
     * @brief Perform sync operation
     */
    CloudSyncResult performSync();

    /**
     * @brief Check for conflicts
     */
    std::vector<CloudFileInfo> detectConflicts();

    /**
     * @brief Resolve single conflict
     */
    bool resolveConflictInternal(CloudFileInfo& fileInfo, ConflictResolution resolution);

    /**
     * @brief Report progress
     */
    void reportProgress(float progress, const std::string& status);

    /**
     * @brief Get provider name
     */
    std::string getProviderName() const;

    // === Steam Cloud Integration ===

    /**
     * @brief Steam Cloud file write
     */
    bool steamCloudWriteFile(const std::string& fileName, const void* data, size_t size);

    /**
     * @brief Steam Cloud file read
     */
    bool steamCloudReadFile(const std::string& fileName, std::vector<uint8_t>& data);

    /**
     * @brief Steam Cloud file exists
     */
    bool steamCloudFileExists(const std::string& fileName) const;

    /**
     * @brief Steam Cloud file delete
     */
    bool steamCloudDeleteFile(const std::string& fileName);

    /**
     * @brief Steam Cloud get file size
     */
    size_t steamCloudGetFileSize(const std::string& fileName) const;

    /**
     * @brief Steam Cloud get file timestamp
     */
    std::chrono::system_clock::time_point steamCloudGetFileTimestamp(const std::string& fileName) const;

    /**
     * @brief Get all Steam Cloud files
     */
    std::vector<std::string> steamCloudGetAllFiles() const;

    Config m_Config;
    bool m_Initialized;
    SyncStatus m_Status;

    // Timing
    float m_TimeSinceLastSync;
    std::chrono::system_clock::time_point m_LastSyncTime;

    // Results
    CloudSyncResult m_LastSyncResult;

    // File tracking
    std::map<std::string, CloudFileInfo> m_CloudFiles;
    std::vector<CloudFileInfo> m_Conflicts;

    // Callbacks
    SyncCallback m_SyncCallback;
    ProgressCallback m_ProgressCallback;
    ConflictCallback m_ConflictCallback;

    // Statistics
    Stats m_Stats;

    // Thread safety
    std::mutex m_Mutex;
    bool m_IsSyncing;

    // Steam integration
    uint32_t m_SteamAppId;
    bool m_SteamInitialized;

    // Cloud storage tracking
    size_t m_CloudStorageUsed;
};

/**
 * @brief RAII class to pause cloud sync temporarily
 */
class CloudSyncPauser {
public:
    CloudSyncPauser(CloudSync& cloudSync)
        : m_CloudSync(cloudSync)
        , m_WasEnabled(cloudSync.isEnabled())
    {
        if (m_WasEnabled) {
            m_CloudSync.setEnabled(false);
        }
    }

    ~CloudSyncPauser() {
        if (m_WasEnabled) {
            m_CloudSync.setEnabled(true);
        }
    }

private:
    CloudSync& m_CloudSync;
    bool m_WasEnabled;
};

} // namespace Save
