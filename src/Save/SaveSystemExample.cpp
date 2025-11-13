/**
 * @file SaveSystemExample.cpp
 * @brief Example usage of the save system
 *
 * This file demonstrates how to use the save system, including:
 * - Basic save/load operations
 * - Autosave setup
 * - Cloud sync integration
 * - Custom data serialization
 * - Error handling and recovery
 */

#include "SaveSystem.h"
#include "AutoSave.h"
#include "CloudSync.h"
#include "Serializer.h"

// Example game data structures
struct PlayerData : public Save::ISerializable {
    std::string name;
    int level;
    float health;
    float mana;
    std::vector<int> inventory;

    std::string serialize() const override {
        Save::Serializer serializer(Save::SerializationFormat::JSON);
        serializer.beginObject();
        serializer.write("name", name);
        serializer.write("level", level);
        serializer.write("health", health);
        serializer.write("mana", mana);
        serializer.writeVector("inventory", inventory);
        serializer.endObject();
        return serializer.toString();
    }

    bool deserialize(const std::string& data) override {
        Save::Serializer serializer(Save::SerializationFormat::JSON);
        if (!serializer.fromString(data)) {
            return false;
        }

        serializer.read("name", name);
        serializer.read("level", level);
        serializer.read("health", health);
        serializer.read("mana", mana);
        serializer.readVector("inventory", inventory);

        return true;
    }
};

struct WorldData : public Save::ISerializable {
    std::string currentLocation;
    int daysPassed;
    std::map<std::string, bool> questsCompleted;

    std::string serialize() const override {
        Save::Serializer serializer(Save::SerializationFormat::JSON);
        serializer.beginObject();
        serializer.write("currentLocation", currentLocation);
        serializer.write("daysPassed", daysPassed);
        // For maps, we need to convert to JSON manually or use writeMap
        serializer.endObject();
        return serializer.toString();
    }

    bool deserialize(const std::string& data) override {
        Save::Serializer serializer(Save::SerializationFormat::JSON);
        if (!serializer.fromString(data)) {
            return false;
        }

        serializer.read("currentLocation", currentLocation);
        serializer.read("daysPassed", daysPassed);

        return true;
    }
};

/**
 * @brief Example 1: Basic Save and Load
 */
void exampleBasicSaveLoad() {
    // Initialize save system
    Save::SaveSystem& saveSystem = Save::SaveSystem::getInstance();

    Save::SaveSystem::Config config;
    config.maxSaveSlots = 10;
    config.saveDirectory = "./saves";
    config.enableAutomaticBackup = true;
    config.encryptionLevel = Save::SaveFile::EncryptionLevel::LIGHT;

    if (!saveSystem.initialize(config)) {
        // Handle initialization error
        return;
    }

    // Prepare game data
    PlayerData player;
    player.name = "Hero";
    player.level = 15;
    player.health = 100.0f;
    player.mana = 50.0f;
    player.inventory = {1, 2, 3, 5, 8};

    WorldData world;
    world.currentLocation = "Ancient Forest";
    world.daysPassed = 42;

    // Set save data
    saveSystem.setData("player", player.serialize());
    saveSystem.setData("world", world.serialize());

    // Update metadata
    Save::SaveMetadata& metadata = saveSystem.getMetadata();
    metadata.characterName = player.name;
    metadata.characterLevel = player.level;
    metadata.location = world.currentLocation;

    // Save to slot 0
    Save::SaveResult saveResult = saveSystem.saveToSlot(0, "My First Save");

    if (saveResult.success) {
        printf("Save successful! Saved to: %s\n", saveResult.filePath.c_str());
        printf("Save took: %.2f ms\n", saveResult.saveTimeMs);
    } else {
        printf("Save failed: %s\n", saveResult.error.c_str());
    }

    // Later... load the game
    Save::LoadResult loadResult = saveSystem.loadFromSlot(0);

    if (loadResult.success) {
        printf("Load successful!\n");
        printf("Character: %s (Level %d)\n",
               loadResult.metadata.characterName.c_str(),
               loadResult.metadata.characterLevel);

        // Restore player data
        PlayerData loadedPlayer;
        if (loadedPlayer.deserialize(saveSystem.getData("player"))) {
            printf("Player restored: %s, Health: %.1f\n",
                   loadedPlayer.name.c_str(), loadedPlayer.health);
        }

        if (loadResult.wasMigrated) {
            printf("Save file was migrated to current version\n");
        }
    } else {
        printf("Load failed: %s\n", loadResult.error.c_str());
    }
}

