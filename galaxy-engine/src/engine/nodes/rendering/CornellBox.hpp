#pragma once

#include "nodes/Node3D.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class CornellBox : public Node3D {
public:
    CornellBox(std::string name = "CornellBox")
        : Node3D(name)
    {
        // Initialiser les arrays à 0
        for (int i = 0; i < 6; i++) {
            m_facesID[i]     = 0;
            m_materialsID[i] = 0;
        }
    }
    ~CornellBox() override;

    virtual void draw() override;
    virtual void lightPassDraw() override;

    void accept(NodeVisitor& visitor) override;

protected:
    void enteredRoot() override;

private:
    renderID m_facesID[6];
    renderID m_materialsID[6];
};
}