#pragma once

#include "nodes/Node3D.hpp"
#include "project/UUID.hpp"
#include "resource/ResourceManager.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Render.hpp"
#include "common/BitMask.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
public:
    MeshInstance(std::string name = "MeshInstance")
        : Node3D(name)
        , m_meshSurfaceIdx(-1)
    {
    }
    ~MeshInstance() override;

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(NodeVisitor& visitor) override;
    void loadMesh(std::string path);
    void loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    void setMaterial(ResourceHandle<BaseMaterial> material);

    inline int getSurfaceIdx() const { return m_meshSurfaceIdx; }

    inline ResourceHandle<Mesh> getMeshResource() const { return m_meshResource; }

    inline ResourceHandle<BaseMaterial> getMaterial() const { return m_materialResource; }
    inline MaterialHandle getMaterialId() const { return m_materialHandle; }

    BitMask getVisibilityLayer() const { return m_visibilityLayer; }
    void setVisibilityLayer(const BitMask& layer) { m_visibilityLayer = layer; }

private:
    GeometryHandle m_geometry;
    MaterialHandle m_materialHandle;

    ResourceHandle<Mesh> m_meshResource;
    ResourceHandle<BaseMaterial> m_materialResource;

    int m_meshSurfaceIdx;
    BitMask m_visibilityLayer;
};
}
