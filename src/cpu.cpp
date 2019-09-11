#include "cpu.h"


void Cpu::ExecuteCycles(const uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        instr = memory->Read(pc);
        Interpreter(instr);
    }
}

void Cpu::LinkWithMemory(Memory& mem) {
    memory = &mem;
    pc = (memory->Read(0xFFFD) << 8) | memory->Read(0xFFFC);  // TODO: this is just a hack for the reset vector, implement it properly later
}

uint16_t Cpu::GetComplexAddress(enum class Addressing mode, const uint16_t val) {
    switch (mode) {
    case Addressing::ind:
        return (memory->Read((val + 1) % 65536) << 8) | memory->Read(val);
    case Addressing::ind_X:  // TODO: maybe these need the carry bit?
        assert(val < 256);
        return (memory->Read((val + X + 1) % 256) << 8) | memory->Read((val + X) % 256);
    case Addressing::ind_Y:
        assert(val < 256);
        return ((memory->Read((val + 1) % 256) << 8) | memory->Read(val)) + Y;

    // TODO: remove these later, use them only as a template
    case Addressing::zpg_X:
        assert(val < 256);
        return (val + X) % 256;
    case Addressing::zpg_Y:
        assert(val < 256);
        return (val + Y) % 256;
    default:
        assert(0);  // Impossible to reach
        return 1;
    }
}

inline uint16_t Cpu::GetImmediateAddress() {
    return (memory->Read(pc + 2) << 8) | memory->Read(pc + 1);
}

void Cpu::Push(const uint8_t byte) {
    --sp;
    memory->Write(sp + 0x100, byte);
}

uint8_t Cpu::Pop() {
    uint8_t temp = memory->Read(sp + 0x100);
    ++sp;
    return temp;
}

void Cpu::ShiftLeftWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    //printf("Before left shift: Carry = %X, Zero = %X, Negative = %X, Byte = 0x%X\n",
    //        flags[Flags::carry], flags[Flags::zero], flags[Flags::negative], byte);
    flags[Flags::carry] = (byte >> 7);
    byte <<= 1;
    flags[Flags::zero] = (byte == 0);
    flags[Flags::negative] = (byte >> 7);
    memory->Write(addr, byte);
    //printf("After left shift: Carry = %X, Zero = %X, Negative = %X, Byte = 0x%X\n",
    //        flags[Flags::carry], flags[Flags::zero], flags[Flags::negative], byte);
}

void Cpu::ShiftRightWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    //printf("Before right shift: Carry = %X, Zero = %X, Byte = 0x%X\n",
    //       flags[Flags::carry], flags[Flags::zero], byte);
    flags[Flags::carry] = (byte & 0x1);
    byte >>= 1;
    flags[Flags::zero] = (byte == 0);
    memory->Write(addr, byte);
    //printf("After right shift: Carry = %X, Zero = %X, Byte = 0x%X\n",
    //        flags[Flags::carry], flags[Flags::zero], byte);
}

void Cpu::CompareWithMemory(const uint8_t byte, const uint16_t addr) {
    uint8_t val = memory->Read(addr);
    uint8_t result = byte - val;
    flags[Flags::negative] = (result >> 7);
    flags[Flags::zero] = (result == 0);
    flags[Flags::carry] = (byte < val);
}

