#include "SaveFile.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace Save {

// SaveVersion implementation
std::string SaveVersion::toString() const {
    std::ostringstream oss;
    oss << major << "." << minor << "." << patch;
    return oss.str();
}

SaveVersion SaveVersion::fromString(const std::string& str) {
    SaveVersion version;
    char dot1, dot2;
    std::istringstream iss(str);
    iss >> version.major >> dot1 >> version.minor >> dot2 >> version.patch;
    return version;
}

// SaveMetadata implementation
SaveMetadata::SaveMetadata()
    : characterLevel(1), playtimeSeconds(0), health(100), maxHealth(100)
{
    timestamp = std::chrono::system_clock::now();
    version = SaveFile::getCurrentVersion();

    // Detect platform
    #ifdef _WIN32
        platform = "Windows";
    #elif __APPLE__
        platform = "macOS";
    #elif __linux__
        platform = "Linux";
    #else
        platform = "Unknown";
    #endif
}

std::string SaveMetadata::serialize() const {
    std::ostringstream oss;
    oss << saveName << "|"
        << characterName << "|"
        << characterLevel << "|"
        << location << "|"
        << std::chrono::system_clock::to_time_t(timestamp) << "|"
        << playtimeSeconds << "|"
        << version.toString() << "|"
        << platform << "|"
        << health << "|"
        << maxHealth << "|"
        << characterClass << "|"
        << lastQuest;
    return oss.str();
}

bool SaveMetadata::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(iss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 12) return false;

    try {
        saveName = tokens[0];
        characterName = tokens[1];
        characterLevel = std::stoi(tokens[2]);
        location = tokens[3];

        time_t tt = std::stoll(tokens[4]);
        timestamp = std::chrono::system_clock::from_time_t(tt);

        playtimeSeconds = std::stoull(tokens[5]);
        version = SaveVersion::fromString(tokens[6]);
        platform = tokens[7];
        health = std::stoi(tokens[8]);
        maxHealth = std::stoi(tokens[9]);
        characterClass = tokens[10];
        lastQuest = tokens[11];

        return true;
    } catch (...) {
        return false;
    }
}

// SaveFile implementation
SaveFile::SaveFile()
    : m_EncryptionLevel(EncryptionLevel::LIGHT)
    , m_CompressionLevel(CompressionLevel::BALANCED)
    , m_Checksum(0)
{
}

SaveFile::~SaveFile() {
    // Securely clear sensitive data
    if (!m_EncryptionKey.empty()) {
        std::fill(m_EncryptionKey.begin(), m_EncryptionKey.end(), 0);
    }
}

void SaveFile::initialize(const std::string& saveName) {
    clear();
    m_Metadata.saveName = saveName;
    m_Metadata.timestamp = std::chrono::system_clock::now();
    m_Metadata.version = getCurrentVersion();
}

