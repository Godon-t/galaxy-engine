
#include "GINode.hpp"
#include "rendering/renderer/Renderer.hpp"

namespace Galaxy {
GINode::GINode(std::string name)
    : m_probeVisu(0)
    , Node3D(name)
{
}

GINode::~GINode()
{
    if (m_probeVisu != 0)
        Renderer::getInstance().clearMesh(m_probeVisu);
}

void GINode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void GINode::draw()
{
    if (m_probeVisu != 0 && Renderer::getInstance().canDrawDebug()) {
        Renderer::getInstance().changeUsedProgram(ProgramType::UNICOLOR);
        Renderer::getInstance().setUniform("objectColor", vec3(0, 1, 0));
        for (auto& transfo : m_probeTransforms) {
            Renderer::getInstance().submit(m_probeVisu, transfo);
        }
    }
}
void GINode::updateProbes()
{
    Renderer::getInstance().getLightManager().updateProbeField();

    auto probePositions = Renderer::getInstance().getLightManager().getProbePositions();
    m_probeTransforms.resize(probePositions.size());
    for (int i = 0; i < probePositions.size(); i++) {
        m_probeTransforms[i].setLocalPosition(probePositions[i]);
        m_probeTransforms[i].computeModelMatrix();
    }
}
void GINode::enteredRoot()
{
    m_probeVisu = Renderer::getInstance().generateCube(1.0, false, []() {});
}
} // namespace Galaxy