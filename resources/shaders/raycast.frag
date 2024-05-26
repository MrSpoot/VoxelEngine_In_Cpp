#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

// Paramètres de la caméra
uniform vec3 camPos;      // Position de la caméra
uniform vec3 camDir;      // Direction de la caméra
uniform vec3 camRight;    // Vecteur à droite de la caméra
uniform vec3 camUp;       // Vecteur vers le haut de la caméra
uniform float fov;        // Champ de vision (en radians)
uniform float aspectRatio; // Rapport d'aspect de l'écran (width / height)

struct Voxel {
    vec3 color;
    bool isActive;
};

// Déclarer le SSBO
layout(std430, binding = 0) buffer Voxels {
    Voxel voxels[];
};

// Position et taille de la grille de voxels
const vec3 voxelGridMin = vec3(0.0, 0.0, 0.0);
const float voxelSize = 1.0;
const int voxelGridSize = 32;


float intersectVoxel(vec3 ro, vec3 rd, vec3 voxelPos, float voxelSize) {
    vec3 invDir = 1.0 / rd;
    vec3 t0s = (voxelPos - ro) * invDir;
    vec3 t1s = (voxelPos + voxelSize - ro) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);

    float tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    return (tmax >= max(tmin, 0.0)) ? tmin : -1.0;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    //vec3 voxelPos = vec3(1.0);
    vec3 voxelPos = floor((rayOrigin) / voxelSize) * voxelSize;
    vec3 step = sign(rayDir) * voxelSize;
    vec3 tMax = ((voxelPos + step * 0.5 + step * 0.5 * sign(rayDir) - rayOrigin) / rayDir);
    vec3 tDelta = abs(step / rayDir);

    for (int i = 0; i < 256; i++) {
        float t = intersectVoxel(rayOrigin, rayDir, voxelPos, voxelSize);
        if (t > 0.0) {
            int index = int(voxelPos.x) + int(voxelPos.y) * voxelGridSize + int(voxelPos.z) * voxelGridSize * voxelGridSize;
            if (voxels[index].isActive) {
                FragColor = vec4(voxels[index].color, 1.0);
                return;
            }
        }
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            voxelPos.x += step.x;
            tMax.x += tDelta.x;
        } else if (tMax.y < tMax.z) {
            voxelPos.y += step.y;
            tMax.y += tDelta.y;
        } else {
            voxelPos.z += step.z;
            tMax.z += tDelta.z;
        }
    }

    FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    // Pour l'instant, nous affichons simplement la direction du rayon comme couleur
    //FragColor = vec4(rayDir * 0.5 + 0.5, 1.0); // Convertir de [-1, 1] à [0, 1]
}