bool SaveFile::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read entire file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    if (buffer.empty()) {
        return false;
    }

    // Read header
    size_t offset = 0;
    if (!readHeader(buffer, offset)) {
        return false;
    }

    // Decrypt if needed
    std::vector<uint8_t> data(buffer.begin() + offset, buffer.end());
    if (m_EncryptionLevel != EncryptionLevel::NONE) {
        data = decrypt(data);
        if (data.empty()) {
            return false;
        }
    }

    // Decompress if needed
    if (m_CompressionLevel != CompressionLevel::NONE) {
        data = decompress(data);
        if (data.empty()) {
            return false;
        }
    }

    // Parse data sections
    offset = 0;

    // Read metadata size
    if (offset + sizeof(uint32_t) > data.size()) return false;
    uint32_t metadataSize;
    std::memcpy(&metadataSize, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read metadata
    if (offset + metadataSize > data.size()) return false;
    std::string metadataStr(reinterpret_cast<const char*>(data.data() + offset), metadataSize);
    offset += metadataSize;

    if (!m_Metadata.deserialize(metadataStr)) {
        return false;
    }

    // Read number of string sections
    if (offset + sizeof(uint32_t) > data.size()) return false;
    uint32_t numStringSections;
    std::memcpy(&numStringSections, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read string sections
    for (uint32_t i = 0; i < numStringSections; ++i) {
        // Read section name size
        if (offset + sizeof(uint32_t) > data.size()) return false;
        uint32_t nameSize;
        std::memcpy(&nameSize, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read section name
        if (offset + nameSize > data.size()) return false;
        std::string sectionName(reinterpret_cast<const char*>(data.data() + offset), nameSize);
        offset += nameSize;

        // Read section data size
        if (offset + sizeof(uint32_t) > data.size()) return false;
        uint32_t dataSize;
        std::memcpy(&dataSize, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read section data
        if (offset + dataSize > data.size()) return false;
        std::string sectionData(reinterpret_cast<const char*>(data.data() + offset), dataSize);
        offset += dataSize;

        m_StringData[sectionName] = sectionData;
    }

    // Read number of binary sections
    if (offset + sizeof(uint32_t) > data.size()) return false;
    uint32_t numBinarySections;
    std::memcpy(&numBinarySections, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read binary sections
    for (uint32_t i = 0; i < numBinarySections; ++i) {
        // Read section name size
        if (offset + sizeof(uint32_t) > data.size()) return false;
        uint32_t nameSize;
        std::memcpy(&nameSize, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read section name
        if (offset + nameSize > data.size()) return false;
        std::string sectionName(reinterpret_cast<const char*>(data.data() + offset), nameSize);
        offset += nameSize;

        // Read section data size
        if (offset + sizeof(uint32_t) > data.size()) return false;
        uint32_t dataSize;
        std::memcpy(&dataSize, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read section data
        if (offset + dataSize > data.size()) return false;
        std::vector<uint8_t> sectionData(data.begin() + offset, data.begin() + offset + dataSize);
        offset += dataSize;

        m_BinaryData[sectionName] = sectionData;
    }

    // Validate checksum
    return validate();
}

bool SaveFile::save(const std::string& filePath) {
    // Update metadata
    m_Metadata.timestamp = std::chrono::system_clock::now();

    // Prepare data buffer
    std::vector<uint8_t> data;

    // Write metadata
    std::string metadataStr = m_Metadata.serialize();
    uint32_t metadataSize = static_cast<uint32_t>(metadataStr.size());

    size_t currentPos = data.size();
    data.resize(data.size() + sizeof(uint32_t));
    std::memcpy(data.data() + currentPos, &metadataSize, sizeof(uint32_t));

    currentPos = data.size();
    data.resize(data.size() + metadataStr.size());
    std::memcpy(data.data() + currentPos, metadataStr.data(), metadataStr.size());

    // Write string sections
    uint32_t numStringSections = static_cast<uint32_t>(m_StringData.size());
    currentPos = data.size();
    data.resize(data.size() + sizeof(uint32_t));
    std::memcpy(data.data() + currentPos, &numStringSections, sizeof(uint32_t));

    for (const auto& [name, value] : m_StringData) {
        uint32_t nameSize = static_cast<uint32_t>(name.size());
        currentPos = data.size();
        data.resize(data.size() + sizeof(uint32_t));
        std::memcpy(data.data() + currentPos, &nameSize, sizeof(uint32_t));

        currentPos = data.size();
        data.resize(data.size() + name.size());
        std::memcpy(data.data() + currentPos, name.data(), name.size());

        uint32_t dataSize = static_cast<uint32_t>(value.size());
        currentPos = data.size();
        data.resize(data.size() + sizeof(uint32_t));
        std::memcpy(data.data() + currentPos, &dataSize, sizeof(uint32_t));

        currentPos = data.size();
        data.resize(data.size() + value.size());
        std::memcpy(data.data() + currentPos, value.data(), value.size());
    }

    // Write binary sections
    uint32_t numBinarySections = static_cast<uint32_t>(m_BinaryData.size());
    currentPos = data.size();
    data.resize(data.size() + sizeof(uint32_t));
    std::memcpy(data.data() + currentPos, &numBinarySections, sizeof(uint32_t));

    for (const auto& [name, value] : m_BinaryData) {
        uint32_t nameSize = static_cast<uint32_t>(name.size());
        currentPos = data.size();
        data.resize(data.size() + sizeof(uint32_t));
        std::memcpy(data.data() + currentPos, &nameSize, sizeof(uint32_t));

        currentPos = data.size();
        data.resize(data.size() + name.size());
        std::memcpy(data.data() + currentPos, name.data(), name.size());

        uint32_t dataSize = static_cast<uint32_t>(value.size());
        currentPos = data.size();
        data.resize(data.size() + sizeof(uint32_t));
        std::memcpy(data.data() + currentPos, &dataSize, sizeof(uint32_t));

        currentPos = data.size();
        data.resize(data.size() + value.size());
        std::memcpy(data.data() + currentPos, value.data(), value.size());
    }

    // Compress if needed
    if (m_CompressionLevel != CompressionLevel::NONE) {
        data = compress(data);
    }

    // Encrypt if needed
    if (m_EncryptionLevel != EncryptionLevel::NONE) {
        data = encrypt(data);
    }

    // Calculate checksum
    m_Checksum = calculateChecksum();

    // Write header and data to file
    std::vector<uint8_t> fileData;
    writeHeader(fileData);
    fileData.insert(fileData.end(), data.begin(), data.end());

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    file.close();

    return file.good();
}

bool SaveFile::validate() const {
    uint32_t calculatedChecksum = calculateChecksum();
    return calculatedChecksum == m_Checksum;
}

uint32_t SaveFile::calculateChecksum() const {
    // Simple CRC32-like checksum
    uint32_t checksum = 0xFFFFFFFF;

    // Hash metadata
    std::string metadataStr = m_Metadata.serialize();
    for (char c : metadataStr) {
        checksum = ((checksum >> 8) ^ (checksum << 24)) + static_cast<uint8_t>(c);
    }

    // Hash string data
    for (const auto& [name, value] : m_StringData) {
        for (char c : name) checksum = ((checksum >> 8) ^ (checksum << 24)) + static_cast<uint8_t>(c);
        for (char c : value) checksum = ((checksum >> 8) ^ (checksum << 24)) + static_cast<uint8_t>(c);
    }

    // Hash binary data
    for (const auto& [name, value] : m_BinaryData) {
        for (char c : name) checksum = ((checksum >> 8) ^ (checksum << 24)) + static_cast<uint8_t>(c);
        for (uint8_t b : value) checksum = ((checksum >> 8) ^ (checksum << 24)) + b;
    }

    return checksum;
}

void SaveFile::setData(const std::string& section, const std::string& data) {
    m_StringData[section] = data;
}

std::string SaveFile::getData(const std::string& section) const {
    auto it = m_StringData.find(section);
    return (it != m_StringData.end()) ? it->second : "";
}

bool SaveFile::hasSection(const std::string& section) const {
    return m_StringData.find(section) != m_StringData.end() ||
           m_BinaryData.find(section) != m_BinaryData.end();
}

std::vector<std::string> SaveFile::getSections() const {
    std::vector<std::string> sections;
    for (const auto& [name, _] : m_StringData) {
        sections.push_back(name);
    }
    for (const auto& [name, _] : m_BinaryData) {
        if (std::find(sections.begin(), sections.end(), name) == sections.end()) {
            sections.push_back(name);
        }
    }
    return sections;
}

void SaveFile::setBinaryData(const std::string& section, const std::vector<uint8_t>& data) {
    m_BinaryData[section] = data;
}

std::vector<uint8_t> SaveFile::getBinaryData(const std::string& section) const {
    auto it = m_BinaryData.find(section);
    return (it != m_BinaryData.end()) ? it->second : std::vector<uint8_t>();
}

void SaveFile::clear() {
    m_StringData.clear();
    m_BinaryData.clear();
    m_Metadata = SaveMetadata();
    m_Checksum = 0;
}

size_t SaveFile::getSize() const {
    size_t size = 0;
    for (const auto& [name, value] : m_StringData) {
        size += name.size() + value.size();
    }
    for (const auto& [name, value] : m_BinaryData) {
        size += name.size() + value.size();
    }
    return size;
}

void SaveFile::setEncryption(EncryptionLevel level, const std::string& key) {
    m_EncryptionLevel = level;
    m_EncryptionKey = key;
}

SaveVersion SaveFile::getCurrentVersion() {
    return SaveVersion(1, 0, 0);
}

bool SaveFile::needsMigration() const {
    return m_Metadata.version < getCurrentVersion();
}

bool SaveFile::migrate() {
    if (!needsMigration()) {
        return true;
    }

    SaveVersion currentVersion = getCurrentVersion();
    SaveVersion fileVersion = m_Metadata.version;

    // Perform step-by-step migration
    if (fileVersion < SaveVersion(1, 0, 0)) {
        if (!migrateFrom1_0_0()) return false;
    }

    if (fileVersion < SaveVersion(1, 1, 0)) {
        if (!migrateFrom1_1_0()) return false;
    }

    m_Metadata.version = currentVersion;
    return true;
}

bool SaveFile::createBackup(const std::string& backupPath) const {
    // This would need the original file path, so we'll implement in SaveSystem
    return true;
}

std::string SaveFile::exportToJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"metadata\": {\n";
    json << "    \"saveName\": \"" << m_Metadata.saveName << "\",\n";
    json << "    \"characterName\": \"" << m_Metadata.characterName << "\",\n";
    json << "    \"characterLevel\": " << m_Metadata.characterLevel << ",\n";
    json << "    \"location\": \"" << m_Metadata.location << "\",\n";
    json << "    \"version\": \"" << m_Metadata.version.toString() << "\"\n";
    json << "  },\n";
    json << "  \"data\": {\n";

    bool first = true;
    for (const auto& [name, value] : m_StringData) {
        if (!first) json << ",\n";
        json << "    \"" << name << "\": " << value;
        first = false;
    }

    json << "\n  }\n";
    json << "}\n";
    return json.str();
}

bool SaveFile::importFromJson(const std::string& json) {
    // Basic JSON parsing - in production, use a proper JSON library
    // This is a simplified implementation
    return true;
}

std::string SaveFile::getFileSizeString() const {
    size_t bytes = getSize();
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

// Private methods

std::vector<uint8_t> SaveFile::encrypt(const std::vector<uint8_t>& data) const {
    std::vector<uint8_t> encrypted = data;

    switch (m_EncryptionLevel) {
        case EncryptionLevel::NONE:
            return data;

        case EncryptionLevel::LIGHT: {
            // Simple XOR encryption
            const char* key = m_EncryptionKey.empty() ? "DefaultKey" : m_EncryptionKey.c_str();
            size_t keyLen = strlen(key);
            for (size_t i = 0; i < encrypted.size(); ++i) {
                encrypted[i] ^= key[i % keyLen];
            }
            break;
        }

        case EncryptionLevel::STANDARD:
        case EncryptionLevel::PARANOID:
            // In production, implement AES-256 encryption
            // For now, use enhanced XOR
            for (size_t i = 0; i < encrypted.size(); ++i) {
                encrypted[i] ^= (i % 256);
            }
            break;
    }

    return encrypted;
}

std::vector<uint8_t> SaveFile::decrypt(const std::vector<uint8_t>& data) const {
    // XOR encryption is symmetric, so decrypt is same as encrypt
    return encrypt(data);
}

std::vector<uint8_t> SaveFile::compress(const std::vector<uint8_t>& data) const {
    // In production, use zlib or similar compression library
    // For now, return uncompressed data
    return data;
}

std::vector<uint8_t> SaveFile::decompress(const std::vector<uint8_t>& data) const {
    // In production, use zlib or similar decompression
    return data;
}

void SaveFile::writeHeader(std::vector<uint8_t>& buffer) const {
    // Magic number
    uint32_t magic = (m_EncryptionLevel != EncryptionLevel::NONE) ?
                     MAGIC_NUMBER_ENCRYPTED : MAGIC_NUMBER;
    buffer.insert(buffer.end(),
                  reinterpret_cast<const uint8_t*>(&magic),
                  reinterpret_cast<const uint8_t*>(&magic) + sizeof(uint32_t));

    // Version
    buffer.push_back(static_cast<uint8_t>(m_Metadata.version.major));
    buffer.push_back(static_cast<uint8_t>(m_Metadata.version.minor));
    buffer.push_back(static_cast<uint8_t>(m_Metadata.version.patch));

    // Flags
    uint8_t flags = 0;
    flags |= (static_cast<uint8_t>(m_EncryptionLevel) << 4);
    flags |= static_cast<uint8_t>(m_CompressionLevel);
    buffer.push_back(flags);

    // Checksum
    buffer.insert(buffer.end(),
                  reinterpret_cast<const uint8_t*>(&m_Checksum),
                  reinterpret_cast<const uint8_t*>(&m_Checksum) + sizeof(uint32_t));
}

bool SaveFile::readHeader(const std::vector<uint8_t>& buffer, size_t& offset) {
    if (buffer.size() < 12) return false;

    // Read magic number
    uint32_t magic;
    std::memcpy(&magic, buffer.data(), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (magic != MAGIC_NUMBER && magic != MAGIC_NUMBER_ENCRYPTED) {
        return false;
    }

    // Read version
    m_Metadata.version.major = buffer[offset++];
    m_Metadata.version.minor = buffer[offset++];
    m_Metadata.version.patch = buffer[offset++];

    // Read flags
    uint8_t flags = buffer[offset++];
    m_EncryptionLevel = static_cast<EncryptionLevel>((flags >> 4) & 0x0F);
    m_CompressionLevel = static_cast<CompressionLevel>(flags & 0x0F);

    // Read checksum
    std::memcpy(&m_Checksum, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return true;
}

bool SaveFile::migrateFrom1_0_0() {
    // Migration logic from version 1.0.0
    return true;
}

bool SaveFile::migrateFrom1_1_0() {
    // Migration logic from version 1.1.0
    return true;
}

} // namespace Save
