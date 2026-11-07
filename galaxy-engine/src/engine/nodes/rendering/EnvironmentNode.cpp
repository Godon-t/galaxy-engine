#include "EnvironmentNode.hpp"

#include "rendering/Renderer.hpp"

namespace Galaxy {
EnvironmentNode::~EnvironmentNode()
{
}

void EnvironmentNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

inline void EnvironmentNode::draw()
{
}

void EnvironmentNode::setSkybox(ResourceHandle<CubemapResource> skybox)
{
    m_skybox = std::move(skybox);
    Renderer::getInstance().getFrontend().setSkybox(m_skybox);
}

void EnvironmentNode::enteredRoot()
{
}

} // namespace Galaxy
