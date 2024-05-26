//
// Created by Tom on 25/05/2024.
//

#ifndef VOXELENGINE_OCTREENODE_H
#define VOXELENGINE_OCTREENODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Voxel.h"

struct OctreeNode {
    glm::vec3 minBound; // Minimum boundary of the node
    glm::vec3 maxBound; // Maximum boundary of the node
    Voxel* voxel; // Pointer to a voxel if this is a leaf node
    OctreeNode* children[8]; // Pointers to child nodes

    OctreeNode(const glm::vec3& minB, const glm::vec3& maxB)
            : minBound(minB), maxBound(maxB), voxel(nullptr) {
        for (int i = 0; i < 8; ++i) {
            children[i] = nullptr;
        }
    }

    ~OctreeNode() {
        delete voxel;
        for (int i = 0; i < 8; ++i) {
            delete children[i];
        }
    }

    bool isLeaf() const {
        for (int i = 0; i < 8; ++i) {
            if (children[i] != nullptr) {
                return false;
            }
        }
        return true;
    }
};

#endif //VOXELENGINE_OCTREENODE_H
