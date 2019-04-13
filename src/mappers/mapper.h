#pragma once

#include <cstdint>
#include <vector>

class Mapper {
public:
    virtual uint8_t MapperRead(const uint16_t pc, std::vector<uint8_t>& cpu_memory) = 0;
};
