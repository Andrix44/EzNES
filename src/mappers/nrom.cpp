#include "nrom.h"


NROM::NROM(uint32_t prg_rom_size, std::vector<uint8_t> &cpu_memory, std::vector<uint8_t> &game_data) {
    if (prg_rom_size == 32768) {
        NROM_256 = true;
    }

    cpu_memory.resize(0x10000); // TODO: Move this away from here
    for (int i = 0; i < 16384; ++i) {
        cpu_memory[0x8000 + i] = game_data[16 + i];
    }
    if (NROM_256) {
        for (int i = 0; i < 16384; ++i) {
            cpu_memory[0xC000 + i] = game_data[16 + 16384 + i];
        }
    }
}

uint8_t NROM::Read(const uint16_t pc, std::vector<uint8_t> &cpu_memory) {
    printf("Read at %X \n", pc);

    if (pc <= 0x1FFF) {  // TODO: Move this out of here
        return cpu_memory[pc & 0x1FFF];
    }

    else if (pc >= 0x2000 && pc <= 0x3FFF) {  // Same
        return cpu_memory[(pc & 0x7) + 0x2000];
    }

    else if (pc >= 0x8000) {
        return cpu_memory[NROM_256 ? pc : ((pc & 0x3FFF) + 0x8000)];
    }

    else {
        return cpu_memory[pc];
    }
}
