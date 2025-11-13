#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <type_traits>
#include <cstdint>

namespace Save {

/**
 * @brief Interface for serializable objects
 */
class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual std::string serialize() const = 0;
    virtual bool deserialize(const std::string& data) = 0;
};

/**
 * @brief Serialization format
 */
enum class SerializationFormat {
    JSON,      // Human-readable JSON format
    BINARY,    // Compact binary format
    XML,       // XML format for compatibility
    CUSTOM     // Custom format
};

/**
 * @brief Serializer utility class for converting objects to/from strings/bytes
 */
class Serializer {
public:
    Serializer(SerializationFormat format = SerializationFormat::JSON);
    ~Serializer();

    /**
     * @brief Set serialization format
     */
    void setFormat(SerializationFormat format) { m_Format = format; }
    SerializationFormat getFormat() const { return m_Format; }

    // === JSON Serialization ===

    /**
     * @brief Begin JSON object
     */
    void beginObject();

    /**
     * @brief End JSON object
     */
    void endObject();

    /**
     * @brief Begin JSON array
     */
    void beginArray(const std::string& name);

    /**
     * @brief End JSON array
     */
    void endArray();

    /**
     * @brief Write primitive types
     */
    void write(const std::string& key, int value);
    void write(const std::string& key, unsigned int value);
    void write(const std::string& key, int64_t value);
    void write(const std::string& key, uint64_t value);
    void write(const std::string& key, float value);
    void write(const std::string& key, double value);
    void write(const std::string& key, bool value);
    void write(const std::string& key, const std::string& value);
    void write(const std::string& key, const char* value);

    /**
     * @brief Write vectors
     */
    template<typename T>
    void writeVector(const std::string& key, const std::vector<T>& vec);

    /**
     * @brief Write maps
     */
    template<typename K, typename V>
    void writeMap(const std::string& key, const std::map<K, V>& map);

    /**
     * @brief Write serializable object
     */
    void writeObject(const std::string& key, const ISerializable& obj);

    /**
     * @brief Get serialized string (JSON format)
     */
    std::string toString() const;

    /**
     * @brief Get serialized binary data
     */
    std::vector<uint8_t> toBinary() const;

    /**
     * @brief Clear serializer
     */
    void clear();

    // === Deserialization ===

    /**
     * @brief Load from string
     */
    bool fromString(const std::string& data);

    /**
     * @brief Load from binary
     */
    bool fromBinary(const std::vector<uint8_t>& data);

    /**
     * @brief Read primitive types
     */
    bool read(const std::string& key, int& value);
    bool read(const std::string& key, unsigned int& value);
    bool read(const std::string& key, int64_t& value);
    bool read(const std::string& key, uint64_t& value);
    bool read(const std::string& key, float& value);
    bool read(const std::string& key, double& value);
    bool read(const std::string& key, bool& value);
    bool read(const std::string& key, std::string& value);

    /**
     * @brief Read vectors
     */
    template<typename T>
    bool readVector(const std::string& key, std::vector<T>& vec);

    /**
     * @brief Read maps
     */
    template<typename K, typename V>
    bool readMap(const std::string& key, std::map<K, V>& map);

    /**
     * @brief Read serializable object
     */
    bool readObject(const std::string& key, ISerializable& obj);

    /**
     * @brief Check if key exists
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief Get all keys
     */
    std::vector<std::string> getKeys() const;

    // === Binary Serialization Helpers ===

    /**
     * @brief Write binary data
     */
    void writeBinary(const void* data, size_t size);

    /**
     * @brief Read binary data
     */
    bool readBinary(void* data, size_t size);

    /**
     * @brief Get current binary position
     */
    size_t getBinaryPosition() const { return m_BinaryReadPos; }

    /**
     * @brief Set binary read position
     */
    void setBinaryPosition(size_t pos) { m_BinaryReadPos = pos; }

    /**
     * @brief Get binary data size
     */
    size_t getBinarySize() const { return m_BinaryData.size(); }

private:
    /**
     * @brief Add JSON key-value pair
     */
    void addJsonKeyValue(const std::string& key, const std::string& value);

    /**
     * @brief Escape JSON string
     */
    std::string escapeJsonString(const std::string& str) const;

    /**
     * @brief Parse JSON
     */
    bool parseJson(const std::string& json);

    /**
     * @brief Skip JSON whitespace
     */
    void skipWhitespace(const std::string& json, size_t& pos) const;

    /**
     * @brief Parse JSON value
     */
    std::string parseJsonValue(const std::string& json, size_t& pos);

    /**
     * @brief Parse JSON string
     */
    std::string parseJsonString(const std::string& json, size_t& pos);

    /**
     * @brief Parse JSON number
     */
    std::string parseJsonNumber(const std::string& json, size_t& pos);

    SerializationFormat m_Format;

    // JSON data
    std::ostringstream m_JsonStream;
    std::map<std::string, std::string> m_JsonData;
    bool m_FirstEntry;
    int m_IndentLevel;

