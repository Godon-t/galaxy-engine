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

const int HIT     = 0;
const int MISS    = 1;
const int UNKNOWN = 2;

in vec2 TexCoords;

uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform vec3 cameraPos;

uniform sampler2D sceneBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

uniform ivec3 probeFieldGridDim    = ivec3(1, 1, 1);
uniform float probeFieldCellSize   = 100.f;
uniform int probeTextureSingleSize = 512;
uniform vec3 probeFieldOrigin      = vec3(0.f);
uniform vec3[8] probePositions;

uniform sampler2D probeIrradianceField;
uniform sampler2D probeDepthField;

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

float traceRayInProbe(vec3 p0, vec3 p1, vec3 probePos, sampler2D depthTex, float t, vec2 scale, vec2 probeTextureUpperLeft)
{
    // centrer les positions par rapport à la probe
    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;

    // calculer les t ou le rayon change de triangle octahédral
    float ts[8];
    int numSegments = computeOctahedralIntersections(centeredP0, centeredP1, ts);
    numSegments     = numSegments - 1;


    vec2 uvTest = octahedral_mapping(safeNormalize(centeredP1, vec3(1, 0, 0)));
    float probeDepthTest = linearDepth(texture(depthTex, uvTest * scale + probeTextureUpperLeft).r);
    return probeDepthTest;

    // parcourir chaque segment entre les changements de triangle
    for (int segIdx = 0; segIdx < numSegments; segIdx += 2) {
        float t0 = ts[segIdx];
        float t1 = ts[segIdx + 1];

        // calculer les positions 3D aux extrémités du segment
        vec3 p3d0 = mix(centeredP0, centeredP1, t0);
        vec3 p3d1 = mix(centeredP0, centeredP1, t1);

        // calculer les distances aux extrémités
        float depth0 = length(p3d0);
        float depth1 = length(p3d1);

        // calculer les directions et projeter en 2D octahedral
        vec3 fallbackDir = normalize(centeredP1 - centeredP0 + vec3(1e-6));
        vec3 dir0        = safeNormalize(p3d0, fallbackDir);
        vec3 dir1        = safeNormalize(p3d1, fallbackDir);

        vec2 uv0 = octahedral_mapping(dir0);
        vec2 uv1 = octahedral_mapping(dir1);

        // mettre a l'échelle et décaler les UV dans la texture globale
        uv0 = (uv0 * scale) + probeTextureUpperLeft;
        uv1 = (uv1 * scale) + probeTextureUpperLeft;

        // ray marching en 2D le long de ce segment
        int steps = 64; // 32 car c'est un compromis entre précision et perf
        for (int i = 0; i <= steps; ++i) {
            float s = float(i) / float(steps);

            // interpoler en 2D
            vec2 uv          = mix(uv0, uv1, s);
            float depthRay   = mix(depth0, depth1, s);
            float depthProbe = linearDepth(texture(depthTex, uv).r);

            // logique HIT / MISS / UNKNOWN
            if (depthRay > depthProbe + 0.01) {
                return 0.0;
                // return UNKNOWN;
            } else if (depthRay >= depthProbe - 1e-3) {
                return 1.0;
                // return HIT;
            }
        }
    }

    return 0.0;
    // return MISS;
}

int getCellCoord(int x, int y, int z)
{
    return z * probeFieldGridDim.x * probeFieldGridDim.y + y * probeFieldGridDim.x + x;
}

vec3 getClosestProbePosition(vec3 target)
{
    return round((target - probeFieldOrigin) / probeFieldCellSize);
}

int getProbeIdx(vec3 preciseProbePosition)
{
    ivec3 probePositionI = ivec3(preciseProbePosition);
    int pbX              = clamp(int(preciseProbePosition.x), 0, int(probeFieldGridDim.x - 1));
    int pbY              = clamp(int(preciseProbePosition.y), 0, int(probeFieldGridDim.y - 1));
    int pbZ              = clamp(int(preciseProbePosition.z), 0, int(probeFieldGridDim.z - 1));

    return getCellCoord(pbX, pbY, pbZ);
}

vec4 getProbeTexRect(int probeGridIdx, sampler2D probeTex, int probeTextureSingleSize)
{
    ivec2 texSizeI = textureSize(probeTex, 0);
    vec2 texSize   = vec2(texSizeI);

    int probesByWidth = texSizeI.x / probeTextureSingleSize;
    int xPosition     = (probeGridIdx % probesByWidth) * probeTextureSingleSize;
    int yPosition     = (probeGridIdx / probesByWidth) * probeTextureSingleSize;

    vec2 upperLeft = vec2(xPosition, yPosition) / texSize;
    vec2 sizeNorm  = vec2(probeTextureSingleSize) / texSize;

    return vec4(upperLeft, sizeNorm);
}

