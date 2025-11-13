# Save System Documentation

A robust, feature-rich save/load system with cloud sync capability for game development.

## Features

### Core Features
- **Multiple Save Slots**: Support for up to 10 (configurable) save slots
- **Autosave System**: Automatic saves at intervals and on events
- **Save File Serialization**: Both JSON (human-readable) and binary (compact) formats
- **Cloud Save Integration**: Steam Cloud ready, with support for other platforms
- **Save File Versioning**: Automatic migration of old save files
- **Checkpoint System**: Quick save points for temporary progress
- **Backup and Recovery**: Automatic backups with corruption detection and recovery
- **Cross-Platform Compatibility**: Save files work across Windows, macOS, and Linux

### Security Features
- **Encryption**: Multiple encryption levels (None, Light, Standard, Paranoid)
- **Checksums**: CRC32-style integrity verification
- **Corruption Detection**: Automatic detection and recovery
- **Secure Deletion**: Sensitive data cleared securely

### Performance Features
- **Fast Saves**: Optimized serialization and compression
- **Compact Storage**: Compression support for smaller file sizes
- **Async Operations**: Non-blocking save/load operations
- **Background Sync**: Cloud sync in background without interrupting gameplay

## File Structure

```
src/Save/
├── SaveFile.h/cpp          # Save file format with versioning
├── Serializer.h/cpp        # Object serialization utilities
├── SaveSystem.h/cpp        # Main save manager
├── AutoSave.h/cpp          # Autosave manager
├── CloudSync.h/cpp         # Cloud save sync
├── SaveSystemExample.cpp   # Usage examples
└── README.md              # This file
```

## Quick Start

### 1. Initialize the Save System

```cpp
#include "SaveSystem.h"

Save::SaveSystem& saveSystem = Save::SaveSystem::getInstance();

Save::SaveSystem::Config config;
config.maxSaveSlots = 10;
config.saveDirectory = "./saves";
config.enableAutomaticBackup = true;
config.encryptionLevel = Save::SaveFile::EncryptionLevel::LIGHT;

saveSystem.initialize(config);
```

### 2. Save Game Data

```cpp
// Prepare your game data
PlayerData player;
player.name = "Hero";
player.level = 15;

// Set save data
saveSystem.setData("player", player.serialize());

// Update metadata
Save::SaveMetadata& metadata = saveSystem.getMetadata();
metadata.characterName = player.name;
metadata.characterLevel = player.level;

// Save to slot
Save::SaveResult result = saveSystem.saveToSlot(0, "My Save");

if (result.success) {
    printf("Save successful!\n");
}
```

### 3. Load Game Data

```cpp
Save::LoadResult result = saveSystem.loadFromSlot(0);

if (result.success) {
    // Restore game data
    PlayerData player;
    player.deserialize(saveSystem.getData("player"));

    printf("Loaded: %s (Level %d)\n",
           player.name.c_str(), player.level);
}
```

## Component Documentation

### SaveFile

The `SaveFile` class handles the low-level save file format.

**Features:**
- Binary header with magic number
- Version information
- Encryption (XOR, AES-256)
- Compression (zlib compatible)
- Checksum verification
- Cross-platform compatibility

**Usage:**
```cpp
Save::SaveFile saveFile;
saveFile.initialize("My Save");
saveFile.setEncryption(Save::SaveFile::EncryptionLevel::STANDARD, "mykey");
saveFile.setCompression(Save::SaveFile::CompressionLevel::BALANCED);

saveFile.setData("player", playerData);
saveFile.save("save_001.sav");

// Later...
saveFile.load("save_001.sav");
if (saveFile.validate()) {
    std::string data = saveFile.getData("player");
}
```

### Serializer

The `Serializer` class provides utilities for converting objects to/from strings.

**Supported Formats:**
- JSON (human-readable, good for debugging)
- Binary (compact, good for performance)

**Usage:**
```cpp
// Writing
Save::Serializer serializer(Save::SerializationFormat::JSON);
serializer.beginObject();
serializer.write("name", "Hero");
serializer.write("level", 15);
serializer.write("health", 100.0f);
serializer.writeVector("inventory", items);
serializer.endObject();

std::string json = serializer.toString();

// Reading
Save::Serializer deserializer(Save::SerializationFormat::JSON);
deserializer.fromString(json);

std::string name;
int level;
float health;
deserializer.read("name", name);
deserializer.read("level", level);
deserializer.read("health", health);
```

### SaveSystem

The `SaveSystem` class is the main interface for save operations.

**Key Methods:**
- `saveToSlot(slotNumber, saveName)` - Save to specific slot
- `loadFromSlot(slotNumber)` - Load from specific slot
- `quickSave()` / `quickLoad()` - Quick save/load operations
- `createCheckpoint(id)` - Create temporary checkpoint
- `createBackup(slotNumber)` - Manual backup creation
- `verifySaveIntegrity(slotNumber)` - Check for corruption

