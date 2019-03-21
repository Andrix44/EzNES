#pragma once

#include <bitset>
#include <stdint.h>
#include <string>
#include <vector>


constexpr uint8_t HEADER_SIZE = 16;
constexpr uint32_t CPU_MEMORY_SIZE = 0x10000;
constexpr uint16_t PPU_MEMORY_SIZE = 0x4000;

class Memory {
public:
    bool LoadROM(const char* location);
    bool ReadHeader();
    uint8_t Read(const uint16_t pc);
private:
    std::vector<uint8_t> cpu_memory = {};
    std::vector<uint8_t> ppu_memory = {};
    std::vector<uint8_t> game_data = {};
    FILE* input_ROM = nullptr;

    uint8_t header[HEADER_SIZE] = {0 * HEADER_SIZE};
    size_t prg_rom_size = NULL, chr_rom_size = NULL;
    std::bitset<8> flags_6 = 0b00000000;
    /*             mapper(0-3)-++++||||
                                   ||||
                  four-screen mode-||||
                  512-byte trainer--|||
               non-volatile memory---||
          nametable mirroring type----|*/
    bool prg_ram_battery = false;
    bool trainer = false;
    enum Mirroring {
        horizontal,
        vertical
    } mirroring;
    uint16_t mapper = NULL;
    uint8_t submapper = NULL;
    size_t prg_ram_size = NULL, eeprom_size = NULL;
    size_t chr_ram_size = NULL, chr_nvram_size = NULL;
    enum ConsoleType {
        nes_famicom,
        vs_system,
        playchoice,
        extended
    } console_type;
    enum Region {
        ntsc,
        pal,
        multi,
        dendy
    } region;
    uint8_t wip0 = NULL, wip1 = NULL, wip2 = NULL;
};
