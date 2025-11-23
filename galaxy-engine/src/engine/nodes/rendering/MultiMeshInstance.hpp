#pragma once

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class MultiMeshInstance : public Node3D {
public:
    MultiMeshInstance(std::string name = "MultiMeshInstance")
        : Node3D(name)
    {
    }

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void loadMesh(std::string path);
};
}