// 8 differents probes in this test
vec3 indexToColor(int probeIdx)
{
    if (probeIdx == 0)
        return vec3(1, 0, 0); // Rouge
    else if (probeIdx == 1)
        return vec3(0, 1, 0); // Vert
    else if (probeIdx == 2)
        return vec3(0, 0, 1); // Bleu
    else if (probeIdx == 3)
        return vec3(1, 1, 0); // Jaune
    else if (probeIdx == 4)
        return vec3(1, 1, 1); // Blanc
    else if (probeIdx == 5)
        return vec3(0, 1, 1); // Cyan
    else if (probeIdx == 6)
        return vec3(1, 0.5, 0); // Orange
    else if (probeIdx == 7)
        return vec3(0, 0, 0); // Noir
    else
        return vec3(0.5);
}

vec3 getColorFromProbeField(vec3 rayStart, vec3 rayEnd, sampler2D probeIrradianceField, sampler2D probeDepthField, int probeTexSingleSize)
{
    float t        = 0.0;
    float stepSize = 0.01;

    int ite = 0;
    int visitedProbes[8];
    int actualProbeIdx = 0;

    vec3 probePosition = getClosestProbePosition(rayStart);
    int probeGridIdx   = getProbeIdx(probePosition);

    vec4 probeTexRect = getProbeTexRect(probeGridIdx, probeIrradianceField, probeTexSingleSize);
    float result      = traceRayInProbe(rayStart, rayEnd, probePosition * probeFieldCellSize, probeDepthField, t, probeTexRect.zw, probeTexRect.xy);
    return vec3(result);

    // if (result == HIT) {
    //     return vec3(0, 1, 0);
    //     // return texture(probeIrradianceField, probeCoords);
    // } else if (result == MISS)
    //     return vec3(1, 0, 0);
    // else
    //     return vec3(0, 0, 1);

    // while (t <= 1.0 && ite < 10) {
    //     vec3 p3d = mix(rayStart, rayEnd, t);

    //     vec3 probePosition = getClosestProbePosition(p3d);
    //     int probeGridIdx   = getProbeIdx(probePosition);

    //     for (int i = 0; i < actualProbeIdx; i++) {
    //         if (visitedProbes[i] == probeGridIdx) {
    //             return vec3(1, 0, 0);
    //         }
    //     }
    //     visitedProbes[actualProbeIdx] = probeGridIdx;

    //     vec2 probeCoords = getProbeTexCoord(probeGridIdx, probeIrradianceField, probeTexSingleSize);

    //     int result = traceRayInProbe(p3d, rayEnd, probePosition, probeDepthField, t, probeCoords, vec2(4, 1));

    //     if (result == HIT) {
    //         return vec3(0, 1, 0);
    //         // return texture(probeIrradianceField, probeCoords);
    //     }

    //     ite += 1;
    // }

    return vec3(0, 0, 1);
}

vec3 backgroundBlur(sampler2D colorTexture, sampler2D depthTexture, vec2 uv)
{
    float depth      = texture(depthTexture, uv).r;
    float focusDepth = texture(depthTexture, vec2(0.5, 0.5)).r;
    float diffDepth  = focusDepth - depth;
    diffDepth *= diffDepth;
    diffDepth = sqrt(diffDepth);

    vec3 col1 = texture(colorTexture, uv + vec2(0, diffDepth)).rgb;
    vec3 col2 = texture(colorTexture, uv - vec2(0, diffDepth)).rgb;
    vec3 col3 = texture(colorTexture, uv + vec2(diffDepth, 0)).rgb;
    vec3 col4 = texture(colorTexture, uv - vec2(diffDepth, 0)).rgb;
    return (col1 + col2 + col3 + col4) / 4.f;
}

vec3 reconstructWorldPos(vec2 uv, float depth)
{
    float z  = depth * 2.0 - 1.0; // NDC
    vec4 ndc = vec4(uv * 2.0 - 1.0, z, 1.0);

    vec4 viewPos = inverseProjection * ndc;
    viewPos /= viewPos.w;

    vec4 worldPos = inverseView * viewPos;
    return worldPos.xyz;
}

void main()
{
    color.a = 1.0;

    float depthN = texture(depthBuffer, TexCoords).r;

    vec3 worldCoord = reconstructWorldPos(TexCoords, depthN);

    vec3 rayStart = cameraPos; // uniform
    vec3 rayEnd   = worldCoord;

    color.rgb = getColorFromProbeField(
        rayStart, rayEnd,
        probeIrradianceField, probeDepthField,
        probeTextureSingleSize);

    // color.rgb = texture(probeIrradianceField, vec2(1) - TexCoords).rgb;
    // gl_FragDepth    = 0.f;

    // color.rgb = backgroundBlur(sceneBuffer, depthBuffer, TexCoords);
}