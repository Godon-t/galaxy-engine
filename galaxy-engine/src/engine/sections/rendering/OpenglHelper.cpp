#include "OpenglHelper.hpp"

#include "pch.hpp"

#include "engine/gl_headers.hpp"

#include <iostream>

namespace Galaxy {
void checkOpenGLErrors(const char* context)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* errorStr = "";
        switch (err) {
        case GL_INVALID_ENUM:
            errorStr = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorStr = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorStr = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            errorStr = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            errorStr = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            errorStr = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            errorStr = "Unknown error";
            break;
        }
        std::cerr << "[OpenGL Error] (" << errorStr << ") at: " << context << std::endl;
    }
}
}