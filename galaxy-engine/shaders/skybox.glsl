#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords              = vertices_position_modelspace;
    mat4 viewNoTranslation = mat4(mat3(view));
    gl_Position            = projection * viewNoTranslation * vec4(vertices_position_modelspace, 1.0);
}
///////////////////////////////////////////////////////////

#type fragment

#version 330 core
out vec4 color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    color = texture(skybox, TexCoords);
}