
#include "GINode.hpp"
#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {
GINode::GINode(std::string name)
    : m_fieldsVisu(0)
    , Node3D(name)
{
}

GINode::~GINode()
{
    if (m_fieldsVisu != 0)
        Renderer::getInstance().clearMesh(m_fieldsVisu);
}

void GINode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void GINode::draw()
{
    if (m_fieldsVisu != 0) {
        renderID tex = Renderer::getInstance().getLightManager().getProbesRadianceTexture();
        Renderer::getInstance().changeUsedProgram(TEXTURE);
        Renderer::getInstance().bindTexture(tex, "sampledTexture");
        Renderer::getInstance().submit(m_fieldsVisu);
    }
}
void GINode::updateProbes()
{
    Renderer::getInstance().getLightManager().updateProbeField();
}
void GINode::enteredRoot()
{
    m_fieldsVisu = Renderer::getInstance().generateQuad(vec2(2), []() {});
}
} // namespace Galaxy