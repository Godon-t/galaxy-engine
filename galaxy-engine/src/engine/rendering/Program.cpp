#include "Program.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "OpenglHelper.hpp"
#include "gl_headers.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace Galaxy {
namespace {

std::string shaderTypeStr(GLenum type)
{
    switch (type) {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    case GL_GEOMETRY_SHADER:
        return "geometry";
    case GL_COMPUTE_SHADER:
        return "compute";
    default:
        return "unknown";
    }
}

GLenum shaderTypeFromString(const std::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    if (type == "geometry")
        return GL_GEOMETRY_SHADER;
    if (type == "compute")
        return GL_COMPUTE_SHADER;
    return 0;
}

ShaderValueType shaderValueType(GLenum type)
{
    switch (type) {
    case GL_BOOL:
        return ShaderValueType::Bool;
    case GL_INT:
        return ShaderValueType::Int;
    case GL_UNSIGNED_INT:
        return ShaderValueType::UnsignedInt;
    case GL_FLOAT:
        return ShaderValueType::Float;
    case GL_FLOAT_VEC2:
        return ShaderValueType::Float2;
    case GL_FLOAT_VEC3:
        return ShaderValueType::Float3;
    case GL_FLOAT_VEC4:
        return ShaderValueType::Float4;
    case GL_INT_VEC2:
        return ShaderValueType::Int2;
    case GL_INT_VEC3:
        return ShaderValueType::Int3;
    case GL_INT_VEC4:
        return ShaderValueType::Int4;
    case GL_FLOAT_MAT2:
        return ShaderValueType::Matrix2;
    case GL_FLOAT_MAT3:
        return ShaderValueType::Matrix3;
    case GL_FLOAT_MAT4:
        return ShaderValueType::Matrix4;
    case GL_SAMPLER_2D:
        return ShaderValueType::Sampler2D;
    case GL_SAMPLER_2D_ARRAY:
        return ShaderValueType::Sampler2DArray;
    case GL_SAMPLER_CUBE:
        return ShaderValueType::SamplerCube;
    default:
        return ShaderValueType::Unknown;
    }
}

std::string removeFirstArrayIndex(const std::string& name)
{
    constexpr const char suffix[] = "[0]";
    constexpr size_t suffixLength = sizeof(suffix) - 1;
    if (name.size() >= suffixLength
        && name.compare(name.size() - suffixLength, suffixLength, suffix) == 0) {
        return name.substr(0, name.size() - suffixLength);
    }
    return name;
}

} // namespace

bool Program::compile(unsigned int shaderID, const char* content) const
{
    const char* sourcePointer = content;
    glShaderSource(shaderID, 1, &sourcePointer, nullptr);
    glCompileShader(shaderID);

    GLint compiled = GL_FALSE;
    GLint infoLogLength = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1) {
        std::vector<char> errorMessage(static_cast<size_t>(infoLogLength));
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, errorMessage.data());
        GLX_CORE_ERROR("Shader compilation log: {0}", errorMessage.data());
    }

    return compiled == GL_TRUE;
}

std::unordered_map<unsigned int, std::string> Program::preProcess(const std::string& source) const
{
    std::unordered_map<unsigned int, std::string> result;
    std::string processedSource = source;

    constexpr const char* includeToken = "#include";
    constexpr size_t includeTokenLength = 8;
    size_t includePos = processedSource.find(includeToken);

    while (includePos != std::string::npos) {
        const size_t eol = processedSource.find_first_of("\r\n", includePos);
        GLX_CORE_ASSERT(eol != std::string::npos, "Shader include directive has no end of line");
        if (eol == std::string::npos)
            return {};

        const size_t begin = includePos + includeTokenLength + 1;
        const std::string includeFile = processedSource.substr(begin, eol - begin);
        const std::string fullPath = engineRes("shaders/include/") + includeFile;
        std::ifstream includeStream(fullPath, std::ios::in);
        GLX_CORE_ASSERT(includeStream.is_open(), "Can't open include file '{0}'", fullPath);
        if (!includeStream.is_open())
            return {};

        std::stringstream stream;
        stream << includeStream.rdbuf();
        processedSource.replace(includePos, eol - includePos, stream.str());
        includePos = processedSource.find(includeToken);
    }

    constexpr const char* typeToken = "#type";
    constexpr size_t typeTokenLength = 5;
    size_t position = processedSource.find(typeToken);

    while (position != std::string::npos) {
        const size_t eol = processedSource.find_first_of("\r\n", position);
        GLX_CORE_ASSERT(eol != std::string::npos, "Shader type directive has no end of line");
        if (eol == std::string::npos)
            return {};

        const size_t begin = position + typeTokenLength + 1;
        const std::string type = processedSource.substr(begin, eol - begin);
        const GLenum typeEnum = shaderTypeFromString(type);
        GLX_CORE_ASSERT(typeEnum != 0, "Unknown shader type '{0}'", type);
        if (typeEnum == 0)
            return {};

        const size_t nextLine = processedSource.find_first_not_of("\r\n", eol);
        if (nextLine == std::string::npos)
            break;

        position = processedSource.find(typeToken, nextLine);
        result[typeEnum] = processedSource.substr(nextLine, position - nextLine);
    }

    return result;
}

