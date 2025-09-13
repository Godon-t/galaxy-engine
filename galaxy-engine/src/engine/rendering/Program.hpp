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
    void link(unsigned int vertID, unsigned int fragID);

    void init(const char* vertexContent, const char* fragmentContent);

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

    virtual ProgramType type() const = 0;
};

class ProgramPBR : public Program {
public:
    ProgramPBR() = default;
    ProgramPBR(std::string path);
    void updateMaterial(MaterialInstance& mat, std::array<Texture, TextureType::COUNT>& materialTextures);
    ProgramType type() const override { return ProgramType::PBR; }

private:
    unsigned int albedoLocation, metallicLocation, roughnessLocation, ambientLocation;
    unsigned int albedoTexLocation, metallicTexLocation, roughnessTexLocation, ambientTexLocation, normalTexLocation;
    unsigned int useAlbedoMapLocation, useNormalMapLocation, useMetallicMapLocation, useRoughnessMapLocation, useAmbientMapLocation;
};

class ProgramSkybox : public Program {
public:
    ProgramSkybox() = default;
    ProgramSkybox(std::string path);
    ProgramType type() const override { return ProgramType::SKYBOX; }

private:
    unsigned int m_skyboxMapLocation;
};
}