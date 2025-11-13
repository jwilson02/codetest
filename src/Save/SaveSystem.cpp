#include "SaveSystem.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace Save {

SaveSystem& SaveSystem::getInstance() {
    static SaveSystem instance;
    return instance;
}

SaveSystem::SaveSystem()
    : m_Initialized(false)
    , m_IsBusy(false)
{
}

SaveSystem::~SaveSystem() {
    shutdown();
}

bool SaveSystem::initialize(const Config& config) {
    if (m_Initialized) {
        return true;
    }

    m_Config = config;

    // Create save directories
    if (!createSaveDirectory()) {
        return false;
    }

    // Scan for existing saves
    scanSaveDirectory();

    m_Initialized = true;
    return true;
}

void SaveSystem::shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Wait for any pending operations
    waitForCompletion();

    m_SaveSlots.clear();
    m_Checkpoints.clear();
    m_CurrentSaveFile.clear();

    m_Initialized = false;
}

SaveResult SaveSystem::saveToSlot(int slotNumber, const std::string& saveName) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    SaveResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!m_Initialized) {
        result.error = "Save system not initialized";
        return result;
    }

    if (slotNumber < 0 || slotNumber >= m_Config.maxSaveSlots) {
        result.error = "Invalid slot number";
        return result;
    }

    reportProgress(0.0f, "Preparing save data...");

    // Update metadata
    if (!saveName.empty()) {
        m_CurrentSaveFile.getMetadata().saveName = saveName;
    }
    m_CurrentSaveFile.getMetadata().timestamp = std::chrono::system_clock::now();

    reportProgress(0.2f, "Creating backup...");

    // Create backup if enabled
    if (m_Config.enableAutomaticBackup && isSlotValid(slotNumber)) {
        createBackup(slotNumber);
    }

    reportProgress(0.4f, "Writing save file...");

    // Set encryption and compression
    m_CurrentSaveFile.setEncryption(m_Config.encryptionLevel);
    m_CurrentSaveFile.setCompression(m_Config.compressionLevel);

    // Save to file
    std::string filePath = getSlotFilePath(slotNumber);
    bool saveSuccess = m_CurrentSaveFile.save(filePath);

    if (!saveSuccess) {
        result.error = "Failed to write save file";
        return result;
    }

    reportProgress(0.8f, "Verifying save...");

    // Verify save integrity
    SaveFile verifyFile;
    if (!verifyFile.load(filePath) || !verifyFile.validate()) {
        result.error = "Save verification failed";

        // Attempt to restore from backup if available
        if (m_Config.enableCorruptionRecovery) {
            restoreFromBackup(slotNumber);
        }

        return result;
    }

    reportProgress(1.0f, "Save complete!");

    // Update slot info
    SaveSlot& slot = m_SaveSlots[slotNumber];
    slot.slotNumber = slotNumber;
    slot.isEmpty = false;
    slot.metadata = m_CurrentSaveFile.getMetadata();
    slot.filePath = filePath;
    slot.fileSize = m_CurrentSaveFile.getSize();
    slot.isCorrupted = false;

    // Rotate backups if needed
    rotateBackups(slotNumber);

    auto endTime = std::chrono::high_resolution_clock::now();
    result.saveTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    result.success = true;
    result.filePath = filePath;

    // Trigger callback
    if (m_SaveCallback) {
        m_SaveCallback(result);
    }

    return result;
}

