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

in vec2 TexCoords;

uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform mat4 projection;
uniform vec3 cameraPos;

uniform sampler2D sceneBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D depthBuffer;
uniform mat4 view;
uniform float zNear = 0.1;
uniform float zFar  = 9999.0;

uniform int numSamples = 16;
uniform float rayStep = 0.1;
uniform int maxSteps = 32;
uniform float giIntensity = 1.0;
uniform float radius = 0.5;
uniform float bias = 0.02;

const float PI = 3.14159265359;

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

vec3 reconstructWorldPosFromDepth(vec2 uv, float depth)
{
    vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewSpacePos = inverseProjection * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;
    vec4 worldSpacePos = inverseView * viewSpacePos;
    return worldSpacePos.xyz;
}

vec3 worldToScreen(vec3 worldPos)
{
    vec4 clipSpacePos = projection * view * vec4(worldPos, 1.0);
    clipSpacePos.xyz /= clipSpacePos.w;
    return clipSpacePos.xyz * 0.5 + 0.5;
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// Générer une direction aléatoire dans l'hémisphère
vec3 cosineWeightedHemisphere(vec3 normal, vec2 seed)
{
    float r1 = rand(seed);
    float r2 = rand(seed + vec2(1.0, 0.0));
    
    float phi = 2.0 * PI * r1;
    float cosTheta = sqrt(r2);
    float sinTheta = sqrt(1.0 - r2);
    
    vec3 tangent;
    if (abs(normal.y) < 0.999) {
        tangent = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
    } else {
        tangent = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
    }
    vec3 bitangent = cross(normal, tangent);
    
    vec3 direction = tangent * cos(phi) * sinTheta +
                     bitangent * sin(phi) * sinTheta +
                     normal * cosTheta;
    
    return normalize(direction);
}

// ray marching en screen space
vec4 screenSpaceRayMarch(vec3 startPos, vec3 direction, vec3 normal)
{
    vec3 rayPos = startPos + normal * bias;
    vec3 rayEnd = rayPos + direction * radius;
    
    vec3 screenStart = worldToScreen(rayPos);
    vec3 screenEnd = worldToScreen(rayEnd);
    
    // si le rayon sort de l'ecran, retourner violer pour debug
    if (screenStart.z <= 0.0 || screenEnd.z <= 0.0) {
        return vec4(1.0, 0.0, 1.0, 1.0);
    }
    
    vec3 screenDir = screenEnd - screenStart;
    vec3 screenStep = screenDir / float(maxSteps);
    
    vec3 currentScreen = screenStart;
    
    for (int i = 0; i < maxSteps; ++i) {
        currentScreen += screenStep;
        
        // Verifier si on est toujours dans l'ecran
        if (currentScreen.x < 0.0 || currentScreen.x > 1.0 ||
            currentScreen.y < 0.0 || currentScreen.y > 1.0) {
            break;
        }
        
        // Echantillonner la profondeur a cette position
        float sampledDepth = texture(depthBuffer, currentScreen.xy).r;
        float depthDiff = currentScreen.z - sampledDepth;
        
        // si on a touche
        if (depthDiff > 0.0 && depthDiff < 0.01) {
            // prendre la couleur
            vec3 hitColor = texture(sceneBuffer, currentScreen.xy).rgb;
            
            // Calculer l'attenuation due a la distance
            float distance = length(currentScreen.xy - screenStart.xy);
            float attenuation = 1.0 - smoothstep(0.0, 1.0, distance);
            
            return vec4(hitColor * attenuation, attenuation);
        }
    }
    
    return vec4(0.0);
}

void main()
{
    // recuperer les informations du Buffer
    float depth = texture(depthBuffer, TexCoords).r;
    
    // Si on est sur le fond (skybox), pas de GI
    if (depth >= 0.9999) {
        color = texture(sceneBuffer, TexCoords);
        return;
    }
    
    vec3 normal = normalize(texture(normalBuffer, TexCoords).rgb * 2.0 - 1.0);
    vec3 worldPos = reconstructWorldPosFromDepth(TexCoords, depth);
    vec3 baseColor = texture(sceneBuffer, TexCoords).rgb;
    
    // Accumuler l'illumination indirecte
    vec3 indirectLight = vec3(0.0);
    float totalWeight = 0.0;
    
    // Echantillonnage Monte Carlo dans l'hemisphere
    for (int i = 0; i < numSamples; ++i) {
        vec2 seed = TexCoords + vec2(float(i) * 0.1, float(i) * 0.05);
        vec3 sampleDir = cosineWeightedHemisphere(normal, seed);
        
        // Ray march dans cette direction
        vec4 hitResult = screenSpaceRayMarch(worldPos, sampleDir, normal);
        
        if (hitResult.a > 0.0) {
            float NdotL = max(dot(normal, sampleDir), 0.0);
            indirectLight += hitResult.rgb * NdotL * hitResult.a;
            totalWeight += hitResult.a;
        }
    }
    
    // Normaliser et appliquer l'intensite
    if (totalWeight > 0.0) {
        indirectLight /= float(numSamples);
        indirectLight *= giIntensity;
    }
    
    // Combiner la couleur de base avec l'illumination indirecte
    color.rgb = baseColor + indirectLight;
    color.a = 1.0;
}