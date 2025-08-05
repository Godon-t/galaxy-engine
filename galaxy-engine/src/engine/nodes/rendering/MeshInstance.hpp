#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

class MeshInstance: public Node3D {
private:
    renderID m_renderId;
public:
    void generateTriangle();

    ~MeshInstance() override;
};
