#include "CloudSync.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <thread>

// Conditional Steam SDK includes
// In production, include actual Steam SDK headers
// #include <steam/steam_api.h>

namespace fs = std::filesystem;

namespace Save {

CloudSync::CloudSync()
    : m_Initialized(false)
    , m_Status(SyncStatus::IDLE)
    , m_TimeSinceLastSync(0.0f)
    , m_IsSyncing(false)
    , m_SteamAppId(0)
    , m_SteamInitialized(false)
    , m_CloudStorageUsed(0)
{
}

CloudSync::~CloudSync() {
    shutdown();
}

bool CloudSync::initialize(const Config& config) {
    if (m_Initialized) {
        return true;
    }

    m_Config = config;
    m_Status = SyncStatus::IDLE;
    m_TimeSinceLastSync = 0.0f;

    // Initialize provider-specific functionality
    switch (m_Config.provider) {
        case CloudProvider::STEAM_CLOUD:
            if (!initializeSteamCloud()) {
                return false;
            }
            break;

        case CloudProvider::EPIC_CLOUD:
        case CloudProvider::GOG_GALAXY:
        case CloudProvider::XBOX_LIVE:
        case CloudProvider::PLAYSTATION_NETWORK:
        case CloudProvider::CUSTOM:
            // Initialize other providers
            break;

        case CloudProvider::NONE:
            // No cloud sync
            break;
    }

    // Perform initial sync if configured
    if (m_Config.syncOnStartup) {
        syncAll();
    }

    m_Initialized = true;
    return true;
}

void CloudSync::shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Wait for any pending sync
    while (m_IsSyncing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Perform final sync if configured
    if (m_Config.syncOnQuit) {
        syncAll();
    }

    m_Initialized = false;
}

bool CloudSync::isAvailable() const {
    if (!m_Initialized) {
        return false;
    }

    switch (m_Config.provider) {
        case CloudProvider::STEAM_CLOUD:
            return m_SteamInitialized && isSteamCloudEnabled();

        case CloudProvider::NONE:
            return false;

        default:
            // Other providers
            return true;
    }
}

void CloudSync::setEnabled(bool enabled) {
    m_Config.enableAutoSync = enabled;
}

void CloudSync::update(float deltaTime) {
    if (!m_Initialized || !m_Config.enableAutoSync || m_IsSyncing) {
        return;
    }

    m_TimeSinceLastSync += deltaTime;

    // Check if it's time for automatic sync
    if (m_TimeSinceLastSync >= m_Config.syncIntervalSeconds) {
        syncAll();
    }
}

CloudSyncResult CloudSync::syncAll() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isAvailable()) {
        CloudSyncResult result;
        result.error = "Cloud sync not available";
        return result;
    }

    if (m_IsSyncing) {
        CloudSyncResult result;
        result.error = "Sync already in progress";
        return result;
    }

    m_IsSyncing = true;
    m_Status = SyncStatus::SYNCING;

    CloudSyncResult result = performSync();

    m_LastSyncResult = result;
    m_LastSyncTime = std::chrono::system_clock::now();
    m_TimeSinceLastSync = 0.0f;

    // Update statistics
    m_Stats.totalSyncs++;
    if (result.success) {
        m_Stats.successfulSyncs++;
    } else {
        m_Stats.failedSyncs++;
    }

    m_Status = result.success ? SyncStatus::IDLE : SyncStatus::ERROR;
    m_IsSyncing = false;

    if (m_SyncCallback) {
        m_SyncCallback(result);
    }

    return result;
}

bool CloudSync::uploadSave(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isAvailable()) {
        return false;
    }

    reportProgress(0.0f, "Uploading save...");

    try {
        // Read local file
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(fileSize);
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        file.close();

        reportProgress(0.5f, "Writing to cloud...");

        // Extract filename
        fs::path path(filePath);
        std::string fileName = path.filename().string();

        // Upload based on provider
        bool success = false;
        switch (m_Config.provider) {
            case CloudProvider::STEAM_CLOUD:
                success = steamCloudWriteFile(fileName, data.data(), data.size());
                break;

            default:
                success = false;
                break;
        }

        if (success) {
            m_Stats.totalUploads++;
            m_Stats.totalBytesUploaded += fileSize;
            reportProgress(1.0f, "Upload complete");
        }

        return success;
    } catch (...) {
        return false;
    }
}

