#include "nrom.h"


NROM::NROM(uint32_t prg_rom_size, std::vector<uint8_t>& cpu_memory, std::vector<uint8_t>& game_data) {
    if (prg_rom_size == 32768) {
        NROM_256 = true;
    }

    for (int i = 0; i < 16384; ++i) {  // TODO: replace with some kind of copy
        cpu_memory[0x8000 + i] = game_data[16 + i];
    }
    if (NROM_256) {
        for (int i = 0; i < 16384; ++i) {
            cpu_memory[0xC000 + i] = game_data[16 + 16384 + i];
        }
    }
}

uint16_t NROM::TranslateAddress(const uint16_t addr) {
    if (addr >= 0x8000) {
        return NROM_256 ? addr : ((addr & 0x3FFF) + 0x8000);
    }

    else {
        return addr;
    }
}
