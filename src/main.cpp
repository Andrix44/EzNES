#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "portable-file-dialogs/portable-file-dialogs.h"

#include "cpu.h"
#include "memory.h"
#include "ppu.h"

#include <log.h>

constexpr int display_width = 256;
constexpr int display_height = 240;

bool LoadROM(Memory& mem, Cpu& cpu, Ppu& ppu);
void RunNES(int count, Cpu& cpu, Ppu& ppu);

int main(int argc, char* argv[]){
    if (!glfwInit()) {
        log_helper.AddLog("Error while initialising glfw!\n");
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
        log_helper.AddLog("Error while initialising OpenGL!\n");
        return 1;
    }
    if (gladLoadGL() == 0) {
        log_helper.AddLog("Error while initialising glad!\n");
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
    Ppu ppu;
    Cpu cpu;

	cpu.memory = &mem;
	ppu.memory = &mem;

    bool show_log_window = true;
    bool show_demo_window = false;
    bool show_debug_window = true;
    bool show_pattern_table = true;

    ImVec4 red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    std::string flag_names[8] = { "Carry", "Zero", "Interrupt", "BCD", "Breakpoint", "-", "Overflow", "Negative" };

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load ROM", "", false)) {
                    LoadROM(mem, cpu, ppu);
                }
                if (ImGui::MenuItem("Exit", "", false)) {
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Emulation")) {
                if (ImGui::MenuItem("Resume - Unimplemented", "", false)) {
                    // TODO
                }
                if (ImGui::MenuItem("Pause - Unimplemented", "", false)) {
                    // TODO
                }
                if (ImGui::MenuItem("Reload", "", false)) {
                    if (mem.rom_path.size() != 0) {
                        mem.LoadROM(mem.rom_path.c_str());
                        cpu.Reset();
                    }
                }
                if (ImGui::MenuItem("Stop - Unimplemented", "", false)) {
                    // TODO
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Show/hide log", "CTRL+L", &show_log_window);
            ImGui::MenuItem("Show/hide debug window", "CTRL+D", &show_debug_window);
            ImGui::MenuItem("Show/hide demo window", "", &show_demo_window);
            ImGui::EndMainMenuBar();
        }

        {
            ImGui::Begin("Game");
            ImGui::Image(nullptr, ImVec2(display_width, display_height));
            ImGui::End();
        }

        if(show_pattern_table) {
            ImGui::Begin("Pattern table");
            ImGui::Image(nullptr, ImVec2(128, 128));
            ImGui::SameLine();
            ImGui::Image(nullptr, ImVec2(128, 128));
            ImGui::End();
        }

        if (show_log_window) {
            ImGui::Begin("Log", &show_log_window);
            ImGui::Checkbox("Autoscroll", &log_helper.scroll_enabled);
            ImGui::SameLine();
            log_helper.Draw(&show_log_window);
            ImGui::End();
        }

        if (show_debug_window) {
            ImGui::Begin("Debug", &show_debug_window);
            if (ImGui::Button("Jump to 0xC000")) {
                cpu.pc = 0xC000;
            }
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
                RunNES(1, cpu, ppu);
            }
            ImGui::SameLine();
            if (ImGui::Button("Step * 10")) {
                RunNES(10, cpu, ppu);
            }
            ImGui::SameLine();
            if (ImGui::Button("Step * 100")) {
                RunNES(100, cpu, ppu);
            }
            ImGui::SameLine();
            if (ImGui::Button("Step * 1000")) {
                RunNES(1000, cpu, ppu);
            }
            ImGui::SameLine();
            if (ImGui::Button("Step * 10000")) {
                RunNES(10000, cpu, ppu);
            }
            ImGui::Separator();

            ImGui::Text("CPU: \n"
                        "A = 0x%X \n"
                        "X = 0x%X \n"
                        "Y = 0x%X \n"
                        "PC = 0x%X \n"
                        "SP = 0x%X \n",
                        cpu.A, cpu.X, cpu.Y, cpu.pc, cpu.sp + 0x100);
            ImGui::Text("Total cycles: %d", cpu.cycles);
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

            ImGui::Text("PPU: \n"
                        "Scanline: %d \n"
                        "Cycle: %d \n",
                        ppu.scanline, ppu.cycle);
            ImGui::Separator();

            ImGui::Text("\n%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
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

bool LoadROM(Memory& mem, Cpu& cpu, Ppu& ppu) {
    std::vector<std::string> file = pfd::open_file("Select a file", ".", { "NES ROMS", "*" }).result();
    if (file.empty()) {
        return false;
    }
    mem.rom_path = file[0];
    if (!mem.LoadROM(mem.rom_path.c_str())) {
        if (!mem.SetupMapper()) {
            cpu.Reset();
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

void RunNES(int count, Cpu& cpu, Ppu& ppu) {
    for (int i = 0; i <= count; ++i) {
        cpu.Run();
        ppu.Run();
        ppu.Run();
        ppu.Run();
    }
}
