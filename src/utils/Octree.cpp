#include "Octree.h"
#include <iostream>
#include <random>

Octree::Octree(const glm::vec3 &origin, const glm::vec3 &halfDimension)
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

int Octree::getOctantContainingPoint(const glm::vec3 &point) const {
    int oct = 0;
    if (point.x >= origin.x) oct |= 4;
    if (point.y >= origin.y) oct |= 2;
    if (point.z >= origin.z) oct |= 1;
    return oct;
}

void Octree::insert(const Voxel &voxel, float voxelSize) {
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

void Octree::serialize(std::vector<GPUOctreeNode> &data) const {
    serializeNode(this, data);
}

bool Octree::serializeNode(const Octree *node, std::vector<GPUOctreeNode> &data) const {
    if (node == nullptr) return false;

    GPUOctreeNode gpuNode;
    gpuNode.center = node->origin;
    gpuNode.size = node->halfDimension.x * 2.0f;
    gpuNode.isLeaf = node->isLeafNode() ? 1 : 0;
    gpuNode.isActive = node->voxel != nullptr ? 1 : 0;

    if (node->voxel) {
        gpuNode.color = node->voxel->color;
    } else {
        gpuNode.color = glm::vec3(0.0f);
    }

    int startIndex = data.size();
    data.push_back(gpuNode);
    bool hasActiveChildren = false;

    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            gpuNode.children[i] = data.size();
            if (serializeNode(node->children[i], data)) {
                hasActiveChildren = true;
            }
        } else {
            gpuNode.children[i] = -1;
        }
    }

    if (hasActiveChildren) {
        gpuNode.isActive = 1;
    }

//    std::cout << "Serializing node at (" << node->origin.x << ", " << node->origin.y << ", " << node->origin.z
//              << ") with size " << gpuNode.size << ", isLeaf: " << gpuNode.isLeaf
//              << ", isActive: " << gpuNode.isActive
//              << ", color: (" << gpuNode.color[0] << ", " << gpuNode.color[1] << ", " << gpuNode.color[2] << ")\n";

    data[startIndex] = gpuNode;

    return gpuNode.isActive > 0;
}

void Octree::generateTerrain(float voxelSize, float frequency) {
    int i = 0;
    int j;
    int k;

    for (float x = -halfDimension.x; x < halfDimension.x; x += voxelSize) {
        i++;
        j = 0;
        for (float y = -halfDimension.y; y < halfDimension.y; y += voxelSize) {
            j++;
            k = 0;
            for (float z = -halfDimension.z; z < halfDimension.z; z += voxelSize) {
                k++;


                //if (i % 2 + j % 2 + k % 2 == 0) {

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

                    //insert(Voxel(origin + glm::vec3(x, y, z),glm::vec3(dis(gen), dis(gen), dis(gen))), voxelSize);
                    insert(Voxel(origin + glm::vec3(x, y, z),
                                 glm::vec3(i / (halfDimension.x * 2), j / (halfDimension.y * 2),
                                           k / (halfDimension.z * 2))), voxelSize);

                //}

            }
        }
    }
}