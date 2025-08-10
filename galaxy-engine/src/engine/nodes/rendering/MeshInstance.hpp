#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
private:
    renderID m_renderId;

public:
    void generateTriangle();

    ~MeshInstance() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
};
}
