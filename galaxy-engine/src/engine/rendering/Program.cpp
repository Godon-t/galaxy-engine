#include "Program.hpp"

#include "Log.hpp"
#include "OpenglHelper.hpp"
#include "gl_headers.hpp"
#include "pch.hpp"

#include <fstream>
#include <sstream>

using namespace math;
namespace Galaxy {
static GLenum shaderTypeFromString(const std::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;

    return 0;
}
void Program::compile(unsigned int shaderID, const char* content)
{
    // printf("Compiling shader : %s\n", vertex_file_path);
    char const* sourcePointer = content;
    glShaderSource(shaderID, 1, &sourcePointer, NULL);
    glCompileShader(shaderID);

    // Check Shader
    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(shaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
}

std::unordered_map<unsigned int, std::string> Program::preProcess(const std::string& source)
{
    std::unordered_map<unsigned int, std::string> res;

    const char* typeToken  = "#type";
    size_t typeTokenLength = strlen(typeToken);
    size_t pos             = source.find(typeToken, 0);
    while (pos != std::string::npos) {
        size_t eol = source.find_first_of("\r\n", pos);
        GLX_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t begin     = pos + typeTokenLength + 1;
        std::string type = source.substr(begin, eol - begin);

        unsigned int typeEnum = shaderTypeFromString(type);
        GLX_CORE_ASSERT(typeEnum != 0, "Unknown shader type '{0}'", type);

        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        pos                = source.find(typeToken, nextLinePos);
        res[typeEnum]      = source.substr(
            nextLinePos,
            pos - (nextLinePos == source.size() ? source.size() - 1 : nextLinePos));
    }

    return res;
}

void Program::link(unsigned int vertID, unsigned int fragID)
{
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertID);
    glAttachShader(m_programID, fragID);
    glLinkProgram(m_programID);

    // check
    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &Result);
    glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(m_programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(m_programID, vertID);
    glDetachShader(m_programID, fragID);

    glDeleteShader(vertID);
    glDeleteShader(fragID);
}

void Program::init(const char* vertexContent, const char* fragmentContent)
{
    GLuint VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLX_CORE_INFO("Compiling vertex shader");
    compile(VertexShaderID, vertexContent);

    GLX_CORE_INFO("Compiling fragment shader");
    compile(FragmentShaderID, fragmentContent);

    GLX_CORE_INFO("Linking program");
    link(VertexShaderID, FragmentShaderID);

    glUseProgram(m_programID);

    m_modelLocation      = glGetUniformLocation(m_programID, "model");
    m_viewLocation       = glGetUniformLocation(m_programID, "view");
    m_projectionLocation = glGetUniformLocation(m_programID, "projection");

    mat4 view = lookAt(vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0));
    updateViewMatrix(view);
    mat4 projection = perspective(radians(45.f), 16.f / 9.f, 0.1f, 9999.0f);
    updateProjectionMatrix(projection);
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
    // Read the Vertex Shader code from the file
    std::string shaderCode;
    std::ifstream shaderStream(shaderPath, std::ios::in);

