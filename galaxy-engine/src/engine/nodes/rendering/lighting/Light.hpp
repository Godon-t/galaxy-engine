#pragma once

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class Light : public Node3D {
public:
    Light(std::string name = "Light")
        : Node3D(name)
        , m_intensity(1.0f)
        , m_color(vec3(1.0f, 1.0f, 1.0f))
        , m_range(10.0f)
    {
    }

    void setIntensity(float intensity) { m_intensity = intensity; }
    inline float getIntensity() const { return m_intensity; }
    
    void setColor(const vec3& color) { m_color = color; }
    inline vec3 getColor() const { return m_color; }

    inline float getRange() const { return m_range; }
    void setRange(float range) { m_range = range; }


protected:
    float m_intensity;
    vec3 m_color;
    float m_range;

};
} // namespace Galaxy
