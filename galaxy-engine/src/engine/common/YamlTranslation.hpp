#pragma once

#include "types/Math.hpp"

#include <yaml-cpp/yaml.h>

using namespace math;

namespace YAML {
Emitter& operator<<(Emitter& out, const vec3& v);
Emitter& operator<<(Emitter& out, const vec4& v);

template <>
struct convert<vec3> {
    static Node encode(const vec3& vec);
    static bool decode(const Node& node, vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
inline Node convert<vec3>::encode(const vec3& vec)
{
    Node node;
    node.push_back(vec.x);
    node.push_back(vec.y);
    node.push_back(vec.z);
    return node;
}
}

namespace YAML {
}