**Async Operations:**
```cpp
saveSystem.saveAsync(0, [](const Save::SaveResult& result) {
    if (result.success) {
        printf("Save completed in %.2f ms\n", result.saveTimeMs);
    }
});
```

### AutoSave

The `AutoSave` class manages automatic saves.

**Configuration:**
```cpp
Save::AutoSave autoSave;

Save::AutoSave::Config config;
config.enabled = true;
config.intervalSeconds = 300.0f;  // 5 minutes
config.maxAutoSaveSlots = 3;
config.preventDuringCombat = true;

autoSave.initialize(config);
```

**Update in Game Loop:**
```cpp
void gameLoop() {
    float deltaTime = getDeltaTime();
    autoSave.update(deltaTime);
}
```

**Event Triggers:**
```cpp
autoSave.onLevelUp();
autoSave.onQuestComplete("MainQuest1");
autoSave.onEnterSafeZone("Town");
```

**Blocking Autosave:**
```cpp
// Manual blocking
autoSave.setInCombat(true);  // Blocks autosave
// ... combat ...
autoSave.setInCombat(false); // Unblocks

// RAII-style blocking (automatic)
{
    Save::AutoSaveBlocker blocker(autoSave, "Boss Fight");
    // Autosave blocked in this scope
}
// Automatically unblocked here
```

### CloudSync

The `CloudSync` class handles cloud save synchronization.

**Supported Platforms:**
- Steam Cloud (ready to use with Steam SDK)
- Epic Games Store Cloud
- GOG Galaxy Cloud
- Xbox Live
- PlayStation Network
- Custom implementations

**Configuration:**
```cpp
Save::CloudSync cloudSync;

Save::CloudSync::Config config;
config.provider = Save::CloudProvider::STEAM_CLOUD;
config.enableAutoSync = true;
config.syncIntervalSeconds = 600.0f;  // 10 minutes
config.syncOnStartup = true;
config.syncOnQuit = true;
config.conflictResolution = Save::ConflictResolution::USE_NEWEST;

cloudSync.initialize(config);
```

**Sync Operations:**
```cpp
// Sync all saves
Save::CloudSyncResult result = cloudSync.syncAll();

// Upload specific file
cloudSync.uploadSave("save_001.sav");

// Download specific file
cloudSync.downloadSave("save_001.sav");

// Async sync
cloudSync.syncAsync([](const Save::CloudSyncResult& result) {
    printf("Synced: %d up, %d down\n",
           result.filesUploaded, result.filesDownloaded);
});
```

**Conflict Resolution:**
```cpp
if (cloudSync.hasConflicts()) {
    auto conflicts = cloudSync.getConflicts();

    for (const auto& conflict : conflicts) {
        // Resolve using newest version
        cloudSync.resolveConflict(
            conflict.fileName,
            Save::ConflictResolution::USE_NEWEST
        );
    }
}
```

## Advanced Features

### Custom Serialization

Implement `ISerializable` for your game objects:

```cpp
struct PlayerData : public Save::ISerializable {
    std::string name;
    int level;

    std::string serialize() const override {
        Save::Serializer s(Save::SerializationFormat::JSON);
        s.beginObject();
        s.write("name", name);
        s.write("level", level);
        s.endObject();
        return s.toString();
    }

    bool deserialize(const std::string& data) override {
        Save::Serializer s(Save::SerializationFormat::JSON);
        if (!s.fromString(data)) return false;

        s.read("name", name);
        s.read("level", level);
        return true;
    }
};
```

### Binary Data for Large Objects

Use binary serialization for large data (maps, textures, etc.):

```cpp
// Save large binary data
std::vector<uint8_t> mapData = generateMapData();
saveSystem.setBinaryData("mapData", mapData);

// Load binary data
std::vector<uint8_t> loadedMap = saveSystem.getBinaryData("mapData");
```

### Save File Migration

Implement custom migration logic:

```cpp
// SaveFile.cpp
bool SaveFile::migrateFrom1_0_0() {
    // Convert old format to new format
    std::string oldData = getData("oldSection");

    // Transform data
    std::string newData = transformOldToNew(oldData);

    setData("newSection", newData);
    return true;
}
```

### Progress Callbacks

Monitor save/load progress:

```cpp
saveSystem.setProgressCallback([](float progress, const std::string& status) {
    printf("Progress: %.0f%% - %s\n", progress * 100.0f, status.c_str());
});
```

## Best Practices

### 1. Initialize Early
Initialize the save system at application startup:

```cpp
int main() {
    Save::SaveSystem::getInstance().initialize();
    // ... game code ...
}
```

### 2. Save Regularly
Use autosave for regular saves, manual saves for important milestones:

```cpp
autoSave.setInterval(300.0f);  // Autosave every 5 minutes
// Manual save on quest complete
```

### 3. Handle Errors
Always check save/load results:

