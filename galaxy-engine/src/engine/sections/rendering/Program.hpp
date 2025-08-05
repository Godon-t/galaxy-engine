#pragma once

#include <GL/glew.h>

#include "engine/types/Math.hpp"

using namespace math;

class Program {
    private:
    GLuint m_modelLocation, m_viewLocation, m_projectionLocation;

    public:
    GLuint programID;

    Program() = default;
    Program(const char *vertexPath, const char *fragmentPath);
    Program(const std::string& vertexPath, const std::string& fragmentPath);
 
    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;


    ~Program();

    void updateViewMatrix(const mat4 &v);
    void updateProjectionMatrix(const mat4 &p);
    void updateModelMatrix(const mat4 &model);


    void use();
    void setUniform(const char* uniformName,float value);
    void setUniform(const char* uniformName, int value);
};