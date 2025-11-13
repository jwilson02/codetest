#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace Rendering {

/**
 * Shader management class for loading, compiling, and using OpenGL shaders
 */
class Shader {
public:
    Shader();
    ~Shader();

    // Load shader from files
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // Load shader from source strings
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    // Use/activate the shader
    void Use() const;

    // Unbind shader
    void Unbind() const;

    // Utility uniform functions
    void SetBool(const std::string& name, bool value);
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& mat);
    void SetMat4(const std::string& name, const glm::mat4& mat);

    // Get program ID
    GLuint GetProgramID() const { return m_ProgramID; }

    // Check if shader is valid
    bool IsValid() const { return m_ProgramID != 0; }

private:
    GLuint m_ProgramID;
    std::unordered_map<std::string, GLint> m_UniformLocationCache;

    // Helper functions
    GLint GetUniformLocation(const std::string& name);
    bool CompileShader(GLuint shader, const std::string& source, const std::string& type);
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
    std::string ReadFile(const std::string& filepath);
};

} // namespace Rendering
