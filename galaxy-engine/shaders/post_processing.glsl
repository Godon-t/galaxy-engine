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
#include utils.glsl

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
uniform sampler2D directDiffuseBuffer;
uniform sampler2D directAmbiantBuffer;
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
uniform sampler2D probeNormalField;
uniform sampler2D probeDepthField;

const float minThickness = 0.03; // meters
const float maxThickness = 0.50;

vec3 reconstructWorldPosFromProbe(vec2 uv, sampler2D depthTex, vec3 probePos,
    float zFar, vec2 scale, vec2 offset)
{
    float storedDepth = texture(depthTex, uv * scale + offset).r;

    float distance = storedDepth * zFar;

    vec3 direction = octahedral_unmapping(uv);

    return probePos + direction * distance;
}

float distanceToIntersection(vec3 rayStart, vec3 rayDirection, vec3 v)
{
    float numer;
    float denom = v.y * rayDirection.z - v.z * rayDirection.y;

    if (abs(denom) > 0.1) {
        numer = rayStart.y * rayDirection.z - rayStart.z * rayDirection.y;
    } else {
        // We're in the yz plane; use another one
        numer = rayStart.x * rayDirection.y - rayStart.y * rayDirection.x;
        denom = v.x * rayDirection.y - v.y * rayDirection.x;
    }

    return numer / denom;
}

int traceRaySegmentHighRes(in sampler2D depthField, in sampler2D normalField, in vec3 probeSpaceRayStart, in vec3 probeSpaceRayDirection,
    in vec2 startTexCoord, in vec2 endTexCoord, in vec2 upperLeft, in vec2 scale,
    inout float tMin, inout float tMax, out vec2 hitProbeTexCoord)
{
    vec2 texCoordDelta     = endTexCoord - startTexCoord;
    float texCoordDistance = length(texCoordDelta);
    vec2 texCoordDirection = texCoordDelta / texCoordDistance;
    ivec2 textureSize2d    = textureSize(depthField, 0);

    // Not the way the article do it
    float texCoordStep = 1 / 32.0;

    // Initial value to detect variation in depth from ray to probe between two steps
    vec3 directionFromProbeBefore      = octahedral_unmapping(startTexCoord);
    float distanceFromProbeToRayBefore = max(0.0, distanceToIntersection(probeSpaceRayStart, probeSpaceRayDirection, directionFromProbeBefore));

    for (float d = 0.0; d <= texCoordDistance; d += texCoordStep) {
        // First half of ray marching
        vec2 texCoord           = (texCoordDirection * min(d + texCoordStep * 0.5, texCoordDistance)) + startTexCoord;
        vec3 directionFromProbe = octahedral_unmapping(texCoord);

        ivec2 pixelCoord = ivec2(textureSize2d * (texCoord * scale + upperLeft));

        float distanceFromProbeToSurface = texelFetch(depthField, pixelCoord, 0).r * zFar;

        // Second half of ray marching
        vec2 texCoordAfter                = (texCoordDirection * min(d + texCoordStep, texCoordDistance)) + startTexCoord;
        vec3 directionFromProbeAfter      = octahedral_unmapping(texCoordAfter);
        float distanceFromProbeToRayAfter = max(0.0, distanceToIntersection(probeSpaceRayStart, probeSpaceRayDirection, directionFromProbeAfter));
        float maxDistanceFromProbeToRay   = max(distanceFromProbeToRayAfter, distanceFromProbeToRayBefore);

        // Potential HIT
        if (maxDistanceFromProbeToRay >= distanceFromProbeToSurface) {
            float minDistanceFromProbeToRay = min(distanceFromProbeToRayBefore, distanceFromProbeToRayAfter);

            // Intersection of texel from prob onto ray
            float distanceFromProbeToRay = (minDistanceFromProbeToRay + maxDistanceFromProbeToRay) * 0.5;
            vec3 probeSpaceHitPoint      = distanceFromProbeToSurface * directionFromProbe;
            float distanceAlongRay       = dot(probeSpaceHitPoint - probeSpaceRayStart, probeSpaceRayDirection);

            // // I don't understand this normal calculation
            vec3 normal = octahedral_unmapping(
                texelFetch(normalField, pixelCoord, 0).xy * 2.0 + vec2(1.0));
            // vec3 normal = vec3(0, 0, -1);

            float surfaceThickness = minThickness + (maxThickness - minThickness) *
                    // probe and view ray alignment
                    max(dot(probeSpaceRayDirection, directionFromProbe), 0.0) *

                    // facing normal == thicker surface
                    (2.0 - abs(dot(probeSpaceRayDirection, normal)));

            //  *

            // // scale with distance along the ray
            // clamp(distanceAlongRay * 0.1, 0.05, 1.0);

            if ((minDistanceFromProbeToRay < distanceFromProbeToSurface + surfaceThickness) && (dot(normal, probeSpaceRayDirection) < 0)) {
                tMax             = distanceAlongRay;
                hitProbeTexCoord = texCoord;
                return HIT;
            } else {
                vec3 probeSpaceHitPointBefore = distanceFromProbeToRayBefore * directionFromProbeBefore;
                float distanceAlongRayBefore  = dot(probeSpaceHitPointBefore - probeSpaceRayStart, probeSpaceRayDirection);

                // didn't understand
                tMin = max(tMin, min(distanceAlongRay, distanceAlongRayBefore));

                return UNKNOWN;
            }
        }
    }

    return MISS;
}

