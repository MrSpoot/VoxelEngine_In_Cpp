#version 330 core

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 lightPos;

void main(){
    FragColor = vec4(Color,1.0);
}