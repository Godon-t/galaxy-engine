#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class MeshInstance : public Node3D {
private:
    renderID m_renderId;

protected:
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override;

public:
    void generateTriangle();

    MeshInstance(std::string name = "MeshInstance")
        : Node3D(name)
    {
    }
    ~MeshInstance() override;

    void process(double delta) override;

    void accept(Galaxy::NodeVisitor& visitor) override;
};
}