int traceRayInProbe(vec3 p0, vec3 p1, vec3 probePos, sampler2D depthTex, sampler2D normalTex, in out float t, vec2 scale, vec2 probeTextureUpperLeft, out vec2 texelCoords)
{

    // centrer les positions par rapport à la probe
    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;

    vec3 rayDirection = normalize(centeredP1 - centeredP0);

    // calculer les t ou le rayon change de triangle octahédral
    float ts[8];
    int numSegments = computeOctahedralIntersections(centeredP0, centeredP1, ts);
    numSegments     = numSegments - 1;

    // parcourir chaque segment entre les changements de triangle
    for (int segIdx = 0; segIdx < numSegments; segIdx += 2) {
        float t0 = ts[segIdx];
        float t1 = ts[segIdx + 1];

        vec3 p3d0 = mix(centeredP0, centeredP1, t0);
        vec3 p3d1 = mix(centeredP0, centeredP1, t1);

        vec3 dir0 = normalize(centeredP0);
        vec3 dir1 = normalize(centeredP1);

        vec2 uv0 = octahedral_mapping(dir0);
        vec2 uv1 = octahedral_mapping(dir1);

        float segmentTMin = t0;
        float segmentTMax = t1;
        vec2 hitTexCoord;

        int result = traceRaySegmentHighRes(depthTex, normalTex, centeredP0, rayDirection,
            uv0, uv1, probeTextureUpperLeft, scale,
            segmentTMin, segmentTMax, hitTexCoord);

        if (result == HIT) {
            t           = segmentTMax;
            texelCoords = hitTexCoord;
            return HIT;
        }
        if (result == UNKNOWN) {
            t = segmentTMin;
            return UNKNOWN;
        }
    }

    return MISS;
}

