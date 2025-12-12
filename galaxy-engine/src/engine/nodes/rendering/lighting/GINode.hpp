#pragma once

#include "nodes/Node3D.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class GINode : public Node3D {

public:
    GINode(std::string name = "GINode");
    ~GINode() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    virtual void draw() override;

    void updateProbes();

    inline void setGrid(ivec3 newGrid) { m_probeGrid = newGrid; }
    inline ivec3 getGrid() { return m_probeGrid; }
    inline void setSpaceBetween(float newVal) { m_spaceBetween = newVal; }
    inline float getSpaceBetween() { return m_spaceBetween; }
    inline void setProbeResolution(unsigned int newRes) { m_probeResolution = newRes; }
    inline unsigned int getProbeResolution() { return m_probeResolution; }

protected:
    virtual void enteredRoot() override;

private:
    renderID m_probeVisu;
    std::vector<Transform> m_probeTransforms;

    ivec3 m_probeGrid;
    float m_spaceBetween;
    unsigned int m_probeResolution;
};
// expose lightManager method for updating probes field transform
// expose lightManager method to instantiate probe sphere at the right pos (using global) inside GINode.
//

} // namespace Galaxy
