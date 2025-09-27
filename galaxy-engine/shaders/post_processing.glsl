#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoords;

uniform mat4 projection;
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
uniform mat4 view;
uniform float zNear = 0.0;
uniform float zFar  = 999.0;

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

// float screenSpaceShadow(vec3 surfacePosition, vec3 lightPos)
// {

// }

void main()
{
    vec4 pix = texture(sceneBuffer, TexCoords);

    float res = 0.005f;

    float depthUp    = texture(depthBuffer, TexCoords + vec2(0, res)).r;
    float depthDown  = texture(depthBuffer, TexCoords - vec2(0, res)).r;
    float depthRight = texture(depthBuffer, TexCoords + vec2(res, 0)).r;
    float depthLeft  = texture(depthBuffer, TexCoords - vec2(res, 0)).r;
    float depth      = texture(depthBuffer, TexCoords).r;

    float filter = depth - (depthUp + depthDown + depthLeft + depthRight) / 4.f;
    filter       = smoothstep(0.f, 1.f, filter * 100.f);

    color = vec4(pix.rgb * (1.f - filter), 1);
}