int traceRayInProbeUnprecise(vec3 rayOrigin, vec3 rayDir, vec3 probePos, sampler2D depthTex, out float tHit, vec2 scale, vec2 probeTextureUpperLeft, out vec2 texelCoords)
{
    // 1. Rayon dans l'espace de la probe
    vec3 centeredOrigin = rayOrigin - probePos;
    rayDir              = normalize(rayDir);

    // 2. Créer un point d'arrivée virtuel pour computeOctahedralIntersections
    vec3 centeredEnd = centeredOrigin + rayDir * 100;

    // 3. Obtenir la taille de la texture
    ivec2 texSize = textureSize(depthTex, 0);

    // 4. Calculer les intersections octaédrales
    float ts[8];
    int numSegments = computeOctahedralIntersections(centeredOrigin, centeredEnd, ts);

    if (numSegments <= 0) {
        return MISS;
    }

    numSegments = numSegments - 1;

    vec3 probeSpaceRayDirection = rayDir; // Déjà normalisé

    // 5. Parcourir les segments d'intersection
    for (int segIdx = 0; segIdx < numSegments; segIdx += 2) {
        float t0 = ts[segIdx];
        float t1 = ts[segIdx + 1];

        // Points 3D aux extrémités du segment
        vec3 p3d0 = mix(centeredOrigin, centeredEnd, t0);
        vec3 p3d1 = mix(centeredOrigin, centeredEnd, t1);

        vec3 dir0 = normalize(p3d0);
        vec3 dir1 = normalize(p3d1);

        // 6. Projection octaédrique
        vec2 uv0_oct = octahedral_mapping(dir0);
        vec2 uv1_oct = octahedral_mapping(dir1);

        // 7. UVs dans l'atlas
        vec2 uv0 = uv0_oct * scale + probeTextureUpperLeft;
        vec2 uv1 = uv1_oct * scale + probeTextureUpperLeft;

        // 8. Ray marching adaptatif
        vec2 uvDir     = uv1 - uv0;
        float uvLength = length(uvDir);
        int steps      = max(8, int(ceil(uvLength * float(texSize.x))));

        // Distance initiale du rayon à la probe
        vec3 directionFromProbeBefore      = dir0;
        float distanceFromProbeToRayBefore = max(0.0, distanceToIntersection(centeredOrigin, probeSpaceRayDirection, directionFromProbeBefore));

        // 9. Marcher le long du segment
        for (int i = 1; i <= steps; ++i) {
            float s = float(i) / float(steps);

            // Position courante en UV et 3D
            vec2 uvCurrent  = mix(uv0, uv1, s);
            vec3 currentPos = mix(p3d0, p3d1, s);

            // Direction depuis la probe vers la position courante
            vec3 currentDir = normalize(currentPos);

            // Lire la profondeur de la probe dans cette direction
            float depthProbe = texture(depthTex, uvCurrent).r * zFar;

            // **LOGIQUE CLEF : Calculer la distance min/max du rayon à la probe**
            vec3 directionFromProbeAfter      = octahedral_unmapping(mix(uv0_oct, uv1_oct, s));
            float distanceFromProbeToRayAfter = max(0.0, distanceToIntersection(centeredOrigin, probeSpaceRayDirection, directionFromProbeAfter));

            float minDistanceFromProbeToRay = min(distanceFromProbeToRayAfter, distanceFromProbeToRayBefore);
            float maxDistanceFromProbeToRay = max(distanceFromProbeToRayAfter, distanceFromProbeToRayBefore);

            // **TEST D'INTERSECTION EFFICACE**
            // Si le rayon passe à une distance >= depthProbe, c'est un hit
            vec3 rawNormal = texture(probeNormalField, mix(uv0, uv1, s)).rgb;
            if (dot(directionFromProbeAfter, (rawNormal - vec3(0.5)) * 2.0) > 0.0) {
                return UNKNOWN;
            } else if (maxDistanceFromProbeToRay >= depthProbe) {
                texelCoords = uvCurrent;

                // Calculer tHit : distance réelle sur le rayon
                float currentT = mix(t0, t1, s);
                tHit           = currentT * zFar; // Convertir de [0,1] à distance réelle

                return HIT;
            }

            // Mettre à jour pour la prochaine itération
            distanceFromProbeToRayBefore = distanceFromProbeToRayAfter;
            directionFromProbeBefore     = directionFromProbeAfter;
        }
    }

    return MISS;
}