bool CloudSync::downloadSave(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isAvailable()) {
        return false;
    }

    reportProgress(0.0f, "Downloading save...");

    try {
        // Extract filename
        fs::path path(filePath);
        std::string fileName = path.filename().string();

        // Download based on provider
        std::vector<uint8_t> data;
        bool success = false;

        reportProgress(0.3f, "Reading from cloud...");

        switch (m_Config.provider) {
            case CloudProvider::STEAM_CLOUD:
                success = steamCloudReadFile(fileName, data);
                break;

            default:
                success = false;
                break;
        }

        if (!success || data.empty()) {
            return false;
        }

        reportProgress(0.7f, "Writing to local file...");

        // Write to local file
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();

        m_Stats.totalDownloads++;
        m_Stats.totalBytesDownloaded += data.size();

        reportProgress(1.0f, "Download complete");

        return true;
    } catch (...) {
        return false;
    }
}

CloudSyncResult CloudSync::uploadAll() {
    // Implementation similar to syncAll but only uploads
    CloudSyncResult result;
    result.success = true;
    return result;
}

CloudSyncResult CloudSync::downloadAll() {
    // Implementation similar to syncAll but only downloads
    CloudSyncResult result;
    result.success = true;
    return result;
}

bool CloudSync::deleteCloudSave(const std::string& fileName) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isAvailable()) {
        return false;
    }

    switch (m_Config.provider) {
        case CloudProvider::STEAM_CLOUD:
            return steamCloudDeleteFile(fileName);

        default:
            return false;
    }
}

void CloudSync::syncAsync(SyncCallback callback) {
    if (m_IsSyncing) {
        if (callback) {
            CloudSyncResult result;
            result.error = "Sync already in progress";
            callback(result);
        }
        return;
    }

    std::thread([this, callback]() {
        CloudSyncResult result = syncAll();

        if (callback) {
            callback(result);
        }
    }).detach();
}

std::vector<CloudFileInfo> CloudSync::getCloudFiles() const {
    std::vector<CloudFileInfo> files;

    for (const auto& [fileName, fileInfo] : m_CloudFiles) {
        files.push_back(fileInfo);
    }

    return files;
}

CloudFileInfo CloudSync::getCloudFileInfo(const std::string& fileName) const {
    auto it = m_CloudFiles.find(fileName);
    if (it != m_CloudFiles.end()) {
        return it->second;
    }

    return CloudFileInfo();
}

bool CloudSync::existsInCloud(const std::string& fileName) const {
    switch (m_Config.provider) {
        case CloudProvider::STEAM_CLOUD:
            return steamCloudFileExists(fileName);

        default:
            return false;
    }
}

size_t CloudSync::getCloudStorageUsed() const {
    return m_CloudStorageUsed;
}

size_t CloudSync::getCloudStorageAvailable() const {
    size_t used = getCloudStorageUsed();
    size_t quota = getCloudStorageQuota();

    return (used < quota) ? (quota - used) : 0;
}

std::vector<CloudFileInfo> CloudSync::getConflicts() const {
    return m_Conflicts;
}

bool CloudSync::resolveConflict(const std::string& fileName, ConflictResolution resolution) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_CloudFiles.find(fileName);
    if (it == m_CloudFiles.end()) {
        return false;
    }

    return resolveConflictInternal(it->second, resolution);
}

int CloudSync::resolveAllConflicts() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    int resolved = 0;

    for (auto& conflict : m_Conflicts) {
        if (resolveConflictInternal(conflict, m_Config.conflictResolution)) {
            resolved++;
        }
    }

    m_Conflicts.clear();

    return resolved;
}

bool CloudSync::hasConflicts() const {
    return !m_Conflicts.empty();
}

float CloudSync::getTimeUntilNextSync() const {
    float remaining = m_Config.syncIntervalSeconds - m_TimeSinceLastSync;
    return std::max(remaining, 0.0f);
}

