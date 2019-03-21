#include <stdio.h>

#include "memory.h"


bool Memory::LoadROM(const char* location) {
    if (fopen_s(&input_ROM, location, "rb")) {
        printf("Error while opening ROM!\n");
    }
    printf("Loading ROM at %s \n", location);
    // TODO: add more error checking

    if (ReadHeader()) {
        return true; // Error occured
    }
    uint32_t romsize = HEADER_SIZE + prg_rom_size + chr_rom_size;
    fread(&game_data[0], 1, romsize, input_ROM);

    fclose(input_ROM);
    return false;
}

bool Memory::ReadHeader() {
    fread(&header[0], 1, 16, input_ROM);

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
        prg_rom_size = static_cast<size_t>(header[4]) * 16384;
        chr_rom_size = static_cast<size_t>(header[5]) * 8192;  // 0 means that CHR RAM is being used
        mirroring = (header[6] & 0x1) ? vertical : horizontal;
        prg_ram_battery = header[6] & 0x2;
        trainer = header[6] & 0x4;
        flags_6 = header[6];  // This can be separated later

        bool overwritten = (header[11] + header[12] + header[13] + header[14] + header[15]) != 0;
        if (overwritten) {
            printf("Incorrect header, ignoring bytes 7-15!\n");
            mapper = header[6] >> 4;
        }
        else {
            switch (header[7] & 0x3) { // Can probably be ignored
            case 0:
                console_type = nes_famicom;
            case 1:
                console_type = vs_system;
            case 2:
                console_type = playchoice;
            case 3:
                console_type = extended;
            }

            prg_ram_size = static_cast<size_t>(header[8]) * 8192;

            switch (header[9] & 0x1) { // Maybe this should be ignored
            case 0:
                region = ntsc;
            case 1:
                region = pal;
            }

            // header[10] is mostly unused
            mapper = (header[7] & 0xF0) | (header[6] >> 4);
        }


    }
    else {  // NES 2.0 header
        printf("NES 2.0 is currently unsupported!\n");
        return true;
        /*
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
        wip2 = header[15]; */
    }

    printf("Header loading successful!\n"
        "PRG ROM size - %zu, CHR ROM size - %zu \n"
        "PRG RAM size - %zu, mapper - %u \n",
        prg_rom_size, chr_rom_size, prg_ram_size, mapper);
    return false;
}
