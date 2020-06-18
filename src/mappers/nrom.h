#pragma once

#include <mappers/mapper.h>


class NROM : public Mapper {
public:
    NROM(bool nrom_256);
    uint16_t TranslateAddress(const uint16_t addr) override;
    uint16_t TranslatePpuAddress(const uint16_t addr) override;
private:
    bool NROM_256 = false;
};