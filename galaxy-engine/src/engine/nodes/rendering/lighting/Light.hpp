#pragma once

#include "engine/nodes/Node3D.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class Light : public Node3D {
public:
    Light(std::string name = "Light")
        : Node3D(name)
        , m_lightID(0)
        , m_intensity(1.0f)
        , m_color(vec3(1.0f, 1.0f, 1.0f))
        , m_range(10.0f)
    {
    }

    void setIntensity(float intensity);
    float getIntensity() const;
    
    void setColor(const vec3& color);
    vec3 getColor() const;

    void setRange(float range);
    float getRange() const;

    void setCastShadows(bool castShadows);
    bool getCastShadows() const;


protected:
    void onWorldTransformChanged() override;

    lightID m_lightID;

    float m_intensity;
    vec3 m_color;
    float m_range;
    bool m_castShadows = false;

};
} // namespace Galaxy
