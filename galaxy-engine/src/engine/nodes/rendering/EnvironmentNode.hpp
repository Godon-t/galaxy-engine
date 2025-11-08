#pragma once

#include "Application.hpp"
#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/Environment.hpp"
#include "resource/ResourceHandle.hpp"

namespace Galaxy {
class EnvironmentNode : public Node {
public:
    EnvironmentNode(std::string name = "EnvironmentNode")
        : Node(name)
        , m_skyboxCubemapID(0)
    {
    }

    ~EnvironmentNode();

    void accept(Galaxy::NodeVisitor& visitor) override;

    inline virtual void draw() override;
    void testingFunc()
    {
        renderID provisoryCubemap = Renderer::getInstance().instanciateCubemap();
        Renderer::getInstance().renderFromPoint(vec3(0), *Application::getInstance().getRootNodePtr().get(), provisoryCubemap);
        Renderer::getInstance().clearCubemap(m_skyboxCubemapID);
        m_skyboxCubemapID = provisoryCubemap;
        // TODO: Delete cubemap
    }
    void loadEnv(ResourceHandle<Environment> env);

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<Environment> m_env;
    renderID m_skyboxCubemapID;
    renderID m_irradianceCubemapID;
    renderID m_cubeMeshID;

    Transform m_transform;
};
} // namespace Galaxy
