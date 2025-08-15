#version 330 core

in vec2 texCoords;
out vec4 color;

void main()
{
    color = vec4(0.2f, texCoords, 1.f);
}
