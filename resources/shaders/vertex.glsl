#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in mat4 modelMatrix;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * modelMatrix  * vec4(aPos, 1.0);
    FragPos = vec3(modelMatrix  * vec4(aPos, 1.0));
    Normal = aNormal;
    Color = vec3(0.8);
}