#pragma once

#include "nodes/Node3D.hpp"
#include "project/UUID.hpp"
#include "resource/ResourceManager.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
public:
    MeshInstance(std::string name = "MeshInstance")
        : Node3D(name)
        , m_meshSurfaceIdx(-1)
        , m_renderId(0)
        , m_materialId(0)
    {
    }
    ~MeshInstance() override;

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(NodeVisitor& visitor) override;
    void loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);

    inline int getSurfaceIdx() const { return m_meshSurfaceIdx; }

    inline ResourceHandle<Mesh> getMeshResource() const { return m_meshResource; }

    inline ResourceHandle<Material> getMaterial() const { return m_materialResource; }
    inline renderID getMaterialId() const { return m_materialId; }

private:
    renderID m_renderId;
    renderID m_materialId;

    ResourceHandle<Mesh> m_meshResource;
    ResourceHandle<Material> m_materialResource;

    int m_meshSurfaceIdx;
};
}
