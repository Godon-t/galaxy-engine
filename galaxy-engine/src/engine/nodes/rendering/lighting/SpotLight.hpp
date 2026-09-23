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

    inline float getInnerCutoffAngle() const { return m_innerCutoffAngle; }
    inline float getOuterCutoffAngle() const { return m_outerCutoffAngle; }

    void setInnerCutoffAngle(float angle);
    void setOuterCutoffAngle(float angle);

    void updateLight();

private:
    lightID m_lightID;
    GeometryHandle m_debugShadowMap; // For debugging purposes
    GeometryHandle m_visualPyramid; // Mesh for the pyramid visualisation

    float m_innerCutoffAngle;
    float m_outerCutoffAngle;

    bool m_castShadows;
    bool m_initialized;
};
}
