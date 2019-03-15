#pragma once

#include <bitset>
#include <stdint.h>
#include <string>


constexpr uint8_t HEADER_SIZE = 16;
constexpr uint8_t MEMORY_SIZE = 2 << 15;

class Memory {
public:
    bool LoadROM(const char* location);
    bool ReadHeader();
    uint8_t Read(uint16_t pc);
private:
    uint8_t* memory = new uint8_t[MEMORY_SIZE];
    FILE* input_ROM;

    uint8_t header[HEADER_SIZE];
    size_t prg_rom_size, chr_rom_size = NULL;
    std::bitset<8> flags_6 = 0b00000000;
    /*             mapper(0-3)-++++||||
                                   ||||
                  four-screen mode-||||
                  512-byte trainer--|||
               non-volatile memory---||
          nametable mirroring type----|*/
    std::bitset<8> flags_7 = 0b00000000;
    /*             mapper(4-7)-++++||||
                                   ||||
                    header version-++||
                                     ||
                        console type-++*/
    uint16_t mapper = NULL;
    uint8_t submapper = NULL;
    size_t prg_ram_size, eeprom_size, chr_ram_size, chr_nvram_size = NULL;
    enum Region {
        ntsc,
        pal,
        multi,
        dendy
    } region;
    uint8_t wip0, wip1, wip2 = NULL;
};
