#version 330 core

in vec2 texCoords;
out vec4 color;

void main(){
    color = vec4(texCoords, 0.1f, 1.f);
}
