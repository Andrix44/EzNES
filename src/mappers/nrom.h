#pragma once

#include <mappers/mapper.h>


class NROM : public Mapper {
public:
    NROM(uint32_t prg_rom_size, std::vector<uint8_t>& cpu_memory, std::vector<uint8_t>& game_data);
    uint8_t MapperRead(const uint16_t pc, std::vector<uint8_t>& cpu_memory);
private:
    bool NROM_256 = false;
};