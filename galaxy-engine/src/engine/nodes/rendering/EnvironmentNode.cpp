#include "EnvironmentNode.hpp"

#include "rendering/Renderer.hpp"

#include <algorithm>
#include <cmath>

namespace Galaxy {
EnvironmentNode::~EnvironmentNode()
{
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().unregisterLight(m_directionalLightID);
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
    if (m_directionalLightID < 0)
        m_directionalLightID = Renderer::getInstance().getLightManager().registerDirectionalLight(m_direction, m_color, m_intensity);
}

void EnvironmentNode::setDirection(const vec3& direction)
{
    const float magnitude = length(direction);
    if (!std::isfinite(magnitude) || magnitude < 1e-6f)
        return;
    m_direction = direction / magnitude;
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightDirection(m_directionalLightID, m_direction);
}

void EnvironmentNode::setColor(const vec3& color)
{
    if (!std::isfinite(color.x) || !std::isfinite(color.y) || !std::isfinite(color.z))
        return;
    m_color = glm::max(color, vec3(0.0f));
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightColor(m_directionalLightID, m_color);
}

void EnvironmentNode::setIntensity(float intensity)
{
    if (!std::isfinite(intensity))
        return;
    m_intensity = std::max(0.0f, intensity);
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightIntensity(m_directionalLightID, m_intensity);
}

} // namespace Galaxy
