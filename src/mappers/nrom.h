#pragma once

#include <cstdint>
#include <vector>


class NROM {
public:
    NROM(uint32_t prg_rom_size, std::vector<uint8_t> &cpu_memory, std::vector<uint8_t> &game_data);
    uint8_t Read(const uint16_t pc, std::vector<uint8_t> &cpu_memory);
private:
    bool NROM_256 = NULL;
};