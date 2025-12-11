#include "UBOInstance.hpp"

#include "gl_headers.hpp"

namespace Galaxy {
void UBOInstance::init(size_t dataSize)
{
    size = dataSize;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOInstance::destroy()
{
    if (buffer)
        glDeleteBuffers(1, &buffer);
}

void UBOInstance::bind(unsigned int idx)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, idx, buffer);
}

void UBOInstance::update(const void* data, size_t dataSize)
{
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
} // namespace Galaxy
