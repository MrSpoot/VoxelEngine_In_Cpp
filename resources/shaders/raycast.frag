#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

// Camera parameters
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camRight;
uniform vec3 camUp;

// Voxel world parameters
uniform vec3 voxelWorldMin;
uniform vec3 voxelWorldMax;
uniform float voxelSize;

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
    // Generate ray
    vec2 uv = TexCoords * 2.0 - 1.0;
    vec3 rayDir = normalize(camDir + uv.x * camRight + uv.y * camUp);
    vec3 rayOrigin = camPos;

    vec3 voxelPos = floor((rayOrigin - voxelWorldMin) / voxelSize) * voxelSize + voxelWorldMin;
    vec3 step = sign(rayDir) * voxelSize;
    vec3 tMax = ((step + voxelPos - rayOrigin) / rayDir) * step / voxelSize;
    vec3 tDelta = step / rayDir;

    // Ray marching loop
    for (int i = 0; i < 256; i++) {
        float t = intersectVoxel(rayOrigin, rayDir, voxelPos, voxelSize);
        if (t > 0.0) {
            FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0); // Simple red color for hit voxel
            return;
        }
        voxelPos += step * vec3(lessThanEqual(tMax.xyz, vec3(0.0)));
        tMax += tDelta;
    }

    FragColor = vec4(0.0); // Background color
}