void Cpu::Interpreter(const uint8_t instr) {  // TODO: for now, let's just hope that the compiler optimizes this into a jumptable
    bool increment_pc = true;
    switch (instr) {
    // case 0x00: break;
    case 0x01: {  // ORA (ind, X) -NZ
        A = A | memory->Read(GetComplexAddress(Addressing::ind_X, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x02: break;
    case 0x03: break;
    case 0x04: break; */
    case 0x05: {  // ORA -NZ
        A |= memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x06: {  // ASL (zpg) -NZC
        ShiftLeftWithFlags(memory->Read(pc + 1));
        pc += 1;
        break;
    }
    // case 0x07: break;
    case 0x08: {  // PHP --
        Push(static_cast<uint8_t>(flags.to_ulong()));
        break;
    }
    case 0x09: {  // ORA -NZ
        A |= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x0a: {  // ASL (acc) -NZC
        flags[Flags::carry] = (A >> 7);
        A <<= 1;
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    /* case 0x0b: break;
    case 0x0c: break; */
    case 0x0d: {  // ORA -NZ
        A |= memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x0e: {  // ASL (abs) -NZC
        ShiftLeftWithFlags(GetImmediateAddress());
        pc += 2;
        break;
    }
    //case 0x0f: break;
    case 0x10: {  // BPL (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::negative]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    case 0x11: {  // ORA -NZ
        A |= memory->Read(GetComplexAddress(Addressing::ind_Y, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x12: break;
    case 0x13: break;
    case 0x14: break;
    case 0x15: break; */
    case 0x16: {  // ASL (zpg, X) -NZC
        ShiftLeftWithFlags((memory->Read(pc + 1) + X) % 256);
        pc += 1;
        break;
    }
    //case 0x17: break;
    case 0x18: {  // CLC -C
        flags[Flags::carry] = false;
        break;
    }
    /* case 0x19: break;
    case 0x1a: break;
    case 0x1b: break;
    case 0x1c: break;
    case 0x1d: break; */
    case 0x1e: {  // ASL (abs, X) -NZC
        ShiftLeftWithFlags(GetImmediateAddress() + X);
        pc += 2;
        break;
    }
    //case 0x1f: break;
    case 0x20: {  // JSR --
        Push((pc + 2) >> 8);
        Push((pc + 2) & 0xFF);  // MAYBE the other way around???
        pc = GetImmediateAddress();
        increment_pc = false;
        break;
    }
    /* case 0x21: break;
    case 0x22: break;
    case 0x23: break; */
    case 0x24: {  // BIT (zpg) -NZV
        uint8_t value = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (value >> 7);
        flags[Flags::zero] = (value & A) == 0;
        flags[Flags::overflow] = (value >> 6) & 1;
        pc += 1;
        break;
    }
    /* case 0x25: break;
    case 0x26: break;
    case 0x27: break; */
    case 0x28: { // PHA -NZCIDV
        flags = Pop();
        break;
    }
    case 0x29: {  // AND (imm) -NZ
        A |= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x2a: break;
    case 0x2b: break; */
    case 0x2c: {  // BIT (abs) -NZV
        uint8_t value = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = value >> 7;
        flags[Flags::zero] = (value & A) == 0;
        flags[Flags::overflow] = (value >> 6) & 1;
        pc += 2;
        break;
    }
    /* case 0x2d: break;
    case 0x2e: break;
    case 0x2f: break; */
    case 0x30: {  // BMI (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::negative]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    /* case 0x31: break;
    case 0x32: break;
    case 0x33: break;
    case 0x34: break;
    case 0x35: break;
    case 0x36: break;
    case 0x37: break; */
    case 0x38: {  // SEC -C
        flags[Flags::carry] = true;
        break;
    }
    /* case 0x39: break;
    case 0x3a: break;
    case 0x3b: break;
    case 0x3c: break;
    case 0x3d: break;
    case 0x3e: break;
    case 0x3f: break;
    case 0x40: break; */
    case 0x41: { //  EOR (ind_X) -NZ
        A ^= memory->Read(GetComplexAddress(Addressing::ind_X, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x42: break;
    case 0x43: break;
    case 0x44: break; */
    case 0x45: { //  EOR (zpg) -NZ
        A ^= memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x46: {  // LSR (zpg) -ZC
        uint16_t addr = memory->Read(pc + 1);
        ShiftRightWithFlags(addr);
        pc += 1;
        break;
    }
    // case 0x47: break;
    case 0x48: { // PHA
        Push(A);
        break;
    }
    case 0x49: { //  EOR (imm) -NZ
        A ^= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x4a: break;
    case 0x4b: break; */
    case 0x4c: {  // JMP (abs) --
        pc = GetImmediateAddress();
        increment_pc = false;
        break;
    }
    case 0x4d: { //  EOR (abs) -NZ
        A ^= memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x4e: break;
    case 0x4f: break; */
    case 0x50: {  // BVC (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::overflow]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    case 0x51: { //  EOR (ind_Y) -NZ
        A ^= memory->Read(GetComplexAddress(Addressing::ind_Y, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x52: break;
    case 0x53: break;
    case 0x54: break; */
    case 0x55: { //  EOR (zpg_X) -NZ
        A ^= memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x56: break;
    case 0x57: break;
    case 0x58: break; */
    case 0x59: { //  EOR (abs_Y) -NZ
        A ^= memory->Read(GetImmediateAddress() + Y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x5a: break;
    case 0x5b: break;
    case 0x5c: break; */
    case 0x5d: { //  EOR (abs_X) -NZ
        A ^= memory->Read(GetImmediateAddress() + X);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x5e: break;
    case 0x5f: break; */
    case 0x60: { //  RTS --
        pc = Pop() | (Pop() << 8);
        break;
    }
    /* case 0x61: break;
    case 0x62: break;
    case 0x63: break;
    case 0x64: break;
    case 0x65: break;
    case 0x66: break;
    case 0x67: break; */
    case 0x68: { // PLA -NZ
        A = Pop();
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    /* case 0x69: break;
    case 0x6a: break;
    case 0x6b: break;
    case 0x6c: break;
    case 0x6d: break;
    case 0x6e: break;
    case 0x6f: break; */
    case 0x70: {  // BVS (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::overflow]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    /* case 0x71: break;
    case 0x72: break;
    case 0x73: break;
    case 0x74: break;
    case 0x75: break;
    case 0x76: break;
    case 0x77: break; */
    case 0x78: {  // SEI -I
        flags[Flags::interrupt] = false;
        break;
    }
    /* case 0x79: break;
    case 0x7a: break;
    case 0x7b: break;
    case 0x7c: break;
    case 0x7d: break;
    case 0x7e: break;
    case 0x7f: break;
    case 0x80: break; */
    case 0x81: { //  STA (ind_X) --
        memory->Write(GetComplexAddress(Addressing::ind_X, memory->Read(pc + 1)), A);
        pc += 1;
        break;
    }
    /* case 0x82: break;
    case 0x83: break; */
    case 0x84: {  // STY (zpg) --
        memory->Write(memory->Read(pc + 1), Y);
        pc += 1;
        break;
    }
    case 0x85: {  // STA (zpg) --
        memory->Write(memory->Read(pc + 1), A);
        pc += 1;
        break;
    }
    case 0x86: {  // STX (zpg) --
        memory->Write(memory->Read(pc + 1), X);
        pc += 1;
        break;
    }
    //case 0x87: break;
    case 0x88: {  // DEY -NZ
        Y -= 1;
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        break;
    }
    /* case 0x89: break; */
    case 0x8a: { // TXA -NZ
        A = X;
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    /* case 0x8b: break;
    case 0x8c: break; */
    case 0x8d: {  // STA (abs) --
        memory->Write(GetImmediateAddress(), A);
        pc += 2;
        break;
    }
    case 0x8e: {  // STX (abs) --
        memory->Write(GetImmediateAddress(), X);
        pc += 2;
        break;
    }
    // case 0x8f: break;
    case 0x90: {  // BCC (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::carry]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    case 0x91: { //  STA (ind_Y) --
        memory->Write(GetComplexAddress(Addressing::ind_Y, memory->Read(pc + 1)), A);
        pc += 1;
        break;
    }
    /* case 0x92: break;
    case 0x93: break;
    case 0x94: break; */
    case 0x95: {  // STA (zpg, X) --
        memory->Write((memory->Read(pc + 1) + X) % 256, A);
        pc += 1;
        break;
    }
    /* case 0x96: break;
    case 0x97: break; */
    case 0x98: { // TYA -NZ
        A = Y;
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    case 0x99: { //  STA (abs_Y) --
        memory->Write(GetImmediateAddress() + Y, A);
        pc += 2;
        break;
    }
    case 0x9a: {  // TXS --
        sp = X;
        break;
    }
    /* case 0x9b: break;
    case 0x9c: break; */
    case 0x9d: { //  STA (abs_X) --
        memory->Write(GetImmediateAddress() + X, A);
        pc += 2;
        break;
    }
    /* case 0x9e: break;
    case 0x9f: break; */
    case 0xa0: {  // LDY (imm) -NZ
        Y = memory->Read(pc + 1);
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        pc += 1;
        break;
    }
    case 0xa1: {  // LDA (ind_X) -NZ
        A = memory->Read(GetComplexAddress(Addressing::ind_X, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0xa2: {  // LDX (imm) -NZ
        X = memory->Read(pc + 1);
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        pc += 1;
        break;
    }
    /* case 0xa3: break;
    case 0xa4: break; */
    case 0xa5: break; {  // LDA (zpg) -NZ
        A = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0xa6: break;
    case 0xa7: break; */
    case 0xa8: { // TAY -NZ
        Y = A;
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        break;
    }
    case 0xa9: {  // LDA (imm) -NZ
        A = memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0xaa: { // TAX -NZ
        X = A;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    /* case 0xab: break;
    case 0xac: break; */
    case 0xad: {  // LDA (abs) -NZ
        A = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0xae: break;
    case 0xaf: break; */
    case 0xb0: {  // BCS (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::carry]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    case 0xb1: {  // LDA (ind_Y) -NZ
        A = memory->Read(GetComplexAddress(Addressing::ind_Y, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0xb2: break;
    case 0xb3: break;
    case 0xb4: break; */
    case 0xb5: break; {  // LDA (zpg_X) -NZ
        A = memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0xb6: break;
    case 0xb7: break; */
    case 0xb8: {  // CLV -V
        flags[Flags::overflow] = false;
        break;
    }
    case 0xb9: {  // LDA (abs_Y) -NZ
        A = memory->Read(GetImmediateAddress() + Y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0xba: break;
    case 0xbb: break;
    case 0xbc: break; */
    case 0xbd: {  // LDA (abs_X) -NZ
        A = memory->Read(GetImmediateAddress() + X);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0xbe: break;
    case 0xbf: break;
    case 0xc0: break;
    case 0xc1: break;
    case 0xc2: break;
    case 0xc3: break;
    case 0xc4: break;
    case 0xc5: break; */
    case 0xc6: { // DEC (zpg) -NZ
        uint8_t temp;
        uint8_t addr;
        addr = memory->Read(pc + 1);
        temp = (memory->Read(addr) - 1);
        flags[Flags::negative] = (temp >> 7);
        flags[Flags::zero] = (temp == 0);
        memory->Write(addr, temp);
        pc += 1;
        break;
    }
    /* case 0xc7: break; */
    case 0xc8: {  // INY -NZ
        Y += 1;
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        break;
    }
    case 0xc9: {  // CMP (imm) -NZC
        CompareWithMemory(A, pc + 1);
        pc += 1;
        break;
    }
    case 0xca: {  // DEX -NZ
        X -= 1;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    /* case 0xcb: break;
    case 0xcc: break;
    case 0xcd: break; */
    case 0xce: { // DEC (abs) -NZ
        uint8_t temp;
        uint16_t addr;
        addr = GetImmediateAddress();
        temp = (memory->Read(addr) - 1);
        flags[Flags::negative] = (temp >> 7);
        flags[Flags::zero] = (temp == 0);
        memory->Write(addr, temp);
        pc += 2;
        break;
    }
    /* case 0xcf: break; */
    case 0xd0: {  // BNE (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::zero]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    /* case 0xd1: break;
    case 0xd2: break;
    case 0xd3: break;
    case 0xd4: break;
    case 0xd5: break; */
    case 0xd6: { // DEC (zpg_X) -NZ
        uint8_t temp;
        uint8_t addr;
        addr = (memory->Read(pc + 1) + X) % 256;
        temp = (memory->Read(addr) - 1);
        flags[Flags::negative] = (temp >> 7);
        flags[Flags::zero] = (temp == 0);
        memory->Write(addr, temp);
        pc += 1;
        break;
    }
    /* case 0xd7: break; */
    case 0xd8: {  // CLD -D
        flags[Flags::decimal] = false;
        break;
    }
    /* case 0xd9: break;
    case 0xda: break;
    case 0xdb: break;
    case 0xdc: break;
    case 0xdd: break; */
    case 0xde: { // DEC (abs_X) -NZ
        uint8_t temp;
        uint16_t addr;
        addr = GetImmediateAddress() + X;
        temp = (memory->Read(addr) - 1);
        flags[Flags::negative] = (temp >> 7);
        flags[Flags::zero] = (temp == 0);
        memory->Write(addr, temp);
        pc += 2;
        break;
    }
    /* case 0xdf: break;
    case 0xe0: break;
    case 0xe1: break;
    case 0xe2: break;
    case 0xe3: break;
    case 0xe4: break;
    case 0xe5: break;
    case 0xe6: break;
    case 0xe7: break; */
    case 0xe8: {  // INX -NZ
        X += 1;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    // case 0xe9: break;
    case 0xea: {  // NOP --
        break;
    }
    /* case 0xeb: break;
    case 0xec: break;
    case 0xed: break;
    case 0xee: break;
    case 0xef: break; */
    case 0xf0: {  // BEQ (rel)--
        increment_pc = false;
        pc += 2;
        if (flags[Flags::zero]) {
            pc += static_cast<int8_t>(memory->Read(pc - 1));
            break;
        }
        break;
    }
    /* case 0xf1: break;
    case 0xf2: break;
    case 0xf3: break;
    case 0xf4: break;
    case 0xf5: break;
    case 0xf6: break;
    case 0xf7: break; */
    case 0xf8: { //  SED -D
        flags[Flags::decimal] = true;
        break;
    }
    /* case 0xf9: break;
    case 0xfa: break;
    case 0xfb: break;
    case 0xfc: break;
    case 0xfd: break;
    case 0xfe: break;
    case 0xff: break; */
    default:
        increment_pc = false;
        char unimpl_instr[5];
        sprintf_s(&unimpl_instr[0], sizeof(unimpl_instr), "0x%X", memory->Read(pc));
        log_helper.AddLog("\nUnimplemented instruction " + std::string(unimpl_instr));
    }

    if (increment_pc) {
        pc += 1;
    }
    // TODO: add cycle counter
}
