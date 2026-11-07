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

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<CubemapResource> m_skybox;
};
} // namespace Galaxy
