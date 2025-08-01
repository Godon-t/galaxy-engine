#pragma once

#include <src/types/Render.hpp>

#include <src/nodes/Node3D.hpp>

class MeshInstance: Node3D {
private:
    renderID renderId;
public:
    void generateTriangle();

    ~MeshInstance();
};
