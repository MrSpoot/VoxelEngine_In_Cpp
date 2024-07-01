#ifndef VOXELENGINE_OCTREENODE_H
#define VOXELENGINE_OCTREENODE_H

#include "Voxel.h"

class OctreeNode {
public:
    OctreeNode* children[8];
    bool isLeaf;

    void insert(Voxel& voxel);
private:

};

#endif //VOXELENGINE_OCTREENODE_H
