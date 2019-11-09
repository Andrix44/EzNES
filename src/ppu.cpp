#include "ppu.h"


void Ppu::Run() {
    ++cycle;
    if (cycle >= 341) {
        ++scanline;
        cycle = 0;
        if (scanline >= 261) {
            scanline = -1;
            frame_done = true;
        }
    }
}
