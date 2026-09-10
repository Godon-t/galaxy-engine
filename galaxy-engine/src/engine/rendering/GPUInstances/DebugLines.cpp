#include "DebugLines.hpp"

#include "gl_headers.hpp"

#include <utility>

namespace Galaxy {
DebugLines::~DebugLines()
{
    destroy();
}

DebugLines::DebugLines(DebugLines&& other) noexcept
    : m_vertices(std::move(other.m_vertices))
    , m_maxLines(other.m_maxLines)
    , m_currentLineCount(std::exchange(other.m_currentLineCount, 0))
    , m_thickness(other.m_thickness)
    , m_vao(std::exchange(other.m_vao, 0))
    , m_vbo(std::exchange(other.m_vbo, 0))
{
}

DebugLines& DebugLines::operator=(DebugLines&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_vertices         = std::move(other.m_vertices);
    m_maxLines         = other.m_maxLines;
    m_currentLineCount = std::exchange(other.m_currentLineCount, 0);
    m_thickness        = other.m_thickness;
    m_vao              = std::exchange(other.m_vao, 0);
    m_vbo              = std::exchange(other.m_vbo, 0);
    return *this;
}

void DebugLines::init()
{
    destroy();
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Couleur (location = 1) - si vous utilisez la version avec couleur
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_vertices.resize(m_maxLines * 2);
}
void DebugLines::addLine(math::vec3 start, math::vec3 end, math::vec3 color)

{
    addVertex(start, color);
    addVertex(end, color);
}
void DebugLines::draw()
{
    if (m_vertices.empty())
        return;

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
        m_vertices.data(), GL_DYNAMIC_DRAW);

    GLboolean cullEnabled;
    glGetBooleanv(GL_CULL_FACE, &cullEnabled);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_LINES, 0, m_vertices.size());
    if (cullEnabled)
        glEnable(GL_CULL_FACE);
}
void DebugLines::destroy()
{
    if (m_vbo != 0)
        glDeleteBuffers(1, &m_vbo);
    if (m_vao != 0)
        glDeleteVertexArrays(1, &m_vao);
    m_vbo = 0;
    m_vao = 0;
    m_currentLineCount = 0;
    m_vertices.clear();
}
void DebugLines::addVertex(math::vec3 pos, math::vec3 color)
{
    if (m_currentLineCount >= m_maxLines * 2) {
        m_currentLineCount = 0;
    }
    m_vertices[m_currentLineCount] = { pos, color };
    m_currentLineCount++;
}
} // namespace Galxy
