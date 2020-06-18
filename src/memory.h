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
    std::bitset<8> controller[2] = {0b00000000, 0b00000000};
    uint8_t controller_shift[2] = {0, 0};

private:
    std::vector<uint8_t> cpu_memory{};
    std::vector<uint8_t> prg_memory{};
    std::vector<uint8_t> chr_memory{};
    uint8_t cpu_ram[0x800]{};

    FILE* input_ROM = nullptr;

    Mapper *curr_mapper = nullptr;

    uint8_t header[0x10]{};
    uint32_t prg_rom_size{}, chr_rom_size{};
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
    uint16_t mapper{};
    uint8_t submapper{};
    uint32_t prg_ram_size{}, eeprom_size{};
    uint32_t chr_ram_size{}, chr_nvram_size{};
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
    uint8_t wip0{}, wip1{}, wip2{};
};
