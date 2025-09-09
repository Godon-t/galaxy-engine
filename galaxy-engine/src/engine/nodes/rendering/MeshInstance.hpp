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
        , m_initialized(false)
    {
    }
    ~MeshInstance() override;

    virtual void draw() override;

    void accept(NodeVisitor& visitor) override;
    void loadMesh(ResourceHandle<Mesh> mesh, int surfaceIdx);

    inline int getSurfaceIdx() const { return m_meshSurfaceIdx; }

    inline ResourceHandle<Mesh> getMeshResource() const { return m_meshResource; }

private:
    renderID m_renderId;

    bool m_initialized;

    ResourceHandle<Mesh> m_meshResource;

    int m_meshSurfaceIdx;
};
}
