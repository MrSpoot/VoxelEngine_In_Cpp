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

struct GPUOctreeNode {
    vec3 center;
    float size;
    int children[8];
    int isLeaf;
    vec3 color;
    int isActive;
};

layout(std430, binding = 0) buffer OctreeBuffer {
    GPUOctreeNode nodes[];
};

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

bool traverseOctree(vec3 ro, vec3 rd, out vec3 hitPos, out vec3 normal, out vec3 color) {
    const int MAX_STACK_SIZE = 256;
    int stack[MAX_STACK_SIZE];
    int stackPtr = 0;
    stack[stackPtr++] = 0; // Start with the root node

    while (stackPtr > 0 && stackPtr < MAX_STACK_SIZE) {
        int nodeIndex = stack[--stackPtr];

        if (nodeIndex < 0) {
            continue;
        }

        GPUOctreeNode node = nodes[nodeIndex];

        if (node.isLeaf == 1) {
            if (node.isActive == 1) {
                float t = intersectVoxel(ro, rd, node.center - node.size * 0.5, node.size);
                if (t > 0.0) {
                    hitPos = ro + t * rd;
                    normal = computeNormal(hitPos, node.center);
                    color = node.color;
                    return true;
                }
            }
        } else {
            for (int i = 0; i < 8; ++i) {
                int childIndex = node.children[i];
                if (childIndex >= 0) {
                    stack[stackPtr++] = childIndex;
                }
            }
        }
    }

    return false;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 rayOrigin = camPos;

    vec3 hitPos, normal, color;

    if (traverseOctree(rayOrigin, rayDir, hitPos, normal, color)) {
        color = pow(color, vec3(0.4545)); // Apply gamma correction
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(color, 1.0); // Debug color (blue) if no voxel is hit
    }
}
