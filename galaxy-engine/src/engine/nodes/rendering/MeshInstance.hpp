#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

#include "engine/project/UUID.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
public:
    void generateTriangle();

    MeshInstance(std::string name = "MeshInstance")
        : Node3D(name)
        , m_meshSurfaceIdx(-1)
    {
    }
    ~MeshInstance() override;

    void process(double delta) override;
    virtual void draw() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void loadMesh(std::string path, int surfaceIdx);

    inline int getSurfaceIdx() const { return m_meshSurfaceIdx; }
    inline uuid getMeshResourceID() const { return m_meshResourceID; }

protected:
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override;

private:
    renderID m_renderId;
    renderID m_textureID;
    bool m_renderIDInitialized = false;

    uuid m_meshResourceID;
    int m_meshSurfaceIdx;
};
}
