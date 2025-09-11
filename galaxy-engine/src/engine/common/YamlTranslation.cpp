#include "YamlTranslation.hpp"

namespace YAML {
Emitter& operator<<(Emitter& out, const vec3& v)
{
    out << Flow;
    out << BeginSeq << v.x << v.y << v.z << EndSeq;
    return out;
}

Emitter& operator<<(Emitter& out, const vec4& v)
{
    out << Flow;
    out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
    return out;
}
} // namespace YAML
