#version 330

layout (location=0) in vec3 vPos;
layout (location=1) in vec2 vTex;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 translation;


out vec2 fTex;

void main()
{
    gl_Position = projection * view * translation * vec4(vPos, 1.0);
    fTex = vTex;
};