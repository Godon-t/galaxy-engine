#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

out vec4 fragPosLightSpace;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(vertices_position_modelspace, 1.0);

    fragPosLightSpace = gl_Position;
}

///////////////////////////////////////////////////////////
#type fragment
#version 330 core

in vec4 fragPosLightSpace;

uniform float zNear = 0.1;
uniform float zFar  = 999.0;
float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    // float ndc    = fragPosLightSpace.z / fragPosLightSpace.w;
    // float d01    = ndc * 0.5 + 0.5;
    // gl_FragDepth = d01;
    // gl_FragDepth = fragPosLightSpace.z;

    // gl_FragDepth = linearDepth(fragPosLightSpace.z) / zFar;
}
