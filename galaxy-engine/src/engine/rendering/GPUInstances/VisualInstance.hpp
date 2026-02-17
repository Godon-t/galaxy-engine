#pragma once

#include "types/Math.hpp"
#include "types/Render.hpp"
#include "common/geometry/Shapes.hpp"

using namespace math;

namespace Galaxy {
class VisualInstance {
    int m_nbOfIndices = 0;
    unsigned int m_VAO, m_VBO, m_EBO;
    CullMode m_cullMode;
    Sphere m_boundingVolume;

    
    static CullMode s_cullMode;
    
    public:
    VisualInstance();
    ~VisualInstance();
    
    VisualInstance(const VisualInstance&)            = delete;
    VisualInstance& operator=(const VisualInstance&) = delete;
    
    VisualInstance(VisualInstance&&);
    VisualInstance& operator=(VisualInstance&&);
    
    Sphere& getBoundingVolume();
    void init(const std::vector<Vertex>& vertices, const std::vector<short unsigned int>& indices);
    void draw();
    inline void setCullMode(CullMode newMode) { m_cullMode = newMode; }
};
}