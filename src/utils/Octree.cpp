#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Voxel.h"

struct alignas(16) GPUOctreeNode {
    alignas(16) glm::vec3 center;   // Centre du nœud dans l'espace
    float size;         // Taille du nœud (longueur de l'arête)
    int children[8];    // Indices des enfants dans le tableau des nœuds
    int isLeaf;         // Indicateur de feuille
    alignas(16) glm::vec3 color;    // Couleur moyenne des voxels dans ce nœud
    int isActive;       // Indicateur de nœud actif
    float minDistance;  // Distance signée minimale
    float maxDistance;  // Distance signée maximale
};

class Octree {
public:
    glm::vec3 origin; // Origine du nœud
    glm::vec3 halfDimension; // Moitié de la taille du nœud
    Voxel* voxel; // Pointeur vers un voxel si c'est une feuille
    Octree* children[8]; // Enfants du nœud
    float minDistance; // Distance SDF minimale
    float maxDistance; // Distance SDF maximale

    Octree(const glm::vec3& origin, const glm::vec3& halfDimension)
            : origin(origin), halfDimension(halfDimension), voxel(nullptr), minDistance(FLT_MAX), maxDistance(-FLT_MAX) {
        for (int i = 0; i < 8; ++i) {
            children[i] = nullptr;
        }
    }

    ~Octree() {
        for (int i = 0; i < 8; ++i) {
            delete children[i];
        }
        delete voxel;
    }

    int getOctantContainingPoint(const glm::vec3& point) const {
        int oct = 0;
        if (point.x >= origin.x) oct |= 4;
        if (point.y >= origin.y) oct |= 2;
        if (point.z >= origin.z) oct |= 1;
        return oct;
    }

    float calculateVoxelSDF(const glm::vec3& p, const glm::vec3& voxelCenter, float voxelSize) {
        glm::vec3 voxelMin = voxelCenter - glm::vec3(voxelSize * 0.5f);
        glm::vec3 voxelMax = voxelCenter + glm::vec3(voxelSize * 0.5f);

        glm::vec3 d = glm::max(voxelMin - p, p - voxelMax);

        float dist = glm::length(glm::max(d, glm::vec3(0.0f)));  // Distance extérieure
        float insideDist = glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f); // Distance intérieure

        return dist + insideDist;
    }

    void insert(const Voxel& voxel, float voxelSize) {
        // Si c'est une feuille sans voxel, insérer ici
        if (this->voxel == nullptr && halfDimension.x * 2.0f <= voxelSize) {
            this->voxel = new Voxel(voxel);

            // Calculer les distances SDF minimales et maximales pour ce voxel
            glm::vec3 voxelMin = origin - halfDimension;
            glm::vec3 voxelMax = origin + halfDimension;
            this->minDistance = calculateVoxelSDF(voxelMin, voxel.position, voxelSize);
            this->maxDistance = calculateVoxelSDF(voxelMax, voxel.position, voxelSize);
            return;
        }

        // Sinon, subdiviser et insérer dans le bon enfant
        if (isLeafNode()) {
            for (int i = 0; i < 8; ++i) {
                glm::vec3 newOrigin = origin;
                newOrigin.x += halfDimension.x * (i & 4 ? 0.5f : -0.5f);
                newOrigin.y += halfDimension.y * (i & 2 ? 0.5f : -0.5f);
                newOrigin.z += halfDimension.z * (i & 1 ? 0.5f : -0.5f);
                children[i] = new Octree(newOrigin, halfDimension * 0.5f);
            }
        }

        // Réinsérer le voxel actuel
        if (this->voxel != nullptr) {
            int octant = getOctantContainingPoint(this->voxel->position);
            children[octant]->insert(*(this->voxel), voxelSize);
            delete this->voxel;
            this->voxel = nullptr;
        }

        // Insérer le nouveau voxel
        int octant = getOctantContainingPoint(voxel.position);
        children[octant]->insert(voxel, voxelSize);

        // Mettre à jour les distances SDF minimales et maximales
        this->minDistance = std::min(this->minDistance, children[octant]->minDistance);
        this->maxDistance = std::max(this->maxDistance, children[octant]->maxDistance);
    }

    bool isLeafNode() const {
        return children[0] == nullptr;
    }

    void serialize(std::vector<GPUOctreeNode>& data) const {
        serializeNode(this, data);
    }

    bool serializeNode(const Octree* node, std::vector<GPUOctreeNode>& data) const {
        if (node == nullptr) return false;

        GPUOctreeNode gpuNode;
        gpuNode.center = node->origin;
        gpuNode.size = node->halfDimension.x * 2.0f;
        gpuNode.isLeaf = node->isLeafNode() ? 1 : 0;
        gpuNode.isActive = node->voxel != nullptr ? 1 : 0;

        if (node->voxel) {
            gpuNode.color = node->voxel->color;
        } else {
            gpuNode.color = glm::vec3(0.0f);  // Couleur par défaut si aucun voxel
        }

        // Index actuel du noeud dans le tableau de données
        int startIndex = data.size();
        data.push_back(gpuNode);

        bool hasActiveChildren = false;

        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                gpuNode.children[i] = data.size(); // L'indice de l'enfant dans le tableau
                if (serializeNode(node->children[i], data)) {
                    hasActiveChildren = true;
                }
            } else {
                gpuNode.children[i] = -1; // Pas d'enfant
            }
        }

        if (hasActiveChildren) {
            gpuNode.isActive = 1;
        }

        gpuNode.minDistance = node->minDistance;
        gpuNode.maxDistance = node->maxDistance;

        // Mise à jour du noeud dans le tableau
        data[startIndex] = gpuNode;

        return gpuNode.isActive > 0;
    }
};
