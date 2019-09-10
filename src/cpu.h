#pragma once

#include <stdint.h>
#include <bitset>
#include <cassert>

#include "memory.h"


constexpr int MASTER_CLOCKSPEED = 21477272;  // NTSC clockspeed
constexpr int CPU_CLOCKSPEED = MASTER_CLOCKSPEED / 12;

enum Flags {
    carry = 0, zero, interrupt,
    decimal, breakpoint, padding,
    overflow, negative
};

enum class Addressing {
    ind = 0,
    ind_X,
    ind_Y,

    // TODO: remove these later
    rel,
    zpg,
    zpg_X,
    zpg_Y
};

class Cpu {
public:
    void ExecuteCycles(const uint32_t cycles);
    void LinkWithMemory(Memory& mem);

    Memory* memory;

    uint8_t A = 0x00, X = 0x00, Y = 0x00;
    uint8_t sp = 0xFD;
    uint16_t pc = 0x0000;
    std::bitset<8> flags = 0b00100100;
    /*              negative-|| |||||
                    overflow--| |||||
                                |||||
                  breakpoint----|||||
               BCD(disabled)-----||||
                   interrupt------||| TODO: Is it 1 on init?
                        zero-------||
                       carry--------| */

private:
    void Interpreter(const uint8_t instr);
    uint16_t GetComplexAddress(enum class Addressing mode, const uint16_t val);
    inline uint16_t GetImmediateAddress();
    void Push(const uint8_t byte);
    uint8_t Pop();
    void ShiftLeftWithFlags(const uint16_t addr);
    void ShiftRightWithFlags(const uint16_t addr);
    void CompareWithMemory(const uint8_t byte, const uint16_t addr);

    uint8_t instr = NULL;
};

