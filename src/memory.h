#pragma once

#include <bitset>
#include <stdint.h>
#include <string>
#include <vector>

#include "log.h"

#include "mappers/nrom.h"


class Ppu;

class Memory {
public:
    Ppu* ppu = nullptr;

    Memory();
    ~Memory();
    bool LoadROM(const char* location);
    bool ReadHeader();
    bool SetupMapper();
    uint8_t Read(const uint16_t addr);
    void Write(const uint16_t addr, const uint8_t byte);
    uint8_t PpuRead(/*const*/ uint16_t addr);
    void PpuWrite(const uint16_t addr, const uint8_t byte);

    std::string rom_path = "";

private:
    std::vector<uint8_t> cpu_memory = {};
    std::vector<uint8_t> prg_memory = {};
    std::vector<uint8_t> chr_memory = {};
    uint8_t cpu_ram[0x800] = {};

    FILE* input_ROM = nullptr;

    Mapper *curr_mapper = nullptr;

    uint8_t header[0x10] = {};
    uint32_t prg_rom_size = 0, chr_rom_size = 0;
    std::bitset<8> flags_6 = 0b00000000;
    /*             mapper(0-3)-++++||||
                                   ||||
                  four-screen mode-||||
                  512-byte trainer--|||
               non-volatile memory---||
          nametable mirroring type----|*/
    bool prg_ram_battery = false;
    bool trainer = false;
    enum class Mirroring {
        horizontal = 0,
        vertical
    } mirroring{};
    uint16_t mapper = 0;
    uint8_t submapper = 0;
    uint32_t prg_ram_size = 0, eeprom_size = 0;
    uint32_t chr_ram_size = 0, chr_nvram_size = 0;
    enum class ConsoleType {
        nes_famicom = 0,
        vs_system,
        playchoice,
        extended
    } console_type{};
    enum class Region {
        ntsc = 0,
        pal,
        multi,
        dendy
    } region{};
    uint8_t wip0 = 0, wip1 = 0, wip2 = 0;
};
