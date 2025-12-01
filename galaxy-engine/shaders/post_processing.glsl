#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoords;
out vec3 worldCoord;
out vec3 cameraPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords   = texCoord;
    TexCoords.y = 1.f - TexCoords.y;
    gl_Position = vec4(vertices_position_modelspace.x, vertices_position_modelspace.y, 0, 1);

    cameraPos              = vec3(inverse(view)[3]);
    vec4 ndc               = vec4(vertices_position_modelspace.x, vertices_position_modelspace.y, -1.0, 1.0);
    mat4 invViewProjection = inverse(projection * view);
    vec4 viewSpacePos      = invViewProjection * ndc;
    viewSpacePos /= viewSpacePos.w;

    worldCoord = viewSpacePos.xyz;
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
in vec3 cameraPos;
in vec3 worldCoord;

uniform sampler2D sceneBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

uniform ivec3 probeFieldGridDim      = ivec3(2, 2, 2);
uniform float probeFieldCellSize     = 100.f;
uniform float probeTextureSingleSize = 512.f;
uniform vec3 probeFieldOrigin        = vec3(0.f);
uniform vec3[8] probePositions;

uniform sampler2D probeIrradianceField;
uniform sampler2D probeDepthField;

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

int traceRayInProbe(vec3 p0, vec3 p1, vec3 probePos, sampler2D depthTex, float t, vec2 scale, vec2 probeTextureUpperLeft)
{
    // centrer les positions par rapport à la probe
    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;
    
    // calculer les t ou le rayon change de triangle octahédral
    float ts[9];
    int numSegments = computeOctahedralIntersections(centeredP0, centeredP1, ts);
    numSegments = numSegments-1;
    
    // parcourir chaque segment entre les changements de triangle
    for (int segIdx = 0; segIdx < numSegments;++segIdx) {
        float t0 = ts[segIdx];
        float t1 = ts[segIdx +1];
        
        // calculer les positions 3D aux extrémités du segment
        vec3 p3d0 = mix(centeredP0, centeredP1, t0);
        vec3 p3d1 = mix(centeredP0, centeredP1, t1);
        
        // calculer les distances aux extrémités
        float depth0 = length(p3d0);
        float depth1 = length(p3d1);
        
        // calculer les directions et projeter en 2D octahedral
        vec3 fallbackDir = normalize(centeredP1 - centeredP0 +vec3(1e-6));
        vec3 dir0 = safeNormalize(p3d0, fallbackDir);
        vec3 dir1 = safeNormalize(p3d1, fallbackDir);
        
        vec2 uv0 = octahedral_mapping(dir0);
        vec2 uv1 = octahedral_mapping(dir1);
        
        // mettre a l'échelle et décaler les UV dans la texture globale
        uv0 = (uv0+scale)+probeTextureUpperLeft;
        uv1 = (uv1+scale)+probeTextureUpperLeft;
        
        // ray marching en 2D le long de ce segment
        int steps = 32; //32 car c'est un compromis entre précision et perf
        for (int i = 0; i <= steps; ++i) {
            float s = float(i) / float(steps);
            
            // interpoler en 2D
            vec2 uv = mix(uv0, uv1, s);
            float depthRay = mix(depth0, depth1, s);
            float depthProbe = texture(depthTex, uv).r;
            
            // logique HIT / MISS / UNKNOWN
            if (depthRay > depthProbe + 1e-3) {
                return UNKNOWN;
            } else if (depthRay >= depthProbe - 1e-3) {
                return HIT;
            }
        }
    }
    
    return MISS;
}

