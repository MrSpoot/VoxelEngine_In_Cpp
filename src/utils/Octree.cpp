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

void Octree::insert(const Voxel& voxel, float voxelSize) {
    // Si ce noeud ne contient pas de voxel et que sa taille est adaptée, insérez-le ici
    if (this->voxel == nullptr && halfDimension.x * 2.0f <= voxelSize) {
        this->voxel = new Voxel(voxel);
        return;
    }

    // Sinon, nous devons subdiviser le noeud
    // Créer les enfants si ce noeud est une feuille
    if (isLeafNode()) {
        for (int i = 0; i < 8; ++i) {
            // Calculer la nouvelle origine pour cet enfant
            glm::vec3 newOrigin = origin;
            newOrigin.x += halfDimension.x * (i & 4 ? 0.5f : -0.5f);
            newOrigin.y += halfDimension.y * (i & 2 ? 0.5f : -0.5f);
            newOrigin.z += halfDimension.z * (i & 1 ? 0.5f : -0.5f);
            children[i] = new Octree(newOrigin, halfDimension * 0.5f);
        }
    }

    // Réinsérer l'ancien voxel dans l'enfant approprié
    if (this->voxel != nullptr) {
        int octant = getOctantContainingPoint(this->voxel->position);
        children[octant]->insert(*(this->voxel), voxelSize);
        delete this->voxel;  // Supprimer l'ancien voxel du noeud actuel
        this->voxel = nullptr;
    }

    // Insérer le nouveau voxel dans l'enfant approprié
    int octant = getOctantContainingPoint(voxel.position);
    children[octant]->insert(voxel, voxelSize);
}

void Octree::serialize(std::vector<GPUOctreeNode>& data) const {
    serializeNode(this, data);
}

void Octree::serializeNode(const Octree* node, std::vector<GPUOctreeNode>& data) const {
    if (node == nullptr) return;

    GPUOctreeNode gpuNode;
    gpuNode.center = origin;
    gpuNode.size = node->halfDimension.x * 2.0f;
//    gpuNode.isLeaf = node->isLeafNode() ? 1 : 0;
//    gpuNode.isActive = node->voxel != nullptr ? 1 : 0;
    gpuNode.isActive = 1;
    gpuNode.isLeaf = 1;
    gpuNode.color = node->voxel ? node->voxel->color : glm::vec3(1);

    std::cout << "Serializing node at (" << node->origin.x << ", " << node->origin.y << ", " << node->origin.z
              << ") with size " << gpuNode.size << ", isLeaf: " << gpuNode.isLeaf
              << ", isActive: " << gpuNode.isActive
              << ", color: (" << gpuNode.color.r << ", " << gpuNode.color.g << ", " << gpuNode.color.b << ")\n";

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