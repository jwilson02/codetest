#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <memory>
#include <chrono>

namespace Save {

/**
 * @brief Save file version for migration support
 */
struct SaveVersion {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;

    SaveVersion(uint16_t maj = 1, uint16_t min = 0, uint16_t p = 0)
        : major(maj), minor(min), patch(p) {}

    bool operator==(const SaveVersion& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    bool operator<(const SaveVersion& other) const {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        return patch < other.patch;
    }

    bool operator>(const SaveVersion& other) const {
        return other < *this;
    }

    std::string toString() const;
    static SaveVersion fromString(const std::string& str);
};

/**
 * @brief Save file metadata
 */
struct SaveMetadata {
    std::string saveName;
    std::string characterName;
    int characterLevel;
    std::string location;
    std::chrono::system_clock::time_point timestamp;
    uint64_t playtimeSeconds;
    SaveVersion version;
    std::string platform; // For cross-platform compatibility tracking

    // Quick preview data
    int health;
    int maxHealth;
    std::string characterClass;
    std::string lastQuest;

    SaveMetadata();
    std::string serialize() const;
    bool deserialize(const std::string& data);
};

/**
 * @brief Save file format with versioning and integrity checking
 */
class SaveFile {
public:
    /**
     * @brief Encryption level for save files
     */
    enum class EncryptionLevel {
        NONE,      // No encryption (faster, less secure)
        LIGHT,     // XOR encryption (fast, basic security)
        STANDARD,  // AES-256 encryption (balanced)
        PARANOID   // AES-256 + RSA signing (slowest, most secure)
    };

    /**
     * @brief Compression level
     */
    enum class CompressionLevel {
        NONE,
        FAST,
        BALANCED,
        BEST
    };

    SaveFile();
    ~SaveFile();

    /**
     * @brief Initialize a new save file
     */
    void initialize(const std::string& saveName);

    /**
     * @brief Load save file from disk
     * @param filePath Path to save file
     * @return True if loaded successfully
     */
    bool load(const std::string& filePath);

    /**
     * @brief Save to disk
     * @param filePath Path to save file
     * @return True if saved successfully
     */
    bool save(const std::string& filePath);

    /**
     * @brief Validate save file integrity
     * @return True if file is valid and not corrupted
     */
    bool validate() const;

    /**
     * @brief Calculate checksum for data integrity
     */
    uint32_t calculateChecksum() const;

    /**
     * @brief Get metadata
     */
    SaveMetadata& getMetadata() { return m_Metadata; }
    const SaveMetadata& getMetadata() const { return m_Metadata; }

    /**
     * @brief Set/get save data sections
     */
    void setData(const std::string& section, const std::string& data);
    std::string getData(const std::string& section) const;
    bool hasSection(const std::string& section) const;
    std::vector<std::string> getSections() const;

    /**
     * @brief Binary data support for compressed/efficient storage
     */
    void setBinaryData(const std::string& section, const std::vector<uint8_t>& data);
    std::vector<uint8_t> getBinaryData(const std::string& section) const;

    /**
     * @brief Clear all data
     */
    void clear();

    /**
     * @brief Get save file size
     */
    size_t getSize() const;

    /**
     * @brief Encryption settings
     */
    void setEncryption(EncryptionLevel level, const std::string& key = "");
    EncryptionLevel getEncryptionLevel() const { return m_EncryptionLevel; }

    /**
     * @brief Compression settings
     */
    void setCompression(CompressionLevel level) { m_CompressionLevel = level; }
    CompressionLevel getCompressionLevel() const { return m_CompressionLevel; }

    /**
     * @brief Get current save file version
     */
    static SaveVersion getCurrentVersion();

    /**
     * @brief Check if migration is needed
     */
    bool needsMigration() const;

    /**
     * @brief Migrate save file to current version
     */
    bool migrate();

    /**
     * @brief Create a backup of this save
     */
    bool createBackup(const std::string& backupPath) const;

    /**
     * @brief Export to JSON format (for debugging/editing)
     */
    std::string exportToJson() const;

    /**
     * @brief Import from JSON format
     */
    bool importFromJson(const std::string& json);

    /**
     * @brief Get human-readable file size
     */
    std::string getFileSizeString() const;

private:
    /**
     * @brief Encrypt data
     */
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) const;

    /**
     * @brief Decrypt data
     */
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) const;

    /**
     * @brief Compress data
     */
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) const;

    /**
     * @brief Decompress data
     */
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) const;

    /**
     * @brief Write binary header
     */
    void writeHeader(std::vector<uint8_t>& buffer) const;

    /**
     * @brief Read binary header
     */
    bool readHeader(const std::vector<uint8_t>& buffer, size_t& offset);

    /**
     * @brief Migration helpers
     */
    bool migrateFrom1_0_0();
    bool migrateFrom1_1_0();

    // Data storage
    SaveMetadata m_Metadata;
    std::map<std::string, std::string> m_StringData;
    std::map<std::string, std::vector<uint8_t>> m_BinaryData;

    // Security
    EncryptionLevel m_EncryptionLevel;
    std::string m_EncryptionKey;
    uint32_t m_Checksum;

    // Compression
    CompressionLevel m_CompressionLevel;

    // File format constants
    static constexpr uint32_t MAGIC_NUMBER = 0x53415645; // "SAVE"
    static constexpr uint32_t MAGIC_NUMBER_ENCRYPTED = 0x53415645; // "SENC"
};

/**
 * @brief Save slot information
 */
struct SaveSlot {
    int slotNumber;
    bool isEmpty;
    SaveMetadata metadata;
    std::string filePath;
    size_t fileSize;
    bool isCorrupted;
    bool hasBackup;

    SaveSlot() : slotNumber(-1), isEmpty(true), fileSize(0),
                 isCorrupted(false), hasBackup(false) {}
};

} // namespace Save
