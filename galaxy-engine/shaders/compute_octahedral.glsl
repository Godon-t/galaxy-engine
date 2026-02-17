#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 texCoords;

void main()
{
    texCoords   = texCoord;
    gl_Position = vec4(vertices_position_modelspace, 1);
}

#type fragment
#version 330 core

#include debug.glsl

uniform samplerCube radianceCubemap;
uniform samplerCube normalCubemap;
uniform samplerCube depthCubemap;

uniform vec3 debugStart;
uniform vec3 debugEnd;
uniform vec3 debugProbePos;

in vec2 texCoords;
layout(location = 0) out vec4 irradiance;
layout(location = 1) out vec4 color;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outDepth;

uniform float zNear = 0.1;
uniform float zFar  = 999.0;

// ===================== MAIN =========================
uniform bool showTriangleIndexOverlay = false;

/////////////////////////////////////////////////////////////////////

#include utils.glsl

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    vec3 dir      = octahedral_unmapping(texCoords);
    color = texture(radianceCubemap, dir);
    vec3 envColor = computeIrradiance(dir, radianceCubemap);
    // vec3 envColor = texture(radianceCubemap, dir).rgb;

    //////////////////////////////////////////////////////////////////////:
    vec2 uv       = texCoords;
    vec3 outColor = envColor;

    // Optionnel : overlay couleur selon l'indice du triangle octaédral ainsi que les bord des triangles
    if (showTriangleIndexOverlay)
        showTrianglesOverlay(uv, outColor, radianceCubemap);

    // // tracer le rayon projeté sur la carte octaédrale
    // float minDistRay = computeMinDistLinePolyline(debugStart, debugEnd, debugProbePos, 0.0, vec2(1), vec2(0), uv);
    // float rayAlpha   = lineAlphaFromDist(minDistRay, radianceCubemap);
    // vec3 rayCol      = vec3(1.0, 0.2, 0.1);
    // // // superposer le rayon par-dessus la cubemap et les bords
    // outColor = mix(outColor, rayCol, clamp(rayAlpha, 0.0, 1.0));

    irradiance = vec4(outColor, 1.0);
    outNormal = vec4(texture(normalCubemap, dir).rgb, 1.0);

    float rawDepth        = texture(depthCubemap, dir).r;
    float linearD         = linearDepth(rawDepth);
    float normalizedDepth = linearD / zFar;

    gl_FragDepth = rawDepth;
    outDepth     = vec4(rawDepth, 0, 0, 1);
}