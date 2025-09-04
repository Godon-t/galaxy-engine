#version 330 core

in vec2 texCoords;
out vec4 color;

uniform bool useTexture;
uniform sampler2D testTexture;

void main()
{
    if (useTexture) {
        color = vec4(texture(testTexture, texCoords).rgb, 1.0);
    } else {
        color = vec4(0.2f, texCoords, 1.f);
    }
}
