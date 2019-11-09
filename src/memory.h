#pragma once

#include <bitset>
#include <stdint.h>
#include <string>
#include <vector>

#include <log.h>

#include <mappers/nrom.h>


constexpr int HEADER_SIZE = 0x10;
constexpr int CPU_MEMORY_SIZE = 0x10000;
constexpr int PPU_MEMORY_SIZE = 0x4000;

class Memory {
public:
    Memory();
    ~Memory();
    bool LoadROM(const char* location);
    bool ReadHeader();
    bool SetupMapper();
    uint8_t Read(const uint16_t addr);
    void Write(const uint16_t addr, const uint8_t byte);
    uint8_t PpuRead(const uint16_t addr);
    void PpuWrite(const uint16_t addr, const uint8_t byte);

    std::string rom_path = "";

private:
    std::vector<uint8_t> cpu_memory = {};
    std::vector<uint8_t> ppu_memory = {};
    std::vector<uint8_t> game_data = {};
    FILE* input_ROM = nullptr;

    Mapper *curr_mapper = nullptr;

    uint8_t header[HEADER_SIZE] = {0 * HEADER_SIZE};
    uint32_t prg_rom_size = NULL, chr_rom_size = NULL;
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
        horizontal = 0,
        vertical
    } mirroring{};
    uint16_t mapper = NULL;
    uint8_t submapper = NULL;
    uint32_t prg_ram_size = NULL, eeprom_size = NULL;
    uint32_t chr_ram_size = NULL, chr_nvram_size = NULL;
    enum ConsoleType {
        nes_famicom = 0,
        vs_system,
        playchoice,
        extended
    } console_type{};
    enum Region {
        ntsc = 0,
        pal,
        multi,
        dendy
    } region{};
    uint8_t wip0 = NULL, wip1 = NULL, wip2 = NULL;
};
