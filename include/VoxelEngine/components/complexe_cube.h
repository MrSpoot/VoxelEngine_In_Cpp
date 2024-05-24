#ifndef VOXELENGINE_COMPLEXE_H
#define VOXELENGINE_COMPLEXE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class ComplexeCube {
public:

    float size;
    GLuint textureID;
    GLuint VAO, VBO, EBO;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    ComplexeCube(float size, GLuint textureID);

    void draw();

private:
    void setupMesh();


};

#endif //VOXELENGINE_COMPLEXE_H
