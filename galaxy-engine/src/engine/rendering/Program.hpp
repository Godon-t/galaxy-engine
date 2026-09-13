#pragma once

#include "types/Math.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace Galaxy {

// Description of a value exposed by a shader. The OpenGL location is reflected
// once after linking and is never looked up while executing render commands.
enum class ShaderValueType {
    Unknown,
    Bool,
    Int,
    UnsignedInt,
    Float,
    Float2,
    Float3,
    Float4,
    Int2,
    Int3,
    Int4,
    Matrix2,
    Matrix3,
    Matrix4,
    Sampler2D,
    Sampler2DArray,
    SamplerCube
};

struct UniformInfo {
    std::string name;
    ShaderValueType type = ShaderValueType::Unknown;
    int location = -1;
    int elementCount = 1;
};

struct UniformBlockInfo {
    std::string name;
    unsigned int index = 0;
    unsigned int bindingPoint = 0;
    int byteSize = 0;
};

// OpenGL program resource. Shader roles (PBR, skybox, etc.) are deliberately
// not represented through inheritance.
class Program final {
public:
    Program() = default;
    Program(const char* vertexContent, const char* fragmentContent);
    Program(const std::string& vertexContent, const std::string& fragmentContent);
    explicit Program(const std::string& shaderPath);

    ~Program();

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    void destroy();
    void use() const;

    [[nodiscard]] bool isLinked() const noexcept { return m_programID != 0; }
    [[nodiscard]] unsigned int getProgramID() const noexcept { return m_programID; }

    [[nodiscard]] int getUniformLocation(const std::string& uniformName) const noexcept;
    [[nodiscard]] const UniformInfo* getUniformInfo(const std::string& uniformName) const noexcept;
    [[nodiscard]] bool hasUniform(const std::string& uniformName) const noexcept;
    [[nodiscard]] const std::vector<UniformInfo>& getUniforms() const noexcept { return m_uniforms; }
    [[nodiscard]] const std::vector<UniformBlockInfo>& getUniformBlocks() const noexcept { return m_uniformBlocks; }

    bool bindUniformBlock(const std::string& blockName, unsigned int bindingPoint);

    bool setUniform(const std::string& uniformName, bool value) const;
    bool setUniform(const std::string& uniformName, float value) const;
    bool setUniform(const std::string& uniformName, int value) const;
    bool setUniform(const std::string& uniformName, const math::vec2& value) const;
    bool setUniform(const std::string& uniformName, const math::vec3& value) const;
    bool setUniform(const std::string& uniformName, const math::ivec3& value) const;
    bool setUniform(const std::string& uniformName, const math::vec4& value) const;
    bool setUniform(const std::string& uniformName, const math::mat4& value) const;

private:
    [[nodiscard]] bool compile(unsigned int shaderID, const char* content) const;
    [[nodiscard]] std::unordered_map<unsigned int, std::string> preProcess(const std::string& source) const;
    [[nodiscard]] bool link(const std::vector<unsigned int>& shaderIDs);

    void init(const char* vertexContent, const char* fragmentContent);
    void init(const std::unordered_map<unsigned int, std::string>& shaderContents);
    void reflectBindings();

    unsigned int m_programID = 0;
    std::vector<UniformInfo> m_uniforms;
    std::vector<UniformBlockInfo> m_uniformBlocks;
    std::unordered_map<std::string, int> m_uniformLocations;
    std::unordered_map<std::string, std::size_t> m_uniformInfoIndices;
    std::unordered_map<std::string, std::size_t> m_uniformBlockIndices;
};

} // namespace Galaxy