LoadResult SaveSystem::loadFromSlot(int slotNumber) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    LoadResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!m_Initialized) {
        result.error = "Save system not initialized";
        return result;
    }

    if (slotNumber < 0 || slotNumber >= m_Config.maxSaveSlots) {
        result.error = "Invalid slot number";
        return result;
    }

    reportProgress(0.0f, "Loading save file...");

    std::string filePath = getSlotFilePath(slotNumber);

    // Check if file exists
    if (!fs::exists(filePath)) {
        result.error = "Save file does not exist";
        return result;
    }

    reportProgress(0.3f, "Reading data...");

    // Load save file
    m_CurrentSaveFile.clear();
    bool loadSuccess = m_CurrentSaveFile.load(filePath);

    if (!loadSuccess) {
        result.error = "Failed to load save file";

        // Attempt recovery from backup
        if (m_Config.enableCorruptionRecovery) {
            reportProgress(0.5f, "Attempting recovery from backup...");
            if (restoreFromBackup(slotNumber)) {
                loadSuccess = m_CurrentSaveFile.load(filePath);
            }
        }

        if (!loadSuccess) {
            return result;
        }
    }

    reportProgress(0.6f, "Validating save...");

    // Validate save
    if (!m_CurrentSaveFile.validate()) {
        result.error = "Save file is corrupted";

        // Try backup recovery
        if (m_Config.enableCorruptionRecovery && restoreFromBackup(slotNumber)) {
            if (m_CurrentSaveFile.load(filePath) && m_CurrentSaveFile.validate()) {
                result.wasMigrated = true;
            } else {
                return result;
            }
        } else {
            return result;
        }
    }

    reportProgress(0.8f, "Checking compatibility...");

    // Check for migration
    if (m_CurrentSaveFile.needsMigration()) {
        reportProgress(0.85f, "Migrating save file...");
        if (!m_CurrentSaveFile.migrate()) {
            result.error = "Failed to migrate save file";
            return result;
        }
        result.wasMigrated = true;

        // Save migrated file
        m_CurrentSaveFile.save(filePath);
    }

    // Verify platform compatibility
    if (!verifyPlatformCompatibility(m_CurrentSaveFile.getMetadata())) {
        // Warning but not fatal - allow cross-platform saves
        result.error = "Warning: Save file from different platform";
    }

    reportProgress(1.0f, "Load complete!");

    auto endTime = std::chrono::high_resolution_clock::now();
    result.loadTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    result.success = true;
    result.metadata = m_CurrentSaveFile.getMetadata();

    // Trigger callback
    if (m_LoadCallback) {
        m_LoadCallback(result);
    }

    return result;
}

bool SaveSystem::deleteSlot(int slotNumber) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isSlotValid(slotNumber)) {
        return false;
    }

    std::string filePath = getSlotFilePath(slotNumber);

    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }

        // Remove from slots
        m_SaveSlots.erase(slotNumber);

        return true;
    } catch (...) {
        return false;
    }
}

SaveSlot SaveSystem::getSlotInfo(int slotNumber) const {
    auto it = m_SaveSlots.find(slotNumber);
    if (it != m_SaveSlots.end()) {
        return it->second;
    }

    SaveSlot emptySlot;
    emptySlot.slotNumber = slotNumber;
    emptySlot.isEmpty = true;
    return emptySlot;
}

std::vector<SaveSlot> SaveSystem::getAllSlots() const {
    std::vector<SaveSlot> slots;

    for (int i = 0; i < m_Config.maxSaveSlots; ++i) {
        slots.push_back(getSlotInfo(i));
    }

    return slots;
}

bool SaveSystem::isSlotValid(int slotNumber) const {
    auto it = m_SaveSlots.find(slotNumber);
    return it != m_SaveSlots.end() && !it->second.isEmpty;
}

int SaveSystem::getFirstEmptySlot() const {
    for (int i = 0; i < m_Config.maxSaveSlots; ++i) {
        if (!isSlotValid(i)) {
            return i;
        }
    }
    return -1;
}

bool SaveSystem::copySlot(int sourceSlot, int destSlot) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!isSlotValid(sourceSlot)) {
        return false;
    }

    if (destSlot < 0 || destSlot >= m_Config.maxSaveSlots) {
        return false;
    }

    try {
        std::string sourcePath = getSlotFilePath(sourceSlot);
        std::string destPath = getSlotFilePath(destSlot);

        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);

        // Update slot info
        m_SaveSlots[destSlot] = m_SaveSlots[sourceSlot];
        m_SaveSlots[destSlot].slotNumber = destSlot;
        m_SaveSlots[destSlot].filePath = destPath;

        return true;
    } catch (...) {
        return false;
    }
}

