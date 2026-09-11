#include "Sprite3D.hpp"

#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
Sprite3D::~Sprite3D()
{
    if (m_initialized) {
        Renderer::getInstance().getBackend().clearTexture(m_texture);
    }
}
void Sprite3D::draw()
{
    if (m_initialized) {
        // TODO: integrate in sceneContext
        Renderer::getInstance().getFrontend().changeUsedProgram(TEXTURE);
        Renderer::getInstance().getFrontend().bindTexture(m_texture, "sampledTexture");
        Renderer::getInstance().getFrontend().submit(m_rect, m_transform);
    }
}
void Sprite3D::lightPassDraw()
{
    if (m_initialized)
        Renderer::getInstance().getFrontend().submit(m_rect, m_transform);
}
void Sprite3D::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void Sprite3D::loadTexture(std::string path)
{
    if (m_initialized) {
        Renderer::getInstance().getBackend().clearTexture(m_texture);
    }

    auto resource = ResourceManager::getInstance().load<Image>(path);
    m_imageID     = resource.getResource().getResourceID();
    m_texture     = Renderer::getInstance().getBackend().instantiateTexture(resource);

    resource.getResource().onLoaded([this, path] {
        auto res      = ResourceManager::getInstance().load<Image>(path);
        float max     = (float)std::max(res.getResource().getWidth(), res.getResource().getHeight());
        
        vec2 rectDimension(res.getResource().getWidth() / max, res.getResource().getHeight() / max);
        m_rect = Renderer::getInstance().getBackend().generateQuad(rectDimension, [] {});
        m_initialized = true;
    });
}
void Sprite3D::enteredRoot()
{
    // loadTexture(std::string("Cube_BaseColor.gres"));
}
} // namespace Galaxy
