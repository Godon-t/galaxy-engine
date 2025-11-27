#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 texCoords;

void main()
{
    texCoords   = texCoord;
    gl_Position = vec4(vertices_position_modelspace, 1);
}

#type fragment
#version 330 core

// https://www.shadertoy.com/view/MdsfzN

vec2 octahedral_mapping(vec3 co)
{
    // projection onto octahedron
    co /= dot(vec3(1), abs(co));

    // out-folding of the downward faces
    if (co.y < 0.0) {
        co.xy = (1.0 - abs(co.zx)) * sign(co.xz);
    }

    // mapping to [0;1]ˆ2 texture space
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

uniform samplerCube radianceCubemap;
uniform samplerCube depthCubemap;

in vec2 texCoords;
out vec4 color;

uniform float zNear = 0.1;
uniform float zFar  = 9999.0;
float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    vec3 dir      = octahedral_unmapping(texCoords);
    vec3 envColor = texture(radianceCubemap, dir).rgb;
    color         = vec4(envColor, 1.0);
    gl_FragDepth  = linearDepth(texture(depthCubemap, dir).r) / zFar;
}