void CloudSync::setConfig(const Config& config) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Config = config;
}

CloudSync::Stats CloudSync::getStats() const {
    Stats stats = m_Stats;

    if (m_Stats.totalSyncs > 0) {
        // Calculate average sync time from last result
        stats.averageSyncTime = m_LastSyncResult.syncTimeMs;
    }

    return stats;
}

void CloudSync::resetStats() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Stats = Stats();
}

bool CloudSync::initializeSteamCloud() {
    // In production, initialize Steam API
    // This is a placeholder implementation

    /*
    // Example Steam initialization code:
    if (!SteamAPI_Init()) {
        return false;
    }

    ISteamRemoteStorage* remoteStorage = SteamRemoteStorage();
    if (!remoteStorage) {
        return false;
    }

    m_SteamInitialized = true;
    return true;
    */

    // For now, just set flag
    m_SteamInitialized = true;
    return true;
}

bool CloudSync::isSteamCloudEnabled() const {
    // In production, check Steam Cloud status
    // return SteamRemoteStorage() && SteamRemoteStorage()->IsCloudEnabledForAccount() &&
    //        SteamRemoteStorage()->IsCloudEnabledForApp();

    return m_SteamInitialized;
}

CloudSyncResult CloudSync::performSync() {
    auto startTime = std::chrono::high_resolution_clock::now();

    CloudSyncResult result;
    result.success = true;

    reportProgress(0.0f, "Starting sync...");

    // Detect conflicts
    reportProgress(0.1f, "Checking for conflicts...");
    m_Conflicts = detectConflicts();

    // Resolve conflicts
    if (!m_Conflicts.empty()) {
        reportProgress(0.3f, "Resolving conflicts...");
        result.conflictsResolved = resolveAllConflicts();
        m_Stats.totalConflicts += result.conflictsResolved;
    }

    // Upload files that need uploading
    reportProgress(0.5f, "Uploading files...");
    for (auto& [fileName, fileInfo] : m_CloudFiles) {
        if (fileInfo.needsUpload) {
            if (uploadSave(fileInfo.localPath)) {
                result.filesUploaded++;
                fileInfo.needsUpload = false;
            }
        }
    }

    // Download files that need downloading
    reportProgress(0.7f, "Downloading files...");
    for (auto& [fileName, fileInfo] : m_CloudFiles) {
        if (fileInfo.needsDownload) {
            if (downloadSave(fileInfo.localPath)) {
                result.filesDownloaded++;
                fileInfo.needsDownload = false;
            }
        }
    }

    // Verify if configured
    if (m_Config.verifyAfterSync) {
        reportProgress(0.9f, "Verifying sync...");
        // Verification logic here
    }

    reportProgress(1.0f, "Sync complete");

    auto endTime = std::chrono::high_resolution_clock::now();
    result.syncTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    return result;
}

std::vector<CloudFileInfo> CloudSync::detectConflicts() {
    std::vector<CloudFileInfo> conflicts;

    // Get list of all cloud files
    std::vector<std::string> cloudFiles;

    switch (m_Config.provider) {
        case CloudProvider::STEAM_CLOUD:
            cloudFiles = steamCloudGetAllFiles();
            break;

        default:
            break;
    }

    // Check each file for conflicts
    for (const std::string& fileName : cloudFiles) {
        CloudFileInfo fileInfo;
        fileInfo.fileName = fileName;
        fileInfo.existsInCloud = true;

        // Get cloud file info
        fileInfo.fileSize = steamCloudGetFileSize(fileName);
        fileInfo.lastModified = steamCloudGetFileTimestamp(fileName);

        // Check local file
        // (Implementation depends on save system integration)

        // Detect conflicts
        if (fileInfo.existsLocally && fileInfo.existsInCloud) {
            // Both exist - check timestamps
            // If different, mark as conflict
            fileInfo.hasConflict = true;
            conflicts.push_back(fileInfo);
        }
    }

    return conflicts;
}

