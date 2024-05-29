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
uniform float time;

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
const int voxelGridSize = 64;

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

float getSinusoidalValue(float x) {
    return (voxelGridSize / 2) * sin(x) + (voxelGridSize / 2);
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    vec3 voxelPos = floor((rayOrigin / voxelSize) * voxelSize);
    vec3 step = sign(rayDir) * voxelSize;
    vec3 tMax = ((voxelPos + step * 0.5 + step * 0.5 * sign(rayDir) - rayOrigin) / rayDir);
    vec3 tDelta = abs(step / rayDir);

    for (int i = 0; i < 256; i++) {
        float t = intersectVoxel(rayOrigin, rayDir, voxelPos, voxelSize);
        if (t > 0.0) {
            int indexX = int(voxelPos.x / voxelSize);
            int indexY = int(voxelPos.y / voxelSize);
            int indexZ = int(voxelPos.z / voxelSize);

            if (indexX >= 0 && indexX < voxelGridSize && indexY >= 0 && indexY < voxelGridSize && indexZ >= 0 && indexZ < voxelGridSize) {
                int index = indexX + indexY * voxelGridSize + indexZ * voxelGridSize * voxelGridSize;
                if (voxels[index].isActive) {
                    FragColor = vec4(voxels[index].color, 1.0);
                    FragColor = vec4(float(voxelPos.x) / voxelGridSize, float(voxelPos.y) / voxelGridSize, float(voxelPos.z) / voxelGridSize, 1.0);
                    return;
                }
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
}
