#pragma once

#include <cstdint>
#include <vector>

class Mapper {
public:
    virtual uint16_t TranslateAddress(const uint16_t addr) = 0;
    virtual uint16_t TranslatePpuAddress(const uint16_t addr) = 0;
};
