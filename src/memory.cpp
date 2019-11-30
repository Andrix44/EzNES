#include <stdio.h>

#include "memory.h"
#include "ppu.h"


Memory::Memory() {
    cpu_memory.resize(0x10000);
}

Memory::~Memory() {
    delete curr_mapper;
}

bool Memory::LoadROM(const char* location) {
    if (fopen_s(&input_ROM, location, "rb")) {
        log_helper.AddLog("Error while opening ROM!\n");
    }
    log_helper.AddLog("\nLoading ROM at " + static_cast<std::string>(location) + '\n');
    // TODO: add more error checking

    if (ReadHeader()) return true; // Error occured

    uint16_t offset = 0x10;
    if (trainer) offset += 0x200;

    fseek(input_ROM, offset, SEEK_SET);
    fread(&prg_memory[0], 1, prg_rom_size, input_ROM);
    fread(&chr_memory[0], 1, chr_rom_size, input_ROM);

    fclose(input_ROM);
    return false;
}

bool Memory::ReadHeader() {
    fread(&header[0], 1, 0x10, input_ROM);

    if (!(header[0] == (int)"N"[0] && header[1] == (int)"E"[0] && header[2] == (int)"S"[0] && header[3] == 0x1A)) {
        log_helper.AddLog("Unknown file format!\n");
        return true;
    }

    bool NES_ver_2;
    if ((header[7] >> 2) && 0x3 == 0x2) {
        log_helper.AddLog("NES 2.0 format detected!\n");
        NES_ver_2 = true;
    }
    else {
        log_helper.AddLog("iNES format detected!\n");
        NES_ver_2 = false;
    }
    if (!NES_ver_2) {  // iNES header
        prg_rom_size = (header[4]) * 0x4000;
        prg_memory.resize(prg_rom_size);
        if (header[5] == 0) {
            chr_ram_size = 0x2000;
            chr_rom_size = 0;
        } else {
            chr_ram_size = 0;
            chr_rom_size = (header[5]) * 0x2000;
        }
        chr_memory.resize(static_cast<uint64_t>(chr_rom_size) + chr_ram_size);  // One of them is always going to be 0
        mirroring = (header[6] & 0x1) ? Mirroring::vertical : Mirroring::horizontal;
        prg_ram_battery = header[6] & 0x2;
        trainer = header[6] & 0x4;
        flags_6 = header[6];  // This can be separated later

        bool overwritten = (header[11] + header[12] + header[13] + header[14] + header[15]) != 0;
        if (overwritten) {
            log_helper.AddLog("Incorrect header, ignoring bytes 7-15!\n");
            mapper = header[6] >> 4;
        }
        else {
            switch (header[7] & 0x3) { // Can probably be ignored
            case 0:
                console_type = ConsoleType::nes_famicom;
            case 1:
                console_type = ConsoleType::vs_system;
            case 2:
                console_type = ConsoleType::playchoice;
            case 3:
                console_type = ConsoleType::extended;
            }

            prg_ram_size = (header[8]) * 8192;

            switch (header[9] & 0x1) { // Maybe this should be ignored
            case 0:
                region = Region::ntsc;
            case 1:
                region = Region::pal;
            }

            // header[10] is mostly unused
            mapper = (header[7] & 0xF0) | (header[6] >> 4);
        }


    }
    else {  // NES 2.0 header
        log_helper.AddLog("NES 2.0 is currently unsupported!\n");
        return true;
        /*
        uint8_t prg_nibble = header[9] & 0x0F;
        if (prg_nibble <= 0xE) {
            prg_rom_size = ((prg_nibble << 8) | header[4]) * 0x4000;
        }
        else {
            prg_rom_size = (2 << (header[4] >> 2)) * (((header[4] & 0x3) * 2) + 1);  // (2 ^ exponent) * ((multiplier * 2) + 1) bytes
        }

        uint8_t chr_nibble = header[9] >> 4;
        if (chr_nibble <= 0xE) {
            chr_rom_size = ((chr_nibble << 8) | header[5]) * 0x2000;
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

    return false;
}

bool Memory::SetupMapper() {
    if (mapper == 0) { // NROM
        memcpy(&cpu_memory[0x8000], &prg_memory[0], 0x4000);

        bool nrom_256 = false;
        if (prg_rom_size == 0x8000) nrom_256 = true;
        if (nrom_256) memcpy(&cpu_memory[0xC000], &prg_memory[0x4000], 0x4000);

        memcpy(&ppu->ppu_memory[0], &chr_memory[0], 0x2000);
        curr_mapper = new NROM(nrom_256);
        return 0;
    }

    return 1;
}

uint8_t Memory::Read(const uint16_t addr) {
    if (addr <= 0x1FFF) return cpu_ram[addr & 0x7FF];

    else if (addr >= 0x2000 && addr <= 0x3FFF) return ppu->ReadPpuReg(addr & 0x7);

    else if (addr >= 0x4000 && addr <= 0x4017) return 0;  // TODO: APU

    else return cpu_memory[curr_mapper->TranslateAddress(addr)];
}

void Memory::Write(const uint16_t addr, const uint8_t byte) {
    if (addr <= 0x1FFF) cpu_ram[addr & 0x7FF] = byte;

    else if (addr >= 0x2000 && addr <= 0x3FFF) ppu->WritePpuReg(addr & 0x7, byte);

    else if (addr >= 0x4000 && addr <= 0x4017);  // TODO: APU

    else cpu_memory[curr_mapper->TranslateAddress(addr)] = byte;
}
uint8_t Memory::PpuRead(/*const*/ uint16_t addr) {
    //assert(addr <= 0x3FFF);
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) return ppu->ppu_memory[curr_mapper->TranslatePpuAddress(addr)];

    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        if (mirroring == Mirroring::vertical) {
            return ppu->ppu_memory[addr & 0x27FF];
        }
        else if (mirroring == Mirroring::horizontal) {
            return ppu->ppu_memory[addr & 0x2BFF];
        }
        else {
            assert(0);  // Unimplemented
            return 0;
        }
    }

    else {
        uint16_t temp = addr & 0x3F1F;
        if (temp == 0x3F10) temp = 0x3F00;
        else if (temp == 0x3F14) temp = 0x3F04;
        else if (temp == 0x3F18) temp = 0x3F08;
        else if (temp == 0x3F1C) temp = 0x3F0C;

        return ppu->ppu_memory[temp] & (ppu->GetGreyscale() ? 0x30 : 0x3F);
    }
}

void Memory::PpuWrite(const uint16_t addr, const uint8_t byte) {
    assert(addr <= 0x3FFF);

    if (addr <= 0x1FFF) ppu->ppu_memory[curr_mapper->TranslatePpuAddress(addr)] = byte;

    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        if (mirroring == Mirroring::vertical) {
            ppu->ppu_memory[addr & 0x27FF] = byte;
        }
        else if (mirroring == Mirroring::horizontal) {
            ppu->ppu_memory[addr & 0x2BFF] = byte;
        }
    }

    else {
        uint16_t temp = addr & 0x3F1F;
        if (temp == 0x3F10) temp = 0x3F00;
        else if (temp == 0x3F14) temp = 0x3F04;
        else if (temp == 0x3F18) temp = 0x3F08;
        else if (temp == 0x3F1C) temp = 0x3F0C;

        ppu->ppu_memory[temp] = byte;
    }
}