SaveResult SaveSystem::quickSave() {
    return saveToSlot(QUICK_SAVE_SLOT + m_Config.maxSaveSlots, "Quick Save");
}

LoadResult SaveSystem::quickLoad() {
    return loadFromSlot(QUICK_SAVE_SLOT + m_Config.maxSaveSlots);
}

bool SaveSystem::hasQuickSave() const {
    return isSlotValid(QUICK_SAVE_SLOT + m_Config.maxSaveSlots);
}

bool SaveSystem::createCheckpoint(const std::string& checkpointId, const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    Checkpoint checkpoint;
    checkpoint.id = checkpointId;
    checkpoint.name = name.empty() ? checkpointId : name;
    checkpoint.timestamp = std::chrono::system_clock::now();
    checkpoint.isTemporary = true;

    // Copy current save data to checkpoint
    Serializer serializer(SerializationFormat::JSON);
    serializer.beginObject();

    for (const std::string& section : m_CurrentSaveFile.getSections()) {
        serializer.write(section, m_CurrentSaveFile.getData(section));
    }

    serializer.endObject();
    checkpoint.data["saveData"] = serializer.toString();

    m_Checkpoints[checkpointId] = checkpoint;

    // Cleanup old checkpoints
    clearOldCheckpoints();

    return true;
}

bool SaveSystem::loadCheckpoint(const std::string& checkpointId) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Checkpoints.find(checkpointId);
    if (it == m_Checkpoints.end()) {
        return false;
    }

    const Checkpoint& checkpoint = it->second;

    // Restore checkpoint data
    if (checkpoint.data.find("saveData") != checkpoint.data.end()) {
        Serializer serializer(SerializationFormat::JSON);
        if (serializer.fromString(checkpoint.data.at("saveData"))) {
            for (const std::string& key : serializer.getKeys()) {
                std::string value;
                if (serializer.read(key, value)) {
                    m_CurrentSaveFile.setData(key, value);
                }
            }
        }
    }

    return true;
}

bool SaveSystem::deleteCheckpoint(const std::string& checkpointId) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Checkpoints.erase(checkpointId) > 0;
}

std::vector<Checkpoint> SaveSystem::getAllCheckpoints() const {
    std::vector<Checkpoint> checkpoints;
    for (const auto& [id, checkpoint] : m_Checkpoints) {
        checkpoints.push_back(checkpoint);
    }

    // Sort by timestamp (newest first)
    std::sort(checkpoints.begin(), checkpoints.end(),
              [](const Checkpoint& a, const Checkpoint& b) {
                  return a.timestamp > b.timestamp;
              });

    return checkpoints;
}

void SaveSystem::clearOldCheckpoints() {
    if (m_Checkpoints.size() <= static_cast<size_t>(m_Config.maxCheckpoints)) {
        return;
    }

    // Get sorted checkpoints
    std::vector<Checkpoint> sorted = getAllCheckpoints();

    // Remove oldest ones
    while (sorted.size() > static_cast<size_t>(m_Config.maxCheckpoints)) {
        m_Checkpoints.erase(sorted.back().id);
        sorted.pop_back();
    }
}

void SaveSystem::setData(const std::string& section, const std::string& data) {
    m_CurrentSaveFile.setData(section, data);
}

std::string SaveSystem::getData(const std::string& section) const {
    return m_CurrentSaveFile.getData(section);
}

void SaveSystem::setBinaryData(const std::string& section, const std::vector<uint8_t>& data) {
    m_CurrentSaveFile.setBinaryData(section, data);
}

std::vector<uint8_t> SaveSystem::getBinaryData(const std::string& section) const {
    return m_CurrentSaveFile.getBinaryData(section);
}

void SaveSystem::updateMetadata(const SaveMetadata& metadata) {
    m_CurrentSaveFile.getMetadata() = metadata;
}

