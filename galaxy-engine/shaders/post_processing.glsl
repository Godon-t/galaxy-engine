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

#include octahedral.glsl

out vec4 color;

const int HIT     = 0;
const int MISS    = 1;
const int UNKNOWN = 2;

in vec2 TexCoords;

uniform sampler2D sceneBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

uniform ivec3 probeFieldGridDim      = ivec3(2, 2, 2);
uniform float probeFieldCellSize     = 100.f;
uniform float probeTextureSingleSize = 512.f;
uniform vec3 probeFieldOrigin        = vec3(0.f);

uniform sampler2D probeIrradianceField;
uniform sampler2D probeDepthField;

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

int traceRayInProbe(vec3 p0, vec3 p1, vec3 probePos, sampler2D depthTex, float t)
{
    vec3 fallbackDir = normalize(p1 - p0 + vec3(1e-6));
    float stepSize   = 0.01;

    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;
    while (t <= 1.0) {
        vec3 p3d = mix(centeredP0, centeredP1, t);

        vec3 dir = safeNormalize(p3d, fallbackDir);
        vec2 uv  = octahedral_mapping(dir);

        float depthRay   = length(p3d);
        float depthProbe = texture(depthTex, uv).r;

        // --- logique HIT / MISS / UNKNOWN ---
        if (depthRay > depthProbe + 1e-3) {
            return UNKNOWN;
        } else if (depthRay >= depthProbe - 1e-3) {
            return HIT; // HIT
        }

        t += stepSize;
    }

    return MISS; // MISS (aucune intersection trouvée)
}

void main()
{
    vec4 pix = texture(sceneBuffer, TexCoords);

    float res = 0.005f;

    float depthUp    = texture(depthBuffer, TexCoords + vec2(0, res)).r;
    float depthDown  = texture(depthBuffer, TexCoords - vec2(0, res)).r;
    float depthRight = texture(depthBuffer, TexCoords + vec2(res, 0)).r;
    float depthLeft  = texture(depthBuffer, TexCoords - vec2(res, 0)).r;
    float depth      = texture(depthBuffer, TexCoords).r;

    depth   = linearDepth(depth) / zFar;
    float f = depth - (linearDepth(depthUp) + linearDepth(depthDown) + linearDepth(depthLeft) + linearDepth(depthRight)) / 4.f;
    f       = f / zFar;
    f       = smoothstep(0.f, 1.f, f * 100.f);

    color = vec4(pix.rgb * (1.f - f), 1);
    color = vec4(pix.rgb, 1);
    // color = vec4(vec3(depth), 1.0);

    vec3 viewDir  = normalize((view * vec4(0, 0, -1, 0)).xyz);
    vec3 viewOrig = (inverse(view) * vec4(0, 0, 0, 1)).xyz;
}