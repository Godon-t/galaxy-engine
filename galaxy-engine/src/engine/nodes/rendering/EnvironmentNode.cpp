#include "EnvironmentNode.hpp"

#include "project/Project.hpp"
#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

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

void EnvironmentNode::loadEnv(ResourceHandle<Environment> env)
{
    m_env = std::move(env);
    Renderer::getInstance().getFrontend().setEnvironment(m_env);
}

void EnvironmentNode::enteredRoot()
{
}

} // namespace Galaxy
