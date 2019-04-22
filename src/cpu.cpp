#include "cpu.h"


Cpu::Cpu(Memory& mem) {
    memory = &mem;
    pc = (memory->Read(0xFFFD) << 8) | memory->Read(0xFFFC);  // TODO: this is just a hack for the reset vector, implement it properly later
}

void Cpu::ExecuteCycles(const uint32_t cycles) {
    for (uint32_t i = 0; i <= cycles; i++) {
        printf("-------------------------------------\n");
        instr = memory->Read(pc);
        Interpreter(instr);
    }
}

uint16_t Cpu::GetComplexAddress(enum class Addressing mode, const uint16_t val) {
    switch (mode) {
    case Addressing::ind:
        assert(val < 256);
        return (memory->Read((val + 1) % 65536) << 8) | memory->Read(val);
    case Addressing::ind_X:
        assert(val < 256);
        return (memory->Read((val + X + 1) % 256) << 8) | memory->Read((val + X) % 256);
    case Addressing::ind_Y:
        assert(val < 256);
        return ((memory->Read((val + 1) % 256) << 8) | memory->Read(val)) + Y;

    // TODO: remove these later, use them only as a template
    case Addressing::rel:
        assert(val < 256);
        return pc + static_cast<int8_t>(val);
    case Addressing::zpg:
        assert(val < 256);
        return val;
    case Addressing::zpg_X:
        assert(val < 256);
        return (val + X) % 256;
    case Addressing::zpg_Y:
        assert(val < 256);
        return (val + Y) % 256;
    default:
        printf("It should be impossible for this text to appear!\n");
        return 0xFFFF;
    }
}

inline uint16_t Cpu::GetImmediateAddress() {
    return (memory->Read(pc + 2) << 8) | memory->Read(pc + 1);
}

void Cpu::Push(const uint8_t byte) {
    memory->Write(sp + 0x100, byte);
    --sp;
}

uint8_t Cpu::Pop() {
    uint8_t temp = memory->Read(sp + 0x100);
    ++sp;
    return temp;
}

void Cpu::ShiftLeftWithFlags(const uint16_t addr) {
    return;  // TODO
}

void Cpu::ShiftRightWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    flags[Flags::carry] = (byte & 0x1);
    byte >>= 1;
    flags[Flags::zero] = (byte == 0);
    memory->Write(addr, byte);
}

void Cpu::CompareWithMemory(const uint8_t byte, const uint16_t addr) {
    uint8_t val = memory->Read(addr);
    uint8_t result = byte - val;
    flags[Flags::negative] = (result >> 7);
    flags[Flags::zero] = (result == 0);
    flags[Flags::carry] = (byte < val);
}

