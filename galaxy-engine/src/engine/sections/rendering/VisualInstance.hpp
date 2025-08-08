#pragma once

#include "engine/types/Math.hpp"

#include <GL/glew.h>

using namespace math;

struct Vertex {
    vec3 m_position;
    vec2 m_texCoord;
};

class VisualInstance {
    int m_nbOfIndices = 0;
    GLuint m_VAO, m_VBO, m_EBO;

public:
    VisualInstance();
    ~VisualInstance();

    VisualInstance(const VisualInstance&) = delete;
    VisualInstance& operator=(const VisualInstance&) = delete;

    VisualInstance(VisualInstance&&);
    VisualInstance& operator=(VisualInstance&&);

    void init(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices);
    void draw();
};
