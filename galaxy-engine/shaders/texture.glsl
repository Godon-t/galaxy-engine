#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoords;

void main()
{
    texCoords = texCoord;
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1.0);
}

#type fragment

#version 330 core
in vec2 texCoords;
uniform sampler2D sampledTexture;
uniform bool useTexture = false;
uniform float transparencyVal = 1.0;

out vec4 color;

void main()
{
    color = useTexture ? texture(sampledTexture, texCoords) : vec4(1.0);
    color.a *= transparencyVal;
    if (color.a <= 0.001)
        discard;
}
