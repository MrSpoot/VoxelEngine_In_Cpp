#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utils/Camera.h"
#include "utils/Shader.h"
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void processInput(GLFWwindow *window);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

float calculateFPS(std::vector<float> &fpsValues, float &averageFPS, float &maxFps);

struct RenderingProperties {
    bool showNormals;
    bool showSteps;
};

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, -6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraLock = true;
bool wireframeMode = false;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

GLuint loadTexture(const char* filePath) {
    // Variables pour stocker la largeur, hauteur et nombre de canaux de l'image
    int width, height, nrChannels;

    // Charger l'image à partir du fichier avec stb_image
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);

    GLuint textureID;
    if (data) {
        // Générer un ID de texture OpenGL
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);  // Lier la texture 2D

        // Paramètres de texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Répéter la texture horizontalement
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // Répéter la texture verticalement
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Filtrage quand la texture est réduite
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// Filtrage quand la texture est agrandie

        // Vérifier si l'image contient un canal alpha (4 canaux)
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Charger les données de texture dans OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);  // Générer les mipmaps pour la texture

        // Libérer l'image après l'avoir chargée en OpenGL
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        textureID = 0;  // Si l'image n'a pas pu être chargée, retourner 0
    }

    return textureID;
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VoxelEngine", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(0);

    // Glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    // Vérifier si la version d'OpenGL est suffisante
    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3)) {
        std::cerr << "OpenGL 4.3 or higher is required for SSBO" << std::endl;
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    std::vector<float> fpsValues;
    float averageFPS = 0.0f;
    float maxFps = 0.0f;

    float vertices[] = {
            // positions
            1.0f,  1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
    };
    unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3,
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);

    RenderingProperties properties;
    properties.showNormals = false;
    properties.showSteps = false;

    int size = 32;

// Générer un tableau de données (par exemple des couleurs RGBA)
    std::vector<float> voxelData(size * size * size * 3); // 4 pour RGBA
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int index = (z * size * size + y * size + x) * 3;
                voxelData[index] = x / (float) size;     // R
                voxelData[index + 1] = y / (float) size; // G
                voxelData[index + 2] = z / (float) size;  // B
            }
        }
    }

    int numVoxels = size * size * size;
    int numBytes = (numVoxels + 7) / 8;

    std::vector<uint8_t> compressedVoxels(numBytes);

    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int index = z * size * size + y * size + x;
                int byteIndex = index / 8;
                int bitIndex = index % 8;

                if(y < 5){
                    compressedVoxels[byteIndex] |= (1 << bitIndex);
                }else{
                    compressedVoxels[byteIndex] &= ~(1 << bitIndex);
                }


                //compressedVoxels[byteIndex] |= (1 << bitIndex); == 1
                //compressedVoxels[byteIndex] &= ~(1 << bitIndex); == 0
            }
        }
    }

    std::vector<int> distances(size * size * size, size);  // Initialiser avec une valeur grande (taille max du cube)

// Boucle pour remplir les distances
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int index = z * size * size + y * size + x;

                // Ici, nous déterminons si le voxel est activé (par exemple, un voxel qui fait partie de la surface)
                if (x%4==0 && y%4==0 && z%4 == 0) {
                    distances[index] = 0;  // La distance pour les voxels activés est 0 (surface)
                } else {
                    // Calcul de la distance minimale au voxel activé
                    // On doit parcourir les voxels activés voisins pour trouver la distance la plus courte
                    int minDistance = size;

                    // Parcourir les voisins proches (par exemple, 3x3x3 autour du voxel)
                    for (int dz = -1; dz <= 1; dz++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            for (int dx = -1; dx <= 1; dx++) {
                                int neighborX = x + dx;
                                int neighborY = y + dy;
                                int neighborZ = z + dz;

                                // Vérifier que les coordonnées sont dans les limites du tableau
                                if (neighborX >= 0 && neighborX < size &&
                                    neighborY >= 0 && neighborY < size &&
                                    neighborZ >= 0 && neighborZ < size) {
                                    int neighborIndex = neighborZ * size * size + neighborY * size + neighborX;

                                    // Si le voisin est activé, calculer la distance euclidienne
                                    if (distances[neighborIndex] == 0) {  // Si c'est un voxel activé (surface)
                                        int distance = abs(dx) + abs(dy) + abs(dz);  // Distance Manhattan
                                        minDistance = std::min(minDistance, distance);
                                    }
                                }
                            }
                        }
                    }

                    // Mettre à jour la distance minimale
                    distances[index] = minDistance;
                }
            }
        }
    }


    GLuint texture3D;
    glGenTextures(1, &texture3D);
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, size, size, size, 0, GL_RGB, GL_FLOAT, voxelData.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint binaryTexture;
    glGenTextures(1, &binaryTexture);
    glBindTexture(GL_TEXTURE_3D, binaryTexture);

