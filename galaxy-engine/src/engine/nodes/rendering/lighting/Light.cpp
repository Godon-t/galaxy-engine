#include "Light.hpp"

#include "engine/rendering/renderer/LightManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy
{
    void Light::setIntensity(float intensity){ 
        m_intensity = intensity; 
        Renderer::getInstance().getLightManager().updateLightIntensity(m_lightID, m_intensity);
    }

    float Light::getIntensity() const
    { 
        return m_intensity; 
    }

    void Light::setColor(const vec3& color)
    { 
        m_color = color; 
        Renderer::getInstance().getLightManager().updateLightColor(m_lightID, m_color);
    }

    vec3 Light::getColor() const
    { 
        return m_color; 
    }

    void Light::setRange(float range)
    { 
        m_range = range; 
        Renderer::getInstance().getLightManager().updateLightRange(m_lightID, m_range);
    }

    float Light::getRange() const
    { 
        return m_range; 
    }

    void Light::setCastShadows(bool castShadows)
    { 
        m_castShadows = castShadows; 
        Renderer::getInstance().getLightManager().updateLightCastShadow(m_lightID, m_castShadows);
    }

    bool Light::getCastShadows() const
    { 
        return m_castShadows; 
    }

    void Light::onWorldTransformChanged()
    {
        Renderer::getInstance().getLightManager().updateLightTransform(m_lightID, getTransform().getGlobalModelMatrix());
    }

} // namespace Galaxy

