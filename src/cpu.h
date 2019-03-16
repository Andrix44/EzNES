#pragma once

#include <stdint.h>
#include <bitset>

#include "memory.h"


class Cpu {
public:
    void ExecuteCycles(const uint32_t cycles, Memory& mem);

private:
    void Interpreter(const uint8_t instr);

    uint8_t A = 0x00, X = 0x00, Y = 0x00;
    uint16_t sp = 0x01FF;
    uint16_t pc = 0x0000;
    std::bitset<8> flags = 0b00100000;
 /*                     sign-|| |||||
                    overflow--| |||||
                                |||||
                  breakpoint----|||||
               BCD(disabled)-----||||
                   interrupt------||| Maybe 1 on init?
                        zero-------||
                       carry--------|*/

    uint8_t instr = NULL;
};

