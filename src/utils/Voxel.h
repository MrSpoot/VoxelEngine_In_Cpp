//
// Created by Tom on 25/05/2024.
//

#ifndef VOXELENGINE_VOXEL_H
#define VOXELENGINE_VOXEL_H

#include <vector>
#include <glm/glm.hpp>

struct Voxel {
    glm::vec3 position;
    glm::vec3 color;

    Voxel(const glm::vec3& position, const glm::vec3& color)
            : position(position), color(color) {}
};


#endif //VOXELENGINE_VOXEL_H