// Helper : intersection rayon-octaèdre
bool rayOctahedronIntersection(vec3 origin, vec3 dir, float radius, out float tEnter, out float tExit)
{
    // L'octaèdre est défini par |x| + |y| + |z| <= radius
    // Pour simplifier, on utilise une sphère englobante
    // (Une vraie intersection octaèdre est plus complexe)

    // Intersection rayon-sphère
    float a            = dot(dir, dir);
    float b            = 2.0 * dot(origin, dir);
    float c            = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
        return false;
    }

    float sqrtDisc = sqrt(discriminant);
    tEnter         = (-b - sqrtDisc) / (2.0 * a);
    tExit          = (-b + sqrtDisc) / (2.0 * a);

    return tExit > 0.0;
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
    return round((target - probeFieldOrigin) / probeFieldCellSize);
}

vec3 getClosestCornerCellProbePosition(vec3 target)
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

// Retourne l'irradiance stockée dans l'atlas pour une sonde et des texcoords dans [0,1]
vec3 sampleProbeIrradiance(int probeIdx, vec2 texCoords, sampler2D probeIrradianceField, int probeTexSingleSize)
{
    // texCoords est en UV global sur l'atlas (comme finalTexelCoords dans ton code)
    return texture(probeIrradianceField, texCoords).rgb;
}

// Trilinear interpolation of the 8 probes around world position 'P'.
vec3 trilinearIrradianceAtPosition(vec3 P, sampler2D probeIrradianceField, int probeTexSingleSize)
{
    // convert position to grid coords
    vec3 local = (P - probeFieldOrigin) / probeFieldCellSize;
    vec3 base  = floor(local);
    vec3 frac  = local - base;

    // Clamp to grid
    ivec3 b0 = ivec3(clamp(base, vec3(0), vec3(probeFieldGridDim) - vec3(1)));
    ivec3 b1 = ivec3(clamp(base + vec3(1), vec3(0), vec3(probeFieldGridDim) - vec3(1)));

    vec3 accum = vec3(0.0);
    for (int z = 0; z <= 1; ++z) {
        for (int y = 0; y <= 1; ++y) {

            for (int x = 0; x <= 1; ++x) {
                ivec3 idx = ivec3(b0.x + x, b0.y + y, b0.z + z);
                int pIdx  = getCellCoord(idx.x, idx.y, idx.z); // reuse ta fonction
                // lire le texRect pour la sonde pIdx
                vec4 rect = getProbeTexRect(pIdx, probeIrradianceField, probeTexSingleSize);
                // on sample au centre de la sonde pour l'irradiance (ou à une coord calculée)
                vec2 sampleUV = rect.xy + rect.zw * 0.5;
                vec3 val      = texture(probeIrradianceField, sampleUV).rgb;
                float wx      = (x == 0) ? (1.0 - frac.x) : frac.x;
                float wy      = (y == 0) ? (1.0 - frac.y) : frac.y;
                float wz      = (z == 0) ? (1.0 - frac.z) : frac.z;
                accum += val * wx * wy * wz;
            }
        }
    }
    return accum;
}

vec3 getColorFromProbeField(vec3 rayStart, vec3 rayDir, sampler2D probeIrradianceField, sampler2D probeNormalField, sampler2D probeDepthField, int probeTexSingleSize)
{
    vec3 localStart = (rayStart - probeFieldOrigin) / probeFieldCellSize;
    vec3 baseCell   = floor(localStart);
    vec3 alpha      = fract(localStart);

    ivec3 baseGridCoord = ivec3(clamp(baseCell, vec3(0), vec3(probeFieldGridDim - 1)));

    vec3 sumIrradiance = vec3(0.0);
    float sumWeight    = 0.0;

    for (int i = 0; i < 8; i++) {
        // TODO: check
        // Better than getNextProbeIdx according to AI
        ivec3 offset         = ivec3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
        ivec3 probeGridCoord = clamp(baseGridCoord + offset, ivec3(0), ivec3(probeFieldGridDim - 1));

        vec3 trilinear = mix(1.0 - alpha, alpha, vec3(offset));
        float weight   = trilinear.x * trilinear.y * trilinear.z;

        int probeIdx       = getProbeIdx(vec3(probeGridCoord));
        vec3 probePosition = vec3(probeGridCoord) * probeFieldCellSize + probeFieldOrigin;

        vec4 probeTexRect     = getProbeTexRect(probeIdx, probeIrradianceField, probeTexSingleSize);
        float t               = 0.0;
        vec2 finalTexelCoords = vec2(0);

        int result = traceRayInProbeUnprecise(
            rayStart,
            rayDir,
            probePosition,
            probeDepthField,
            t,
            probeTexRect.zw,
            probeTexRect.xy,
            finalTexelCoords);

        // Weights according to probe proximity
        if (result == HIT) {
            vec3 probeIrradiance = texture(probeIrradianceField, finalTexelCoords).rgb;

            vec3 probeToStart = rayStart - probePosition;
            float distWeight  = 1.0 / (1.0 + length(probeToStart));
            weight *= distWeight;

            sumIrradiance += weight * probeIrradiance;
            sumWeight += weight;
        } else if (result == MISS) {
            weight *= 0.05;
            sumWeight += weight;
        }
    }

    if (sumWeight > 0.0001) {
        return sumIrradiance / sumWeight;
    }

    // Fallback
    return trilinearIrradianceAtPosition(rayStart, probeIrradianceField, probeTexSingleSize);
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

vec3 getPerpendicularVector(vec3 v)
{
    vec3 perpendicular = abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0) : vec3(0.0, -v.z, v.y);
    return normalize(perpendicular);
}

