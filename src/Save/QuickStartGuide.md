# Save System Quick Start Guide

## 5-Minute Integration

### Step 1: Include Headers (30 seconds)

```cpp
#include "Save/SaveSystem.h"
#include "Save/AutoSave.h"
#include "Save/CloudSync.h"
#include "Save/Serializer.h"
```

### Step 2: Initialize (1 minute)

```cpp
class Game {
private:
    Save::SaveSystem& m_SaveSystem = Save::SaveSystem::getInstance();
    Save::AutoSave m_AutoSave;
    Save::CloudSync m_CloudSync;

public:
    void initialize() {
        // Configure save system
        Save::SaveSystem::Config saveConfig;
        saveConfig.maxSaveSlots = 10;
        saveConfig.saveDirectory = "./saves";
        m_SaveSystem.initialize(saveConfig);

        // Configure autosave
        Save::AutoSave::Config autoConfig;
        autoConfig.intervalSeconds = 300.0f; // 5 minutes
        m_AutoSave.initialize(autoConfig);

        // Configure cloud sync
        Save::CloudSync::Config cloudConfig;
        cloudConfig.provider = Save::CloudProvider::STEAM_CLOUD;
        m_CloudSync.initialize(cloudConfig);
    }

    void update(float deltaTime) {
        m_AutoSave.update(deltaTime);
        m_CloudSync.update(deltaTime);
    }
};
```

### Step 3: Make Your Data Serializable (2 minutes)

```cpp
struct GameData : public Save::ISerializable {
    // Your game data
    std::string playerName;
    int playerLevel;
    float playerHealth;
    std::vector<int> inventory;

    // Serialize
    std::string serialize() const override {
        Save::Serializer s(Save::SerializationFormat::JSON);
        s.beginObject();
        SERIALIZE_FIELD(s, playerName);
        SERIALIZE_FIELD(s, playerLevel);
        SERIALIZE_FIELD(s, playerHealth);
        SERIALIZE_VECTOR(s, inventory);
        s.endObject();
        return s.toString();
    }

    // Deserialize
    bool deserialize(const std::string& data) override {
        Save::Serializer s(Save::SerializationFormat::JSON);
        if (!s.fromString(data)) return false;

        DESERIALIZE_FIELD(s, playerName);
        DESERIALIZE_FIELD(s, playerLevel);
        DESERIALIZE_FIELD(s, playerHealth);
        DESERIALIZE_VECTOR(s, inventory);
        return true;
    }
};
```

### Step 4: Save and Load (1 minute)

```cpp
class Game {
    GameData m_GameData;

    void saveGame(int slot) {
        // Serialize game data
        m_SaveSystem.setData("gameData", m_GameData.serialize());

        // Update metadata
        Save::SaveMetadata& meta = m_SaveSystem.getMetadata();
        meta.characterName = m_GameData.playerName;
        meta.characterLevel = m_GameData.playerLevel;

        // Save
        Save::SaveResult result = m_SaveSystem.saveToSlot(slot, "My Save");

        if (result.success) {
            showMessage("Game saved!");
        } else {
            showError("Save failed: " + result.error);
        }
    }

    void loadGame(int slot) {
        // Load
        Save::LoadResult result = m_SaveSystem.loadFromSlot(slot);

        if (result.success) {
            // Deserialize game data
            m_GameData.deserialize(m_SaveSystem.getData("gameData"));
            showMessage("Game loaded!");
        } else {
            showError("Load failed: " + result.error);
        }
    }
};
```

### Step 5: Add Autosave Hooks (30 seconds)

```cpp
void onLevelUp() {
    m_AutoSave.onLevelUp();
}

void onQuestComplete(const std::string& questName) {
    m_AutoSave.onQuestComplete(questName);
}

void onEnterCombat() {
    m_AutoSave.setInCombat(true);
}

void onExitCombat() {
    m_AutoSave.setInCombat(false);
}
```

## Done!

You now have:
- ✅ Multiple save slots
- ✅ Autosave every 5 minutes
- ✅ Autosave on level up and quest complete
- ✅ Autosave blocked during combat
- ✅ Cloud sync (if Steam Cloud enabled)
- ✅ Automatic backups
- ✅ Corruption recovery

