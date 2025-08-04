#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

class MeshInstance: public Node3D {
private:
    renderID renderId;
public:
    void generateTriangle();

    ~MeshInstance() override;
};
