#pragma once

#include "GPUInstances/MaterialInstance.hpp"
#include "GPUInstances/Texture.hpp"
#include "pch.hpp"
#include "types/Math.hpp"

using namespace math;

namespace Galaxy {
class Program {
private:
    unsigned int m_programID = 0;
    int m_modelLocation      = -1;
    int m_viewLocation       = -1;
    int m_projectionLocation = -1;
    void compile(unsigned int id, const char* content);
    std::unordered_map<unsigned int, std::string> preProcess(const std::string& source);

    void link(std::vector<unsigned int> shaderIDs);

    void init(const char* vertexContent, const char* fragmentContent);
    void init(const std::unordered_map<unsigned int, std::string>& shaderContents);

public:
    Program() = default;
    Program(const char* vertexContent, const char* fragmentContent);
    Program(const std::string& vertexContent, const std::string& fragmentContent);
    Program(const std::string& shaderPath);

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    Program(const Program&)            = delete;
    Program& operator=(const Program&) = delete;

    virtual ~Program();

    void destroy();

    inline int getProgramID() const { return m_programID; }

    void updateViewMatrix(const mat4& v);
    void updateProjectionMatrix(const mat4& p);
    void updateModelMatrix(const mat4& model);

    void use();
    void setUniform(const char* uniformName, float value);
    void setUniform(const char* uniformName, int value);
    void setUniform(const char* uniformName, vec2 value);

    virtual ProgramType type() const = 0;
};

class ProgramPBR : public Program {
public:
    ProgramPBR() = default;
    ProgramPBR(std::string path);
    void updateMaterial(const MaterialInstance& mat, const std::array<Texture*, TextureType::COUNT>& materialTextures);
    void setLightSpaceMatrix(const mat4& lightSpaceMatrix);
    ProgramType type() const override { return ProgramType::PBR; }

private:
    int albedoLocation       = -1;
    int metallicLocation     = -1;
    int roughnessLocation    = -1;
    int ambientLocation      = -1;
    int transparencyLocation = -1;
    int albedoTexLocation    = -1;
    int metallicTexLocation  = -1;
    int roughnessTexLocation = -1;
    int ambientTexLocation   = -1;
    int normalTexLocation    = -1;
    int useAlbedoMapLocation    = -1;
    int useNormalMapLocation    = -1;
    int useMetallicMapLocation  = -1;
    int useRoughnessMapLocation = -1;
    int useAmbientMapLocation   = -1;
    int lightSpaceMatrixLocation = -1;

    unsigned int lightBlockidx = 0;
};

class ProgramTexture : public Program {
public:
    ProgramTexture() = default;
    ProgramTexture(std::string path);
    ProgramType type() const override { return ProgramType::TEXTURE; }
};

class ProgramUnicolor : public Program {
public:
    ProgramUnicolor() = default;
    ProgramUnicolor(std::string path);
    void setColor(const vec3& color);
    ProgramType type() const override { return ProgramType::UNICOLOR; }

private:
    int m_colorLocation = -1;
};

class ProgramSkybox : public Program {
public:
    ProgramSkybox() = default;
    ProgramSkybox(std::string path);
    ProgramType type() const override { return ProgramType::SKYBOX; }

private:
    int m_skyboxMapLocation = -1;
};

class ProgramPostProc : public Program {
public:
    ProgramPostProc() = default;
    ProgramPostProc(std::string path);
    virtual ProgramType type() const override { return ProgramType::POST_PROCESSING_PROBE; }

    void updateInverseViewMatrix(const mat4& invView);
    void updateInverseProjectionMatrix(const mat4& invProjection);
    void setTextures(unsigned int colorTexture, unsigned int normalTexture, unsigned int depthTexture, unsigned int directDiffuseTexture, unsigned int direcAmbiantTexture);

private:
    int m_inverseProjectionLocation = -1;
    int m_inverseViewLocation       = -1;
    int m_cameraPositionLocation    = -1;
    int m_depthLocation             = -1;
    int m_colorLocation             = -1;
    int m_normalLocation            = -1;
    int m_directDiffuseLocation     = -1;
    int m_directAmbiantLocation     = -1;
};

class ProgramPostProcSSGI : public ProgramPostProc {
public:
    ProgramPostProcSSGI() = default;
    ProgramPostProcSSGI(std::string path);
    ProgramType type() const override { return ProgramType::POST_PROCESSING_SSGI; }
};

class ProgramShadow : public Program {
public:
    ProgramShadow() = default;
    ProgramShadow(std::string path);
    ProgramType type() const override { return ProgramType::SHADOW_DEPTH; }

    void setLightSpaceMatrix(const mat4& lightSpaceMatrix);

private:
    int m_lightSpaceMatrixLocation = -1;
};

class ProgramComputeOctahedral : public Program {
public:
    ProgramComputeOctahedral() = default;
    ProgramComputeOctahedral(std::string path);
    ProgramType type() const override { return ProgramType::COMPUTE_OCTAHEDRAL; }
};

class ProgramDebugLines : public Program {
public:
    ProgramDebugLines() = default;
    ProgramDebugLines(std::string path);
    ProgramType type() const override { return ProgramType::NONE; }
};

} // namespace Galaxy
