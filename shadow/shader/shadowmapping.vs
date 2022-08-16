#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec3 aNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;


void main()
{
      gl_Position =  projection*view*model* vec4(aPos,1.0);
      vs_out.FragPos = vec3(model* vec4(aPos,1.0));
      vs_out.Normal = aNormal;
      vs_out.TexCoords = aTexcoord;
      vs_out.FragPosLightSpace =  lightSpaceMatrix* model* vec4(aPos,1.0);



   
}