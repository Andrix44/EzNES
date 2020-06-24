#pragma once

#include <cstdint>
#include <vector>

class Mapper {
public:
    virtual uint16_t TranslateAddress(uint16_t addr) = 0;
    virtual uint16_t TranslatePpuAddress(uint16_t addr) = 0;
};
