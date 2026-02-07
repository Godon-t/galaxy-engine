#include "PointLight.hpp"

#include "engine/rendering/renderer/LightManager.hpp"
#include "engine/rendering/renderer/Renderer.hpp"

namespace Galaxy {

PointLight::PointLight(std::string name)
    : Light(name)
    , m_lightID(0)
    , m_visualCubeID(0)
{
}

PointLight::~PointLight()
{
    if (m_lightID != 0) {
        Renderer::getInstance().getLightManager().unregisterLight(m_lightID);
    }
    if (m_visualCubeID != 0) {
        Renderer::getInstance().getBackend().clearMesh(m_visualCubeID);
    }
}

void PointLight::enteredRoot()
{
    Light::enteredRoot();

    LightData desc;
    desc.type                 = LightType::POINTLIGHT;
    desc.transformationMatrix = getTransform()->getGlobalModelMatrix();
    m_lightID                 = Renderer::getInstance().getLightManager().registerLight(desc);

    updateLight();

    // Create visual cube
    m_visualCubeID = Renderer::getInstance().getBackend().generateCube(1.f, false, []() {});
}

void PointLight::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void PointLight::draw()
{
    // Draw visual cube
    // TODO: debug no working
    // auto& ri = Renderer::getInstance();
    // if (m_visualCubeID && ri.canDrawDebug()) {
    //     ri.changeUsedProgram(UNICOLOR);
    //     ri.setUniform("objectColor", m_color);
    //     ri.submit(m_visualCubeID, *getTransform());
    // }
}

void PointLight::updateLight()
{
    m_transform.computeModelMatrix();
    Renderer::getInstance().getLightManager().updateLightColor(m_lightID, m_color);
    Renderer::getInstance().getLightManager().updateLightTransform(m_lightID, getTransform()->getGlobalModelMatrix());
    Renderer::getInstance().getLightManager().updateLightIntensity(m_lightID, m_intensity);
    Renderer::getInstance().getLightManager().updateLightRange(m_lightID, m_range);
}

}
