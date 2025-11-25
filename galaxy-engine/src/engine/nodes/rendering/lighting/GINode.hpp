#pragma once

#include "nodes/Node3D.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class GINode : public Node3D {

public:
    GINode(std::string name = "GINode");
    ~GINode() override;

    void accept(Galaxy::NodeVisitor& visitor) override;

    virtual void draw() override;

    void updateProbes();

protected:
    virtual void enteredRoot() override;

private:
    renderID m_fieldsVisu;
};
// expose lightManager method for updating probes field transform
// expose lightManager method to instantiate probe sphere at the right pos (using global) inside GINode.
//

} // namespace Galaxy
