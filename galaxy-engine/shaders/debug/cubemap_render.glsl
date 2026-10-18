#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 direction;

void main()
{
    direction = vertices_position_modelspace;
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1.0);
}

#type fragment

#version 330 core
in vec3 direction;
uniform samplerCube sampledTexture;
uniform bool useTexture = false;
uniform float transparencyVal = 1.0;

out vec4 color;

void main()
{
    color = useTexture ? texture(sampledTexture, direction) : vec4(1.0);
    color.a *= transparencyVal;
    if (color.a <= 0.001)
        discard;
}
