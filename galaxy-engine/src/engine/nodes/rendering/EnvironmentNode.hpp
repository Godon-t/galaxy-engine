#pragma once

#include "nodes/Node.hpp"
#include "resource/CubemapResource.hpp"
#include "resource/ResourceHandle.hpp"

namespace Galaxy {
class EnvironmentNode : public Node {
public:
    EnvironmentNode(std::string name = "EnvironmentNode")
        : Node(name)
    {
    }

    ~EnvironmentNode();

    void accept(Galaxy::NodeVisitor& visitor) override;

    inline virtual void draw() override;
    void setSkybox(ResourceHandle<CubemapResource> skybox);
    const ResourceHandle<CubemapResource>& getSkybox() const { return m_skybox; }
    uuid getSkyboxResourceID() const { return m_skybox.hasResource() ? m_skybox.getResource().getResourceID() : uuid(0); }

    void setDirection(const vec3& direction);
    vec3 getDirection() const { return m_direction; }
    void setColor(const vec3& color);
    vec3 getColor() const { return m_color; }
    void setIntensity(float intensity);
    float getIntensity() const { return m_intensity; }

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<CubemapResource> m_skybox;
    int m_directionalLightID = -1;

    float m_intensity{1.0};
    vec3 m_color{1.0};
    bool m_castShadows{false};
    vec3 m_direction{-0.3f, -1.0f, -0.2f};
};
} // namespace Galaxy
