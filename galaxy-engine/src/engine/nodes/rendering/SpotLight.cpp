#include "pch.hpp"

#include "SpotLight.hpp"
#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {

SpotLight::SpotLight(std::string name)
    : Node3D(name)
    , m_lightID(0)
    , m_shadowMapID(0)
    , m_intensity(1.0f)
    , m_color(vec3(1.0f, 1.0f, 1.0f))
    , m_cutoffAngle(45.0f)
    , m_outerCutoffAngle(55.0f)
    , m_range(10.0f)
    , m_castShadows(true)
    , m_shadowMapResolution(1024)
{
}

SpotLight::~SpotLight()
{
    if (m_shadowMapID != 0) {
        Renderer::getInstance().clearFrameBuffer(m_shadowMapID);
        m_shadowMapID = 0;
    }
    
    // TODO: Unregister light from LightManager when implemented
}

void SpotLight::enteringRoot()
{
    // TODO: Register light in LightManager
}

void SpotLight::enteredRoot()
{
    if (m_castShadows && m_shadowMapID == 0) {
        m_shadowMapID = Renderer::getInstance().instanciateFrameBuffer(
            m_shadowMapResolution,
            m_shadowMapResolution,
            FramebufferTextureFormat::DEPTH24STENCIL8
        );
    }
}

void SpotLight::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void SpotLight::setCastShadows(bool castShadows)
{
    if (m_castShadows == castShadows) {
        return;
    }
    
    m_castShadows = castShadows;
    
    // Créer ou détruire la shadowmap selon le besoin
    if (Node::nodeExists(this->id) && getInRoot()) { // si le nœud est dans la scène et existe
        if (m_castShadows && m_shadowMapID == 0) {
            m_shadowMapID = Renderer::getInstance().instanciateFrameBuffer(
                m_shadowMapResolution,
                m_shadowMapResolution,
                FramebufferTextureFormat::DEPTH24STENCIL8
            );
        } else if (!m_castShadows && m_shadowMapID != 0) {
            Renderer::getInstance().clearFrameBuffer(m_shadowMapID);
            m_shadowMapID = 0;
        }
    }
}

void SpotLight::setShadowMapResolution(unsigned int resolution)
{
    if (m_shadowMapResolution == resolution) {
        return;
    }
    
    m_shadowMapResolution = resolution;
    
    if (m_shadowMapID != 0) {
        Renderer::getInstance().resizeFrameBuffer(m_shadowMapID, resolution, resolution);
    }
}

vec3 SpotLight::getDirection() const
{

    vec3 dir = vec3(0, 0, -1);
    return normalize(getTransform()->getLocalRotationQuat() * dir);
}

mat4 SpotLight::getLightSpaceMatrix() const
{
    // Créer une matrice de projection en perspective pour le spot
    float aspect = 1.0f; // La shadowmap est carrée
    float fov = glm::radians(m_outerCutoffAngle * 2.0f);
    mat4 projection = glm::perspective(fov, aspect, 0.1f, m_range);
    
    // Créer la matrice de vue depuis la position et direction de la lumière
    vec3 position = getTransform()->getGlobalPosition();
    vec3 direction = getDirection();
    vec3 up = vec3(0, 1, 0);
    
    // Éviter que up soit parallèle à direction
    if (abs(dot(direction, up)) > 0.99f) {
        up = vec3(1, 0, 0);
    }
    
    mat4 view = glm::lookAt(position, position + direction, up);
    
    return projection * view;
}
}
