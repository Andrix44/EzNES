#include "ppu.h"

Ppu::Ppu() {
    ppu_memory.resize(0x4000);

    palette[0x00] = 0x464646FF; palette[0x01] = 0x00065AFF; palette[0x02] = 0x000678FF; palette[0x03] = 0x020673FF; palette[0x04] = 0x35034CFF;
    palette[0x05] = 0x57000EFF; palette[0x06] = 0x5A0000FF; palette[0x07] = 0x410000FF; palette[0x08] = 0x120200FF; palette[0x09] = 0x001400FF;
    palette[0x0a] = 0x001E00FF; palette[0x0b] = 0x001E00FF; palette[0x0c] = 0x001521FF; palette[0x0d] = 0x000000FF; palette[0x0e] = 0x000000FF;
    palette[0x0f] = 0x000000FF; palette[0x10] = 0x9D9D9DFF; palette[0x11] = 0x004AB9FF; palette[0x12] = 0x0530E1FF; palette[0x13] = 0x5718DAFF;
    palette[0x14] = 0x9F07A7FF; palette[0x15] = 0xCC0255FF; palette[0x16] = 0xCF0B00FF; palette[0x17] = 0xA42300FF; palette[0x18] = 0x5C3F00FF;
    palette[0x19] = 0x0B5800FF; palette[0x1a] = 0x006600FF; palette[0x1b] = 0x006713FF; palette[0x1c] = 0x005E6EFF; palette[0x1d] = 0x000000FF;
    palette[0x1e] = 0x000000FF; palette[0x1f] = 0x000000FF; palette[0x20] = 0xFEFFFFFF; palette[0x21] = 0x1F9EFFFF; palette[0x22] = 0x5376FFFF;
    palette[0x23] = 0x9865FFFF; palette[0x24] = 0xFC67FFFF; palette[0x25] = 0xFF6CB3FF; palette[0x26] = 0xFF7466FF; palette[0x27] = 0xFF8014FF;
    palette[0x28] = 0xC49A00FF; palette[0x29] = 0x71B300FF; palette[0x2a] = 0x28C421FF; palette[0x2b] = 0x00C874FF; palette[0x2c] = 0x00BFD0FF;
    palette[0x2d] = 0x2B2B2BFF; palette[0x2e] = 0x000000FF; palette[0x2f] = 0x000000FF; palette[0x30] = 0xFEFFFFFF; palette[0x31] = 0x9ED5FFFF;
    palette[0x32] = 0xAFC0FFFF; palette[0x33] = 0xD0B8FFFF; palette[0x34] = 0xFEBFFFFF; palette[0x35] = 0xFFC0E0FF; palette[0x36] = 0xFFC3BDFF;
    palette[0x37] = 0xFFCA9CFF; palette[0x38] = 0xE7D58BFF; palette[0x39] = 0xC5DF8EFF; palette[0x3a] = 0xA6E6A3FF; palette[0x3b] = 0x94E8C5FF;
    palette[0x3c] = 0x92E4EBFF; palette[0x3d] = 0xA7A7A7FF; palette[0x3e] = 0x000000FF; palette[0x3f] = 0x000000FF;
}