/**
 * @brief Example 2: Autosave System
 */
void exampleAutosave() {
    // Initialize autosave
    Save::AutoSave autoSave;

    Save::AutoSave::Config config;
    config.enabled = true;
    config.intervalSeconds = 300.0f; // Autosave every 5 minutes
    config.maxAutoSaveSlots = 3;     // Keep 3 rotating autosaves
    config.preventDuringCombat = true;
    config.notifyPlayer = true;

    autoSave.initialize(config);

    // Set notification callback
    autoSave.setNotificationCallback([](const std::string& message, bool isError) {
        if (isError) {
            printf("[ERROR] %s\n", message.c_str());
        } else {
            printf("[INFO] %s\n", message.c_str());
        }
    });

    // In your game loop:
    float deltaTime = 0.016f; // ~60 FPS
    autoSave.update(deltaTime);

    // Block autosave during combat
    autoSave.setInCombat(true);  // Autosave will be blocked
    // ... combat happens ...
    autoSave.setInCombat(false); // Autosave re-enabled

    // Manual autosave trigger (e.g., on quest complete)
    autoSave.triggerAutoSave(Save::AutoSave::TriggerType::PROGRESS_BASED, "Quest Complete");

    // RAII-style blocking (automatically unblocks when scope ends)
    {
        Save::AutoSaveBlocker blocker(autoSave, "Critical Section");
        // Autosave blocked in this scope
    }
    // Autosave automatically unblocked here

    // Load most recent autosave
    Save::LoadResult result = autoSave.loadMostRecentAutoSave();
    if (result.success) {
        printf("Autosave loaded successfully\n");
    }
}

/**
 * @brief Example 3: Cloud Sync
 */
void exampleCloudSync() {
    // Initialize cloud sync
    Save::CloudSync cloudSync;

    Save::CloudSync::Config config;
    config.provider = Save::CloudProvider::STEAM_CLOUD;
    config.enableAutoSync = true;
    config.syncIntervalSeconds = 600.0f; // Sync every 10 minutes
    config.syncOnStartup = true;
    config.syncOnQuit = true;
    config.conflictResolution = Save::ConflictResolution::USE_NEWEST;

    cloudSync.setSteamAppId(480); // Your Steam App ID

    if (!cloudSync.initialize(config)) {
        printf("Cloud sync not available\n");
        return;
    }

    // Set progress callback
    cloudSync.setProgressCallback([](float progress, const std::string& status) {
        printf("Cloud sync: %.0f%% - %s\n", progress * 100.0f, status.c_str());
    });

    // Sync all saves with cloud
    Save::CloudSyncResult syncResult = cloudSync.syncAll();

    if (syncResult.success) {
        printf("Cloud sync successful!\n");
        printf("Uploaded: %d files\n", syncResult.filesUploaded);
        printf("Downloaded: %d files\n", syncResult.filesDownloaded);
        printf("Conflicts resolved: %d\n", syncResult.conflictsResolved);
    }

    // Check cloud storage usage
    size_t used = cloudSync.getCloudStorageUsed();
    size_t available = cloudSync.getCloudStorageAvailable();
    printf("Cloud storage: %zu / %zu bytes used\n", used, used + available);

    // Handle conflicts manually
    if (cloudSync.hasConflicts()) {
        auto conflicts = cloudSync.getConflicts();
        for (const auto& conflict : conflicts) {
            printf("Conflict: %s\n", conflict.fileName.c_str());

            // Resolve using newest version
            cloudSync.resolveConflict(conflict.fileName,
                                     Save::ConflictResolution::USE_NEWEST);
        }
    }

    // Async sync with callback
    cloudSync.syncAsync([](const Save::CloudSyncResult& result) {
        printf("Async sync completed: %s\n",
               result.success ? "success" : result.error.c_str());
    });
}

