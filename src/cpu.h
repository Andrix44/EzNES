#pragma once

#include <stdint.h>
#include <bitset>
#include <cassert>

#include "memory.h"


constexpr int MASTER_CLOCKSPEED = 21477272;  // NTSC clockspeed
constexpr int CPU_CLOCKSPEED = MASTER_CLOCKSPEED / 12;

constexpr uint8_t NEGATIVE_f = 7;
constexpr uint8_t OVERFLOW_f = 6;
constexpr uint8_t BREAKPOINT_f = 4;
constexpr uint8_t DECIMAL_f = 3;
constexpr uint8_t INTERRUPT_f = 2;
constexpr uint8_t ZERO_f = 1;
constexpr uint8_t CARRY_f = 0;

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
 /*                 negative-|| |||||
                    overflow--| |||||
                                |||||
                  breakpoint----|||||
               BCD(disabled)-----||||
                   interrupt------||| TODO: Maybe 1 on init?
                        zero-------||
                       carry--------|*/

    uint8_t instr = NULL;
};

