#version 430 core

#define MAX_STEPS 128
#define MIN_STEP_SIZE 1e-4
#define THRESHOLD_DISTANCE 4  // Seuil de distance pour passer au raycasting
#define FIXED_STEP_SIZE 0.1     // Taille de pas fixe pour le raycasting

out vec4 FragColor;
in vec2 TexCoords;

uniform isampler3D sdfTexture;
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float fov;
uniform float aspectRatio;
uniform float voxelSize;

void main() {
    // Normaliser les coordonnées de texture entre [-1, 1]
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    // Générer la direction du rayon pour chaque pixel
    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    // Position initiale du voxel
    vec3 voxelPos = floor(rayOrigin / voxelSize);
    vec3 step = sign(rayDir);
    vec3 tMax = (voxelPos + step * 0.5 - rayOrigin) / rayDir;
    vec3 tDelta = abs(step / rayDir);

    float t = 0.0;
    bool useRaycasting = false;

    // Phase de raymarching
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = rayOrigin + rayDir * t;

        // Lire la distance depuis la texture SDF
        ivec3 texCoords = ivec3(floor(pos / voxelSize));
        texCoords = clamp(texCoords, ivec3(0), ivec3(textureSize(sdfTexture, 0)) - ivec3(1));
        int sdfDistance = texelFetch(sdfTexture, texCoords, 0).r;

        // Si la distance est inférieure au seuil, passer au raycasting
        if (sdfDistance < THRESHOLD_DISTANCE) {
            useRaycasting = true;
            break;
        }

        // Avancer avec du raymarching (sauts proportionnels à la distance)
        t += float(sdfDistance) * voxelSize;
    }

    // Si on passe au raycasting
    if (useRaycasting) {
        for (int j = 0; j < MAX_STEPS; j++) {
            vec3 pos = rayOrigin + rayDir * t;

            // Lire les informations de voxel à la position actuelle
            ivec3 texCoords = ivec3(floor(pos / voxelSize));
            texCoords = clamp(texCoords, ivec3(0), ivec3(textureSize(sdfTexture, 0)) - ivec3(1));
            int sdfDistance = texelFetch(sdfTexture, texCoords, 0).r;

            float t = intersectVoxel(rayOrigin, rayDir, voxelPos, voxelSize);
            if(t < 0.0){

            }

            if (sdfDistance == 0) {
                // Si la distance est 0, on a touché un voxel
                FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                return;
            }

            // Avancer avec un pas fixe
            t += FIXED_STEP_SIZE;

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
    }

    // Aucun voxel trouvé
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