/**
 * @brief Example 4: Advanced Features
 */
void exampleAdvancedFeatures() {
    Save::SaveSystem& saveSystem = Save::SaveSystem::getInstance();

    // --- Checkpoint System ---
    // Create checkpoint before risky action
    saveSystem.createCheckpoint("before_boss_fight", "Boss Fight Checkpoint");

    // If player dies, restore checkpoint
    if (saveSystem.loadCheckpoint("before_boss_fight")) {
        printf("Checkpoint restored\n");
    }

    // --- Multiple Save Slots ---
    // Get all save slots
    auto slots = saveSystem.getAllSlots();
    for (const auto& slot : slots) {
        if (!slot.isEmpty) {
            printf("Slot %d: %s (Level %d) - %s\n",
                   slot.slotNumber,
                   slot.metadata.characterName.c_str(),
                   slot.metadata.characterLevel,
                   slot.metadata.location.c_str());

            if (slot.isCorrupted) {
                printf("  WARNING: Corrupted!\n");
            }
        }
    }

    // --- Copy Save Slot ---
    saveSystem.copySlot(0, 1); // Copy slot 0 to slot 1

    // --- Backup and Recovery ---
    // Create manual backup
    saveSystem.createBackup(0);

    // Verify save integrity
    if (!saveSystem.verifySaveIntegrity(0)) {
        printf("Save file corrupted! Attempting recovery...\n");

        // Attempt recovery from backup
        if (saveSystem.recoverCorruptedSave(0)) {
            printf("Save recovered successfully!\n");
        }
    }

    // --- Export/Import ---
    // Export save to JSON for debugging/editing
    std::string jsonSave = saveSystem.exportSlotToJson(0);
    printf("Save exported to JSON:\n%s\n", jsonSave.c_str());

    // Import modified save
    // saveSystem.importSlotFromJson(1, jsonSave);

    // --- Statistics ---
    auto stats = saveSystem.getSaveStats();
    printf("Save Statistics:\n");
    printf("  Total saves: %d\n", stats.totalSaves);
    printf("  Corrupted: %d\n", stats.corruptedSaves);
    printf("  Total size: %zu bytes\n", stats.totalSize);

    // --- Async Save with Callback ---
    saveSystem.saveAsync(0, [](const Save::SaveResult& result) {
        if (result.success) {
            printf("Async save completed in %.2f ms\n", result.saveTimeMs);
        } else {
            printf("Async save failed: %s\n", result.error.c_str());
        }
    });

    // Wait for completion if needed
    saveSystem.waitForCompletion();
}

/**
 * @brief Example 5: Binary Serialization for Performance
 */
void exampleBinarySerialization() {
    Save::SaveSystem& saveSystem = Save::SaveSystem::getInstance();

    // Large data structure (e.g., map data)
    std::vector<uint8_t> mapData(1024 * 1024); // 1MB of map data

    // Fill with data
    for (size_t i = 0; i < mapData.size(); ++i) {
        mapData[i] = static_cast<uint8_t>(i % 256);
    }

    // Save as binary (more compact and faster than JSON)
    saveSystem.setBinaryData("mapData", mapData);

    // Save to slot
    Save::SaveResult result = saveSystem.saveToSlot(0, "Binary Data Example");

    printf("Binary save size: %s\n",
           saveSystem.getCurrentSaveFile().getFileSizeString().c_str());

    // Load and restore
    saveSystem.loadFromSlot(0);
    std::vector<uint8_t> loadedMapData = saveSystem.getBinaryData("mapData");

    printf("Loaded %zu bytes of map data\n", loadedMapData.size());
}

