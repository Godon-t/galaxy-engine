#pragma once

#include "data/Transform.hpp"
#include "nodes/Node.hpp"
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

    void accept(Galaxy::NodeVisitor& visitor) override;

    inline virtual void draw() override;

protected:
    virtual void enteredRoot() override;

private:
    ResourceHandle<Environment> m_env;
    renderID m_skyboxCubemapID;
    renderID m_cubeMeshID;

    Transform m_transform;
};
} // namespace Galaxy
