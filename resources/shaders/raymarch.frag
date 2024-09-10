#version 430 core

#define MAX_STEPS 128
#define MIN_STEP_SIZE 0.001
#define MAX_STEP_SIZE 64

out vec4 FragColor;
in vec2 TexCoords;

// Paramètres de la caméra
uniform vec3 camPos;      // Position de la caméra
uniform vec3 camDir;      // Direction de la caméra
uniform vec3 camRight;    // Vecteur à droite de la caméra
uniform vec3 camUp;       // Vecteur vers le haut de la caméra
uniform float fov;        // Champ de vision (en radians)
uniform float aspectRatio; // Rapport d'aspect de l'écran (width / height)
uniform float time;

// Uniforme pour la texture 3D contenant les SDF
uniform sampler3D sdfTexture;

// Fonction pour lire la SDF dans la texture 3D
float getSDF(vec3 p) {
    // Les coordonnées doivent être normalisées entre [0,1] pour la texture 3D
    vec3 texCoord = (p + vec3(32.0)) / 64.0;  // Assumes gridSize is 64
    return texture(sdfTexture, texCoord).r;
}

vec2 march(vec3 ro, vec3 rd){
    vec3 cp = ro;
    float d = 0.0;
    vec2 s = vec2(0.0);

    for(int i = 0; i < MAX_STEPS; i++){
        cp = ro + rd * d;
        float sdfDistance = getSDF(cp); // Lire la distance depuis la texture SDF
        s = vec2(1.0, sdfDistance);  // `1.0` est un identifiant arbitraire ici
        d += sdfDistance;

        if(sdfDistance < MIN_STEP_SIZE){
            break;
        }

        if(d > MAX_STEP_SIZE){
            return vec2(100.0,MAX_STEP_SIZE + 10.0);
        }
    }
    s.y = d;
    return s;
}

// Fonction de calcul des normales en utilisant le gradient
vec3 normal(vec3 p) {
    float eps = 0.01;
    vec3 n;
    n.x = getSDF(p + vec3(eps, 0.0, 0.0)) - getSDF(p - vec3(eps, 0.0, 0.0));
    n.y = getSDF(p + vec3(0.0, eps, 0.0)) - getSDF(p - vec3(0.0, eps, 0.0));
    n.z = getSDF(p + vec3(0.0, 0.0, eps)) - getSDF(p - vec3(0.0, 0.0, eps));
    return normalize(n);
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rd = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 ro = camPos;

    vec2 s = march(ro, rd);

    vec3 color = vec3(0.0);
    float d = s.y;

    if(d < MAX_STEP_SIZE){
        vec3 p = ro + rd * d;
        vec3 nor = normal(p);
        color = nor * 0.5 + 0.5;  // Affichage des normales comme couleur
    }

    FragColor = vec4(color, 1.0);
}
