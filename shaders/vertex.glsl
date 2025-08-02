#version 330 core


layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoords;

void main(){
    texCoords = texCoord;
    gl_Position = vec4(vertices_position_modelspace,1);
    gl_Position = projection * view * model * gl_Position;
}