    // Binary data
    std::vector<uint8_t> m_BinaryData;
    size_t m_BinaryReadPos;
};

// === Template Implementations ===

template<typename T>
void Serializer::writeVector(const std::string& key, const std::vector<T>& vec) {
    if (m_Format == SerializationFormat::JSON) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) oss << ",";
            if constexpr (std::is_same_v<T, std::string>) {
                oss << "\"" << escapeJsonString(vec[i]) << "\"";
            } else if constexpr (std::is_same_v<T, bool>) {
                oss << (vec[i] ? "true" : "false");
            } else {
                oss << vec[i];
            }
        }
        oss << "]";
        addJsonKeyValue(key, oss.str());
    } else if (m_Format == SerializationFormat::BINARY) {
        uint32_t size = static_cast<uint32_t>(vec.size());
        writeBinary(&size, sizeof(uint32_t));
        for (const auto& item : vec) {
            writeBinary(&item, sizeof(T));
        }
    }
}

template<typename K, typename V>
void Serializer::writeMap(const std::string& key, const std::map<K, V>& map) {
    if (m_Format == SerializationFormat::JSON) {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& [k, v] : map) {
            if (!first) oss << ",";
            first = false;

            // Write key
            if constexpr (std::is_same_v<K, std::string>) {
                oss << "\"" << escapeJsonString(k) << "\":";
            } else {
                oss << "\"" << k << "\":";
            }

            // Write value
            if constexpr (std::is_same_v<V, std::string>) {
                oss << "\"" << escapeJsonString(v) << "\"";
            } else if constexpr (std::is_same_v<V, bool>) {
                oss << (v ? "true" : "false");
            } else {
                oss << v;
            }
        }
        oss << "}";
        addJsonKeyValue(key, oss.str());
    } else if (m_Format == SerializationFormat::BINARY) {
        uint32_t size = static_cast<uint32_t>(map.size());
        writeBinary(&size, sizeof(uint32_t));
        for (const auto& [k, v] : map) {
            writeBinary(&k, sizeof(K));
            writeBinary(&v, sizeof(V));
        }
    }
}

template<typename T>
bool Serializer::readVector(const std::string& key, std::vector<T>& vec) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;

        const std::string& arrayStr = it->second;
        if (arrayStr.empty() || arrayStr[0] != '[') return false;

        vec.clear();
        size_t pos = 1; // Skip '['

        while (pos < arrayStr.size() && arrayStr[pos] != ']') {
            // Skip whitespace and commas
            while (pos < arrayStr.size() && (arrayStr[pos] == ' ' || arrayStr[pos] == ',')) {
                pos++;
            }

            if (pos >= arrayStr.size() || arrayStr[pos] == ']') break;

            T value;
            if constexpr (std::is_same_v<T, std::string>) {
                if (arrayStr[pos] == '"') {
                    pos++; // Skip opening quote
                    size_t endPos = arrayStr.find('"', pos);
                    if (endPos == std::string::npos) return false;
                    value = arrayStr.substr(pos, endPos - pos);
                    pos = endPos + 1;
                }
            } else {
                size_t endPos = pos;
                while (endPos < arrayStr.size() && arrayStr[endPos] != ',' && arrayStr[endPos] != ']') {
                    endPos++;
                }
                std::istringstream iss(arrayStr.substr(pos, endPos - pos));
                iss >> value;
                pos = endPos;
            }

            vec.push_back(value);
        }

        return true;
    } else if (m_Format == SerializationFormat::BINARY) {
        uint32_t size;
        if (!readBinary(&size, sizeof(uint32_t))) return false;

        vec.clear();
        vec.resize(size);
        for (uint32_t i = 0; i < size; ++i) {
            if (!readBinary(&vec[i], sizeof(T))) return false;
        }
        return true;
    }

    return false;
}

template<typename K, typename V>
bool Serializer::readMap(const std::string& key, std::map<K, V>& map) {
    if (m_Format == SerializationFormat::BINARY) {
        uint32_t size;
        if (!readBinary(&size, sizeof(uint32_t))) return false;

        map.clear();
        for (uint32_t i = 0; i < size; ++i) {
            K k;
            V v;
            if (!readBinary(&k, sizeof(K))) return false;
            if (!readBinary(&v, sizeof(V))) return false;
            map[k] = v;
        }
        return true;
    }

    return false;
}

/**
 * @brief Helper macros for serialization
 */
#define SERIALIZE_FIELD(serializer, field) serializer.write(#field, field)
#define DESERIALIZE_FIELD(serializer, field) serializer.read(#field, field)

#define SERIALIZE_VECTOR(serializer, field) serializer.writeVector(#field, field)
#define DESERIALIZE_VECTOR(serializer, field) serializer.readVector(#field, field)

#define SERIALIZE_MAP(serializer, field) serializer.writeMap(#field, field)
#define DESERIALIZE_MAP(serializer, field) serializer.readMap(#field, field)

} // namespace Save
