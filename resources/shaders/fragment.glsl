#version 330 core

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 lightPos;

void main(){

//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(lightPos - FragPos);
//
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * vec3(1.0);
//
//    vec3 result = vec3(1.0) + diffuse;

    //FragColor = vec4(Color * result, 1.0);
    FragColor = vec4(abs(Normal.x),abs(Normal.y),abs(Normal.z),1.0);
}