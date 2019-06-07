#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "portable-file-dialogs/portable-file-dialogs.h"

#include "cpu.h"
#include "memory.h"
//#include "ppu.h"


bool LoadROM(Memory& mem, Cpu& cpu);

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
    Cpu cpu;

    bool rom_already_opened = false;
    bool show_demo_window = false;
    bool show_debug_window = true;

    ImVec4 red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    std::string flag_names[8] = { "Carry", "Zero", "Interrupt", "BCD", "Breakpoint", "-", "Overflow", "Negative" };

    while (!glfwWindowShouldClose(window)) {
        //cpu.ExecuteCycles(1);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load ROM", "", false)) {
                    if (!rom_already_opened) {
                        if (LoadROM(mem, cpu)) {
                            rom_already_opened = true;
                        }
                    }
                }
                if (ImGui::MenuItem("Exit", "", false)) {
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Emulation")) {
                if (ImGui::MenuItem("Resume", "", false)) {
                    // TODO
                }
                if (ImGui::MenuItem("Pause", "", false)) {
                    // TODO
                }
                if (ImGui::MenuItem("Stop", "", false)) {
                    // TODO
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Show/hide debug window", "CTRL+D", &show_debug_window);
            ImGui::MenuItem("Show/hide demo window", "", &show_demo_window);
            ImGui::EndMainMenuBar();
        }

        if (show_debug_window & rom_already_opened) {
            ImGui::Begin("Debug", &show_debug_window);
            if (ImGui::Button("Step")) {
                cpu.ExecuteCycles(1);
            }
            if (ImGui::Button("Step * 10")) {
                cpu.ExecuteCycles(10);
            }
            if (ImGui::Button("Step * 100")) {
                cpu.ExecuteCycles(100);
            }
            if (ImGui::Button("Step * 1000")) {
                cpu.ExecuteCycles(1000);
            }
            if (ImGui::Button("Step * 10000")) {
                cpu.ExecuteCycles(10000);
            }
            ImGui::Separator();

            ImGui::Text("Registers: \n"
                        "A = 0x%X \n"
                        "X = 0x%X \n"
                        "Y = 0x%X \n"
                        "PC = 0x%X \n"
                        "SP = 0x%X \n",
                        cpu.A, cpu.X, cpu.Y, cpu.pc, cpu.sp + 0x100);  // TODO: add flags
            ImGui::Text("Flags:");
            for (int i = 0; i < 8; ++i) {
                if (cpu.flags[7 - i]) {  // Reverse the order here so that the displayed flags are more readable
                    color = green;
                }
                else {
                    color = red;
                }
                ImGui::SameLine();
                ImGui::TextColored(color, flag_names[7 - i].c_str());  // Same
            }
            ImGui::Separator();

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool LoadROM(Memory& mem, Cpu& cpu) {
    std::string rom = pfd::open_file("Select a file", ".", { "NES ROMS", "*" }).result()[0];
    if (!mem.LoadROM(rom.c_str())) {
        if (!mem.SetupMapper()) {
            cpu.LinkWithMemory(mem);
            return true;
        }
        else {
            pfd::message("Error", "Unsupported mapper!", pfd::choice::ok, pfd::icon::error);
        }
    }
    else {
        pfd::message("Error", "Invalid ROM file!", pfd::choice::ok, pfd::icon::error);
    }
    return false;
}
