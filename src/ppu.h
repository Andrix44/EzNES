#pragma once

#include "memory.h"

class Ppu {
public:
    Memory* memory = nullptr;
    int scanline = 0;
    int cycle = 0;

    void Run();
private:
    bool frame_done = false;
};
