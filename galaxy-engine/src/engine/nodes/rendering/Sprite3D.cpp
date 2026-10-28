#include "Sprite3D.hpp"

#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
Sprite3D::~Sprite3D()
{
    if (m_initialized) {
        auto& backend = Renderer::getInstance().getBackend();
        backend.clearMaterial(m_materialHandle);
        backend.clearMesh(m_rect);
    }
}

void Sprite3D::draw()
{
    if (m_initialized)
        Renderer::getInstance().getFrontend().addObjectToScene(m_rect, m_materialHandle, getTransform(), m_visibilityLayer);
    Node3D::draw();
}

void Sprite3D::lightPassDraw()
{
    Node3D::lightPassDraw();
}

void Sprite3D::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void Sprite3D::loadTexture(std::string path)
{
    loadTexture(ResourceManager::getInstance().load<Image>(path));
}

void Sprite3D::loadTexture(ResourceHandle<Image> image)
{
    if (m_initialized) {
        auto& backend = Renderer::getInstance().getBackend();
        backend.clearMaterial(m_materialHandle);
        backend.clearMesh(m_rect);
        m_initialized = false;
    }
    m_imageID = image.getResource().getResourceID();
    image.getResource().onLoaded([this, image] {
        auto& backend = Renderer::getInstance().getBackend();
        auto& resource = image.getResource();
        const float maxDimension = static_cast<float>(std::max(resource.getWidth(), resource.getHeight()));
        const vec2 dimensions(resource.getWidth() / maxDimension, resource.getHeight() / maxDimension);
        m_rect = backend.generateQuad(dimensions, [] {});
        m_materialResource = ResourceManager::getInstance().createInternalResource<TextureMaterial>();
        m_materialResource.getResource().setTexture(image);
        m_materialResource.getResource().setUseTransparency(resource.hasTransparency());
        m_materialHandle = backend.instantiateMaterial(m_materialResource);
        m_initialized = true;
    });
}

void Sprite3D::enteredRoot()
{
}
} // namespace Galaxy
