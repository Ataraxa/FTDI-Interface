#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "external/glfw/deps/glad/gl.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <fstream>

std::vector<int16_t> loadBinaryData(const char* filename) {
    std::vector<int16_t> data;
    std::ifstream file(filename, std::ios::binary);
    
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return data;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    size_t numSamples = fileSize / sizeof(int16_t);
    data.resize(numSamples);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    std::cout << "Loaded " << numSamples << " samples" << std::endl;
    return data;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Binary Data Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // if (!gladLoadGLLoader((GLADloadfunc)glfwGetProcAddress)) {
    //     std::cerr << "Failed to initialize GLAD" << std::endl;
    //     return -1;
    // }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Data variables
    std::vector<int16_t> rawData;
    std::vector<float> plotData;
    char filename[256] = "data.bin";
    bool dataLoaded = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Binary Data Plotter");
        
        // File input
        ImGui::InputText("Filename", filename, sizeof(filename));
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            rawData = loadBinaryData(filename);
            if (!rawData.empty()) {
                plotData.clear();
                plotData.reserve(rawData.size());
                for (int16_t val : rawData) {
                    plotData.push_back(static_cast<float>(val));
                }
                dataLoaded = true;
            }
        }
        
        if (dataLoaded) {
            ImGui::Text("Samples: %zu", plotData.size());
            
            // Use ImGui's built-in PlotLines
            if (!plotData.empty()) {
                ImGui::PlotLines("Data", plotData.data(), plotData.size(), 
                               0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 200));
            }
        } else {
            ImGui::Text("No data loaded - enter filename and click Load");
        }
        
        ImGui::End();

        // Render
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}