#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "cpu.h"
#include "memory.h"
//#include "ppu.h"


int main(int argc, char* argv[]){
    if (!glfwInit()) {
        printf("Error while initialising glfw!\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "EzNES", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Error while initialising OpenGL!\n");
        return 1;
    }
    if (gladLoadGL() == 0) {
        printf("Error while initialising glad!\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Keyboard
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Maybe later
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Memory mem;
    //Ppu ppu;

    if (!argv[1]) {  // TODO: this freezes glfw, maybe move it somewhere else
        printf("Please enter the ROM path as an argument.\n");
        system("pause");
        return 1;
    }
    if (mem.LoadROM(argv[1])) {
        printf("An error occured during the ROM loading, stopping emulation!\n");
        system("pause");
        return 1;
    }

    if (mem.SetupMapper()) {
        printf("An error occured during mapper setup, mapper probably unsupported!\n");
        return 1;
    }

    Cpu cpu(mem);

    bool show_demo_window = false;
    bool show_debug_window = true;

    while (!glfwWindowShouldClose(window)) {
        cpu.ExecuteCycles(1);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Begin("Main window");
        ImGui::Checkbox("Show debug window", &show_debug_window);
        ImGui::Checkbox("Show demo window", &show_demo_window);
        ImGui::End();

        if (show_debug_window) {
            ImGui::Begin("Debug", &show_debug_window);
            ImGui::Text("Registers: \n"
                "A = 0x%X \n"
                "X = 0x%X \n"
                "Y = 0x%X \n"
                "PC = 0x%X \n"
                "SP = 0x%X \n"
                , cpu.A, cpu.X, cpu.Y, cpu.pc, cpu.sp + 0x100);  // TODO: add flags
            ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        ImGui::Render();

        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
