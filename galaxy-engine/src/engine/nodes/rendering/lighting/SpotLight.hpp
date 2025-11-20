#pragma once

#include "Light.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {

class SpotLight : public Light {
protected:
    virtual void enteredRoot() override;

public:
    SpotLight(std::string name = "SpotLight");
    ~SpotLight() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    virtual void draw() override;

    vec3 getDirection() const;
    mat4 getLightSpaceMatrix() const;

    inline float getIntensity() const { return m_intensity; }
    inline vec3 getColor() const { return m_color; }
    inline float getCutoffAngle() const { return m_cutoffAngle; }
    inline float getOuterCutoffAngle() const { return m_outerCutoffAngle; }
    inline float getRange() const { return m_range; }
    inline bool getCastShadows() const { return m_castShadows; }

    void setIntensity(float intensity) { m_intensity = intensity; }
    void setColor(const vec3& color) { m_color = color; }
    void setCutoffAngle(float angle) { m_cutoffAngle = angle; }
    void setOuterCutoffAngle(float angle) { m_outerCutoffAngle = angle; }
    void setRange(float range) { m_range = range; }
    void setCastShadows(bool castShadows);

    void updateLight();

private:
    lightID m_lightID;
    renderID m_debugShadowMapID; // For debugging purposes
    renderID m_visualPyramidID; // Mesh ID for the pyramid visualisation

    float m_intensity;
    vec3 m_color;
    float m_cutoffAngle;
    float m_outerCutoffAngle;
    float m_range;

    bool m_castShadows;
    bool m_initialized;
};
}
