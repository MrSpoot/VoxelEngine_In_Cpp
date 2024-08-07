#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

// Camera parameters
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float fov;
uniform float aspectRatio;
uniform vec3 lightPos;
uniform int debugMode;
uniform int adaptDebug;

struct GPUOctreeNode {
    vec3 center;
    float size;
    int children[8];
    int isLeaf;
    vec3 color;
    int isActive;
    float minDistance;
    float maxDistance;
};

layout(std430, binding = 0) buffer OctreeBuffer {
    GPUOctreeNode nodes[];
};

// Function to calculate the intersection of a ray with a voxel
float intersectVoxel(vec3 ro, vec3 rd, vec3 voxelCenter, float voxelSize, out float tmin, out float tmax) {
    vec3 invDir = 1.0 / rd;
    vec3 t0s = (voxelCenter - voxelSize * 0.5 - ro) * invDir;
    vec3 t1s = (voxelCenter + voxelSize * 0.5 - ro) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger = max(t0s, t1s);

    tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    return (tmax >= max(tmin, 0.0)) ? tmin : -1.0;
}

// Function to compute the normal based on the hit position and voxel center
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

// Ray traversal function through the octree
bool traverseOctree(vec3 ro, vec3 rd, out vec3 hitPos, out vec3 normal, out vec3 color, out int step) {
    const int MAX_STACK_SIZE = 64;
    int stack[MAX_STACK_SIZE];
    int stackIndex = 0;
    stack[stackIndex++] = 0; // Start with root node

    float closestHit = 1e20;
    bool hit = false;

    while (stackIndex > 0 && stackIndex < MAX_STACK_SIZE) {
        step++;
        int nodeIndex = stack[--stackIndex];
        GPUOctreeNode node = nodes[nodeIndex];

        if (node.isActive == 0) {
            continue;
        }

        float tmin, tmax;
        float t = intersectVoxel(ro, rd, node.center, node.size, tmin, tmax);

        bool insideVoxel = all(greaterThanEqual(ro, node.center - vec3(node.size * 0.5))) &&
        all(lessThanEqual(ro, node.center + vec3(node.size * 0.5)));

        if (insideVoxel || t > 0.0) {
            if (node.isLeaf != 0) {
                if (tmin < closestHit && tmin >= 0.0) {
                    closestHit = tmin;
                    hitPos = ro + tmin * rd;
                    normal = computeNormal(hitPos, node.center);
                    color = node.color;
                    hit = true;
                }
            } else {
                for (int i = 0; i < 8; ++i) {
                    if (node.children[i] != -1) {
                        // Cull node if ray is far from the surface using minDistance
                        if (node.minDistance > closestHit) {
                            continue;
                        }
                        stack[stackIndex++] = node.children[i];
                    }
                }
            }
        }
    }
    return hit;
}

// Lighting function
vec3 light(vec3 pos, vec3 normal){
    vec3 lp = lightPos;
    vec3 ld = lp - pos;
    vec3 ln = normalize(ld);

    vec3 rayDir = ld;
    vec3 rayOrigin = pos;

    int step = 0;

    vec3 hitPos, outNormal, color;

    if (traverseOctree(rayOrigin, rayDir, hitPos, outNormal, color,step)) {
        return vec3(0.0); // Shadowed
    } else {
        return max(vec3(0.0), dot(normal, ln)); // Direct lighting
    }
}

// Main rendering function
void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    vec3 hitPos, normal, color;

    int step;

    if (traverseOctree(rayOrigin, rayDir, hitPos, normal, color,step)) {
        //color *= light(hitPos, normal) + 0.1;
        //color = pow(color, vec3(0.4545)); // Apply gamma correction
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.2); // Background color (dark gray)
    }

    if(debugMode == 1){
        FragColor = vec4(step)/adaptDebug;
    }
}
