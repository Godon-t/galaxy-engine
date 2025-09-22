#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords   = texCoord;
    TexCoords.y = 1.f - TexCoords.y;
    gl_Position = vec4(vertices_position_modelspace.x, vertices_position_modelspace.y, 0, 1);
}

///////////////////////////////////////////////////////////

#type fragment

#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D sceneBuffer;
uniform sampler2D depthBuffer;

void main()
{
    float depth = texture(depthBuffer, TexCoords).r;
    vec4 pix    = texture(sceneBuffer, TexCoords);
    color       = vec4(pix.rgb, 1);
}