#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1);
}

//////////////////////////////////////////////////////////////////////////////////////

#type fragment
#version 330 core

out vec4 color;

uniform vec3 objectColor = vec3(1.0, 1.0, 1.0);

void main()
{
    color = vec4(objectColor, 1.0);
}
