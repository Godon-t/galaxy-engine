#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(vertices_position_modelspace, 1.0);
}

#type fragment
#version 330 core

void main()
{
    // OpenGL écrit automatiquement la profondeur dans gl_FragDepth
}
