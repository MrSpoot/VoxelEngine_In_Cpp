#ifndef VOXELENGINE_CUBE_H
#define VOXELENGINE_CUBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Cube {
public:

    float size;
    GLuint textureID;
    GLuint VAO, VBO, EBO;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Cube(float size, GLuint textureID);

    void draw();

private:
    void setupMesh();


};

#endif //VOXELENGINE_CUBE_H