vec3 getCosHemisphereSample(float rand1, float rand2, vec3 hitNorm)
{
    // Cosine weighted hemisphere sample from RNG
    vec3 bitangent = getPerpendicularVector(hitNorm);
    vec3 tangent   = cross(bitangent, hitNorm);

    float r   = sqrt(rand1);
    float phi = 2.0 * PI * rand2;

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi)) + bitangent * (r * sin(phi)) + hitNorm * sqrt(max(0.0, 1.0 - rand1));
}

float IGN(vec2 pixelCoord, int frameCount)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(pixelCoord.xy + float(frameCount) * vec2(47.0, 17.0) * 0.695, magic.xy)));
}

void main()
{
    // 1. Récupérer les données du G-Buffer
    vec3 albedo     = texture(sceneBuffer, TexCoords).rgb;
    float metallic  = texture(sceneBuffer, TexCoords).a;
    vec3 normal     = normalize(texture(normalBuffer, TexCoords).rgb * 2.0 - 1.0);
    float roughness = texture(normalBuffer, TexCoords).a;
    float depth     = texture(depthBuffer, TexCoords).r;
    float ao        = texture(depthBuffer, TexCoords).g;

    // 2. Lumière directe (déjà calculée)
    vec3 directLight = texture(directDiffuseBuffer, TexCoords).rgb;

    // 3. Position monde
    vec3 worldPos = reconstructWorldPos(TexCoords, depth, inverseProjection, inverseView);

    // 4. Monte Carlo sampling pour l'illumination indirecte
    vec3 indirectIrradiance = vec3(0.0);
    vec2 pixelCoord         = TexCoords * vec2(textureSize(sceneBuffer, 0));

    const int numSamples      = 8; // Ajustable (8, 16, 32, 64)
    const float invNumSamples = 1.0 / float(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        float noise1 = IGN(pixelCoord, i);
        float noise2 = IGN(pixelCoord + vec2(1.0, 1.0), i);

        vec3 sampleDir = getCosHemisphereSample(noise1, noise2, normal);

        vec3 sampleIrradiance = getColorFromProbeField(
            worldPos,
            sampleDir,
            probeIrradianceField,
            probeNormalField,
            probeDepthField,
            probeTextureSingleSize);

        indirectIrradiance += sampleIrradiance;
    }

    indirectIrradiance *= invNumSamples;

    color.a = 1.0;

    vec3 indirectDiffuse = indirectIrradiance * albedo * ao; // * (1.0 - metallic);

    vec3 finalColor = directLight + indirectDiffuse;

    vec3 minAmbient = vec3(0.01) * albedo;
    finalColor      = max(finalColor, minAmbient);

    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    color = vec4(finalColor, 1.0);
    color = vec4(indirectIrradiance, 1.0);
}