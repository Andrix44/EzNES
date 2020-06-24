#pragma once

#include <array>
#include "memory.h"

class Ppu {
public:
    Memory* memory = nullptr;

    std::vector<uint8_t> ppu_memory{};

    bool nmi = false;
    bool frame_done = false;
    // TODO: Why can't I use auto here?
    std::array<std::array<uint32_t, 256>, 240>* image_data = new std::array<std::array<uint32_t, 256>, 240>;
    std::array<std::array<std::array<uint32_t, 128>, 128>, 2>* pattern_table_data = new std::array<std::array<std::array<uint32_t, 128>, 128>, 2>;
    std::array<std::array<uint32_t, 16>, 2>* palette_data = new std::array<std::array<uint32_t, 16>, 2>;
    std::array<std::array<std::array<uint32_t, 256>, 240>, 4>* nametable_data = new std::array<std::array<std::array<uint32_t, 256>, 240>, 4>;

    Ppu();
    void Run();
    void Reset();
    void SetPatternTables(uint8_t palette_id);
    void SetPaletteImage();
    void SetNametables();
    void WritePpuReg(uint8_t id, uint8_t byte);
    uint8_t ReadPpuReg(uint8_t id);

    inline bool GetGreyscale();

private:
    std::array<uint32_t, 0x40> palette;

    int16_t scanline{}, cycle{};
    //uint32_t obj_attr_mem[64] = {};
    uint8_t addr_latch{}, ppu_addr_buff{};
    uint8_t fine_x{};
    uint8_t bg_next_tile_id{}, bg_next_tile_attr{}, bg_next_tile_lsb{}, bg_next_tile_msb{};
    uint16_t bg_shift_pattern_lo{}, bg_shift_pattern_hi{}, bg_shift_attrib_lo{}, bg_shift_attrib_hi{};
    uint16_t bitmask{};
    uint8_t bg_pixel{}, bg_palette{}, pix_hi{}, pix_lo{}, pal_hi{}, pal_lo{};

    union {
        struct {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        };
        uint16_t raw{};
    } vram_addr, temp_vram_addr;

    union {
        struct {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t vram_incr : 1;
            uint8_t sprite_table : 1;
            uint8_t backgrnd_addr : 1;
            uint8_t sprite_size : 1;
            uint8_t ppu_slave : 1;
            uint8_t gen_nmi : 1;
        };
        uint8_t raw{};
    } PPUCTRL;

    union {
        struct {
            uint8_t greyscale : 1;
            uint8_t show_left_backgrnd : 1;
            uint8_t show_left_sprite : 1;
            uint8_t show_backgrnd : 1;
            uint8_t show_sprite : 1;
            uint8_t boost_red : 1;
            uint8_t boost_green : 1;
            uint8_t boost_blue : 1;
        };
        uint8_t raw{};
    } PPUMASK;

    union {
        struct {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_0_hit : 1;
            uint8_t vblank : 1;
        };
        uint8_t raw{};
    } PPUSTATUS;

    inline uint32_t GetColorFromPalette(uint8_t palette_id, uint8_t pixel);
};
