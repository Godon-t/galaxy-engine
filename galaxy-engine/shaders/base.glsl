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

uniform vec3 albedoVal     = vec3(1.0, 0.f, 0.f);
uniform float metallicVal  = 0.5f;
uniform float roughnessVal = 0.5f;
uniform float aoVal        = 1.f;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform bool useAlbedoMap    = false;
uniform bool useNormalMap    = false;
uniform bool useMetallicMap  = false;
uniform bool useRoughnessMap = false;
uniform bool useAoMap        = false;

in vec2 texCoords;
out vec4 color;

void main()
{
    if (useNormalMap)
        color = vec4(texture(normalMap, texCoords).rgb, 1.0f);
    else
        color = vec4(albedoVal, 1.f);
}
