#pragma once

#include "nodes/Node.hpp"
#include "resource/EnvironmentMap.hpp"

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
    void setEnvironmentMap(EnvironmentMapSource environmentMap);
    bool loadEnvironmentMap(const std::string& path);
    const EnvironmentMapSource& getEnvironmentMap() const { return m_environmentMap; }
    ResourceHandle<ResourceBase> getEnvironmentMapResource() const { return Galaxy::getEnvironmentMapResource(m_environmentMap); }
    uuid getEnvironmentMapResourceID() const {
        const auto resource = getEnvironmentMapResource();
        return resource.hasResource() ? resource.getResource().getResourceID() : uuid(0);
    }

    void setDirection(const vec3& direction);
    vec3 getDirection() const { return m_direction; }
    void setColor(const vec3& color);
    vec3 getColor() const { return m_color; }
    void setIntensity(float intensity);
    float getIntensity() const { return m_intensity; }

protected:
    virtual void enteredRoot() override;

private:
    EnvironmentMapSource m_environmentMap;
    int m_directionalLightID = -1;

    float m_intensity{1.0};
    vec3 m_color{1.0};
    bool m_castShadows{false};
    vec3 m_direction{-0.3f, -1.0f, -0.2f};
};
} // namespace Galaxy
