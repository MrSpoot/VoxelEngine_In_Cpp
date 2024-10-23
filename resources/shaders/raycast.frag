#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

// Paramètres de la caméra
uniform vec3 lightPos;
uniform vec3 camPos;      // Position de la caméra
uniform vec3 camDir;      // Direction de la caméra
uniform vec3 camRight;    // Vecteur à droite de la caméra
uniform vec3 camUp;       // Vecteur vers le haut de la caméra
uniform float fov;        // Champ de vision (en radians)
uniform float aspectRatio; // Rapport d'aspect de l'écran (width / height)
uniform float time;
uniform float voxelSize;  // Taille d'un voxel
uniform sampler3D voxelColorTexture; // Texture 3D contenant les voxels
uniform sampler3D binaryTexture; // Texture 3D contenant les voxels
uniform vec3 voxelTextureSize;  // Dimensions de la texture 3D (par ex. 64x64x64)

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

vec3 light(vec3 pos, vec3 normal) {
    //vec3 lp = vec3(cos(time) * 512.0 + voxelTextureSize.x / 2.0, 512.0, sin(time) * 512.0 + voxelTextureSize.z / 2.0);
    vec3 lp = vec3(0,512,0);
    vec3 ld = lp - pos;
    vec3 ln = normalize(ld);
    return max(vec3(0.0), dot(normal, ln));
}

vec3 normal(vec3 hitPos, vec3 voxelCenter) {
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

bool getBitFromByte(float byteValue, int bitIndex) {
    int byteAsInt = int(byteValue * 255.0);  // Assurer la conversion correcte de float en int
    return (byteAsInt & (1 << bitIndex)) != 0;  // Extraire correctement le bit
}

void main() {

    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    vec3 voxelPos = floor((rayOrigin / voxelSize) * voxelSize);
    vec3 step = sign(rayDir);
    vec3 tMax = ((voxelPos + step * 0.5 + step * 0.5 * sign(rayDir) - rayOrigin) / rayDir);
    vec3 tDelta = abs(step / rayDir);

    for (int i = 0; i < 1024; i++) {
        float t = intersectVoxel(rayOrigin, rayDir, voxelPos, voxelSize);
        if (t > 0.0) {
            vec3 voxelTexCoords = voxelPos / voxelTextureSize;

            if (all(greaterThanEqual(voxelTexCoords, vec3(0.0))) && all(lessThanEqual(voxelTexCoords, vec3(1.0)))) {
                vec3 compressedCoords = vec3(voxelPos.x / (8.0 * voxelTextureSize.x), voxelPos.y / voxelTextureSize.y, voxelPos.z / voxelTextureSize.z);
                float byteValue = texture(binaryTexture, compressedCoords).r;
                int voxelIndex = int(voxelPos.x + voxelPos.y * voxelTextureSize.x + voxelPos.z * voxelTextureSize.x * voxelTextureSize.y);
                int bitIndex = voxelIndex % 8;
                bool isActive = getBitFromByte(byteValue, bitIndex);

                if(isActive){

                    vec4 voxelData = texture(voxelColorTexture, voxelTexCoords);
                    vec3 color = voxelData.rgb;

                    vec3 hitPos = rayOrigin + rayDir * t;
                    vec3 voxelCenter = voxelPos + vec3(0.5 * voxelSize);
                    vec3 norm = normal(hitPos, voxelCenter);

                    color *= light(hitPos, norm) + 0.1;
                    color = pow(color, vec3(0.4545)); // Correction gamma

                    FragColor = vec4(color, 1.0);
                    return;
                }
            }

        }

        // Avancer dans la grille de voxels
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

    FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Fond noir si aucun voxel n'est trouvé
}
