#pragma once

#include "nodes/Node3D.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"

namespace Galaxy {
class ProbeNode : public Node3D {
public:
    ProbeNode(std::string name = "ProbeNode");
    ~ProbeNode() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void draw() override;

    void capture();

    bool hasCapture() const { return m_hasCapture; }
    CubemapHandle getCubemapHandle() const { return m_cubemap; }

protected:
    void enteredRoot() override;

private:
    CubemapHandle m_cubemap;
    CubemapFramebufferHandle m_captureFramebuffer;
    bool m_capturePending = false;
    bool m_hasCapture = false;
};
} // namespace Galaxy