bool Program::link(const std::vector<unsigned int>& shaderIDs)
{
    const GLuint programID = glCreateProgram();
    for (const GLuint shaderID : shaderIDs)
        glAttachShader(programID, shaderID);

    glLinkProgram(programID);

    GLint linked = GL_FALSE;
    GLint infoLogLength = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &linked);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1) {
        std::vector<char> errorMessage(static_cast<size_t>(infoLogLength));
        glGetProgramInfoLog(programID, infoLogLength, nullptr, errorMessage.data());
        GLX_CORE_ERROR("Program link log: {0}", errorMessage.data());
    }

    for (const GLuint shaderID : shaderIDs) {
        glDetachShader(programID, shaderID);
        glDeleteShader(shaderID);
    }

    if (linked != GL_TRUE) {
        glDeleteProgram(programID);
        return false;
    }

    destroy();
    m_programID = programID;
    reflectBindings();
    return true;
}

void Program::init(const char* vertexContent, const char* fragmentContent)
{
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    GLX_CORE_INFO("Compiling vertex shader");
    const bool vertexCompiled = compile(vertexShader, vertexContent);
    GLX_CORE_INFO("Compiling fragment shader");
    const bool fragmentCompiled = compile(fragmentShader, fragmentContent);

    if (!vertexCompiled || !fragmentCompiled) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    GLX_CORE_INFO("Linking program");
    (void)link({ vertexShader, fragmentShader });
    checkOpenGLErrors("Program initialization");
}

void Program::init(const std::unordered_map<unsigned int, std::string>& shaderContents)
{
    if (shaderContents.empty()) {
        GLX_CORE_ERROR("Shader contains no '#type' section");
        return;
    }

    std::vector<unsigned int> shaderIDs;
    shaderIDs.reserve(shaderContents.size());

    for (const auto& [type, content] : shaderContents) {
        const GLuint shaderID = glCreateShader(type);
        GLX_CORE_INFO("Compiling shader of type {0}", shaderTypeStr(type));
        if (!compile(shaderID, content.c_str())) {
            glDeleteShader(shaderID);
            for (const GLuint compiledShader : shaderIDs)
                glDeleteShader(compiledShader);
            return;
        }
        shaderIDs.push_back(shaderID);
    }

    GLX_CORE_INFO("Linking program");
    (void)link(shaderIDs);
    checkOpenGLErrors("Program initialization");
}

Program::Program(const char* vertexContent, const char* fragmentContent)
{
    init(vertexContent, fragmentContent);
}

Program::Program(const std::string& vertexContent, const std::string& fragmentContent)
{
    init(vertexContent.c_str(), fragmentContent.c_str());
}

Program::Program(const std::string& shaderPath)
{
    std::ifstream shaderStream(shaderPath, std::ios::in);
    if (!shaderStream.is_open()) {
        GLX_CORE_ERROR("Can't open shader '{0}'", shaderPath);
        return;
    }

    GLX_CORE_TRACE("Loading shader {0}", shaderPath);
    std::stringstream stream;
    stream << shaderStream.rdbuf();
    init(preProcess(stream.str()));
}

Program::Program(Program&& other) noexcept
    : m_programID(std::exchange(other.m_programID, 0))
    , m_uniforms(std::move(other.m_uniforms))
    , m_uniformBlocks(std::move(other.m_uniformBlocks))
    , m_uniformLocations(std::move(other.m_uniformLocations))
    , m_uniformInfoIndices(std::move(other.m_uniformInfoIndices))
    , m_uniformBlockIndices(std::move(other.m_uniformBlockIndices))
{
}

Program& Program::operator=(Program&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_programID = std::exchange(other.m_programID, 0);
    m_uniforms = std::move(other.m_uniforms);
    m_uniformBlocks = std::move(other.m_uniformBlocks);
    m_uniformLocations = std::move(other.m_uniformLocations);
    m_uniformInfoIndices = std::move(other.m_uniformInfoIndices);
    m_uniformBlockIndices = std::move(other.m_uniformBlockIndices);
    return *this;
}

Program::~Program()
{
    destroy();
}

void Program::destroy()
{
    if (m_programID != 0)
        glDeleteProgram(std::exchange(m_programID, 0));

    m_uniforms.clear();
    m_uniformBlocks.clear();
    m_uniformLocations.clear();
    m_uniformInfoIndices.clear();
    m_uniformBlockIndices.clear();
}

void Program::use() const
{
    if (m_programID == 0)
        return;
    glUseProgram(m_programID);
    checkOpenGLErrors("Program usage");
}

