#include <stdio.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "cpu.h"
#include "memory.h"
//#include "ppu.h"


bool SetupWindow(GLFWwindow* window);

int main(int argc, char* argv[]){
    GLFWwindow* window = NULL;
    if (SetupWindow(window)) {
        return 1;
    }

    Cpu cpu;
    //Ppu ppu;
    Memory mem;

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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool SetupWindow(GLFWwindow* window) {
    if (!glfwInit()) {
        printf("Error while initialising glfw!\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(256, 240, "EzNES", NULL, NULL);
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

    return 0;
}
