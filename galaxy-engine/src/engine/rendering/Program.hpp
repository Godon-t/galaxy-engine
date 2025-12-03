#pragma once

#include "GPUInstances/MaterialInstance.hpp"
#include "GPUInstances/Texture.hpp"
#include "pch.hpp"
#include "types/Math.hpp"

using namespace math;

namespace Galaxy {
class Program {
private:
    unsigned int m_programID;
    unsigned int m_modelLocation, m_viewLocation, m_projectionLocation;
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

    ~Program();

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
    void updateMaterial(MaterialInstance& mat, std::array<Texture, TextureType::COUNT>& materialTextures);
    void setLightSpaceMatrix(const mat4& lightSpaceMatrix);
    ProgramType type() const override { return ProgramType::PBR; }

private:
    unsigned int albedoLocation, metallicLocation, roughnessLocation, ambientLocation, transparencyLocation;
    unsigned int albedoTexLocation, metallicTexLocation, roughnessTexLocation, ambientTexLocation, normalTexLocation;
    unsigned int useAlbedoMapLocation, useNormalMapLocation, useMetallicMapLocation, useRoughnessMapLocation, useAmbientMapLocation;
    unsigned int lightSpaceMatrixLocation;
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
    unsigned int m_colorLocation;
};

class ProgramSkybox : public Program {
public:
    ProgramSkybox() = default;
    ProgramSkybox(std::string path);
    ProgramType type() const override { return ProgramType::SKYBOX; }

private:
    unsigned int m_skyboxMapLocation;
};

class ProgramPostProc : public Program {
public:
    ProgramPostProc() = default;
    ProgramPostProc(std::string path);
    ProgramType type() const override { return ProgramType::POST_PROCESSING; }

    void updateInverseViewMatrix(const mat4& invView);
    void updateInverseProjectionMatrix(const mat4& invProjection);
    void setTextures(unsigned int colorTexture, unsigned int depthTexture);

private:
    unsigned int m_inverseProjectionLocation;
    unsigned int m_inverseViewLocation;
    unsigned int m_cameraPositionLocation;
    unsigned int m_depthLocation;
    unsigned int m_colorLocation;
};

class ProgramShadow : public Program {
public:
    ProgramShadow() = default;
    ProgramShadow(std::string path);
    ProgramType type() const override { return ProgramType::SHADOW_DEPTH; }

    void setLightSpaceMatrix(const mat4& lightSpaceMatrix);

private:
    unsigned int m_lightSpaceMatrixLocation;
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