void Program::reflectBindings()
{
    m_uniforms.clear();
    m_uniformBlocks.clear();
    m_uniformLocations.clear();
    m_uniformInfoIndices.clear();
    m_uniformBlockIndices.clear();

    GLint uniformCount = 0;
    GLint maxUniformNameLength = 0;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &uniformCount);
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

    std::vector<char> nameBuffer(static_cast<size_t>(maxUniformNameLength > 0 ? maxUniformNameLength : 1));
    for (GLuint index = 0; index < static_cast<GLuint>(uniformCount); ++index) {
        GLsizei nameLength = 0;
        GLint elementCount = 0;
        GLenum type = 0;
        glGetActiveUniform(m_programID, index, maxUniformNameLength, &nameLength, &elementCount, &type, nameBuffer.data());

        GLint blockIndex = -1;
        glGetActiveUniformsiv(m_programID, 1, &index, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
        if (blockIndex != -1)
            continue;

        const std::string reflectedName(nameBuffer.data(), static_cast<size_t>(nameLength));
        const std::string baseName = removeFirstArrayIndex(reflectedName);
        const GLint location = glGetUniformLocation(m_programID, reflectedName.c_str());

        const std::size_t uniformInfoIndex = m_uniforms.size();
        m_uniforms.push_back({ baseName, shaderValueType(type), location, elementCount });
        m_uniformLocations[reflectedName] = location;
        m_uniformLocations[baseName] = location;
        m_uniformInfoIndices[reflectedName] = uniformInfoIndex;
        m_uniformInfoIndices[baseName] = uniformInfoIndex;

        if (elementCount > 1 && baseName != reflectedName) {
            for (GLint element = 0; element < elementCount; ++element) {
                const std::string elementName = baseName + "[" + std::to_string(element) + "]";
                m_uniformLocations[elementName] = glGetUniformLocation(m_programID, elementName.c_str());
                m_uniformInfoIndices[elementName] = uniformInfoIndex;
            }
        }
    }

    GLint blockCount = 0;
    GLint maxBlockNameLength = 0;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxBlockNameLength);
    nameBuffer.resize(static_cast<size_t>(maxBlockNameLength > 0 ? maxBlockNameLength : 1));

    for (GLuint index = 0; index < static_cast<GLuint>(blockCount); ++index) {
        GLsizei nameLength = 0;
        GLint bindingPoint = 0;
        GLint byteSize = 0;
        glGetActiveUniformBlockName(m_programID, index, maxBlockNameLength, &nameLength, nameBuffer.data());
        glGetActiveUniformBlockiv(m_programID, index, GL_UNIFORM_BLOCK_BINDING, &bindingPoint);
        glGetActiveUniformBlockiv(m_programID, index, GL_UNIFORM_BLOCK_DATA_SIZE, &byteSize);

        const std::string name(nameBuffer.data(), static_cast<size_t>(nameLength));
        m_uniformBlockIndices[name] = m_uniformBlocks.size();
        m_uniformBlocks.push_back({ name, index, static_cast<unsigned int>(bindingPoint), byteSize });
    }
}

int Program::getUniformLocation(const std::string& uniformName) const noexcept
{
    const auto found = m_uniformLocations.find(uniformName);
    return found == m_uniformLocations.end() ? -1 : found->second;
}

const UniformInfo* Program::getUniformInfo(const std::string& uniformName) const noexcept
{
    const auto found = m_uniformInfoIndices.find(uniformName);
    if (found == m_uniformInfoIndices.end())
        return nullptr;
    return &m_uniforms[found->second];
}

bool Program::hasUniform(const std::string& uniformName) const noexcept
{
    return getUniformLocation(uniformName) >= 0;
}

bool Program::bindUniformBlock(const std::string& blockName, unsigned int bindingPoint)
{
    const auto found = m_uniformBlockIndices.find(blockName);
    if (found == m_uniformBlockIndices.end())
        return false;

    UniformBlockInfo& block = m_uniformBlocks[found->second];
    glUniformBlockBinding(m_programID, block.index, bindingPoint);
    block.bindingPoint = bindingPoint;
    return true;
}

bool Program::setUniform(const std::string& uniformName, bool value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Bool)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform1i(location, value ? GL_TRUE : GL_FALSE);
    return true;
}

bool Program::setUniform(const std::string& uniformName, float value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Float)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform1f(location, value);
    return true;
}

bool Program::setUniform(const std::string& uniformName, int value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Int)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform1i(location, value);
    return true;
}

bool Program::setUniform(const std::string& uniformName, const math::vec2& value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Float2)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform2f(location, value.x, value.y);
    return true;
}

bool Program::setUniform(const std::string& uniformName, const math::vec3& value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Float3)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform3f(location, value.x, value.y, value.z);
    return true;
}

bool Program::setUniform(const std::string& uniformName, const math::ivec3& value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Int3)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform3i(location, value.x, value.y, value.z);
    return true;
}

bool Program::setUniform(const std::string& uniformName, const math::vec4& value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Float4)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniform4f(location, value.x, value.y, value.z, value.w);
    return true;
}

bool Program::setUniform(const std::string& uniformName, const math::mat4& value) const
{
    const UniformInfo* uniform = getUniformInfo(uniformName);
    if (uniform == nullptr || uniform->type != ShaderValueType::Matrix4)
        return false;
    const int location = getUniformLocation(uniformName);
    if (location < 0)
        return false;
    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    return true;
}

} // namespace Galaxy
