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

    inline float getCutoffAngle() const { return m_cutoffAngle; }
    inline float getOuterCutoffAngle() const { return m_outerCutoffAngle; }
    inline bool getCastShadows() const { return m_castShadows; }

    void setCutoffAngle(float angle) { m_cutoffAngle = angle; }
    void setOuterCutoffAngle(float angle) { m_outerCutoffAngle = angle; }
    void setCastShadows(bool castShadows);

    void updateLight();

private:
    lightID m_lightID;
    renderID m_debugShadowMapID; // For debugging purposes
    renderID m_visualPyramidID; // Mesh ID for the pyramid visualisation

    float m_cutoffAngle;
    float m_outerCutoffAngle;

    bool m_castShadows;
    bool m_initialized;
};
}