/**
 * @brief Example 6: Complete Save/Load Integration
 */
class GameSaveManager {
public:
    GameSaveManager() {
        // Initialize all save systems
        Save::SaveSystem::Config saveConfig;
        saveConfig.maxSaveSlots = 10;
        saveConfig.enableAutomaticBackup = true;
        m_SaveSystem.initialize(saveConfig);

        Save::AutoSave::Config autoConfig;
        autoConfig.intervalSeconds = 300.0f;
        m_AutoSave.initialize(autoConfig);

        Save::CloudSync::Config cloudConfig;
        cloudConfig.provider = Save::CloudProvider::STEAM_CLOUD;
        m_CloudSync.initialize(cloudConfig);

        // Connect autosave to save system
        m_AutoSave.setNotificationCallback([](const std::string& msg, bool isError) {
            printf("[AutoSave] %s\n", msg.c_str());
        });
    }

    void update(float deltaTime) {
        m_AutoSave.update(deltaTime);
        m_CloudSync.update(deltaTime);
    }

    bool saveGame(int slotNumber, const std::string& saveName) {
        // Prepare all game data
        prepareGameData();

        // Save
        Save::SaveResult result = m_SaveSystem.saveToSlot(slotNumber, saveName);

        if (result.success && m_CloudSync.isAvailable()) {
            // Upload to cloud
            m_CloudSync.uploadSave(result.filePath);
        }

        return result.success;
    }

    bool loadGame(int slotNumber) {
        Save::LoadResult result = m_SaveSystem.loadFromSlot(slotNumber);

        if (result.success) {
            restoreGameData();
            return true;
        }

        return false;
    }

    void onQuit() {
        // Save and quit with autosave
        m_AutoSave.saveAndQuit();

        // Final cloud sync
        m_CloudSync.syncAll();
    }

private:
    void prepareGameData() {
        // Serialize all game systems
        PlayerData player;
        // ... fill player data ...
        m_SaveSystem.setData("player", player.serialize());

        WorldData world;
        // ... fill world data ...
        m_SaveSystem.setData("world", world.serialize());

        // Update metadata
        Save::SaveMetadata& metadata = m_SaveSystem.getMetadata();
        metadata.characterName = player.name;
        metadata.characterLevel = player.level;
    }

    void restoreGameData() {
        // Deserialize all game systems
        PlayerData player;
        player.deserialize(m_SaveSystem.getData("player"));

        WorldData world;
        world.deserialize(m_SaveSystem.getData("world"));

        // Restore game state
        // ...
    }

    Save::SaveSystem& m_SaveSystem = Save::SaveSystem::getInstance();
    Save::AutoSave m_AutoSave;
    Save::CloudSync m_CloudSync;
};

/**
 * @brief Main function demonstrating usage
 */
int main() {
    printf("=== Save System Examples ===\n\n");

    printf("Example 1: Basic Save/Load\n");
    exampleBasicSaveLoad();
    printf("\n");

    printf("Example 2: Autosave System\n");
    exampleAutosave();
    printf("\n");

    printf("Example 3: Cloud Sync\n");
    exampleCloudSync();
    printf("\n");

    printf("Example 4: Advanced Features\n");
    exampleAdvancedFeatures();
    printf("\n");

    printf("Example 5: Binary Serialization\n");
    exampleBinarySerialization();
    printf("\n");

    printf("Example 6: Complete Integration\n");
    GameSaveManager gameManager;
    gameManager.saveGame(0, "Test Save");
    gameManager.loadGame(0);
    gameManager.onQuit();
    printf("\n");

    return 0;
}
