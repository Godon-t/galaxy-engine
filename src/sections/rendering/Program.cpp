#include <src/sections/rendering/Program.hpp>

#include <GL/glew.h>

#include <common/shader.hpp>
#include <src/sections/rendering/OpenglHelper.hpp>


Program::Program(const char *vertexPath, const char *fragmentPath){
    programID = LoadShaders( vertexPath, fragmentPath );
    glUseProgram(programID);

    // modelLocation = glGetUniformLocation(programID, "model");
    // vLocation = glGetUniformLocation(programID, "v");
    // pLocation = glGetUniformLocation(programID, "p");

    // glm::mat4 p = Camera::getInstance().getP();
    // updateProjectionMatrix(p);
    // glm::mat4 camProj = Camera::getInstance().getP();
    // updateProjectionMatrix(camProj);
    checkOpenGLErrors("Program initialization");
}

Program::Program(Program &&other) noexcept
{
    programID = other.programID;
    other.programID = 0;
}

Program &Program::operator=(Program &&other) noexcept
{
    programID = other.programID;
    other.programID = 0;
    return *this;
}

Program::~Program()
{
    if(programID != 0)
        glDeleteProgram(programID);
}

void Program::updateViewMatrix(const mat4 &v){
    glUniformMatrix4fv(vLocation, 1, GL_FALSE, &v[0][0]);
}
void Program::updateProjectionMatrix(const mat4 &p){
    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &p[0][0]);
}
void Program::updateModelMatrix(const mat4 &model){
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
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
