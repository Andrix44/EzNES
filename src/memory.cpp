#include <stdio.h>
//#include <stdlib.h>

#include "memory.h"


bool Memory::LoadROM(const char* location) {
    if (fopen_s(&input_ROM, location, "rb")) {
        printf("Error while opening ROM!\n");
    }
    printf("Loading ROM at %s \n", location);
    // TODO: add more error checking

    if (ReadHeader()) {
        return true;
    }

    fclose(input_ROM);
    return false;
}

bool Memory::ReadHeader() {
    fread(header, 1, 16, input_ROM);

    if (header[0] == (int)"N"[0] && header[1] == (int)"E"[0] && header[2] == (int)"S"[0] && header[3] == 0x1A) {
        printf("Compatible format detected!\n");
    } else {
        printf("Unknown file format!\n");
        return true;
    }

    bool NES_ver_2;
    if ((header[7] >> 2) && 0x3 == 0x2) {
        printf("NES 2.0 format detected!\n");
        NES_ver_2 = true;
    }
    else {
        printf("iNES format detected!\n");
        NES_ver_2 = false;
    }
    if (!NES_ver_2) {  // iNES header
        prg_rom_size = header[4] * 16384;
        chr_rom_size = header[5] * 8192;  // 0 means that CHR RAM is being used
        flags_6 = header[6];
        flags_7 = header[7];
        prg_ram_size = header[8] * 8192;
        switch (header[9] & 0x1) { // Maybe this should be ignored
        case 0:
            region = ntsc;
        case 1:
            region = pal;
        }
        // header[10] is mostly unused
        if ((header[11] + header[12] + header[13] + header[14] + header[15]) != 0) {
            mapper = header[6] >> 4;
            printf("Incorrect header bytes, only using the lower nibble of the mapper byte!\n");
        }
        else {
            mapper = (header[7] & 0xF0) | (header[6] >> 4);
        }
    }
    else {  // NES 2.0 header
        printf("NES 2.0 is currently unsupported, things will break!\n");

        uint8_t prg_nibble = header[9] & 0x0F;
        if (prg_nibble <= 0xE) {
            prg_rom_size = ((prg_nibble << 8) | header[4]) * 16384;
        }
        else {
            prg_rom_size = (2 << (header[4] >> 2)) * (((header[4] & 0x3) * 2) + 1);  // (2 ^ exponent) * ((multiplier * 2) + 1) bytes
        }

        uint8_t chr_nibble = header[9] >> 4;
        if (chr_nibble <= 0xE) {
            chr_rom_size = ((chr_nibble << 8) | header[5]) * 8192;
        }
        else {
            chr_rom_size = (2 << (header[5] >> 2)) * (((header[5] & 0x3) * 2) + 1);  // (2 ^ exponent) * ((multiplier * 2) + 1) bytes
        }

        flags_6 = header[6];
        flags_7 = header[7];
        submapper = header[8] >> 4;
        //mapper = ((header[8] & 0x0F) << 8) | (flags_7 >> 4) | (flags_6 >> 4);
        prg_ram_size = 64 << (header[10] & 0xF);  // TODO: handle it properly when the shift is 0
        eeprom_size = 64 << (header[10] >> 4);  // same
        chr_ram_size = 64 << (header[11] & 0xF);  // same
        chr_nvram_size = 64 << (header[11] >> 4);  // same
        switch (header[12] & 0x3) {
        case 0:
            region = ntsc;
        case 1:
            region = pal;
        case 2:
            region = multi;
        case 3:
            region = dendy;
        }
        wip0 = header[13];
        wip1 = header[14];
        wip2 = header[15];
    }

    // TODO: log loaded info from the header
    return false;
}
