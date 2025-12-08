#type vertex

#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoords;
out vec3 v_camPos;
out vec3 v_worldPos;

void main()
{
    texCoords   = texCoord;
    v_worldPos  = vec3(model * vec4(vertices_position_modelspace, 1.0));
    v_camPos    = vec3(inverse(view)[3]);
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1);
}

////////////////////////////////

#type fragment
#version 330 core

uniform float zFar = 999.0;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outDepth;

in vec2 texCoords;
in vec3 v_camPos;
in vec3 v_worldPos;
// in vec3 v_normal;

uniform sampler2D sampledTexture;

void main()
{
    color    = texture(sampledTexture, texCoords);
    outDepth = vec4(length(v_camPos - v_worldPos) / zFar, 0, 0, 1);
}
