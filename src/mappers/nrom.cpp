#include "nrom.h"


NROM::NROM(bool nrom_256) {
    NROM_256 = nrom_256;
}

uint16_t NROM::TranslateAddress(const uint16_t addr) {
    if (addr >= 0x8000) return NROM_256 ? addr : ((addr & 0x3FFF) + 0x8000);

    else return addr;
}

uint16_t NROM::TranslatePpuAddress(const uint16_t addr) {
    return addr;
}