// Taille de la texture 3D sera plus petite car on utilise 1 octet pour 8 voxels
    int compressedWidth = (size + 7) / 8;  // On réduit la largeur en fonction de la compression

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, compressedWidth, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, compressedVoxels.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint texture3D2;
    glGenTextures(1, &texture3D2);
    glBindTexture(GL_TEXTURE_3D, texture3D2);

// Créer la texture 3D avec GL_R32I pour stocker des entiers
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32I, size, size, size, 0, GL_RED_INTEGER, GL_INT, distances.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint colorTexture = loadTexture("../resources/dirt.png");



    Shader shader("../resources/shaders/raycast.vert","../resources/shaders/simple_sdf.frag");

    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, binaryTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, texture3D2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    shader.setInt("voxelColorTexture",0);
    shader.setInt("binaryTexture",1);
    shader.setInt("sdfTexture",2);
    shader.setInt("colorTexture",3);
    shader.setFloat("voxelSize",1.0f);
    shader.setVec3("voxelTextureSize",glm::vec3(size));

    shader.setFloat("fov", glm::radians(90.0f));
    shader.setFloat("aspectRatio", (float)SCR_WIDTH/(float)SCR_HEIGHT);

    glm::vec3 lightPos = glm::vec3(32.0,80.0,32.0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float fps = calculateFPS(fpsValues, averageFPS, maxFps);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setVec3("camPos", camera.Position);
        shader.setVec3("camDir", camera.Front);
        shader.setVec3("camRight", camera.Right);
        shader.setVec3("camUp", camera.Up);
        shader.setFloat("time",glfwGetTime());
        shader.setBool("showSteps",properties.showSteps);
        shader.setBool("showNormals",properties.showNormals);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(325, 250));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.3);
        // Begin window with no title bar, no resize, no move, no scrollbar, no collapse, no nav, no background
        ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // Green line
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));

        ImGui::Begin("Performance", nullptr, window_flags);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::PlotLines("FPS Over Time", fpsValues.data(), fpsValues.size(), 0, nullptr, 0.0f, maxFps + maxFps * 0.1,
                         ImVec2(0, 80));

        ImGui::PopStyleColor(2);

        ImGui::Text("Average FPS: %.1f", averageFPS);
        ImGui::Text("Max FPS: %.1f", maxFps);
        ImGui::DragFloat("Camera Speed",&camera.MovementSpeed);
        ImGui::Checkbox("Show normal",&properties.showNormals);
        ImGui::Checkbox("Show steps",&properties.showSteps);
        ImGui::End();

        // Set wireframe mode
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader.ID);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        cameraLock = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        cameraLock = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (!cameraLock) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.setSpeed(camera.MovementSpeed + (yoffset * 0.0025));
    //camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

float calculateFPS(std::vector<float> &fpsValues, float &averageFPS, float &maxFps) {
    static float lastTime = glfwGetTime();
    static int frameCount = 0;

    float currentTime = glfwGetTime();
    frameCount++;
    float deltaTime = currentTime - lastTime;

    if (deltaTime >= 1.0) {
        float fps = frameCount / deltaTime;
        fpsValues.push_back(fps);
        if (fpsValues.size() > 50) {
            fpsValues.erase(fpsValues.begin());
        }

        averageFPS = std::accumulate(fpsValues.begin(), fpsValues.end(), 0.0f) / fpsValues.size();

        maxFps = *std::max_element(fpsValues.begin(), fpsValues.end());

        lastTime = currentTime;
        frameCount = 0;
    }
    if (fpsValues.size() > 0) {
        return fpsValues[fpsValues.size() - 1];
    } else {
        return 0.0f;
    }
}
