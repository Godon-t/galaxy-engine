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
        // Renderer::getInstance().changeUsedProgram(BaseProgramEnum::TEXTURE);
        // Renderer::getInstance().bindTexture(m_textureID, "sampledTexture");
        // Renderer::getInstance().submit(m_rectID, m_transform);
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
    loadTexture(std::string("Cube_BaseColor.gres"));
}
renderID Sprite3D::generateRect(vec2 dimmensions)
{
    vec2 half = dimmensions / 2.f;

    std::vector<Vertex> vertices;
    Vertex v1, v2, v3, v4;
    v1.position = vec3(-half.x, half.y, 0);
    v1.texCoord = vec2(0, 0);

    v2.position = vec3(half.x, half.y, 0);
    v2.texCoord = vec2(1, 0);

    v3.position = vec3(-half.x, -half.y, 0);
    v3.texCoord = vec2(0, 1);

    v4.position = vec3(half.x, -half.y, 0);
    v4.texCoord = vec2(1, 1);

    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    std::vector<short unsigned int> indices;
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);

    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);

    return Renderer::getInstance().instanciateMesh(vertices, indices);
}
} // namespace Galaxy
