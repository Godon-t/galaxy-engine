#pragma once

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class MultiMeshInstance : public Node3D {
protected:
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override;

public:
    MultiMeshInstance(std::string name = "MultiMeshInstance")
        : Node3D(name)
    {
    }
    ~MultiMeshInstance() override;

    void process(double delta) override;
    virtual void draw() override;

    void accept(Galaxy::NodeVisitor& visitor) override;
    void loadMesh(std::string path);
};
}
