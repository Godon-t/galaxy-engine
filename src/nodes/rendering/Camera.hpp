#pragma once

#include <src/types/Render.hpp>

#include <src/nodes/Node3D.hpp>

class Camera: public Node3D {
private:
    bool current;
public:
    camID id;
    Camera();
    ~Camera() override;

    void handleInputFromBot(const InputAction& input) override;
};