```cpp
Save::SaveResult result = saveSystem.saveToSlot(0);
if (!result.success) {
    showErrorMessage(result.error);
    // Maybe try a different slot or warn the user
}
```

### 4. Verify Integrity
Periodically verify save file integrity:

```cpp
if (!saveSystem.verifySaveIntegrity(currentSlot)) {
    // Attempt recovery
    saveSystem.recoverCorruptedSave(currentSlot);
}
```

### 5. Use Encryption
Enable encryption for sensitive data:

```cpp
config.encryptionLevel = Save::SaveFile::EncryptionLevel::STANDARD;
```

### 6. Cloud Sync
Enable cloud sync for better player experience:

```cpp
cloudSync.setConfig(config);
cloudSync.setEnabled(true);
```

### 7. Test Cross-Platform
Test save files on all target platforms to ensure compatibility.

## Performance Considerations

### Save Times
Typical save times (depends on data size):
- Small save (< 1 MB): 10-50 ms
- Medium save (1-10 MB): 50-200 ms
- Large save (> 10 MB): 200-1000 ms

### Optimization Tips
1. **Use Binary Format**: Binary serialization is 2-5x faster than JSON
2. **Enable Compression**: Reduces file size by 30-70%
3. **Use Async Operations**: Don't block gameplay during saves
4. **Batch Updates**: Group multiple changes before saving
5. **Cache Serialized Data**: Serialize once, save multiple times

## Troubleshooting

### Save File Corrupted
```cpp
if (!saveSystem.verifySaveIntegrity(slot)) {
    // Try to recover from backup
    if (saveSystem.recoverCorruptedSave(slot)) {
        printf("Save recovered!\n");
    } else {
        printf("Recovery failed. Save lost.\n");
    }
}
```

### Cloud Sync Conflicts
```cpp
if (cloudSync.hasConflicts()) {
    // Manual resolution
    cloudSync.resolveAllConflicts();

    // Or use custom callback
    cloudSync.setConflictCallback([](const Save::CloudFileInfo& info) {
        // Show UI to let player choose
        return Save::ConflictResolution::USE_NEWEST;
    });
}
```

### Migration Fails
```cpp
if (loadResult.success && saveFile.needsMigration()) {
    // Create backup before migration
    saveSystem.createBackup(slot);

    if (!saveFile.migrate()) {
        // Restore from backup
        saveSystem.restoreFromBackup(slot);
    }
}
```

## Steam Integration

### Setup Steam Cloud

1. **Enable Steam Cloud in Steamworks**:
   - Go to Steamworks Dashboard
   - Navigate to "Technical Settings"
   - Enable "Steam Cloud"
   - Set quota (e.g., 100 MB)

2. **Configure in Code**:
```cpp
cloudSync.setProvider(Save::CloudProvider::STEAM_CLOUD);
cloudSync.setSteamAppId(YOUR_APP_ID);
cloudSync.initialize();
```

3. **Link with Steam SDK**:
```cmake
# CMakeLists.txt
find_package(SteamSDK REQUIRED)
target_link_libraries(YourGame PRIVATE SteamSDK::steam_api)
```

## Examples

See `SaveSystemExample.cpp` for complete working examples of:
- Basic save/load operations
- Autosave system usage
- Cloud sync integration
- Custom data serialization
- Error handling and recovery
- Complete game integration

## API Reference

### SaveSystem

| Method | Description |
|--------|-------------|
| `initialize(config)` | Initialize save system |
| `saveToSlot(slot, name)` | Save to specific slot |
| `loadFromSlot(slot)` | Load from specific slot |
| `deleteSlot(slot)` | Delete save slot |
| `quickSave()` | Quick save to dedicated slot |
| `quickLoad()` | Quick load from dedicated slot |
| `createCheckpoint(id)` | Create temporary checkpoint |
| `loadCheckpoint(id)` | Load checkpoint |
| `createBackup(slot)` | Create backup |
| `restoreFromBackup(slot)` | Restore from backup |

### AutoSave

| Method | Description |
|--------|-------------|
| `initialize(config)` | Initialize autosave |
| `update(deltaTime)` | Update (call every frame) |
| `triggerAutoSave()` | Manual autosave trigger |
| `setInCombat(bool)` | Block during combat |
| `setInCutscene(bool)` | Block during cutscene |
| `onLevelUp()` | Autosave on level up |
| `onQuestComplete()` | Autosave on quest complete |

### CloudSync

| Method | Description |
|--------|-------------|
| `initialize(config)` | Initialize cloud sync |
| `syncAll()` | Sync all saves |
| `uploadSave(path)` | Upload specific save |
| `downloadSave(path)` | Download specific save |
| `hasConflicts()` | Check for conflicts |
| `resolveConflict()` | Resolve conflict |

## License

This save system is part of the game engine project and follows the same license.

## Support

For issues, questions, or contributions, please refer to the main project documentation.