    if (!shaderStream.is_open()) {
        GLX_CORE_ERROR("Can't open shader '{0}'", shaderPath);
        return;
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    shaderCode = sstr.str();
    shaderStream.close();

    auto shaders = preProcess(shaderCode);

    init(shaders[GL_VERTEX_SHADER].c_str(), shaders[GL_FRAGMENT_SHADER].c_str());
}

Program::Program(Program&& other) noexcept
{
    m_programID          = other.m_programID;
    other.m_programID    = 0;
    m_modelLocation      = other.m_modelLocation;
    m_viewLocation       = other.m_viewLocation;
    m_projectionLocation = other.m_projectionLocation;
}

Program& Program::operator=(Program&& other) noexcept
{
    m_programID          = other.m_programID;
    other.m_programID    = 0;
    m_modelLocation      = other.m_modelLocation;
    m_viewLocation       = other.m_viewLocation;
    m_projectionLocation = other.m_projectionLocation;
    return *this;
}

Program::~Program()
{
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

void Program::updateViewMatrix(const mat4& v)
{
    glUniformMatrix4fv(m_viewLocation, 1, GL_FALSE, &v[0][0]);
}
void Program::updateProjectionMatrix(const mat4& p)
{
    glUniformMatrix4fv(m_projectionLocation, 1, GL_FALSE, &p[0][0]);
}
void Program::updateModelMatrix(const mat4& model)
{
    glUniformMatrix4fv(m_modelLocation, 1, GL_FALSE, &model[0][0]);
}

void Program::use()
{
    glUseProgram(m_programID);
    checkOpenGLErrors("Program usage");
}

void Program::setUniform(const char* uniformName, float value)
{
    glUniform1f(glGetUniformLocation(m_programID, uniformName), value);
}

void Program::setUniform(const char* uniformName, int value)
{
    glUniform1i(glGetUniformLocation(m_programID, uniformName), value);
}

void Program::setUniform(const char* uniformName, vec2 value)
{
    glUniform2f(glGetUniformLocation(m_programID, uniformName), value.x, value.y);
}

ProgramPBR::ProgramPBR(std::string path)
    : Program(path)
{
    auto programID       = getProgramID();
    albedoLocation       = glGetUniformLocation(programID, "albedoVal");
    metallicLocation     = glGetUniformLocation(programID, "metallicVal");
    roughnessLocation    = glGetUniformLocation(programID, "roughnessVal");
    ambientLocation      = glGetUniformLocation(programID, "aoVal");
    transparencyLocation = glGetUniformLocation(programID, "transparencyVal");

    albedoTexLocation    = glGetUniformLocation(programID, "albedoMap");
    metallicTexLocation  = glGetUniformLocation(programID, "metallicMap");
    ambientTexLocation   = glGetUniformLocation(programID, "aoMap");
    normalTexLocation    = glGetUniformLocation(programID, "normalMap");
    roughnessTexLocation = glGetUniformLocation(programID, "roughnessMap");

    useAlbedoMapLocation    = glGetUniformLocation(programID, "useAlbedoMap");
    useNormalMapLocation    = glGetUniformLocation(programID, "useNormalMap");
    useMetallicMapLocation  = glGetUniformLocation(programID, "useMetallicMap");
    useRoughnessMapLocation = glGetUniformLocation(programID, "useRoughnessMap");
    useAmbientMapLocation   = glGetUniformLocation(programID, "useAoMap");

    lightSpaceMatrixLocation = glGetUniformLocation(programID, "lightSpaceMatrix");

    use();
    glUniform1i(glGetUniformLocation(programID, "useIrradianceMap"), GL_FALSE);
}

void ProgramPBR::updateMaterial(MaterialInstance& material, std::array<Texture, TextureType::COUNT>& materialTextures)
{
    glUniform1f(metallicLocation, material.metallic);
    glUniform1f(roughnessLocation, material.roughness);
    glUniform1f(ambientLocation, material.ambient);
    glUniform3f(albedoLocation, material.albedo[0], material.albedo[1], material.albedo[2]);
    glUniform1f(transparencyLocation, material.transparency);

    auto activateTexture = [&material, &materialTextures](TextureType type, GLuint useLocation, GLuint mapLocation) {
        glUniform1i(useLocation, material.useImage[type]);
        if (material.useImage[type])
            materialTextures[type].activate(mapLocation);
    };

    activateTexture(ALBEDO, useAlbedoMapLocation, albedoTexLocation);
    activateTexture(METALLIC, useMetallicMapLocation, metallicTexLocation);
    activateTexture(ROUGHNESS, useRoughnessMapLocation, roughnessTexLocation);
    activateTexture(NORMAL, useNormalMapLocation, normalTexLocation);
    activateTexture(AO, useAmbientMapLocation, ambientTexLocation);
}

void ProgramPBR::setLightSpaceMatrix(const mat4& lightSpaceMatrix)
{
    glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}

ProgramSkybox::ProgramSkybox(std::string path)
    : Program(path)
{
    m_skyboxMapLocation = glGetUniformLocation(getProgramID(), "skybox");
}
ProgramTexture::ProgramTexture(std::string path)
    : Program(path)
{
}

ProgramUnicolor::ProgramUnicolor(std::string path)
    : Program(path)
{
    m_colorLocation = glGetUniformLocation(getProgramID(), "objectColor");
}

void ProgramUnicolor::setColor(const vec3& color)
{
    glUniform3f(m_colorLocation, color.r, color.g, color.b);
}

ProgramPostProc::ProgramPostProc(std::string path)
    : Program(path)
{
    m_colorLocation = glGetUniformLocation(getProgramID(), "sceneBuffer");
    m_depthLocation = glGetUniformLocation(getProgramID(), "depthBuffer");
}
void ProgramPostProc::setTextures(unsigned int colorTexture, unsigned int depthTexture)
{
    int actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(m_colorLocation, actInt);

    actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(m_depthLocation, actInt);
}

ProgramShadow::ProgramShadow(std::string path)
    : Program(path)
{
    m_lightSpaceMatrixLocation = glGetUniformLocation(getProgramID(), "lightSpaceMatrix");
}

void ProgramShadow::setLightSpaceMatrix(const mat4& lightSpaceMatrix)
{
    glUniformMatrix4fv(m_lightSpaceMatrixLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}
}