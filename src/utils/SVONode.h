#include <array>
#include <vector>
#include <memory>
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

class SVONode {
public:
    SVONode(float x, float y, float z, float size)
            : center_x(x), center_y(y), center_z(z), size(size), is_leaf(true), voxel(0, 0, 0, glm::vec3(0), false) {
        for (auto& child : children) {
            child = nullptr; // Initialize each child with nullptr
        }
    }

    bool isLeaf() const {
        return is_leaf;
    }

    void insert(const Voxel& voxel);
    void subdivide();
    void serialize(std::vector<GPUOctreeNode>& nodes, int& currentIndex) const;

private:
    float center_x, center_y, center_z; // Center of this node
    float size; // Size of the node (edge length)
    bool is_leaf;
    bool allChildrenSame;
    std::array<std::unique_ptr<SVONode>, 8> children;
    Voxel voxel; // Voxel contained in this node if it is a leaf

    int getOctant(const Voxel& voxel) const;
    void reinsertVoxels();
    bool checkAndMergeChildren();
};

int SVONode::getOctant(const Voxel& voxel) const {
    int octant = 0;
    if (voxel.x >= center_x) octant |= 1;
    if (voxel.y >= center_y) octant |= 2;
    if (voxel.z >= center_z) octant |= 4;
    return octant;
}

void SVONode::subdivide() {
    float half_size = size / 2.0f;
    float quarter_size = size / 4.0f;
    for (int i = 0; i < 8; ++i) {
        float new_x = center_x + quarter_size * ((i & 1) ? 1.0f : -1.0f);
        float new_y = center_y + quarter_size * ((i & 2) ? 1.0f : -1.0f);
        float new_z = center_z + quarter_size * ((i & 4) ? 1.0f : -1.0f);
        children[i] = std::make_unique<SVONode>(new_x, new_y, new_z, half_size);
    }
    is_leaf = false;
}

void SVONode::insert(const Voxel& voxel) {
    if (is_leaf) {
        if (!this->voxel.isActive) {
            this->voxel = voxel;
            std::cout << "Inserted voxel at (" << voxel.x << ", " << voxel.y << ", " << voxel.z << ") with color (" << voxel.color.r << ", " << voxel.color.g << ", " << voxel.color.b << ")\n";
        } else {
            subdivide();
            int octant = getOctant(this->voxel);
            children[octant]->insert(this->voxel);
            this->voxel = Voxel(0, 0, 0, glm::vec3(0), false); // Reset voxel as it's no longer a leaf
            octant = getOctant(voxel);
            children[octant]->insert(voxel);
            checkAndMergeChildren();
        }
    } else {
        int octant = getOctant(voxel);
        children[octant]->insert(voxel);
        checkAndMergeChildren();
    }
}

bool SVONode::checkAndMergeChildren() {
    if (is_leaf) return false;

    glm::vec3 commonColor = children[0]->voxel.color;
    bool allSame = children[0]->is_leaf;

    for (int i = 1; i < 8; ++i) {
        if (children[i] && (children[i]->voxel.color != commonColor || !children[i]->is_leaf)) {
            allSame = false;
            break;
        }
    }

    if (allSame) {
        voxel.color = commonColor;
        voxel.isActive = true;
        is_leaf = true;
        for (auto& child : children) {
            child = nullptr;
        }
        return true;
    }
    return false;
}

void SVONode::serialize(std::vector<GPUOctreeNode>& nodes, int& currentIndex) const {
    GPUOctreeNode gpuNode;
    gpuNode.center[0] = center_x;
    gpuNode.center[1] = center_y;
    gpuNode.center[2] = center_z;
    gpuNode.size = size;
    gpuNode.isLeaf = is_leaf ? 1 : 0;
    gpuNode.color[0] = voxel.color.r;
    gpuNode.color[1] = voxel.color.g;
    gpuNode.color[2] = voxel.color.b;
    gpuNode.isActive = voxel.isActive ? 1 : 0;

    std::cout << "Serializing node at (" << center_x << ", " << center_y << ", " << center_z << ") with size " << size << ", leaf: " << gpuNode.isLeaf << ", color: (" << voxel.color.r << ", " << voxel.color.g << ", " << voxel.color.b << ")\n";

    if (!is_leaf) {
        for (int i = 0; i < 8; ++i) {
            if (children[i]) {
                int childIndex = nodes.size();
                gpuNode.children[i] = childIndex;
                children[i]->serialize(nodes, childIndex);
            } else {
                gpuNode.children[i] = -1;
            }
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            gpuNode.children[i] = -1;
        }
    }

    nodes.push_back(gpuNode);
}
