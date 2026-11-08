#include "EnvironmentNode.hpp"

#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"

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

void EnvironmentNode::setEnvironmentMap(EnvironmentMapSource environmentMap)
{
    const auto current = getEnvironmentMapResource();
    const auto next = Galaxy::getEnvironmentMapResource(environmentMap);
    if (current.hasResource() == next.hasResource()
        && (!next.hasResource() || &current.getResource() == &next.getResource()))
        return;

    m_environmentMap = std::move(environmentMap);
    Renderer::getInstance().getFrontend().setEnvironmentMap(m_environmentMap);
}

bool EnvironmentNode::loadEnvironmentMap(const std::string& path)
{
    if (path.empty() || !Project::doesPathExist(path)) {
        GLX_CORE_WARN("Environment map resource is not registered: '{0}'", path);
        return false;
    }
    try {
        const auto type = IResourceMaker::readType((std::filesystem::path(Project::getProjectRootPath()) / path).string());
        auto& manager = ResourceManager::getInstance();
        if (type == "Image") {
            setEnvironmentMap(manager.load<Image>(path));
            return true;
        }
        if (type == "CubemapResource") {
            setEnvironmentMap(manager.load<CubemapResource>(path));
            return true;
        }
        GLX_CORE_WARN("Environment map must be an Image or CubemapResource: '{0}'", path);
    } catch (const YAML::Exception& error) {
        GLX_CORE_WARN("Cannot read environment map '{0}': {1}", path, error.what());
    }
    return false;
}

} // namespace Galaxy
