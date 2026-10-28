#pragma once

#include "Application.hpp"
#include "data/Transform.hpp"
#include "nodes/Node.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "resource/Environment.hpp"
#include "resource/ResourceHandle.hpp"


// GLX-TODO: crash when saving (maybe when without resource attached)

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
    void loadEnv(ResourceHandle<Environment> env);
    inline uuid getEnvResourceID() const { return m_env.getResource().getResourceID(); }

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<Environment> m_env;
};
} // namespace Galaxy
