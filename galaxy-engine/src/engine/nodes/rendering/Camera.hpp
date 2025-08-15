#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class Camera : public Node3D {
private:
    bool m_current;

protected:
    virtual void enteringRoot() override {};
    virtual void enteredRoot() override {};

public:
    camID id;
    Camera(std::string name = "Camera");
    ~Camera() override;

    void handleInputFromBot(const Event& input) override;
    void accept(Galaxy::NodeVisitor& visitor) override;

    void setCurrent(bool state);

    inline virtual void draw() override { Node3D::draw(); }
};
}
