#pragma once

#include "Light.hpp"
#include "engine/types/Render.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"

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

    inline float getCutoffAngle() const { return m_cutoffAngle; }
    inline float getOuterCutoffAngle() const { return m_outerCutoffAngle; }
    inline bool getCastShadows() const { return m_castShadows; }

    void setCutoffAngle(float angle) { m_cutoffAngle = angle; }
    void setOuterCutoffAngle(float angle) { m_outerCutoffAngle = angle; }
    void setCastShadows(bool castShadows);

    void updateLight();

private:
    lightID m_lightID;
    GeometryHandle m_debugShadowMap; // For debugging purposes
    GeometryHandle m_visualPyramid; // Mesh for the pyramid visualisation

    float m_cutoffAngle;
    float m_outerCutoffAngle;

    bool m_castShadows;
    bool m_initialized;
};
}
