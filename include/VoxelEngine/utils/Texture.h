#ifndef VOXELENGINE_TEXTURE_H
#define VOXELENGINE_TEXTURE_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "VoxelEngine/utils/stb_image.h"

class Texture {
public:
    // Constructor
    Texture();
    Texture(const glm::vec4 &color, int width, int height);
    Texture(const std::string &path);
    Texture(const std::string &path, bool flip);
    // Destructor
    ~Texture();

    void bind();

private:
    unsigned int textureID;
    void generateTexture();
    void loadImage(const std::string &path, bool flip);
    void generateUnicolorTexture(const glm::vec4 &color, int width, int height);
};

#endif //VOXELENGINE_TEXTURE_H
