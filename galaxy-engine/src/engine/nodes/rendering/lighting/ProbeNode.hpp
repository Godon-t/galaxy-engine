#pragma once

#include "nodes/Node3D.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "resource/CubemapResource.hpp"
#include "resource/ResourceHandle.hpp"

namespace Galaxy {
class ProbeNode : public Node3D {
public:
    ProbeNode(std::string name = "ProbeNode");
    ~ProbeNode() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void draw() override;

    void capture();
    bool saveCapture(const std::string& folder = "");
    bool loadCubemap(const std::string& resourcePath);

    CubemapHandle getCubemapHandle() const { return m_cubemap; }
    std::string getCubemapResourcePath() const
    {
        return m_cubemapResource.hasResource() ? m_cubemapResource.getResource().getPath() : std::string {};
    }
    uuid getCubemapResourceID() const;

protected:
    void enteredRoot() override;

private:
    void loadCubemapData();

    CubemapHandle m_cubemap;
    CubemapFramebufferHandle m_captureFramebuffer;
    ResourceHandle<CubemapResource> m_cubemapResource;
    unsigned int m_resolution = 512;
    bool m_capturePending = false;
};
} // namespace Galaxy
