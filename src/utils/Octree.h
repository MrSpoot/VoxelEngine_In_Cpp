#ifndef VOXELENGINE_OCTREE_H
#define VOXELENGINE_OCTREE_H

#include <vector>
#include <glm/glm.hpp>
#include "Voxel.h"

struct GPUOctreeNode {
    float center[3];
    float size;
    int children[8];
    int isLeaf;
    float color[3];
    int isActive;
};

class Octree {
public:
    Octree(const glm::vec3& origin, const glm::vec3& halfDimension);
    ~Octree();

    void insert(const Voxel& voxel);
    bool isLeafNode() const;
    void serialize(std::vector<GPUOctreeNode>& data) const;
    void serializeNode(const Octree* node, std::vector<GPUOctreeNode>& data) const;

private:
    // The tree has up to eight children
    Octree* children[8];
    // The tree stores a single voxel at its node
    Voxel* voxel;
    // The tree's origin
    glm::vec3 origin;
    // Half the dimension of the node
    glm::vec3 halfDimension;

    int getOctantContainingPoint(const glm::vec3& point) const;
};

inline bool Octree::isLeafNode() const {
    // This is correct, since if this node has any children, it cannot be a leaf node
    return children[0] == nullptr;
}

#endif //VOXELENGINE_OCTREE_H
