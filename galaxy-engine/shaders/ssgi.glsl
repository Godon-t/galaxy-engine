#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;

void main()
{
    TexCoords   = texCoord;
    gl_Position = vec4(vertices_position_modelspace.xy, 0.0, 1.0);
}

///////////////////////////////////////////////////////////

#type fragment
#version 330 core

#include octahedral.glsl

out vec4 color;

in vec2 TexCoords;

uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform vec3 cameraPos;

uniform sampler2D sceneBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

void main()
{
    color.a = 1.0;

    color.rgb = texture(normalBuffer, TexCoords).rgb;
    // color.rgb = texture(sceneBuffer, TexCoords).rgb;
}