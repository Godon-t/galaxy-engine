#pragma once

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
};
}