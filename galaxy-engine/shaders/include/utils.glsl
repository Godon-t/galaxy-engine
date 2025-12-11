///// Constants

const float PI = 3.14159265359;

///// Functions
vec3 computeIrradiance(vec3 dir, samplerCube cubemap)
{
    vec3 irradiance = vec3(0.0);
    vec3 up         = vec3(0.0, 1.0, 0.0);
    vec3 right      = normalize(cross(up, dir));
    up              = normalize(cross(dir, right));

    float sampleDelta = 0.025;
    float nrSamples   = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * dir;

            irradiance += texture(cubemap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    return irradiance;
}

vec3 reconstructWorldPos(vec2 uv, float depth, mat4 inverseProjection, mat4 inverseView)
{
    float z  = depth * 2.0 - 1.0; // NDC
    vec4 ndc = vec4(uv * 2.0 - 1.0, z, 1.0);

    vec4 viewPos = inverseProjection * ndc;
    viewPos /= viewPos.w;

    vec4 worldPos = inverseView * viewPos;
    return worldPos.xyz;
}

float linearDepth(float depth, float zNear, float zFar)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}