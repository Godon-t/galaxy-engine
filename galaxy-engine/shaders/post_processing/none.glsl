#type vertex

#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;
out vec3 v_camPos;

void main()
{
    TexCoords   = texCoord;
    gl_Position = vec4(vertices_position_modelspace.xy, 0.0, 1.0);
    v_camPos            = vec3(inverse(view)[3]);
}

///////////////////////////////////////////////////////////

#type fragment
#version 330 core

// Octahedral mapping and unmapping
#include octahedral.glsl

// Utility functions
#include utils.glsl

layout(location = 0) out vec4 color;

const int HIT     = 0;
const int MISS    = 1;
const int UNKNOWN = 2;

in vec2 TexCoords;
in vec3 v_camPos;

uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform vec3 cameraPos;

uniform sampler2D sceneBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;
uniform sampler2D roughnessBuffer;
uniform sampler2D directBuffer;
uniform mat4 view;
uniform float zNear     = 0.1;
uniform float zFar      = 9999.0;
uniform float traceBias = 0.05;


const float minThickness = 0.03; // meters
const float maxThickness = 0.50;

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

    return tangent * (r * cos(phi)) + bitangent * (r * sin(phi)) + hitNorm * sqrt(max(0.0, 1.0 - rand1));
}

float IGN(vec2 pixelCoord, int frameCount)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(pixelCoord.xy + float(frameCount) * vec2(47.0, 17.0) * 0.695, magic.xy)));
}

void main()
{
    color = texture(sceneBuffer, TexCoords);
}