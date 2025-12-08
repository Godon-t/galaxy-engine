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

layout(location = 0) out vec4 color;

const int HIT     = 0;
const int MISS    = 1;
const int UNKNOWN = 2;

in vec2 TexCoords;

uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform vec3 cameraPos;

uniform sampler2D sceneBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear     = 0.1;
uniform float zFar      = 999.0;
uniform float traceBias = 0.05;

uniform ivec3 probeFieldGridDim    = ivec3(2, 2, 2);
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

vec3 reconstructWorldPosFromProbe(vec2 uv, sampler2D depthTex, vec3 probePos,
    float zFar, vec2 scale, vec2 offset)
{
    float storedDepth = texture(depthTex, uv * scale + offset).r;

    float distance = storedDepth * zFar;

    vec3 direction = octahedral_unmapping(uv);

    return probePos + direction * distance;
}

int traceRayInProbe(vec3 p0, vec3 p1, vec3 probePos, sampler2D depthTex, in out float t, vec2 scale, vec2 probeTextureUpperLeft, out vec2 texelCoords)
{
    // centrer les positions par rapport à la probe
    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;

    // calculer les t ou le rayon change de triangle octahédral
    float ts[8];
    int numSegments = computeOctahedralIntersections(centeredP0, centeredP1, ts);
    numSegments     = numSegments - 1;

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
            t       = mix(t0, t1, s);

            // interpoler en 2D
            texelCoords      = mix(uv0, uv1, s);
            float depthRay   = mix(depth0, depth1, s);
            float depthProbe = texture(depthTex, texelCoords).r * zFar;

            float adaptiveBias = traceBias * (1.0 + depthRay * 0.001);
            float diff         = depthRay - depthProbe;

            // logique HIT / MISS / UNKNOWN
            if (diff > adaptiveBias) {
                return UNKNOWN;
            } else if (diff >= -adaptiveBias) {
                return HIT;
            }
        }
    }

    return MISS;
}

int getCellCoord(int x, int y, int z)
{
    return z * probeFieldGridDim.x * probeFieldGridDim.y + y * probeFieldGridDim.x + x;
}

vec3 fromProbeIdxToCoords(int idx)
{
    int z = idx / (probeFieldGridDim.x * probeFieldGridDim.y);

    int remainder = idx % (probeFieldGridDim.x * probeFieldGridDim.y);

    int y = remainder / probeFieldGridDim.x;

    int x = remainder % probeFieldGridDim.x;

    return vec3(x, y, z);
}

vec3 getClosestProbePosition(vec3 target)
{
    return floor((target - probeFieldOrigin) / probeFieldCellSize);
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

// To cover all probes in one cell
int getNextProbeIdx(int probeIdx, int iterator)
{
    int nextIdx = iterator * 3 % 8;

    if (nextIdx == 0)
        return probeIdx;
    else if (nextIdx == 1)
        return probeIdx + 1;
    else if (nextIdx == 2)
        return probeIdx + probeFieldGridDim.x;
    else if (nextIdx == 3)
        return probeIdx + probeFieldGridDim.x + 1;
    else if (nextIdx == 4)
        return probeIdx + probeFieldGridDim.x * probeFieldGridDim.y;
    else if (nextIdx == 5)
        return probeIdx + probeFieldGridDim.x * probeFieldGridDim.y + 1;
    else if (nextIdx == 6)
        return probeIdx + probeFieldGridDim.x * probeFieldGridDim.y + probeFieldGridDim.x;
    else if (nextIdx == 7)
        return probeIdx + probeFieldGridDim.x * probeFieldGridDim.y + probeFieldGridDim.x + 1;
}

vec3 getColorFromProbeField(vec3 rayStart, vec3 rayEnd, sampler2D probeIrradianceField, sampler2D probeDepthField, int probeTexSingleSize)
{
    float t               = 0.0;
    int ite               = 0;
    vec2 finalTexelCoords = vec2(0);

    int baseCellIdx; // Index de la probe au coin (0,0,0) de la cellule
    bool changeCell = true;

    int loopLimit   = 32;
    int currentLoop = 0;

    while (t <= 1.0 && currentLoop < loopLimit) {
        currentLoop += 1;

        if (changeCell) {
            changeCell = false;

            // Trouver la cellule contenant le point actuel
            vec3 currentPos  = mix(rayStart, rayEnd, t);
            vec3 cellGridPos = getClosestProbePosition(currentPos);

            // Vérifier qu'on est dans la grille
            if (any(lessThan(cellGridPos, vec3(0))) || any(greaterThanEqual(cellGridPos, vec3(probeFieldGridDim)))) {
                return vec3(0, 0, 1); // Hors grille
            }

            baseCellIdx = getProbeIdx(cellGridPos);
            ite         = 0;
        }

        if (ite >= 8) {
            t += 0.01;
            changeCell = true;
            continue;
        }

        int currentProbeIdx    = getNextProbeIdx(baseCellIdx, ite);
        vec3 probeGridPosition = fromProbeIdxToCoords(currentProbeIdx);

        if (any(greaterThanEqual(probeGridPosition, vec3(probeFieldGridDim))) || any(lessThan(probeGridPosition, vec3(0)))) {
            ite++;
            continue;
        }

        vec3 probePosition = probeGridPosition * probeFieldCellSize + probeFieldOrigin;

        vec3 p3d = mix(rayStart, rayEnd, t);

        vec3 currentCellGridPos = getClosestProbePosition(p3d);
        vec3 baseCellGridPos    = fromProbeIdxToCoords(baseCellIdx);

        if (any(notEqual(currentCellGridPos, baseCellGridPos))) {
            changeCell = true;
            continue;
        }

        vec4 probeTexRect = getProbeTexRect(currentProbeIdx, probeIrradianceField, probeTexSingleSize);
        float tBackup     = t;
        int result        = traceRayInProbe(p3d, rayEnd, probePosition, probeDepthField, t, probeTexRect.zw, probeTexRect.xy, finalTexelCoords);

        if (result == HIT) {
            return texture(probeIrradianceField, finalTexelCoords).rgb;
        } else if (result == MISS) {
            t = tBackup;
            ite++;
        } else if (result == UNKNOWN) {
            // t a été mis à jour, on change probablement de cellule
            // changeCell = true;
            ite++;
        }
    }

    // Couleurs de debug selon la raison de sortie
    if (t >= 1.0)
        return vec3(1, 0, 0); // Rayon complet sans hit
    else
        return vec3(0, 0, 1); // Limite d'itérations
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