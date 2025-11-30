// https://www.shadertoy.com/view/MdsfzN

vec2 octahedral_mapping(vec3 co)
{
    // Projection onto octahedron
    co /= dot(vec3(1), abs(co));

    // Out-folding of the downward faces
    if (co.z < 0.0) {
        co.xy = (1.0 - abs(co.yx)) * sign(co.xy);
    }

    // Mapping to [0;1]^2 texture space
    return co.xy * 0.5 + 0.5;
}

vec3 octahedral_unmapping(vec2 co)
{
    co = co * 2.0 - 1.0;

    vec2 abs_co = abs(co);
    vec3 v      = vec3(co, 1.0 - (abs_co.x + abs_co.y));

    if (abs_co.x + abs_co.y > 1.0) {
        v.xy = (abs(co.yx) - 1.0) * -sign(co.xy);
    }

    return v;
}
////////////////////////////////////////////////