#pragma once

#include "engine/types/Math.hpp"
#include "engine/types/Render.hpp"

using namespace math;

namespace Galaxy {
class VisualInstance {
    int m_nbOfIndices = 0;
    unsigned int m_VAO, m_VBO, m_EBO;

public:
    VisualInstance();
    ~VisualInstance();

    VisualInstance(const VisualInstance&)            = delete;
    VisualInstance& operator=(const VisualInstance&) = delete;

    VisualInstance(VisualInstance&&);
    VisualInstance& operator=(VisualInstance&&);

    void init(const std::vector<Vertex>& vertices, const std::vector<short unsigned int>& indices);
    void draw();
};
}