void Ppu::Run() {
    if (memory->rom_path == "") return;  // Do not start until the game has launched

    if (scanline >= -1 && scanline < 240) {
        if (scanline == 0 && cycle == 0) {
            cycle = 1;
        }
        else if (scanline == -1 && cycle == 1) {
            PPUSTATUS.vblank = 0;
        }

        else if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
            if (PPUMASK.show_backgrnd) {
                bg_shift_pattern_hi <<= 1; bg_shift_pattern_lo <<= 1;
                bg_shift_attrib_hi <<= 1; bg_shift_attrib_lo <<= 1;
            }

            uint16_t temp = 0;
            switch ((cycle - 1) % 8) {
            case 0:
                bg_shift_pattern_hi &= 0xFF00; bg_shift_pattern_hi |= bg_next_tile_msb;
                bg_shift_pattern_lo &= 0xFF00; bg_shift_pattern_lo |= bg_next_tile_lsb;
                bg_shift_attrib_hi &= 0xFF00; bg_shift_attrib_hi |= ((bg_next_tile_attr & 2) ? 0xFF : 0x00);
                bg_shift_attrib_lo &= 0xFF00; bg_shift_attrib_lo |= ((bg_next_tile_attr & 1) ? 0xFF : 0x00);

                bg_next_tile_id = memory->PpuRead(0x2000 | (vram_addr.raw & 0xFFF));
                break;
            case 2:
                temp = (vram_addr.nametable_y << 11) | (vram_addr.nametable_x << 10) | ((vram_addr.coarse_y >> 2) << 3) | (vram_addr.coarse_x >> 2);
                bg_next_tile_attr = memory->PpuRead(0x2000 | 0x3C0 | temp);
                if (vram_addr.coarse_y & 2) bg_next_tile_attr >>= 4;
                if (vram_addr.coarse_x & 2) bg_next_tile_attr >>= 2;
                bg_next_tile_attr &= 3;
                break;
            case 4:
                temp = (PPUCTRL.backgrnd_addr << 12) + (static_cast<uint16_t>(bg_next_tile_id) << 4) + vram_addr.fine_y;
                bg_next_tile_lsb = memory->PpuRead(temp);
                break;
            case 6:
                temp = (PPUCTRL.backgrnd_addr << 12) + (static_cast<uint16_t>(bg_next_tile_id) << 4) + vram_addr.fine_y + 8;
                bg_next_tile_msb = memory->PpuRead(temp);
                break;
            case 7:
                if (PPUMASK.show_backgrnd || PPUMASK.show_sprite) {
                    if (vram_addr.coarse_x == 31) {
                        vram_addr.coarse_x = 0;
                        vram_addr.nametable_x = ~vram_addr.nametable_x;
                    }
                    else ++vram_addr.coarse_x;
                }
                break;
            }
        }
        if (cycle == 256) {
            if (PPUMASK.show_backgrnd || PPUMASK.show_sprite) {
                if (vram_addr.fine_y < 7) ++vram_addr.fine_y;
                else {
                    vram_addr.fine_y = 0;
                    if (vram_addr.coarse_y == 29) {
                        vram_addr.coarse_y = 0;
                        vram_addr.coarse_y = ~vram_addr.coarse_y;
                    }
                    else if (vram_addr.coarse_y == 31) vram_addr.coarse_y = 0;
                    else ++vram_addr.coarse_y;
                }
            }
        }
        else if (cycle == 257) {
            bg_shift_pattern_hi &= 0xFF00; bg_shift_pattern_hi |= bg_next_tile_msb;
            bg_shift_pattern_lo &= 0xFF00; bg_shift_pattern_lo |= bg_next_tile_lsb;
            bg_shift_attrib_hi &= 0xFF00; bg_shift_attrib_hi |= ((bg_next_tile_attr & 2) ? 0xFF : 0x00);
            bg_shift_attrib_lo &= 0xFF00; bg_shift_attrib_lo |= ((bg_next_tile_attr & 1) ? 0xFF : 0x00);

            if (PPUMASK.show_backgrnd || PPUMASK.show_sprite) {
                vram_addr.nametable_x = temp_vram_addr.nametable_x;
                vram_addr.coarse_x = temp_vram_addr.coarse_x;
            }
        }
        else if (cycle == 338 || cycle == 340) {
            bg_next_tile_id = memory->PpuRead(0x2000 | (vram_addr.raw & 0xFFF));
        }
        if (scanline == -1 && cycle >= 280 && cycle < 305) {
            if (PPUMASK.show_backgrnd || PPUMASK.show_sprite) {
                vram_addr.fine_y = temp_vram_addr.fine_y;
                vram_addr.nametable_y = temp_vram_addr.nametable_y;
                vram_addr.coarse_y = temp_vram_addr.coarse_y;
            }
        }
    }
    if (scanline == 241 && cycle == 1) {
        PPUSTATUS.vblank = 1;
        if (PPUCTRL.gen_nmi) nmi = true;

    }

    if (PPUMASK.show_backgrnd) {
        bitmask = 0x8000 >> fine_x;

        pix_hi = (bg_shift_pattern_hi & bitmask) > 0;
        pix_lo = (bg_shift_pattern_lo & bitmask) > 0;
        bg_pixel = (pix_hi << 1) | pix_lo;

        pal_hi = (bg_shift_attrib_hi & bitmask) > 0;
        pal_lo = (bg_shift_attrib_lo & bitmask) > 0;
        bg_palette = (pal_hi << 1) | pal_lo;
    }

    if(scanline >= 0 && scanline <= 239 && cycle > 0 && cycle <= 255) (*image_data)[scanline][static_cast<uint64_t>(cycle) - 1] = GetColorFromPalette(bg_palette, bg_pixel);

    ++cycle;
    if (cycle >= 341) {
        ++scanline;
        cycle = 0;
        if (scanline >= 261) {
            scanline = -1;
            frame_done = true;
        }
    }
}

void Ppu::Reset() {
    scanline = 0; cycle = 0;
    addr_latch = 0; ppu_addr_buff = 0;
    fine_x = 0;
    bg_next_tile_id = 0; bg_next_tile_attr = 0; bg_next_tile_lsb = 0; bg_next_tile_msb = 0;
    bg_shift_pattern_lo = 0; bg_shift_pattern_hi = 0; bg_shift_attrib_lo = 0; bg_shift_attrib_hi = 0;
}

uint32_t Ppu::GetColorFromPalette(uint8_t palette_id, uint8_t pixel) {
    return palette[memory->PpuRead(0x3F00 + (palette_id << 2) + pixel) & 0x3F];
}

