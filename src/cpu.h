#pragma once

#include <stdint.h>
#include <bitset>
#include <cassert>

#include "memory.h"


constexpr int MASTER_CLOCKSPEED = 21477272;  // NTSC clockspeed
constexpr int CPU_CLOCKSPEED = MASTER_CLOCKSPEED / 12;

class Cpu {
public:
    void ExecuteCycles(const uint32_t cycles, Memory& mem);

private:
    void Interpreter(const uint8_t instr, Memory& mem);
    void Push(const uint8_t byte, Memory& mem);
    uint8_t Pop(Memory& mem);

    uint8_t A = 0x00, X = 0x00, Y = 0x00;
    uint8_t sp = 0xFF;
    uint16_t pc = 0x8000;  // TODO: make this mapper-dependant !!!!!!!!!!!!!!
    std::bitset<8> flags = 0b00100000;
 /*                     sign-|| |||||
                    overflow--| |||||
                                |||||
                  breakpoint----|||||
               BCD(disabled)-----||||
                   interrupt------||| TODO: Maybe 1 on init?
                        zero-------||
                       carry--------|*/

    uint8_t instr = NULL;
};

