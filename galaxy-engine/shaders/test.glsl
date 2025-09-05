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
    texCoords   = texCoord;
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1);
}

//////////////////////////////////////////////////////////////////////////////////////

#type fragment
#version 330 core

in vec2 texCoords;
out vec4 color;

uniform bool useTexture;
uniform sampler2D testTexture;

void main()
{
    if (useTexture) {
        color = vec4(texture(testTexture, texCoords).rgb, 1.0);
    } else {
        color = vec4(0.2f, texCoords, 1.f);
    }
}
