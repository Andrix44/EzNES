#pragma once

#include <bitset>
#include <stdint.h>

class Cpu {
public:
    Cpu();
    ~Cpu();
private:
    uint8_t A, X, Y = 0x00;
    uint16_t sp = 0x01FF;
    uint16_t pc = 0x0000;
    std::bitset<8> flags = 0b00000000;
 /*                     sign-|| |||||
                     overflow-| |||||
                                |||||
                     breakpoint-|||||
                   BCD(disabled)-||||
                        interrupt-|||
                              zero-||
                              carry-|*/
};

