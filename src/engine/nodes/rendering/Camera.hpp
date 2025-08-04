#pragma once

#include "engine/types/Render.hpp"

#include "engine/nodes/Node3D.hpp"

class Camera: public Node3D {
private:
    bool current;
public:
    camID id;
    Camera();
    ~Camera() override;

    void handleInputFromBot(const InputAction& input) override;
};
