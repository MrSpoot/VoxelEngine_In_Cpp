#include "Octree.h"
#include <iostream>

Octree::Octree(const glm::vec3& origin, const glm::vec3& halfDimension)
        : origin(origin), halfDimension(halfDimension), voxel(nullptr) {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
}

Octree::~Octree() {
    for (int i = 0; i < 8; ++i) {
        delete children[i];
    }
    delete voxel;
}

int Octree::getOctantContainingPoint(const glm::vec3& point) const {
    int oct = 0;
    if (point.x >= origin.x) oct |= 4;
    if (point.y >= origin.y) oct |= 2;
    if (point.z >= origin.z) oct |= 1;
    return oct;
}

void Octree::insert(const Voxel& voxel) {
    // If this node doesn't have a voxel yet, simply store it here
    if (this->voxel == nullptr) {
        this->voxel = new Voxel(voxel);
        return;
    }

    // Otherwise, we need to subdivide the node
    // Create children
    if (isLeafNode()) {
        for (int i = 0; i < 8; ++i) {
            // Compute new bounding box for this child
            glm::vec3 newOrigin = origin;
            newOrigin.x += halfDimension.x * (i & 4 ? 0.5f : -0.5f);
            newOrigin.y += halfDimension.y * (i & 2 ? 0.5f : -0.5f);
            newOrigin.z += halfDimension.z * (i & 1 ? 0.5f : -0.5f);
            children[i] = new Octree(newOrigin, halfDimension * 0.5f);
        }
    }

    // Reinsert the old voxel into the appropriate child
    int octant = getOctantContainingPoint(this->voxel->position);
    children[octant]->insert(*(this->voxel));

    // Insert the new voxel into the appropriate child
    octant = getOctantContainingPoint(voxel.position);
    children[octant]->insert(voxel);

    // Clear the voxel from this node
    delete this->voxel;
    this->voxel = nullptr;
}

void Octree::serialize(std::vector<GPUOctreeNode>& data) const {
    serializeNode(this, data);
}

void Octree::serializeNode(const Octree* node, std::vector<GPUOctreeNode>& data) const {
    if (node == nullptr) return;

    GPUOctreeNode gpuNode;
    gpuNode.center[0] = origin.x;
    gpuNode.center[1] = origin.y;
    gpuNode.center[2] = origin.z;
    gpuNode.size = node->halfDimension.x * 2.0f;
    gpuNode.isLeaf = node->isLeafNode() ? 1 : 0;
    gpuNode.isActive = node->voxel != nullptr ? 1 : 0;

    if(node->voxel){
        gpuNode.color[0] = node->voxel->color.r;
        gpuNode.color[1] = node->voxel->color.g;
        gpuNode.color[2] = node->voxel->color.b;
    }else{
        gpuNode.color[0] = 0.0f;
        gpuNode.color[1] = 0.0f;
        gpuNode.color[2] = 0.0f;
    }

    std::cout << "Serializing node at (" << node->origin.x << ", " << node->origin.y << ", " << node->origin.z
              << ") with size " << gpuNode.size << ", isLeaf: " << gpuNode.isLeaf
              << ", isActive: " << gpuNode.isActive
              << ", color: (" << gpuNode.color[0] << ", " << gpuNode.color[1] << ", " << gpuNode.color[2] << ")\n";

    int startIndex = data.size();
    data.push_back(gpuNode);

    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            gpuNode.children[i] = data.size();
            serializeNode(node->children[i], data);
        } else {
            gpuNode.children[i] = -1;
        }
    }

    data[startIndex] = gpuNode; // Mettre à jour le nœud parent avec les indices des enfants
}