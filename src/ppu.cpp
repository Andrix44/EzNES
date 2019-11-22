#include "ppu.h"


void Ppu::Run() {
	if (memory->rom_path == "") return;  // Do not start counting until the game has launched
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
