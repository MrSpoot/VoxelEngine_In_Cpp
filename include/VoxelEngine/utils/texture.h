#ifndef VOXELENGINE_TEXTURE_H
#define VOXELENGINE_TEXTURE_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "VoxelEngine/utils/stb_image.h"

class texture {
public:
    unsigned int textureID;
    // Constructor
    texture();
    texture(const glm::vec4 &color, int width, int height);
    texture(const std::string &path);
    texture(const std::string &path, bool flip);
    // Destructor
    ~texture();

    void bind();

private:

    void generateTexture();
    void loadImage(const std::string &path, bool flip);
    void generateUnicolorTexture(const glm::vec4 &color, int width, int height);
};

#endif //VOXELENGINE_TEXTURE_H
