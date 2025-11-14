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
    mat4 viewNoTranslation = view;
    viewNoTranslation[3]   = vec4(0.0, 0.0, 0.0, 1.0);

    TexCoords = vertices_position_modelspace;

    vec4 pos = projection * viewNoTranslation * vec4(vertices_position_modelspace, 1.0);

    gl_Position = pos.xyww;
}
///////////////////////////////////////////////////////////

#type fragment

#version 330 core
out vec4 color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    vec3 sampleDir = normalize(TexCoords);
    color          = texture(skybox, sampleDir);
}