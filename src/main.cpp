#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include "VoxelEngine/utils/shader.h"
#include "VoxelEngine/utils/stb_image.h"
#include "VoxelEngine/utils/camera.h"
#include "VoxelEngine/utils/texture.h"
#include "VoxelEngine/components/cube.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
float calculateFPS(std::vector<float>& fpsValues, float& averageFPS, float& maxFps);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraLock = true;

bool showSecondWindow = false;
bool wireframeMode = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VoxelEngine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval( 0 );

    // Glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    shader shader("C:\\Users\\Betterfly\\Documents\\GitHub\\VoxelEngine_In_Cpp\\resources\\shaders\\vertex.glsl", "C:\\Users\\Betterfly\\Documents\\GitHub\\VoxelEngine_In_Cpp\\resources\\shaders\\fragment.glsl");

    texture texture(glm::vec4(0.36, 0.76, 0.4, 1.0), 8, 8);
    texture.bind();

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Cube cube(1.0f,texture.textureID);

    float i = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::vector<float> fpsValues;
    float averageFPS = 0.0f;
    float maxFps = 0.0f;

    int simulationWidth = 10;
    int simulationDepth = 10;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float fps = calculateFPS(fpsValues, averageFPS, maxFps);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw our first triangle
        shader.use();

        glm::vec3 lightpos = glm::vec3(0.0,15.0,0.0);

        shader.setVec3("lightPos",lightpos);

        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        shader.setMat4("projection", proj);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        for (unsigned int x = 0; x < simulationWidth; x++)
        {
            for (unsigned int z = 0; z < simulationDepth; z++)
            {

                glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                model = glm::translate(model, glm::vec3(x*2,-3,z*2));
                shader.setMat4("model", model);
                cube.draw();
            }
        }

        glBindVertexArray(0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(325,125));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.3);
        // Begin window with no title bar, no resize, no move, no scrollbar, no collapse, no nav, no background
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav ;

        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // Green line
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));

        ImGui::Begin("Performance", nullptr, window_flags);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::PlotLines("FPS Over Time", fpsValues.data(), fpsValues.size(), 0, nullptr, 0.0f, maxFps + maxFps * 0.1, ImVec2(0, 80));

        ImGui::PopStyleColor(2);

        ImGui::Text("Average FPS: %.1f", averageFPS);
        ImGui::Text("Max FPS: %.1f", maxFps);
        ImGui::End();

        if (showSecondWindow) {
            ImGui::Begin("Options");
            ImGui::Checkbox("Wireframe Mode", &wireframeMode);
            ImGui::InputInt("Size X", &simulationWidth);
            ImGui::InputInt("Size Y", &simulationDepth);
            ImGui::Text("Cube number: %i",simulationDepth * simulationWidth);
            ImGui::Text("Triangle render: %i",simulationDepth * simulationWidth * 12);
            ImGui::End();
        }

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

        i++;
    }

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
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        std::cout << "ALLO\n";
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        cameraLock = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
        cameraLock = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        showSecondWindow = !showSecondWindow;


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if(!cameraLock){
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

float calculateFPS(std::vector<float>& fpsValues, float& averageFPS, float& maxFps) {
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

        maxFps = *std::max_element(fpsValues.begin(),fpsValues.end());

        lastTime = currentTime;
        frameCount = 0;

    }
    if(fpsValues.size() > 0){
        return fpsValues[fpsValues.size()-1];
    }else{
        return 0.0f;
    }


}