//
// Created by Tom on 25/05/2024.
//

#ifndef VOXELENGINE_VOXEL_H
#define VOXELENGINE_VOXEL_H

#include <vector>
#include <glm/glm.hpp>

struct Voxel {
    float x, y, z;
    glm::vec3 color;
    bool isActive;
    Voxel(float x, float y, float z, glm::vec3 color, bool isActive = true)
            : x(x), y(y), z(z), color(color), isActive(isActive) {}
};


#endif //VOXELENGINE_VOXEL_H
