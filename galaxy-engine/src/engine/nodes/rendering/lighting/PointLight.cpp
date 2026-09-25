#include "PointLight.hpp"

#include "engine/rendering/renderer/Renderer.hpp"


namespace Galaxy {

PointLight::PointLight(std::string name)
    : Light(name)
{
}

PointLight::~PointLight()
{
    if (m_lightID != 0) {
        Renderer::getInstance().getLightManager().unregisterLight(m_lightID);
    }
    if (m_visualCube) {
        Renderer::getInstance().getBackend().clearMesh(m_visualCube);
    }
}

void PointLight::enteredRoot()
{
    Light::enteredRoot();

    LightData desc;
    desc.type                 = LightType::POINTLIGHT;
    desc.transformationMatrix = getTransform().getGlobalModelMatrix();
    desc.castShadow = m_castShadows;
    desc.color = m_color;
    desc.range = m_range;
    desc.intensity = m_intensity;

    m_lightID                 = Renderer::getInstance().getLightManager().registerLight(desc);

    // Create visual cube
    m_visualCube = Renderer::getInstance().getBackend().generateCube(1.f, false, []() {});
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
    //     ri.submit(m_visualCubeID, getTransform());
    // }
}
}
