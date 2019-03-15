#include <stdio.h>
//#include <stdlib.h>

#include "memory.h"


bool Memory::LoadROM(const char* location) {
    if (fopen_s(&input_ROM, location, "rb")) {
        printf("Error while opening ROM!");
    }
    // TODO: add more error checking

    if (ReadHeader()) {
        return true;
    }

    fclose(input_ROM);
    return false;
}

bool Memory::ReadHeader() {
    fread(header, 1, 16, input_ROM);  // TODO: make sure there are no mistakes in the header loading process

    if (header[0] == (int)"N"[0] && header[1] == (int)"E"[0] && header[2] == (int)"S"[0] && header[3] == 0x1A) {
        printf("iNES format detected!");  // TODO: add iNES 2.0 format
    } else {
        printf("Unknown file format!");
        return true;
    }
    // Load the information from the header
    prg_rom_size = ((header[9] & 0x0F) << 8) | header[4];  // TODO: this is wrong, they have to be multiplied
    chr_rom_size = ((header[9] & 0xF0) << 4) | header[4];
    flags_6 =   header[6];
    flags_7 =   header[7];
    submapper = header[8] >> 4;
    //mapper = ((header[8] & 0x0F) << 8) | (flags_7 >> 4) | (flags_6 >> 4);
    prg_ram_size =   64 << (header[10] & 0xF);  // TODO: handle it properly when the shift is 0
    eeprom_size =    64 << (header[10] >> 4);
    chr_ram_size =   64 << (header[11] & 0xF);
    chr_nvram_size = 64 << (header[11] >> 4);
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

    return false;
}
