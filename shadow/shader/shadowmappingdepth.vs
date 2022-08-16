#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
   
}