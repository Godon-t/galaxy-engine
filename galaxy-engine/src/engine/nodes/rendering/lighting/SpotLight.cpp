#include "pch.hpp"

#include "Application.hpp"
#include "SpotLight.hpp"
#include "rendering/renderer/LightManager.hpp"
#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {

SpotLight::SpotLight(std::string name)
    : Light(name)
    , m_innerCutoffAngle(20.0f)
    , m_outerCutoffAngle(45.0f)
    , m_castShadows(true)
    , m_initialized(false)
{
}

SpotLight::~SpotLight()
{
    if (m_visualPyramid)
        Renderer::getInstance().getBackend().clearMesh(m_visualPyramid);

    Renderer::getInstance().getLightManager().unregisterLight(m_lightID);
}

void SpotLight::enteredRoot()
{
    LightData desc;
    desc.type                 = LightType::SPOTLIGHT;
    desc.transformationMatrix = getTransform().getGlobalModelMatrix();
    desc.castShadow = m_castShadows;
    desc.color = m_color;
    desc.range = m_range;
    desc.intensity = m_intensity;
    desc.innerCutoff = m_innerCutoffAngle;
    desc.outerCutoff = m_outerCutoffAngle;

    m_lightID = Renderer::getInstance().getLightManager().registerLight(desc);
    // Créer la pyramide de visualisation
    // La base de la pyramide est orientée dans la direction de projection (vers -Z local)
    if (!m_visualPyramid) {
        m_visualPyramid  = Renderer::getInstance().getBackend().generatePyramid(0.3f, 0.5f, []() {});
        m_initialized = true;
    }
}

void SpotLight::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void SpotLight::draw()
{
    // Dessiner la pyramide de visualisation si initialisé
    if (m_initialized && m_visualPyramid) {
        // Renderer::getInstance().changeUsedProgram(UNICOLOR);
        // Renderer::getInstance().setUnicolorObjectColor(m_color);

        // auto& ri = Renderer::getInstance();

        // ri.getFrontend().changeUsedProgram(UNICOLOR);
        // ri.getFrontend().setUniform("objectColor", m_color);
        // ri.getFrontend().submit(m_visualPyramidID, m_transform);

        // TODO: integrate in sceneContext
        // ri.getFrontend().changeUsedProgram(TEXTURE);
        // ri.getFrontend().bindTexture(ri.getLightManager().getShadowMapID(m_lightID), "sampledTexture");
        // ri.getFrontend().submit(m_debugShadowMapID, m_transform);
    }

    // Appeler le draw de la classe parente pour dessiner les enfants
    Node3D::draw();
}

void SpotLight::setInnerCutoffAngle(float angle)
{
    m_innerCutoffAngle = angle;
    m_outerCutoffAngle = std::max(m_innerCutoffAngle, m_outerCutoffAngle);
    Renderer::getInstance().getLightManager().updateLightCutoffs(m_lightID, m_innerCutoffAngle, m_outerCutoffAngle);
}

void SpotLight::setOuterCutoffAngle(float angle)
{
    m_outerCutoffAngle = angle;
    m_innerCutoffAngle = std::min(m_innerCutoffAngle, m_outerCutoffAngle);
    Renderer::getInstance().getLightManager().updateLightCutoffs(m_lightID, m_innerCutoffAngle, m_outerCutoffAngle);
}

vec3 SpotLight::getDirection() const
{

    vec3 dir = vec3(0, 0, -1);
    return normalize(getTransform().getLocalRotationQuat() * dir);
}

mat4 SpotLight::getLightSpaceMatrix() const
{
    // Créer une matrice de projection en perspective pour le spot
    float aspect    = 1.0f; // La shadowmap est carrée
    float fov       = glm::radians(m_outerCutoffAngle * 2.0f);
    mat4 projection = glm::perspective(fov, aspect, 0.1f, m_range);

    // Créer la matrice de vue depuis la position et direction de la lumière
    vec3 position  = getTransform().getGlobalPosition();
    vec3 direction = getDirection();
    vec3 up        = vec3(0, 1, 0);

    // Éviter que up soit parallèle à direction
    if (abs(dot(direction, up)) > 0.99f) {
        up = vec3(1, 0, 0);
    }

    mat4 view = glm::lookAt(position, position + direction, up);

    return projection * view;
}
}
