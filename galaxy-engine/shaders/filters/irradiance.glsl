#type vertex
#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords              = vertices_position_modelspace;
    mat4 viewNoTranslation = mat4(mat3(view));
    gl_Position            = projection * viewNoTranslation * vec4(vertices_position_modelspace, 1.0);
}

////////////////////////////////////////////////////////////////////

#type fragment
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(TexCoords);

    vec3 irradiance = vec3(0.0);
    vec3 up         = vec3(0.0, 1.0, 0.0);
    vec3 right      = normalize(cross(up, normal));
    up              = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples   = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(skybox, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    FragColor  = vec4(irradiance, 1.0);
}