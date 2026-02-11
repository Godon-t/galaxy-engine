
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
        Renderer::getInstance().getBackend().clearMesh(m_probeVisu);
}

void GINode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}
void GINode::draw()
{
    // TODO: integrate in sceneContext
    if (m_probeVisu != 0) {
        Renderer::getInstance().getFrontend().changeUsedProgram(ProgramType::UNICOLOR);
        Renderer::getInstance().getFrontend().setUniform("objectColor", vec3(0, 1, 0));
        for (auto& transfo : m_probeTransforms) {
            Renderer::getInstance().getFrontend().submit(m_probeVisu, transfo);
        }
    }
}
void GINode::updateProbes()
{
    auto& lm = Renderer::getInstance().getLightManager();
    lm.resizeProbeFieldGrid(m_probeGrid.x, m_probeGrid.y, m_probeGrid.z, m_spaceBetween, m_probeResolution, getTransform()->getGlobalPosition());
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
    m_probeVisu = Renderer::getInstance().getBackend().generateCube(1.0, false, []() {});
}
} // namespace Galaxy