bool SaveSystem::createBackup(int slotNumber) {
    if (!isSlotValid(slotNumber)) {
        return false;
    }

    try {
        std::string sourcePath = getSlotFilePath(slotNumber);

        // Find next backup index
        int backupIndex = 0;
        while (fs::exists(getBackupFilePath(slotNumber, backupIndex))) {
            backupIndex++;
        }

        std::string backupPath = getBackupFilePath(slotNumber, backupIndex);

        // Create backup directory if needed
        fs::path backupDir = fs::path(backupPath).parent_path();
        if (!fs::exists(backupDir)) {
            fs::create_directories(backupDir);
        }

        fs::copy_file(sourcePath, backupPath, fs::copy_options::overwrite_existing);

        m_SaveSlots[slotNumber].hasBackup = true;

        return true;
    } catch (...) {
        return false;
    }
}

bool SaveSystem::restoreFromBackup(int slotNumber, int backupIndex) {
    try {
        std::string backupPath = getBackupFilePath(slotNumber, backupIndex);

        if (!fs::exists(backupPath)) {
            return false;
        }

        std::string slotPath = getSlotFilePath(slotNumber);
        fs::copy_file(backupPath, slotPath, fs::copy_options::overwrite_existing);

        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> SaveSystem::getBackups(int slotNumber) const {
    std::vector<std::string> backups;

    int backupIndex = 0;
    while (backupIndex < m_Config.maxBackupsPerSlot) {
        std::string backupPath = getBackupFilePath(slotNumber, backupIndex);
        if (fs::exists(backupPath)) {
            backups.push_back(backupPath);
        }
        backupIndex++;
    }

    return backups;
}

bool SaveSystem::verifySaveIntegrity(int slotNumber) const {
    if (!isSlotValid(slotNumber)) {
        return false;
    }

    std::string filePath = getSlotFilePath(slotNumber);

    SaveFile saveFile;
    return saveFile.load(filePath) && saveFile.validate();
}

bool SaveSystem::recoverCorruptedSave(int slotNumber) {
    // Try to restore from most recent backup
    std::vector<std::string> backups = getBackups(slotNumber);

    for (size_t i = 0; i < backups.size(); ++i) {
        if (restoreFromBackup(slotNumber, static_cast<int>(i))) {
            if (verifySaveIntegrity(slotNumber)) {
                return true;
            }
        }
    }

    return false;
}

void SaveSystem::cleanupOldBackups() {
    for (int slotNumber = 0; slotNumber < m_Config.maxSaveSlots; ++slotNumber) {
        rotateBackups(slotNumber);
    }
}

void SaveSystem::saveAsync(int slotNumber, SaveCallback callback) {
    if (m_IsBusy) {
        if (callback) {
            SaveResult result;
            result.error = "Save operation already in progress";
            callback(result);
        }
        return;
    }

    m_IsBusy = true;

    std::thread([this, slotNumber, callback]() {
        SaveResult result = saveToSlot(slotNumber);

        if (callback) {
            callback(result);
        }

        m_IsBusy = false;
    }).detach();
}

void SaveSystem::loadAsync(int slotNumber, LoadCallback callback) {
    if (m_IsBusy) {
        if (callback) {
            LoadResult result;
            result.error = "Load operation already in progress";
            callback(result);
        }
        return;
    }

    m_IsBusy = true;

    std::thread([this, slotNumber, callback]() {
        LoadResult result = loadFromSlot(slotNumber);

        if (callback) {
            callback(result);
        }

        m_IsBusy = false;
    }).detach();
}

void SaveSystem::waitForCompletion() {
    while (m_IsBusy) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string SaveSystem::getSlotFilePath(int slotNumber) const {
    std::ostringstream oss;
    oss << m_Config.saveDirectory << "/save_" << std::setfill('0') << std::setw(3) << slotNumber << ".sav";
    return oss.str();
}

size_t SaveSystem::getTotalSaveSize() const {
    size_t totalSize = 0;

    for (const auto& [slotNum, slot] : m_SaveSlots) {
        totalSize += slot.fileSize;
    }

    return totalSize;
}

std::string SaveSystem::exportSlotToJson(int slotNumber) const {
    if (!isSlotValid(slotNumber)) {
        return "{}";
    }

    std::string filePath = getSlotFilePath(slotNumber);
    SaveFile saveFile;

    if (saveFile.load(filePath)) {
        return saveFile.exportToJson();
    }

    return "{}";
}

bool SaveSystem::importSlotFromJson(int slotNumber, const std::string& json) {
    SaveFile saveFile;

    if (!saveFile.importFromJson(json)) {
        return false;
    }

    std::string filePath = getSlotFilePath(slotNumber);
    return saveFile.save(filePath);
}

SaveSystem::SaveStats SaveSystem::getSaveStats() const {
    SaveStats stats;
    stats.totalSaves = 0;
    stats.corruptedSaves = 0;
    stats.totalSize = 0;
    stats.oldestSave = std::chrono::system_clock::now();
    stats.newestSave = std::chrono::system_clock::time_point::min();

    for (const auto& [slotNum, slot] : m_SaveSlots) {
        if (!slot.isEmpty) {
            stats.totalSaves++;
            stats.totalSize += slot.fileSize;

            if (slot.isCorrupted) {
                stats.corruptedSaves++;
            }

            if (slot.metadata.timestamp < stats.oldestSave) {
                stats.oldestSave = slot.metadata.timestamp;
            }

            if (slot.metadata.timestamp > stats.newestSave) {
                stats.newestSave = slot.metadata.timestamp;
            }
        }
    }

    return stats;
}

bool SaveSystem::createSaveDirectory() {
    try {
        if (!fs::exists(m_Config.saveDirectory)) {
            fs::create_directories(m_Config.saveDirectory);
        }

        if (!fs::exists(m_Config.backupDirectory)) {
            fs::create_directories(m_Config.backupDirectory);
        }

        return true;
    } catch (...) {
        return false;
    }
}

void SaveSystem::scanSaveDirectory() {
    m_SaveSlots.clear();

    try {
        for (int slotNumber = 0; slotNumber < m_Config.maxSaveSlots; ++slotNumber) {
            std::string filePath = getSlotFilePath(slotNumber);

            if (fs::exists(filePath)) {
                SaveSlot slot;
                slot.slotNumber = slotNumber;
                slot.filePath = filePath;
                slot.fileSize = fs::file_size(filePath);
                slot.isEmpty = false;

                // Try to load metadata
                SaveFile saveFile;
                if (saveFile.load(filePath)) {
                    slot.metadata = saveFile.getMetadata();
                    slot.isCorrupted = !saveFile.validate();
                } else {
                    slot.isCorrupted = true;
                }

                // Check for backups
                slot.hasBackup = !getBackups(slotNumber).empty();

                m_SaveSlots[slotNumber] = slot;
            }
        }
    } catch (...) {
        // Ignore errors during scan
    }
}

std::string SaveSystem::getBackupFilePath(int slotNumber, int backupIndex) const {
    std::ostringstream oss;
    oss << m_Config.backupDirectory << "/save_" << std::setfill('0') << std::setw(3)
        << slotNumber << "_backup_" << backupIndex << ".sav";
    return oss.str();
}

void SaveSystem::rotateBackups(int slotNumber) {
    std::vector<std::string> backups = getBackups(slotNumber);

    // If we have too many backups, delete the oldest ones
    while (backups.size() > static_cast<size_t>(m_Config.maxBackupsPerSlot)) {
        try {
            fs::remove(backups.back());
            backups.pop_back();
        } catch (...) {
            break;
        }
    }
}

void SaveSystem::reportProgress(float progress, const std::string& status) {
    if (m_ProgressCallback) {
        m_ProgressCallback(progress, status);
    }
}

bool SaveSystem::verifyPlatformCompatibility(const SaveMetadata& metadata) const {
    // Allow cross-platform saves but log warning if different
    std::string currentPlatform;

    #ifdef _WIN32
        currentPlatform = "Windows";
    #elif __APPLE__
        currentPlatform = "macOS";
    #elif __linux__
        currentPlatform = "Linux";
    #else
        currentPlatform = "Unknown";
    #endif

    return metadata.platform == currentPlatform || metadata.platform.empty();
}

} // namespace Save
