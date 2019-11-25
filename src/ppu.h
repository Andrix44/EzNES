#pragma once

#include "memory.h"

class Ppu {
public:
    Memory* memory = nullptr;
    int scanline = 0;
    int cycle = 0;
    bool frame_done = false;

    void Run();
};
