#include "Serializer.h"
#include <cstring>
#include <algorithm>
#include <iomanip>

namespace Save {

Serializer::Serializer(SerializationFormat format)
    : m_Format(format)
    , m_FirstEntry(true)
    , m_IndentLevel(0)
    , m_BinaryReadPos(0)
{
}

Serializer::~Serializer() {
}

void Serializer::beginObject() {
    if (m_Format == SerializationFormat::JSON) {
        m_JsonStream << "{\n";
        m_IndentLevel++;
        m_FirstEntry = true;
    }
}

void Serializer::endObject() {
    if (m_Format == SerializationFormat::JSON) {
        m_JsonStream << "\n";
        m_IndentLevel--;
        for (int i = 0; i < m_IndentLevel; ++i) {
            m_JsonStream << "  ";
        }
        m_JsonStream << "}";
    }
}

void Serializer::beginArray(const std::string& name) {
    if (m_Format == SerializationFormat::JSON) {
        if (!m_FirstEntry) {
            m_JsonStream << ",\n";
        }
        m_FirstEntry = false;

        for (int i = 0; i < m_IndentLevel; ++i) {
            m_JsonStream << "  ";
        }
        m_JsonStream << "\"" << name << "\": [\n";
        m_IndentLevel++;
    }
}

void Serializer::endArray() {
    if (m_Format == SerializationFormat::JSON) {
        m_JsonStream << "\n";
        m_IndentLevel--;
        for (int i = 0; i < m_IndentLevel; ++i) {
            m_JsonStream << "  ";
        }
        m_JsonStream << "]";
    }
}

void Serializer::write(const std::string& key, int value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, std::to_string(value));
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(int));
    }
}

void Serializer::write(const std::string& key, unsigned int value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, std::to_string(value));
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(unsigned int));
    }
}

void Serializer::write(const std::string& key, int64_t value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, std::to_string(value));
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(int64_t));
    }
}

void Serializer::write(const std::string& key, uint64_t value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, std::to_string(value));
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(uint64_t));
    }
}

void Serializer::write(const std::string& key, float value) {
    if (m_Format == SerializationFormat::JSON) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << value;
        addJsonKeyValue(key, oss.str());
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(float));
    }
}

void Serializer::write(const std::string& key, double value) {
    if (m_Format == SerializationFormat::JSON) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(10) << value;
        addJsonKeyValue(key, oss.str());
    } else if (m_Format == SerializationFormat::BINARY) {
        writeBinary(&value, sizeof(double));
    }
}

void Serializer::write(const std::string& key, bool value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, value ? "true" : "false");
    } else if (m_Format == SerializationFormat::BINARY) {
        uint8_t bval = value ? 1 : 0;
        writeBinary(&bval, sizeof(uint8_t));
    }
}

void Serializer::write(const std::string& key, const std::string& value) {
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, "\"" + escapeJsonString(value) + "\"");
    } else if (m_Format == SerializationFormat::BINARY) {
        uint32_t size = static_cast<uint32_t>(value.size());
        writeBinary(&size, sizeof(uint32_t));
        writeBinary(value.data(), value.size());
    }
}

void Serializer::write(const std::string& key, const char* value) {
    write(key, std::string(value));
}

void Serializer::writeObject(const std::string& key, const ISerializable& obj) {
    std::string serialized = obj.serialize();
    if (m_Format == SerializationFormat::JSON) {
        addJsonKeyValue(key, serialized);
    } else if (m_Format == SerializationFormat::BINARY) {
        write(key, serialized);
    }
}

std::string Serializer::toString() const {
    if (m_Format == SerializationFormat::JSON) {
        return m_JsonStream.str();
    }
    return "";
}

std::vector<uint8_t> Serializer::toBinary() const {
    return m_BinaryData;
}

void Serializer::clear() {
    m_JsonStream.str("");
    m_JsonStream.clear();
    m_JsonData.clear();
    m_BinaryData.clear();
    m_BinaryReadPos = 0;
    m_FirstEntry = true;
    m_IndentLevel = 0;
}

bool Serializer::fromString(const std::string& data) {
    if (m_Format == SerializationFormat::JSON) {
        return parseJson(data);
    }
    return false;
}

bool Serializer::fromBinary(const std::vector<uint8_t>& data) {
    m_BinaryData = data;
    m_BinaryReadPos = 0;
    return true;
}

bool Serializer::read(const std::string& key, int& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stoi(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(int));
    }
    return false;
}

bool Serializer::read(const std::string& key, unsigned int& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stoul(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(unsigned int));
    }
    return false;
}

bool Serializer::read(const std::string& key, int64_t& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stoll(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(int64_t));
    }
    return false;
}

bool Serializer::read(const std::string& key, uint64_t& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stoull(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(uint64_t));
    }
    return false;
}

bool Serializer::read(const std::string& key, float& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stof(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(float));
    }
    return false;
}

bool Serializer::read(const std::string& key, double& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        try {
            value = std::stod(it->second);
            return true;
        } catch (...) {
            return false;
        }
    } else if (m_Format == SerializationFormat::BINARY) {
        return readBinary(&value, sizeof(double));
    }
    return false;
}

bool Serializer::read(const std::string& key, bool& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;
        value = (it->second == "true" || it->second == "1");
        return true;
    } else if (m_Format == SerializationFormat::BINARY) {
        uint8_t bval;
        if (!readBinary(&bval, sizeof(uint8_t))) return false;
        value = (bval != 0);
        return true;
    }
    return false;
}

