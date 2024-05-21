#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 modelMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform bool drawingtype;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    if(drawingtype){
        gl_Position = projection * view * model  * vec4(aPos, 1.0);
        FragPos = vec3(model  * vec4(aPos, 1.0));
        TexCoord = aTexCoord;
        Normal = aNormal;
    }else{
        gl_Position = projection * view * modelMatrix  * vec4(aPos, 1.0);
        FragPos = vec3(modelMatrix  * vec4(aPos, 1.0));
        TexCoord = aTexCoord;
        Normal = aNormal;
    }
}