#pragma once

#include "engine/nodes/Node3D.hpp"

namespace Galaxy {
class Light : public Node3D {
public:
    Light(std::string name = "Light")
        : Node3D(name)
    {
    }
};
} // namespace Galaxy
