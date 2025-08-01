#version 330 core


layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;


out vec2 texCoords;

void main(){
    texCoords = texCoord;
    gl_Position = vec4(vertices_position_modelspace,1);
}