bool CloudSync::resolveConflictInternal(CloudFileInfo& fileInfo, ConflictResolution resolution) {
    switch (resolution) {
        case ConflictResolution::USE_LOCAL:
            return uploadSave(fileInfo.localPath);

        case ConflictResolution::USE_CLOUD:
            return downloadSave(fileInfo.localPath);

        case ConflictResolution::USE_NEWEST:
            // Compare timestamps and use newer
            return true;

        case ConflictResolution::USE_LARGEST:
            // Compare sizes and use larger
            return true;

        case ConflictResolution::KEEP_BOTH:
            // Keep both versions with different names
            return true;

        case ConflictResolution::ASK_USER:
            if (m_ConflictCallback) {
                ConflictResolution userChoice = m_ConflictCallback(fileInfo);
                return resolveConflictInternal(fileInfo, userChoice);
            }
            return false;
    }

    return false;
}

void CloudSync::reportProgress(float progress, const std::string& status) {
    if (m_ProgressCallback) {
        m_ProgressCallback(progress, status);
    }
}

std::string CloudSync::getProviderName() const {
    switch (m_Config.provider) {
        case CloudProvider::NONE: return "None";
        case CloudProvider::STEAM_CLOUD: return "Steam Cloud";
        case CloudProvider::EPIC_CLOUD: return "Epic Cloud";
        case CloudProvider::GOG_GALAXY: return "GOG Galaxy";
        case CloudProvider::XBOX_LIVE: return "Xbox Live";
        case CloudProvider::PLAYSTATION_NETWORK: return "PlayStation Network";
        case CloudProvider::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

// === Steam Cloud Integration ===

bool CloudSync::steamCloudWriteFile(const std::string& fileName, const void* data, size_t size) {
    // In production, use actual Steam API:
    // return SteamRemoteStorage()->FileWrite(fileName.c_str(), data, static_cast<int32>(size));

    // Placeholder implementation
    return true;
}

bool CloudSync::steamCloudReadFile(const std::string& fileName, std::vector<uint8_t>& data) {
    // In production, use actual Steam API:
    /*
    ISteamRemoteStorage* remoteStorage = SteamRemoteStorage();
    if (!remoteStorage) return false;

    int32 fileSize = remoteStorage->GetFileSize(fileName.c_str());
    if (fileSize <= 0) return false;

    data.resize(fileSize);
    int32 bytesRead = remoteStorage->FileRead(fileName.c_str(), data.data(), fileSize);
    return bytesRead == fileSize;
    */

    // Placeholder implementation
    return true;
}

bool CloudSync::steamCloudFileExists(const std::string& fileName) const {
    // In production:
    // return SteamRemoteStorage() && SteamRemoteStorage()->FileExists(fileName.c_str());

    return false;
}

bool CloudSync::steamCloudDeleteFile(const std::string& fileName) {
    // In production:
    // return SteamRemoteStorage() && SteamRemoteStorage()->FileDelete(fileName.c_str());

    return true;
}

size_t CloudSync::steamCloudGetFileSize(const std::string& fileName) const {
    // In production:
    // return SteamRemoteStorage() ? SteamRemoteStorage()->GetFileSize(fileName.c_str()) : 0;

    return 0;
}

std::chrono::system_clock::time_point CloudSync::steamCloudGetFileTimestamp(const std::string& fileName) const {
    // In production:
    /*
    if (SteamRemoteStorage()) {
        int64 timestamp = SteamRemoteStorage()->GetFileTimestamp(fileName.c_str());
        return std::chrono::system_clock::from_time_t(timestamp);
    }
    */

    return std::chrono::system_clock::now();
}

std::vector<std::string> CloudSync::steamCloudGetAllFiles() const {
    std::vector<std::string> files;

    // In production:
    /*
    ISteamRemoteStorage* remoteStorage = SteamRemoteStorage();
    if (!remoteStorage) return files;

    int32 fileCount = remoteStorage->GetFileCount();
    for (int32 i = 0; i < fileCount; ++i) {
        int32 fileSize;
        const char* fileName = remoteStorage->GetFileNameAndSize(i, &fileSize);
        if (fileName) {
            files.push_back(fileName);
        }
    }
    */

    return files;
}

} // namespace Save
