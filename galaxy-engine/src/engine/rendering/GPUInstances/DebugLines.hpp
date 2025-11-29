#pragma once

#include "pch.hpp"
#include "types/Math.hpp"

namespace Galaxy {
class DebugLines {
public:
    void init();
    void addLine(math::vec3 start, math::vec3 end, math::vec3 color = math::vec3(1, 0, 0));

    void draw();
    void destroy();

private:
    void addVertex(math::vec3 pos, math::vec3 color);
    struct Vertex {
        math::vec3 position;
        math::vec3 color;
    };

    std::vector<Vertex> m_vertices;
    unsigned int m_maxLines         = 32;
    unsigned int m_currentLineCount = 0;
    float m_thickness               = 0.02f;
    unsigned int m_vao              = 0;
    unsigned int m_vbo              = 0;
};
} // namespace Galaxy
