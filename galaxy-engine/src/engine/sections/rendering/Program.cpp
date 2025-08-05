#include "pch.hpp"
#include "Program.hpp"

#include <GL/glew.h>
#include <common/shader.hpp>

#include "OpenglHelper.hpp"
#include "engine/types/Math.hpp"

using namespace math;


Program::Program(const char *vertexPath, const char *fragmentPath){
    programID = LoadShaders( vertexPath, fragmentPath );
    glUseProgram(programID);

    m_modelLocation = glGetUniformLocation(programID, "model");
    m_viewLocation = glGetUniformLocation(programID, "view");
    m_projectionLocation = glGetUniformLocation(programID, "projection");

    // glm::mat4 p = Camera::getInstance().getP();
    // updateProjectionMatrix(p);
    // glm::mat4 camProj = Camera::getInstance().getP();
    // updateProjectionMatrix(camProj);
    mat4 view = lookAt(vec3(0,0,0), vec3(0,0,-1), vec3(0,1,0));
    updateViewMatrix(view);
    mat4 projection = perspective(radians(45.f), 16.f/9.f, 0.1f, 999.0f);
    updateProjectionMatrix(projection);
    checkOpenGLErrors("Program initialization");
}

Program::Program(const std::string &vertexPath, const std::string &fragmentPath):Program(vertexPath.c_str(), fragmentPath.c_str()){}

Program::Program(Program &&other) noexcept
{
    programID = other.programID;
    other.programID = 0;
    m_modelLocation = other.m_modelLocation;
    m_viewLocation = other.m_viewLocation;
    m_projectionLocation = other.m_projectionLocation;
}

Program &Program::operator=(Program &&other) noexcept
{
    programID = other.programID;
    other.programID = 0;
    m_modelLocation = other.m_modelLocation;
    m_viewLocation = other.m_viewLocation;
    m_projectionLocation = other.m_projectionLocation;
    return *this;
}

Program::~Program()
{
    if(programID != 0)
        glDeleteProgram(programID);
}

void Program::updateViewMatrix(const mat4 &v){
    glUniformMatrix4fv(m_viewLocation, 1, GL_FALSE, &v[0][0]);
}
void Program::updateProjectionMatrix(const mat4 &p){
    glUniformMatrix4fv(m_projectionLocation, 1, GL_FALSE, &p[0][0]);
}
void Program::updateModelMatrix(const mat4 &model){
    glUniformMatrix4fv(m_modelLocation, 1, GL_FALSE, &model[0][0]);
}

void Program::use()
{
    glUseProgram(programID);
    checkOpenGLErrors("Program usage");
}

void Program::setUniform(const char *uniformName, float value)
{
    glUniform1f(glGetUniformLocation(programID,uniformName),value);
}

void Program::setUniform(const char *uniformName, int value)
{
    glUniform1i(glGetUniformLocation(programID,uniformName),value);
}