bool Serializer::read(const std::string& key, std::string& value) {
    if (m_Format == SerializationFormat::JSON) {
        auto it = m_JsonData.find(key);
        if (it == m_JsonData.end()) return false;

        value = it->second;
        // Remove quotes if present
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        return true;
    } else if (m_Format == SerializationFormat::BINARY) {
        uint32_t size;
        if (!readBinary(&size, sizeof(uint32_t))) return false;

        if (m_BinaryReadPos + size > m_BinaryData.size()) return false;

        value.resize(size);
        return readBinary(&value[0], size);
    }
    return false;
}

bool Serializer::readObject(const std::string& key, ISerializable& obj) {
    std::string serialized;
    if (!read(key, serialized)) return false;
    return obj.deserialize(serialized);
}

bool Serializer::hasKey(const std::string& key) const {
    return m_JsonData.find(key) != m_JsonData.end();
}

std::vector<std::string> Serializer::getKeys() const {
    std::vector<std::string> keys;
    for (const auto& [key, _] : m_JsonData) {
        keys.push_back(key);
    }
    return keys;
}

void Serializer::writeBinary(const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    m_BinaryData.insert(m_BinaryData.end(), bytes, bytes + size);
}

bool Serializer::readBinary(void* data, size_t size) {
    if (m_BinaryReadPos + size > m_BinaryData.size()) {
        return false;
    }

    std::memcpy(data, m_BinaryData.data() + m_BinaryReadPos, size);
    m_BinaryReadPos += size;
    return true;
}

void Serializer::addJsonKeyValue(const std::string& key, const std::string& value) {
    if (!m_FirstEntry) {
        m_JsonStream << ",\n";
    }
    m_FirstEntry = false;

    for (int i = 0; i < m_IndentLevel; ++i) {
        m_JsonStream << "  ";
    }
    m_JsonStream << "\"" << key << "\": " << value;

    // Also store in map for reading
    m_JsonData[key] = value;
}

std::string Serializer::escapeJsonString(const std::string& str) const {
    std::string escaped;
    escaped.reserve(str.size());

    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 32) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
        }
    }

    return escaped;
}

bool Serializer::parseJson(const std::string& json) {
    m_JsonData.clear();

    size_t pos = 0;
    skipWhitespace(json, pos);

    if (pos >= json.size() || json[pos] != '{') {
        return false;
    }

    pos++; // Skip '{'

    while (pos < json.size()) {
        skipWhitespace(json, pos);

        if (pos >= json.size()) return false;
        if (json[pos] == '}') break;

        // Parse key
        if (json[pos] != '"') return false;
        std::string key = parseJsonString(json, pos);

        skipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':') return false;
        pos++; // Skip ':'

        skipWhitespace(json, pos);

        // Parse value
        std::string value = parseJsonValue(json, pos);
        m_JsonData[key] = value;

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            pos++; // Skip ','
        }
    }

    return true;
}

void Serializer::skipWhitespace(const std::string& json, size_t& pos) const {
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        pos++;
    }
}

std::string Serializer::parseJsonValue(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);

    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        return "\"" + parseJsonString(json, pos) + "\"";
    } else if (json[pos] == '{' || json[pos] == '[') {
        // Parse nested object or array
        char openChar = json[pos];
        char closeChar = (openChar == '{') ? '}' : ']';
        int depth = 1;
        size_t startPos = pos;
        pos++;

        while (pos < json.size() && depth > 0) {
            if (json[pos] == openChar) depth++;
            else if (json[pos] == closeChar) depth--;
            pos++;
        }

        return json.substr(startPos, pos - startPos);
    } else {
        return parseJsonNumber(json, pos);
    }
}

std::string Serializer::parseJsonString(const std::string& json, size_t& pos) {
    if (pos >= json.size() || json[pos] != '"') return "";

    pos++; // Skip opening quote
    std::string result;

    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            pos++;
            switch (json[pos]) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case '/':  result += '/'; break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        pos++;
    }

    if (pos < json.size() && json[pos] == '"') {
        pos++; // Skip closing quote
    }

    return result;
}

std::string Serializer::parseJsonNumber(const std::string& json, size_t& pos) {
    size_t startPos = pos;

    if (pos < json.size() && json[pos] == '-') pos++;

    while (pos < json.size() && (json[pos] >= '0' && json[pos] <= '9')) {
        pos++;
    }

    if (pos < json.size() && json[pos] == '.') {
        pos++;
        while (pos < json.size() && (json[pos] >= '0' && json[pos] <= '9')) {
            pos++;
        }
    }

    if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
        pos++;
        if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) pos++;
        while (pos < json.size() && (json[pos] >= '0' && json[pos] <= '9')) {
            pos++;
        }
    }

    // Handle true/false/null
    if (startPos < json.size()) {
        if (json.substr(startPos, 4) == "true") {
            pos = startPos + 4;
            return "true";
        } else if (json.substr(startPos, 5) == "false") {
            pos = startPos + 5;
            return "false";
        } else if (json.substr(startPos, 4) == "null") {
            pos = startPos + 4;
            return "null";
        }
    }

    return json.substr(startPos, pos - startPos);
}

} // namespace Save
