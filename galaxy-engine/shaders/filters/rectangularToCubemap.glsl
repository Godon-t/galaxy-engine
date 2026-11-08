#type vertex
#version 330 core
layout(location = 0) in vec3 vertices_position_modelspace;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    localPos               = vertices_position_modelspace;
    mat4 viewNoTranslation = mat4(mat3(view));
    gl_Position            = projection * viewNoTranslation * vec4(vertices_position_modelspace, 1.0);
}

////////////////////////////////////////////////////////////////////

#type fragment
#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform sampler2D environmentHDR;

const float PI = 3.14159265359;

vec4 sampleEquirectangular(vec3 direction, sampler2D env){
    float u = atan(direction.z, direction.x) / (2.0 * PI) + 0.5;

    float v = acos(clamp(direction.y, -1.0, 1.0)) / PI;

    return texture(env, vec2(u, v));
}


void main()
{
    vec3 direction = normalize(localPos);
    vec3 color = sampleEquirectangular(direction, environmentHDR).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor  = vec4(color, 1.0);
}
