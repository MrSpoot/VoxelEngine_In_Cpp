#include "VoxelEngine/utils/texture.h"
#include <iostream>

// Constructor
texture::texture() {
    texture::generateTexture();
    generateUnicolorTexture(glm::vec4(1.0) ,16,16);
}

texture::texture(const glm::vec4 &color, int width, int height) {
    texture::generateTexture();
    generateUnicolorTexture(color,width,height);
}

texture::texture(const std::string &path) {
    texture::generateTexture();
    loadImage(path,false);
}

texture::texture(const std::string &path, bool flip) {
    texture::generateTexture();
    loadImage(path,flip);
}

// Destructor
texture::~texture() {
    glDeleteTextures(1, &textureID);
}

void texture::generateTexture(){
    glGenTextures(1, &textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bind();
}

void texture::loadImage(const std::string &path, bool flip){
    int width,height,nrChannels;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void texture::bind(){
    glBindTexture(GL_TEXTURE_2D,textureID);
}

// Method to generate a unicolor texture
void texture::generateUnicolorTexture(const glm::vec4 &color, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Create a buffer for the unicolor texture
    unsigned char* data = new unsigned char[width * height * 4];
    for (int i = 0; i < width * height; ++i) {
        data[i * 4] = static_cast<unsigned char>(color.r * 255.0f);
        data[i * 4 + 1] = static_cast<unsigned char>(color.g * 255.0f);
        data[i * 4 + 2] = static_cast<unsigned char>(color.b * 255.0f);
        data[i * 4 + 3] = static_cast<unsigned char>(color.a * 255.0f);
    }

    // Generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clean up
    delete[] data;
}
