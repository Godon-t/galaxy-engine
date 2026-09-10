#include "UBOInstance.hpp"

#include "gl_headers.hpp"

#include <utility>

namespace Galaxy {
UBOInstance::~UBOInstance()
{
    destroy();
}

UBOInstance::UBOInstance(UBOInstance&& other) noexcept
    : m_buffer(std::exchange(other.m_buffer, 0))
    , m_size(std::exchange(other.m_size, 0))
{
}

UBOInstance& UBOInstance::operator=(UBOInstance&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_buffer = std::exchange(other.m_buffer, 0);
    m_size   = std::exchange(other.m_size, 0);
    return *this;
}

void UBOInstance::init(size_t dataSize)
{
    destroy();
    m_size = dataSize;
    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
    glBufferData(GL_UNIFORM_BUFFER, m_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOInstance::destroy()
{
    if (m_buffer)
        glDeleteBuffers(1, &m_buffer);
    m_buffer = 0;
    m_size   = 0;
}

void UBOInstance::bind(unsigned int idx)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, idx, m_buffer);
}

void UBOInstance::update(const void* data, size_t dataSize)
{
    if (dataSize > m_size)
        return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
} // namespace Galaxy
