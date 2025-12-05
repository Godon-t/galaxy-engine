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
#include utils.glsl

uniform samplerCube radianceCubemap;
uniform samplerCube normalCubemap;
uniform samplerCube depthCubemap;

uniform vec3 debugStart;
uniform vec3 debugEnd;
uniform vec3 debugProbePos;

in vec2 texCoords;
layout(location = 0) out vec4 color;
layout(location = 1) out vec3 normal;
layout(location = 2) out float depth;

uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

// ===================== MAIN =========================
uniform bool showTriangleIndexOverlay = false;

/////////////////////////////////////////////////////////////////////

void main()
{
    vec3 dir      = octahedral_unmapping(texCoords);
    vec3 envColor = texture(radianceCubemap, dir).rgb;
    color         = vec4(envColor, 1.0);

    float radial = texture(depthCubemap, dir).r;

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

    depth      = texture(depthCubemap, dir).r;
    color.rgb  = outColor;
    normal.rgb = texture(normalCubemap, dir).rgb;
}