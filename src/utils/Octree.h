//
// Created by Tom on 11/06/2024.
//

#ifndef VOXELENGINE_OCTREE_H
#define VOXELENGINE_OCTREE_H

#include "Voxel.h"
#include "OctreeNode.h"

class Octree {

public:

    Octree();

    void Insert(Voxel *voxel);

    ~Octree();

private:
    void InsertImpl(Voxel *voxel);

    OctreeNode *root;
};

#endif //VOXELENGINE_OCTREE_H
