#pragma once

#include <stdint.h>
#include <bitset>
#include <cassert>

#include "memory.h"


constexpr int MASTER_CLOCKSPEED = 21477272;  // NTSC clockspeed
constexpr int CPU_CLOCKSPEED = MASTER_CLOCKSPEED / 12;

enum Flags {
    carry = 0, zero, interrupt,
    decimal, breakpoint, unused,
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
    void Run();
    void Reset();
    void IRQ();
    void NMI();

    Memory* memory = nullptr;

    uint8_t A = 0x00, X = 0x00, Y = 0x00;
    uint8_t sp = 0x0;
    uint16_t pc = 0x0000;
    std::bitset<8> flags = 0b00000000;
    /*              negative-||||||||
                    overflow--|||||||
                      unused---||||||
                  breakpoint----|||||
               BCD(disabled)-----||||
           interrupt disable------|||
                        zero-------||
                       carry--------| */
    uint64_t cycles = 0;
    uint8_t cycle_lut[256] = {7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
                              6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
                              6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
                              6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
                              0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
                              2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
                              2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
                              2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
                              2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
                              2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
                              2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0};

private:
    void Interpreter(const uint8_t instr);
    inline uint16_t GetImmediateAddress();
    inline void Push(const uint8_t byte);
    inline uint8_t Pop();
    void ShiftLeftWithFlags(const uint16_t addr);
    void ShiftRightWithFlags(const uint16_t addr);
    void RotateLeftWithFlags(const uint16_t addr);
    void RotateRightWithFlags(const uint16_t addr);
    void AddMemToAccWithCarry(const uint16_t addr);
    void SubMemFromAccWithBorrow(const uint16_t addr);
    void CompareWithMemory(const uint8_t byte, const uint16_t addr);

    uint8_t instr = NULL;
};