void Ppu::SetPatternTables(uint8_t palette_id) {
    for (uint8_t ptable = 0; ptable < 2; ++ptable) {
        for (uint8_t Y = 0; Y < 16; ++Y) {
            for (uint8_t X = 0; X < 16; ++X) {
                uint16_t offset = (256 * Y) + (16 * X);
                for (uint8_t row = 0; row < 8; ++row) {
                    uint8_t lsb = memory->PpuRead(ptable * 0x1000 + offset + row);
                    uint8_t msb = memory->PpuRead(ptable * 0x1000 + offset + row + 8);
                    for (uint8_t col = 0; col < 8; ++col) {
                        uint8_t pixel = ((msb & 1) << 1) | (lsb & 1);
                        lsb >>= 1; msb >>= 1;

                        (*pattern_table_data)[ptable][Y * 8 + row][X * 8 + (7 - col)] = GetColorFromPalette(palette_id, pixel);
                    }
                }
            }
        }
    }
}

void Ppu::SetPaletteImage() {
    for (uint8_t Y = 0; Y < 2; ++Y) {
        for (uint8_t X = 0; X < 16; ++X) {
            (*palette_data)[Y][X] = GetColorFromPalette((X / 4) + (4 * Y), X % 4);
        }
    }
}

void Ppu::SetNametables() {  // TODO: optimize this with mirroring detection
    for (uint8_t ntable = 0; ntable < 4; ++ntable) {
        for (uint8_t Y = 0; Y < 30; ++Y) {
            for (uint8_t X = 0; X < 32; ++X) {
                //uint16_t offset = (960 * Y) + (32 * X);
                uint8_t tile_id = memory->PpuRead(0x2000 + ntable * 0x400 + Y * 32 + X);
                uint8_t palette_byte = memory->PpuRead(0x23C0 + ntable * 0x400 + (Y / 4) * 8 + X / 4);
                uint8_t palette_id = (palette_byte >> 2 * ((((Y % 4) / 2) << 1) + ((X % 4) / 2))) & 0b11;
                for (uint8_t row = 0; row < 8; ++row) {
                    uint8_t lsb = memory->PpuRead(PPUCTRL.backgrnd_addr * 0x1000 + (tile_id << 4) + row);
                    uint8_t msb = memory->PpuRead(PPUCTRL.backgrnd_addr * 0x1000 + (tile_id << 4) + row + 8);
                    for (uint8_t col = 0; col < 8; ++col) {
                        uint8_t pixel = ((msb & 1) << 1) | (lsb & 1);
                        lsb >>= 1; msb >>= 1;

                        (*nametable_data)[ntable][Y * 8 + row][X * 8 + (7 - col)] = GetColorFromPalette(palette_id, pixel);
                    }
                }
            }
        }
    }
}

void Ppu::WritePpuReg(uint8_t id, uint8_t byte) {
    switch (id) {
    case 0:  // PPUCTRL
        PPUCTRL.raw = byte;
        temp_vram_addr.nametable_x = PPUCTRL.nametable_x;
        temp_vram_addr.nametable_y = PPUCTRL.nametable_y;
        break;
    case 1:  // PPUMASK
        PPUMASK.raw = byte;
        break;
    case 2:  // PPUSTATUS
        break;
    case 3:  // OAMADDR
        break;
    case 4:  // OAMDATA
        break;
    case 5:  // PPUSCROLL
        if (addr_latch == 0) {
            fine_x = byte & 7;
            temp_vram_addr.coarse_x = byte >> 3;
            addr_latch = 1;
        }
        else {
            temp_vram_addr.fine_y = byte & 7;
            temp_vram_addr.coarse_y = byte >> 3;
            addr_latch = 0;
        }
        break;
    case 6:  // PPUADDR
        if (addr_latch == 0) {
            temp_vram_addr.raw = (temp_vram_addr.raw & 0x00FF) | ((byte & 0x3F) << 8);
            addr_latch = 1;
        }
        else {
            temp_vram_addr.raw = (temp_vram_addr.raw & 0xFF00) | byte;
            vram_addr.raw = temp_vram_addr.raw;
            addr_latch = 0;
        }
        break;
    case 7:  // PPUDATA
        memory->PpuWrite(vram_addr.raw, byte);
        vram_addr.raw += (PPUCTRL.vram_incr ? 32 : 1);
        break;
    }
}

uint8_t Ppu::ReadPpuReg(uint8_t id) {
    uint8_t data = 0;
    switch (id) {
    case 0:  // PPUCTRL
        break;
    case 1:  // PPUMASK
        break;
    case 2:  // PPUSTATUS
        data = (PPUSTATUS.raw & 0xE0) | (ppu_addr_buff & 0x1F);
        PPUSTATUS.vblank = 0;
        addr_latch = 0;
        break;
    case 3:  // OAMADDR
        break;
    case 4:  // OAMDATA
        break;
    case 5:  // PPUSCROLL
        break;
    case 6:  // PPUADDR
        break;
    case 7:  // PPUDATA
        data = ppu_addr_buff;
        ppu_addr_buff = memory->PpuRead(vram_addr.raw);

        if (vram_addr.raw >= 0x3F00) data = ppu_addr_buff;
        vram_addr.raw += (PPUCTRL.vram_incr ? 32 : 1);
        break;
    }
    return data;
}

bool Ppu::GetGreyscale() {
    return PPUMASK.greyscale;
}
