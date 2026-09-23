#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

out vec4 fragPosLightSpace;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(vertices_position_modelspace, 1.0);

    fragPosLightSpace = gl_Position;
}

///////////////////////////////////////////////////////////
#type fragment
#version 330 core

in vec4 fragPosLightSpace;

void main()
{
}
