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
uniform int voxelGridSize;

// Déclarer les constantes
const float MAX_DISTANCE = 250.0;
const int MAX_DETAIL = 2;
const int MIN_DETAIL = -1;
const float SCALE = 10.0;
const int POWER = int(log2(SCALE));

// Fonction pour générer un nombre aléatoire basé sur une position et une taille
float Random(vec4 v) {
    return fract(1223.34 * tan(dot(v, vec4(181.11, 132.52, 171.29, 188.42))));
}

// Fonction pour obtenir le type de voxel (vide, subdivisé ou plein) et une couleur aléatoire
int GetVoxel(vec3 Position, float Size, out vec3 color) {
    //if (length(Position.xy) == 0.0) return 0; // Garder le tunnel au centre vide
    float Value = Random(vec4(Position, Size));
    if (Value < 0.6) {
        color = vec3(0.0);
        return 0;
    }
    if (Value < 0.9) {
        color = vec3(Random(vec4(Position + 1.0, Size)),
        Random(vec4(Position + 2.0, Size)),
        Random(vec4(Position + 3.0, Size)));
        return 1;
    } else {
        color = vec3(Random(vec4(Position + 4.0, Size)),
        Random(vec4(Position + 5.0, Size)),
        Random(vec4(Position + 6.0, Size)));
        return 2;
    }
}

// Fonction pour calculer la lumière
vec3 light(vec3 pos, vec3 normal) {
    vec3 lp = vec3(0,0,0);
    vec3 ld = lp - pos;
    vec3 ln = normalize(ld);
    return max(vec3(0.0), dot(normal, ln));
}

// Fonction pour calculer la normale à l'intersection
vec3 computeNormal(vec3 hitPos, vec3 voxelCenter) {
    vec3 normal = normalize(hitPos - voxelCenter);
    if (abs(normal.x) > abs(normal.y) && abs(normal.x) > abs(normal.z)) {
        normal = vec3(sign(normal.x), 0.0, 0.0);
    } else if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z)) {
        normal = vec3(0.0, sign(normal.y), 0.0);
    } else {
        normal = vec3(0.0, 0.0, sign(normal.z));
    }
    return normal;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;
    vec3 rayDirSign = sign(rayDir);

    vec3 mask = vec3(0.0);
    bool exitLevel = false;
    int level = MIN_DETAIL;
    float size = -float(MIN_DETAIL) * SCALE;
    float distance = 0.0;
    bool hitVoxel = false;
    vec3 color = vec3(0.0);

    vec3 rayPosFloor = floor(rayOrigin / size) * size;
    vec3 rayPosFract = rayOrigin - rayPosFloor;
    vec3 lastRayPosFloor = rayPosFloor;
    vec3 correction = 1.0 / max(abs(rayDir), 1e-4);

    for (int i = 0; i < 200 && distance < MAX_DISTANCE && !hitVoxel; ++i) {
        while (exitLevel) {
            level--;
            size *= SCALE;
            vec3 newRayPosFloor = floor(rayPosFloor / size) * size;
            rayPosFract += rayPosFloor - newRayPosFloor;
            rayPosFloor = newRayPosFloor;
            exitLevel = level > MIN_DETAIL && floor(rayPosFloor / size / SCALE) != floor(lastRayPosFloor / size / SCALE);
        }

        switch (GetVoxel(rayPosFloor, size, color)) {
            case 1: { // Subdivide
                if (level < MAX_DETAIL) {
                    level++;
                    for (int j = 0; j < POWER; ++j) {
                        size /= 2.0;
                        vec3 step = step(vec3(size), rayPosFract) * size;
                        rayPosFloor += step;
                        rayPosFract -= step;
                    }
                    break;
                }
            }
            case 0: { // Empty
                float halfSize = size / 2.0;
                vec3 hit = -correction * (rayDirSign * (rayPosFract - halfSize) - halfSize);
                mask = vec3(lessThanEqual(hit.xyz, min(hit.yzx, hit.zxy)));
                float nearestVoxelDistance = dot(hit, mask);
                distance += nearestVoxelDistance;
                vec3 step = mask * rayDirSign * size;

                rayPosFract += rayDir * nearestVoxelDistance - step;

                lastRayPosFloor = rayPosFloor;
                rayPosFloor += step;

                exitLevel = level > MIN_DETAIL && floor(rayPosFloor / size / SCALE) != floor(lastRayPosFloor / size / SCALE);
                break;
            }
            case 2: hitVoxel = true;
        }
    }

    if (hitVoxel) {
        vec3 hitPos = rayOrigin + rayDir * distance;
        vec3 voxelCenter = rayPosFloor + vec3(0.5 * size);
        vec3 normal = computeNormal(hitPos, voxelCenter);

        color *= light(hitPos, normal) + 0.1;
        color = pow(color, vec3(0.4545));

        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