## Common Operations

### Quick Save/Load

```cpp
// Quick save
m_SaveSystem.quickSave();

// Quick load
m_SaveSystem.quickLoad();
```

### List All Saves

```cpp
auto slots = m_SaveSystem.getAllSlots();
for (const auto& slot : slots) {
    if (!slot.isEmpty) {
        printf("Slot %d: %s (Level %d)\n",
               slot.slotNumber,
               slot.metadata.characterName.c_str(),
               slot.metadata.characterLevel);
    }
}
```

### Delete Save

```cpp
m_SaveSystem.deleteSlot(slotNumber);
```

### Verify Integrity

```cpp
if (!m_SaveSystem.verifySaveIntegrity(slotNumber)) {
    m_SaveSystem.recoverCorruptedSave(slotNumber);
}
```

### Cloud Sync

```cpp
// Sync all
m_CloudSync.syncAll();

// Check conflicts
if (m_CloudSync.hasConflicts()) {
    m_CloudSync.resolveAllConflicts();
}
```

## Advanced Features

### Checkpoints

```cpp
// Create checkpoint before boss fight
m_SaveSystem.createCheckpoint("before_boss");

// If player dies, restore
if (playerDied) {
    m_SaveSystem.loadCheckpoint("before_boss");
}
```

### Binary Data (Large Objects)

```cpp
// Save map data (binary is faster for large data)
std::vector<uint8_t> mapData = generateMapData();
m_SaveSystem.setBinaryData("mapData", mapData);

// Load map data
std::vector<uint8_t> loadedMap = m_SaveSystem.getBinaryData("mapData");
```

### Async Operations

```cpp
// Save asynchronously (doesn't block gameplay)
m_SaveSystem.saveAsync(slot, [](const Save::SaveResult& result) {
    if (result.success) {
        printf("Background save completed!\n");
    }
});
```

### Progress Callbacks

```cpp
m_SaveSystem.setProgressCallback([](float progress, const std::string& status) {
    updateProgressBar(progress);
    showStatusText(status);
});
```

## Troubleshooting

### Save Failed

```cpp
Save::SaveResult result = m_SaveSystem.saveToSlot(slot);
if (!result.success) {
    // Check error message
    printf("Error: %s\n", result.error.c_str());

    // Try another slot
    int emptySlot = m_SaveSystem.getFirstEmptySlot();
    if (emptySlot >= 0) {
        m_SaveSystem.saveToSlot(emptySlot);
    }
}
```

### Load Failed - Corrupted Save

```cpp
Save::LoadResult result = m_SaveSystem.loadFromSlot(slot);
if (!result.success) {
    // Try to recover
    if (m_SaveSystem.recoverCorruptedSave(slot)) {
        // Retry load
        result = m_SaveSystem.loadFromSlot(slot);
    }
}
```

### Cloud Sync Issues

```cpp
if (!m_CloudSync.isAvailable()) {
    printf("Cloud sync not available\n");
    // Continue without cloud sync
}

// Check storage
if (m_CloudSync.getCloudStorageAvailable() < 1024) {
    printf("Cloud storage almost full!\n");
}
```

## Best Practices

1. **Save often**: Enable autosave with reasonable intervals
2. **Show feedback**: Use callbacks to show save/load progress
3. **Handle errors**: Always check result.success
4. **Use encryption**: Enable at least Light encryption
5. **Test recovery**: Regularly test backup/recovery system
6. **Verify integrity**: Check saves after loading
7. **Cloud sync**: Enable for better player experience

## Performance Tips

1. Use **binary format** for large data
2. Enable **compression** for network saves
3. Use **async operations** to avoid blocking
4. **Batch changes** before saving
5. **Cache serialized data** when possible

## Complete Example

See `/home/user/codetest/src/Save/SaveSystemExample.cpp` for complete working examples!

## Need Help?

- Check `/home/user/codetest/src/Save/README.md` for full documentation
- See `/home/user/codetest/src/Save/IMPLEMENTATION_SUMMARY.md` for architecture details
- Run examples in `SaveSystemExample.cpp`

---

**That's it!** You now have a production-ready save system in your game.
