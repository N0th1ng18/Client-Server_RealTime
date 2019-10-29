#version 330

in vec2 fTex;

uniform sampler2D texture1;

uniform vec3 textColor;
uniform float cWidth;
uniform float cEdge;

out vec4 FragColor;

void main()
{
    float distance = 1.0 - texture(texture1, fTex).a;
    float alpha = 1.0 - smoothstep(cWidth, cWidth + cEdge, distance);

    FragColor = vec4(textColor, alpha);
};