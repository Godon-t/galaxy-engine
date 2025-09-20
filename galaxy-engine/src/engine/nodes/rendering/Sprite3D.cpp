#include "Sprite3D.hpp"

#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {
Sprite3D::~Sprite3D()
{
    if (m_initialized) {
        Renderer::getInstance().clearTexture(m_textureID);
    }
}
void Sprite3D::draw()
{
    if (m_initialized) {
        Renderer::getInstance().changeUsedProgram(TEXTURE);
        Renderer::getInstance().bindTexture(m_textureID, "sampledTexture");
        Renderer::getInstance().submit(m_rectID, m_transform);
        Renderer::getInstance().changeUsedProgram(PBR);
    }
}
void Sprite3D::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void Sprite3D::loadTexture(std::string path)
{
    if (m_initialized) {
        Renderer::getInstance().clearTexture(m_textureID);
    }

    auto resource = ResourceManager::getInstance().load<Image>(path);
    m_imageID     = resource.getResource().getResourceID();
    m_textureID   = Renderer::getInstance().instanciateTexture(resource);

    resource.getResource().onLoaded([this, path] {
        auto res      = ResourceManager::getInstance().load<Image>(path);
        float max     = (float)std::max(res.getResource().getWidth(), res.getResource().getHeight());
        m_rectID      = generateRect(vec2(res.getResource().getWidth() / max, res.getResource().getHeight() / max));
        m_initialized = true;
    });
}
void Sprite3D::enteredRoot()
{
    // loadTexture(std::string("Cube_BaseColor.gres"));
}
renderID Sprite3D::generateRect(vec2 dimmensions)
{
    return Renderer::getInstance().generateQuad(dimmensions, [] {});
}
} // namespace Galaxy
