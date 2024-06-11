#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utils/Camera.h"
#include "utils/Shader.h"
#include "utils/OctreeNode.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void processInput(GLFWwindow *window);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

float calculateFPS(std::vector<float> &fpsValues, float &averageFPS, float &maxFps);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraLock = true;
bool wireframeMode = false;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;
int p[512];

const int VOXEL_GRID_SIZE = 256;

// Fonction de bruit de Perlin simplifiée
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

void initPerlin() {
    int permutation[] = { 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,
                          69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,
                          219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,
                          68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,
                          133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161,1,216,
                          80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,
                          100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,
                          255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
                          119,248,152, 2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,
                          253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,
                          34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
                          49,192,214,31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,
                          254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

    for (int i = 0; i < 256; ++i) {
        p[256 + i] = p[i] = permutation[i];
    }
}

float perlinNoise(float x, float y, float z) {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
    int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                        lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
                lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                     lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

void initVoxels(std::vector<Voxel>& voxels) {
    float scale = 0.005f;

    for (int x = 0; x < VOXEL_GRID_SIZE; ++x) {
        for (int y = 0; y < VOXEL_GRID_SIZE; ++y) {
            for (int z = 0; z < VOXEL_GRID_SIZE; ++z) {
                    //float noiseValue = perlinNoise(float(x) * scale, float(y) * scale, float(z) * scale);

                    int spacing = 3;

                    int index = x + y * VOXEL_GRID_SIZE + z * VOXEL_GRID_SIZE * VOXEL_GRID_SIZE;
                    voxels[index].color[0] = static_cast<float>(rand()) / RAND_MAX;
                    voxels[index].color[1] = static_cast<float>(rand()) / RAND_MAX;
                    voxels[index].color[2] = static_cast<float>(rand()) / RAND_MAX;

                    //voxels[index].color = glm::vec3(0.95,0.48,0.06);
                    voxels[index].isActive = (rand() % 50) == 0;
                    //voxels[index].isActive = y == 1 || y == 8;
                    //voxels[index].isActive = noiseValue > 0.5;
                    //voxels[index].isActive = false;
                    //voxels[index].isActive = x % spacing == 0 && y % spacing == 0 && z % spacing == 0;
            }
        }
    }
}

GLuint createSSBO(const std::vector<Voxel>& voxels) {
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, voxels.size() * sizeof(Voxel), voxels.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); // Lier le SSBO à l'index 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return ssbo;
}


int main() {
    initPerlin();

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

    std::vector<Voxel> voxels;
    voxels.resize(VOXEL_GRID_SIZE * VOXEL_GRID_SIZE * VOXEL_GRID_SIZE);
    initVoxels(voxels);
    GLuint ssbo = createSSBO(voxels);

    Shader shader("../resources/shaders/raycast.vert","../resources/shaders/raycast.frag");

    shader.use();

    shader.setFloat("fov", glm::radians(90.0f));
    shader.setFloat("aspectRatio", (float)SCR_WIDTH/(float)SCR_HEIGHT);

    glm::vec3 voxelWorldMin = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 voxelWorldMax = glm::vec3(32.0f, 32.0f, 32.0f);
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
        shader.setVec3("lightPos",lightPos);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(325, 175));
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
        //ImGui::InputFloat3("Light Pos",&lightPos[0]);
        ImGui::DragFloat3("Light Pos",&lightPos[0]);
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
    camera.setSpeed(camera.MovementSpeed + (yoffset * 0.25));
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
