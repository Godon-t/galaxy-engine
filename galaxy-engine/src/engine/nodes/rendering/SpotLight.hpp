#pragma once

#include "engine/types/Render.hpp"
#include "engine/nodes/Node3D.hpp"

namespace Galaxy {

class SpotLight : public Node3D {
private:
    lightID m_lightID;
    renderID m_shadowMapID;
    
    float m_intensity;
    vec3 m_color;
    float m_cutoffAngle;
    float m_outerCutoffAngle;
    float m_range;
    
    bool m_castShadows;
    unsigned int m_shadowMapResolution;

protected:
    virtual void enteringRoot() override;
    virtual void enteredRoot() override;

public:
    SpotLight(std::string name = "SpotLight");
    ~SpotLight() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    inline virtual void draw() override { Node3D::draw(); }

    vec3 getDirection() const;
    mat4 getLightSpaceMatrix() const;
    
    inline lightID getLightID() const { return m_lightID; }
    inline renderID getShadowMapID() const { return m_shadowMapID; }
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
    void setShadowMapResolution(unsigned int resolution);
    


    
    
};
}
