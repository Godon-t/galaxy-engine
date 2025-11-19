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
        updateRendering = true;
        // Renderer::getInstance().applyFilterOnCubemap(m_cubeMeshID, provisoryCubemap, m_irradianceCubemapID, FilterEnum::IRRADIANCE);

        // Renderer::getInstance().bindCubemap(m_irradianceCubemapID, "irradianceMap");
        // Renderer::getInstance().setUniform("useIrradianceMap", true);
    }
    void loadEnv(ResourceHandle<Environment> env);
    inline uuid getEnvResourceID() const { return m_env.getResource().getResourceID(); }

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<Environment> m_env;
    renderID m_skyboxCubemapID;
    renderID m_irradianceCubemapID;
    renderID m_cubeMeshID;
    renderID m_renderingCubemap;

    bool updateRendering;

    Transform m_transform;
};
} // namespace Galaxy
