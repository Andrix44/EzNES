#pragma once

#include <mappers/mapper.h>


class NROM : public Mapper {
public:
    NROM(bool nrom_256);
    uint16_t TranslateAddress(uint16_t addr) override;
    uint16_t TranslatePpuAddress(uint16_t addr) override;
private:
    bool NROM_256 = false;
};