vec3 getColourFromProbeField(vec3 rayStart, vec3 rayEnd, sampler2D probeTex, ivec3 gridDim, float cellSize, float probeTexSingleSize, vec3 fieldOrigin)
{
    vec3 rayDir           = rayEnd - rayStart;
    float rayLength       = length(rayDir);
    vec3 rayDirNorm       = rayDir / rayLength;
    float t               = 0.0;
    float stepSize        = 0.01;
    vec3 accumulatedColor = vec3(0.0);
    int totalSamples      = 0;

    while (t <= 1.0) {
        vec3 p3d = mix(rayStart, rayEnd, t);

        // déterminer dans quelle cellule on est
        vec3 localPos     = p3d - fieldOrigin;
        ivec3 cellIndices = ivec3(floor(localPos / cellSize));

        // vérifier si on est hors de la grille
        if (cellIndices.x < 0 || cellIndices.y < 0 || cellIndices.z < 0 || cellIndices.x >= gridDim.x || cellIndices.y >= gridDim.y || cellIndices.z >= gridDim.z) {
            t += stepSize;
            continue;
        }

        // obtenir la position de la sonde
        int probeGridIdx = cellIndices.z * gridDim.x * gridDim.y + cellIndices.y * gridDim.x + cellIndices.x;
        vec3 probePos    = vec3(cellIndices) * cellSize + fieldOrigin + vec3(cellSize * 0.5);

        // obtenir les coordonnées UV dans la texture de sonde
        int probesByWidth   = (gridDim.x * gridDim.y);
        int xPosition       = (probeGridIdx % probesByWidth) * int(probeTexSingleSize);
        int yPosition       = (probeGridIdx / probesByWidth) * int(probeTexSingleSize);
        vec2 probeTexOrigin = vec2(xPosition, yPosition);

        // échantillonner la sonde
        vec3 centeredP3d = p3d - probePos;
        vec3 dir         = safeNormalize(centeredP3d, vec3(1e-6));
        vec2 uv          = octahedral_mapping(dir);
        uv *= probeTexSingleSize;
        uv += probeTexOrigin;
        uv /= vec2(textureSize(probeTex, 0));

        vec3 probeColor = texture(probeTex, uv).rgb;

        // accumuler la couleur
        accumulatedColor += probeColor;
        totalSamples++;
        t += stepSize;
    }
    if (totalSamples > 0)
        accumulatedColor /= float(totalSamples);
    return accumulatedColor;
}

// int getCellCoord(int x, int y, int z)
// {
//     return z * probeFieldGridDim.x * probeFieldGridDim.y + y * probeFieldGridDim.x + x;
// }


// int getClosesProbeIdx(vec3 position){
//     int closestIdx = -1;
//     float minDist  = 1e20;
    
//     vec3 probePositionF = floor((position - probeFieldOrigin) / probeFieldCellSize);

//     ivec3 probePositionI = ivec3(probePositionF);
//     int pbX = clamp((int) probePositionF.x, 0, int(probeFieldGridDim.x - 1));
//     int pbY = clamp((int) probePositionF.y, 0, int(probeFieldGridDim.y - 1));
//     int pbZ = clamp((int) probePositionF.z, 0, int(probeFieldGridDim.z - 1));

//     return getCellCoord(probePositionI.x, probePositionI.y, probePositionI.z);
// }

// vec2 getProbeTexCoord(int probeGridIdx, sampler2D probeTex)
// {
//     int probesByWidth = textureSize(probeTex, 0).x / probeTextureSingleSize;
//     int xPosition     = (probeGridIdx % probesByWidth) * probeTextureSingleSize;
//     int yPosition     = (probeGridIdx / probesByWidth) * probeTextureSingleSize;
//     return vec2(xPosition, yPosition);
// }

void main()
{
    // vec4 pix = texture(sceneBuffer, TexCoords);

    // float res = 0.005f;

    float depth            = texture(depthBuffer, TexCoords).r;
    float linearDepthValue = linearDepth(depth);

    vec3 rayStart = cameraPos;
    vec3 rayDir   = normalize(worldCoord - rayStart);
    vec3 rayEnd   = rayStart + rayDir * linearDepthValue;
    color.rgb     = getColourFromProbeField(rayStart, rayEnd, probeIrradianceField, probeFieldGridDim, probeFieldCellSize, probeTextureSingleSize, probeFieldOrigin);


    color.a = 1.0;
    // gl_FragDepth    = 0.f;
}