#type vertex
#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;

void main()
{
    gl_Position = projection * view * vec4(inPosition, 1.0);
    vColor      = inColor;
}

#type geometry
#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
uniform float uLineWidth = 200.0;

in vec3 vColor[];
out vec3 fColor;

void main()
{
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;

    vec3 dir = normalize(p0.xyz - p1.xyz);
    vec3 up  = vec3(0.0, 1.0, 0.0);
    if (abs(dot(dir, up)) > 0.99)
        up = vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(dir, up));
    up         = normalize(cross(right, dir));

    vec4 offset = vec4(up.xy * uLineWidth * 0.5, 0.0, 0.0);

    gl_Position = p0 + offset / p0.w;
    fColor      = vColor[0];
    EmitVertex();

    gl_Position = p0 - offset / p0.w;
    fColor      = vColor[0];
    EmitVertex();

    gl_Position = p1 + offset / p1.w;
    fColor      = vColor[1];
    EmitVertex();

    gl_Position = p1 - offset / p1.w;
    fColor      = vColor[1];
    EmitVertex();

    EndPrimitive();
}

#type fragment
#version 330 core

in vec3 fColor;
out vec4 color;

void main()
{
    color = vec4(fColor, 1.0);
}