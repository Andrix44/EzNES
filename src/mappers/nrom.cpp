#include "nrom.h"


NROM::NROM(uint32_t prg_rom_size, std::vector<uint8_t>& cpu_memory, std::vector<uint8_t>& game_data) {
    if (prg_rom_size == 0x8000) {
        NROM_256 = true;
    }

    memcpy(&cpu_memory[0x8000], &game_data[0x10], 0x4000);

    if (NROM_256) {
        memcpy(&cpu_memory[0xC000], &game_data[0x10 + 0x4000], 0x4000);
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
