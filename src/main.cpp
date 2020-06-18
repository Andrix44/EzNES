#include <stdio.h>
#include <ctime>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "portable-file-dialogs/portable-file-dialogs.h"

#include "cpu.h"
#include "memory.h"
#include "ppu.h"

#include "log.h"

constexpr int display_width = 256;
constexpr int display_height = 240;

bool LoadROM(Memory& mem, Cpu& cpu, Ppu& ppu);
void Clock(Cpu& cpu, Ppu& ppu);
void Frame(double elapsed_time, Cpu& cpu, Ppu& ppu, GLuint& framebuffer);
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data);
inline void SetTexParams();

int main(int argc, char* argv[]){
    if (!glfwInit()) {
        log_helper.AddLog("Error while initialising glfw!\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); // DEBUG, DELETE LATER
    GLFWwindow* window = glfwCreateWindow(1280, 720, "EzNES", NULL, NULL);
    if (window == NULL) return 1;
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

    glDebugMessageCallback(DebugCallback, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Keyboard
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Maybe later
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    Memory mem;
    Ppu ppu;
    Cpu cpu;

    cpu.memory = &mem;
    ppu.memory = &mem;
    mem.ppu = &ppu;

    bool multiplayer_enabled = true;

    bool emulation_running = false;
    bool rom_loaded = false;
    bool run_immediately = true;
    double elapsed_time = 0;
    std::clock_t begin = 0, end = 0;

    bool show_log_window = false;
    bool show_demo_window = false;
    bool show_debug_window = false;
    bool show_palette = false;
    bool show_pattern_tables = false;
    bool show_nametables = false;
    uint8_t selected_palette = 0;

    ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 green(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 color(0.0f, 0.0f, 0.0f, 0.0f);
    std::string flag_names[8] = { "Carry", "Zero", "Interrupt", "BCD", "Breakpoint", "-", "Overflow", "Negative" };

    GLuint framebuffer = 0, pattern_table_0 = 0, pattern_table_1 = 0, palette = 0, nametable_0 = 0, nametable_1 = 0, nametable_2 = 0, nametable_3 = 0;
    glGenTextures(1, &framebuffer); glGenTextures(1, &pattern_table_0); glGenTextures(1, &pattern_table_1); glGenTextures(1, &palette);
    glGenTextures(1, &nametable_0); glGenTextures(1, &nametable_1); glGenTextures(1, &nametable_2); glGenTextures(1, &nametable_3);

    glBindTexture(GL_TEXTURE_2D, framebuffer);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, display_width, display_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, pattern_table_0);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, pattern_table_1);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, palette);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, nametable_0);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, nametable_1);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, nametable_2);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, nametable_3);
    SetTexParams();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    while (!glfwWindowShouldClose(window)) {
        elapsed_time = std::fmod(std::difftime(end, begin) / CLOCKS_PER_SEC, 0.016f);
        // Windows is using wall time for clock() and without the fmod() stuff would break during debugging
        // It also assumes that we are running above NES speeds
        begin = std::clock();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (emulation_running) {
            if (show_pattern_tables) {
                ppu.SetPatternTables(selected_palette);
                glBindTexture(GL_TEXTURE_2D, pattern_table_0);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.pattern_table_data->data() + 0);
                glBindTexture(GL_TEXTURE_2D, pattern_table_1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.pattern_table_data->data() + 1);
            }

            if (show_palette) {
                ppu.SetPaletteImage();
                glBindTexture(GL_TEXTURE_2D, palette);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 2, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.palette_data->data());
            }

            if (show_nametables) {
                ppu.SetNametables();
                glBindTexture(GL_TEXTURE_2D, nametable_0);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.nametable_data->data() + 0);
                glBindTexture(GL_TEXTURE_2D, nametable_1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.nametable_data->data() + 1);
                glBindTexture(GL_TEXTURE_2D, nametable_2);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.nametable_data->data() + 2);
                glBindTexture(GL_TEXTURE_2D, nametable_3);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.nametable_data->data() + 3);

            }
            mem.controller[0][0] = ImGui::IsKeyDown(GLFW_KEY_D);  // Right
            mem.controller[0][1] = ImGui::IsKeyDown(GLFW_KEY_A);  // Left
            mem.controller[0][2] = ImGui::IsKeyDown(GLFW_KEY_S);  // Down
            mem.controller[0][3] = ImGui::IsKeyDown(GLFW_KEY_W);  // Up
            mem.controller[0][4] = ImGui::IsKeyDown(GLFW_KEY_O);  // Start
            mem.controller[0][5] = ImGui::IsKeyDown(GLFW_KEY_I);  // Select
            mem.controller[0][6] = ImGui::IsKeyDown(GLFW_KEY_K);  // B
            mem.controller[0][7] = ImGui::IsKeyDown(GLFW_KEY_L);  // A
            if (multiplayer_enabled) {
                mem.controller[1][0] = ImGui::IsKeyDown(GLFW_KEY_RIGHT);  // Right
                mem.controller[1][1] = ImGui::IsKeyDown(GLFW_KEY_LEFT);  // Left
                mem.controller[1][2] = ImGui::IsKeyDown(GLFW_KEY_DOWN);  // Down
                mem.controller[1][3] = ImGui::IsKeyDown(GLFW_KEY_UP);  // Up
                mem.controller[1][4] = ImGui::IsKeyDown(GLFW_KEY_KP_5);  // Start
                mem.controller[1][5] = ImGui::IsKeyDown(GLFW_KEY_KP_4);  // Select
                mem.controller[1][6] = ImGui::IsKeyDown(GLFW_KEY_KP_1);  // B
                mem.controller[1][7] = ImGui::IsKeyDown(GLFW_KEY_KP_2);  // A
            }

            Frame(elapsed_time, cpu, ppu, framebuffer);
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load ROM", "", false)) {
                    rom_loaded = LoadROM(mem, cpu, ppu);
                    if (run_immediately && rom_loaded) emulation_running = true;
                }
                if (ImGui::MenuItem("Exit", "", false)) glfwSetWindowShouldClose(window, 1);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Emulation")) {
                if (ImGui::MenuItem("Resume", "", false)) emulation_running = true;
                if (ImGui::MenuItem("Pause", "", false)) emulation_running = false;
                if (ImGui::MenuItem("Reload", "", false)) {
                    if (rom_loaded) {
                        mem.LoadROM(mem.rom_path.c_str());
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
                ImGui::Checkbox("Show/hide log", &show_log_window);
                ImGui::Checkbox("Show/hide debug window", &show_debug_window);
                ImGui::Checkbox("Show/hide demo window", &show_demo_window);
                ImGui::Checkbox("Show/hide pattern tables", &show_pattern_tables);
                ImGui::Checkbox("Show/hide palette", &show_palette);
                ImGui::Checkbox("Show/hide nametables", &show_nametables);
                ImGui::EndMenu();
                
            }
            ImGui::Checkbox("Enable second controller", &multiplayer_enabled);
            ImGui::EndMainMenuBar();
        }

        {
            ImGui::SetNextWindowSize(ImVec2(display_width + 20, display_height + 40), ImGuiCond_FirstUseEver);
            ImGui::Begin("Game");
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(framebuffer)), ImVec2(display_width * 2, display_height * 2));
            ImGui::End();
        }

        if(show_pattern_tables) {
            ImGui::Begin("Pattern tables");
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(pattern_table_0)), ImVec2(128 * 2, 128 * 2));
            ImGui::SameLine();
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(pattern_table_1)), ImVec2(128 * 2, 128 * 2));
            ImGui::End();
        }

        if (show_palette) {
            ImGui::Begin("Palette");
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(palette)), ImVec2(16 * 32, 2 * 32));
            ImGui::End();
        }

        if (show_nametables) {
            ImGui::Begin("Nametables");
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(nametable_0)), ImVec2(256, 240));
            ImGui::SameLine();
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(nametable_1)), ImVec2(256, 240));
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(nametable_2)), ImVec2(256, 240));
            ImGui::SameLine();
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uint64_t>(nametable_3)), ImVec2(256, 240));
            ImGui::End();
        }

        if (show_log_window) {
            ImGui::Begin("Log", &show_log_window);
            ImGui::Checkbox("Autoscroll", &log_helper.scroll_enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Log instructions", &cpu.log_instr);
            ImGui::SameLine();
            if(ImGui::Button("Clear log")) log_helper.Clear();
            log_helper.Draw(&show_log_window);
            ImGui::End();
        }

        if (show_debug_window) {
            ImGui::Begin("Debug", &show_debug_window);
            if (ImGui::Button("Jump to 0xC000")) cpu.pc = 0xC000;
            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                if (rom_loaded) emulation_running = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) emulation_running = false;
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
                Clock(cpu, ppu); Clock(cpu, ppu); Clock(cpu, ppu);
            }
            ImGui::SameLine();
            if (ImGui::Button("Frame")) {
                if (rom_loaded) Frame(0.017f, cpu, ppu, framebuffer);
            }
            ImGui::SameLine();
            ImGui::Checkbox("Run immediately", &run_immediately);
            ImGui::Text("Palette used: ");
            ImGui::SameLine();
            ImGui::InputScalar("", ImGuiDataType_U8, &selected_palette);
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
                if (cpu.flags[7LL - i]) color = green;  // Reverse the order here so that the displayed flags are more readable
                else color = red;
                ImGui::SameLine();
                ImGui::TextColored(color, flag_names[7 - i].c_str());  // Same
            }
            ImGui::Separator();

            ImGui::Text("\n%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

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

        end = std::clock();
    }

    delete ppu.image_data;
    delete ppu.pattern_table_data;
    delete ppu.palette_data;
    delete ppu.nametable_data;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool LoadROM(Memory& mem, Cpu& cpu, Ppu& ppu) {
    std::vector<std::string> file = pfd::open_file("Select a file", ".", { "NES ROMS", "*" }).result();
    if (file.empty()) return false;
    mem.rom_path = file[0];
    if (!mem.LoadROM(mem.rom_path.c_str())) {
        if (!mem.SetupMapper()) {
            cpu.Power();
            ppu.Reset();
            //cpu.Reset(); TODO: not even sure if this needs to be emulated
            //apu.Reset(); TODO:
            return true;
        }
        else pfd::message error("Error", "Unsupported mapper!", pfd::choice::ok, pfd::icon::error);
    }
    else pfd::message error("Error", "Invalid ROM file!", pfd::choice::ok, pfd::icon::error);
    mem.rom_path = "";
    return false;
}

void Clock(Cpu& cpu, Ppu& ppu) {
    static int8_t clock_count = 0;
    ppu.Run();
    if (clock_count == 2) {  // TODO: Maybe i have to use the returned cpu cycles
        cpu.Run();
        clock_count = -1;
    }
    if (ppu.nmi) {
        ppu.nmi = false;
        cpu.NMI();
    }

    ++clock_count;
}

void Frame(double elapsed_time ,Cpu& cpu, Ppu& ppu, GLuint& framebuffer) {
    static double time_left = 0;
    if (time_left > 0.0f) time_left -= elapsed_time;
    else {
        time_left += (1.0f / 60.0f) - elapsed_time;
        while (!ppu.frame_done) Clock(cpu, ppu);
        glBindTexture(GL_TEXTURE_2D, framebuffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, display_width, display_height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu.image_data->data());
        ppu.frame_done = false;
    }
}

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data) {
    //static std::string deb = "";
    //deb.append(msg);
    //std::cout <<  "debug call: " << msg << std::endl;
    log_helper.AddLog(msg);
}

inline void SetTexParams() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
