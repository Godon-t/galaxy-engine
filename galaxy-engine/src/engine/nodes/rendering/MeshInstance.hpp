#pragma once

#include "nodes/Node3D.hpp"
#include "resource/ResourceManager.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "types/Render.hpp"
#include "common/BitMask.hpp"
#include "rendering/frontend/RenderItem.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
public:
    MeshInstance(std::string name = "MeshInstance");
    ~MeshInstance() override;

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(NodeVisitor& visitor) override;
    void loadMesh(std::string path);
    void loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);
    void setMaterial(ResourceHandle<BaseMaterial> material);

    inline int getSurfaceIdx() const { return m_meshSurfaceIdx; }
    GeometryHandle getGeometryHandle() const { return m_renderItem.geometry; }

    inline ResourceHandle<Mesh> getMeshResource() const { return m_meshResource; }

    inline ResourceHandle<BaseMaterial> getMaterial() const { return m_materialResource; }
    inline MaterialHandle getMaterialId() const { return m_renderItem.material.has_value() ? m_renderItem.material.value() : MaterialHandle(); }

    BitMask getVisibilityLayer() const { return m_renderItem.visibilityLayer; }
    void setVisibilityLayer(const BitMask& layer) { m_renderItem.visibilityLayer = layer; }

    uint32_t getRenderItemId() { return m_renderItem.objectId; }

private:
    RenderItem m_renderItem;

    ResourceHandle<Mesh> m_meshResource;
    ResourceHandle<BaseMaterial> m_materialResource;

    int m_meshSurfaceIdx;

    static std::queue<uint32_t> s_availableIDs;
};
}
