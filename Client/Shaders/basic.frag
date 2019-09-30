#version 330

in vec2 fTex;

uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
    FragColor = texture(texture1, fTex);
};