void Cpu::Interpreter(const uint8_t instr) {  // TODO: for now, let's just hope that the compiler optimizes this into a jumptable
    printf("-------------------------------------\n"
           "A = 0x%X  X = 0x%X  Y = 0x%X\n"
           "SP = 0x%X  PC = 0x%X  instr = 0x%X\n", A, X, Y, sp + 0x100, pc, instr);  // TODO: add flags

    switch (instr) {
    case 0x00: break;
    case 0x01: {  // ORA (ind, X)
        A = A | memory->Read(GetComplexAddress(Addressing::ind_X, memory->Read(pc + 1)));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x02: break;
    case 0x03: break;
    case 0x04: break;
    case 0x05: break;
    case 0x06: break;
    case 0x07: break;
    case 0x08: break;
    case 0x09: break;
    case 0x0a: break;
    case 0x0b: break;
    case 0x0c: break;
    case 0x0d: break;
    case 0x0e: break;
    case 0x0f: break;
    case 0x10: {  // BPL
        if (!flags[Flags::negative]) {
            pc += static_cast<signed char>(memory->Read(pc + 1));
            break;
        }
        pc += 2;
        break;
    }
    case 0x11: break;
    case 0x12: break;
    case 0x13: break;
    case 0x14: break;
    case 0x15: break;
    case 0x16: break;
    case 0x17: break;
    case 0x18: break;
    case 0x19: break;
    case 0x1a: break;
    case 0x1b: break;
    case 0x1c: break;
    case 0x1d: break;
    case 0x1e: break;
    case 0x1f: break;
    case 0x20: {  // JSR
        Push((pc + 3) >> 8);  // MAYBE + 2 ????
        Push((pc + 3) & 0xFF);  // MAYBE the other way around???
        pc = GetImmediateAddress();
        break;
    }
    case 0x21: break;
    case 0x22: break;
    case 0x23: break;
    case 0x24: break;
    case 0x25: break;
    case 0x26: break;
    case 0x27: break;
    case 0x28: break;
    case 0x29: break;
    case 0x2a: break;
    case 0x2b: break;
    case 0x2c: break;
    case 0x2d: break;
    case 0x2e: break;
    case 0x2f: break;
    case 0x30: break;
    case 0x31: break;
    case 0x32: break;
    case 0x33: break;
    case 0x34: break;
    case 0x35: break;
    case 0x36: break;
    case 0x37: break;
    case 0x38: break;
    case 0x39: break;
    case 0x3a: break;
    case 0x3b: break;
    case 0x3c: break;
    case 0x3d: break;
    case 0x3e: break;
    case 0x3f: break;
    case 0x40: break;
    case 0x41: break;
    case 0x42: break;
    case 0x43: break;
    case 0x44: break;
    case 0x45: break;
    case 0x46: {  // LSR (zpg)
        uint16_t addr = memory->Read(pc + 1);
        ShiftRightWithFlags(addr);
        pc += 2;
        break;
    }
    case 0x47: break;
    case 0x48: break;
    case 0x49: break;
    case 0x4a: break;
    case 0x4b: break;
    case 0x4c: break;
    case 0x4d: break;
    case 0x4e: break;
    case 0x4f: break;
    case 0x50: break;
    case 0x51: break;
    case 0x52: break;
    case 0x53: break;
    case 0x54: break;
    case 0x55: break;
    case 0x56: break;
    case 0x57: break;
    case 0x58: break;
    case 0x59: break;
    case 0x5a: break;
    case 0x5b: break;
    case 0x5c: break;
    case 0x5d: break;
    case 0x5e: break;
    case 0x5f: break;
    case 0x60: break;
    case 0x61: break;
    case 0x62: break;
    case 0x63: break;
    case 0x64: break;
    case 0x65: break;
    case 0x66: break;
    case 0x67: break;
    case 0x68: break;
    case 0x69: break;
    case 0x6a: break;
    case 0x6b: break;
    case 0x6c: break;
    case 0x6d: break;
    case 0x6e: break;
    case 0x6f: break;
    case 0x70: break;
    case 0x71: break;
    case 0x72: break;
    case 0x73: break;
    case 0x74: break;
    case 0x75: break;
    case 0x76: break;
    case 0x77: break;
    case 0x78: {  // SEI
        flags[Flags::interrupt] = false;
        pc += 1;
        break;
    }
    case 0x79: break;
    case 0x7a: break;
    case 0x7b: break;
    case 0x7c: break;
    case 0x7d: break;
    case 0x7e: break;
    case 0x7f: break;
    case 0x80: break;
    case 0x81: break;
    case 0x82: break;
    case 0x83: break;
    case 0x84: break;
    case 0x85: break;
    case 0x86: break;
    case 0x87: break;
    case 0x88: break;
    case 0x89: break;
    case 0x8a: break;
    case 0x8b: break;
    case 0x8c: break;
    case 0x8d: {  // STA (abs)
        memory->Write(GetImmediateAddress(), A);
        pc += 2;
        break;
    }
    case 0x8e: break;
    case 0x8f: break;
    case 0x90: break;
    case 0x91: break;
    case 0x92: break;
    case 0x93: break;
    case 0x94: break;
    case 0x95: {  // STA (zpg, X)
        memory->Write((memory->Read(pc + 1) + X) % 256, A);
        pc += 2;
        break;
    }
    case 0x96: break;
    case 0x97: break;
    case 0x98: break;
    case 0x99: break;
    case 0x9a: {  // TXS
        sp = X;
        pc += 1;
        break;
    }
    case 0x9b: break;
    case 0x9c: break;
    case 0x9d: break;
    case 0x9e: break;
    case 0x9f: break;
    case 0xa0: break;
    case 0xa1: break;
    case 0xa2: break;
    case 0xa3: break;
    case 0xa4: break;
    case 0xa5: break;
    case 0xa6: break;
    case 0xa7: break;
    case 0xa8: break;
    case 0xa9: {  // LDA (imm)
        A = memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0xaa: break;
    case 0xab: break;
    case 0xac: break;
    case 0xad: {  // LDA (abs)
        A = memory->Read(GetImmediateAddress());
        pc += 3;
        break;
    }
    case 0xae: break;
    case 0xaf: break;
    case 0xb0: break;
    case 0xb1: break;
    case 0xb2: break;
    case 0xb3: break;
    case 0xb4: break;
    case 0xb5: break;
    case 0xb6: break;
    case 0xb7: break;
    case 0xb8: break;
    case 0xb9: break;
    case 0xba: break;
    case 0xbb: break;
    case 0xbc: break;
    case 0xbd: break;
    case 0xbe: break;
    case 0xbf: break;
    case 0xc0: break;
    case 0xc1: break;
    case 0xc2: break;
    case 0xc3: break;
    case 0xc4: break;
    case 0xc5: break;
    case 0xc6: break;
    case 0xc7: break;
    case 0xc8: break;
    case 0xc9: {  // CMP (imm)
        CompareWithMemory(A, pc + 1);
        pc += 2;
        break;
    }
    case 0xca: break;
    case 0xcb: break;
    case 0xcc: break;
    case 0xcd: break;
    case 0xce: break;
    case 0xcf: break;
    case 0xd0: break;
    case 0xd1: break;
    case 0xd2: break;
    case 0xd3: break;
    case 0xd4: break;
    case 0xd5: break;
    case 0xd6: break;
    case 0xd7: break;
    case 0xd8: {  // CLD
        flags[Flags::decimal] = false;
        pc += 1;
        break;
    }
    case 0xd9: break;
    case 0xda: break;
    case 0xdb: break;
    case 0xdc: break;
    case 0xdd: break;
    case 0xde: break;
    case 0xdf: break;
    case 0xe0: break;
    case 0xe1: break;
    case 0xe2: break;
    case 0xe3: break;
    case 0xe4: break;
    case 0xe5: break;
    case 0xe6: break;
    case 0xe7: break;
    case 0xe8: break;
    case 0xe9: break;
    case 0xea: break;
    case 0xeb: break;
    case 0xec: break;
    case 0xed: break;
    case 0xee: break;
    case 0xef: break;
    case 0xf0: break;
    case 0xf1: break;
    case 0xf2: break;
    case 0xf3: break;
    case 0xf4: break;
    case 0xf5: break;
    case 0xf6: break;
    case 0xf7: break;
    case 0xf8: break;
    case 0xf9: break;
    case 0xfa: break;
    case 0xfb: break;
    case 0xfc: break;
    case 0xfd: break;
    case 0xfe: break;
    case 0xff: break;
    }

    // TODO: add cycle counter
}
