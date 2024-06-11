//
// Created by Tom on 11/06/2024.
//

#include "Octree.h"

Octree::Octree() {
    root = new OctreeNode(glm::vec3(0.0),glm::vec3(8.0));
}

void Octree::Insert(Voxel *voxel) {

}
