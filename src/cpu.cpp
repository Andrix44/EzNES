#include "cpu.h"


void Cpu::Run() {
    instr = memory->Read(pc);
    Interpreter(instr);
    // flags[Flags::unused] = true;
}

void Cpu::Power() {
    pc = (memory->Read(0xFFFD) << 8) | memory->Read(0xFFFC);
    A = X = Y = 0;
    sp = 0xFD;
    flags = 0b00110100;
    cycles = 0;
    cycles += 7;
}

void Cpu::Reset() {
    pc = (memory->Read(0xFFFD) << 8) | memory->Read(0xFFFC);
    sp -= 3;
    flags |= 4;

    // TODO: PPU reset stuff

    memory->Write(0x4015, 0);  // Silence APU
    // reset APU triangle phase to 0
    // APU DPCM output & with 1
    // APU frame counter reset

    cycles = 0;
    cycles += 7;
}

void Cpu::IRQ() {
    if (!flags[Flags::interrupt]) {
        // log_helper.AddLog("IRQ\n");
        memory->Write(sp + 0x100, pc >> 8);
        --sp;
        memory->Write(sp + 0x100, pc & 0xFF);
        --sp;

        flags[Flags::unused] = 1;
        flags[Flags::breakpoint] = 0;
        flags[Flags::interrupt] = 1;
        memory->Write(sp + 0x100, static_cast<uint8_t>(flags.to_ulong()));
        --sp;

        pc = (memory->Read(0xFFFF) << 8) | memory->Read(0xFFFE);
        cycles += 7;
    }
}

void Cpu::NMI() {
    // log_helper.AddLog("NMI\n");
    memory->Write(sp + 0x100, pc >> 8);
    --sp;
    memory->Write(sp + 0x100, pc & 0xFF);
    --sp;

    flags[Flags::unused] = 1;
    flags[Flags::breakpoint] = 0;
    flags[Flags::interrupt] = 1;
    memory->Write(sp + 0x100, static_cast<uint8_t>(flags.to_ulong()));
    --sp;

    pc = (memory->Read(0xFFFB) << 8) | memory->Read(0xFFFA);
    cycles += 8;
}

inline uint16_t Cpu::GetImmediateAddress() {
    return (memory->Read(pc + 2) << 8) | memory->Read(pc + 1);
}

inline void Cpu::Push(const uint8_t byte) {
    memory->Write(sp + 0x100, byte);
    --sp;
}

inline uint8_t Cpu::Pop() {
    ++sp;
    return memory->Read(sp + 0x100);
}

void Cpu::ShiftLeftWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    flags[Flags::carry] = (byte >> 7);
    byte <<= 1;
    byte = static_cast<uint8_t>(byte);
    flags[Flags::zero] = (byte == 0);
    flags[Flags::negative] = (byte >> 7);
    memory->Write(addr, byte);
    pc += 1;
}

void Cpu::ShiftRightWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    flags[Flags::carry] = (byte & 0x1);
    byte >>= 1;
    flags[Flags::zero] = (byte == 0);
    flags[Flags::negative] = false;
    memory->Write(addr, byte);
    pc += 1;
}

void Cpu::RotateLeftWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    bool old_carry = flags[Flags::carry];
    flags[Flags::carry] = (byte >> 7);
    byte <<= 1;
    byte |= static_cast<uint8_t>(old_carry);
    byte = static_cast<uint8_t>(byte);
    flags[Flags::zero] = (byte == 0);
    flags[Flags::negative] = (byte >> 7);
    memory->Write(addr, byte);
    pc += 1;
}

void Cpu::RotateRightWithFlags(const uint16_t addr) {
    uint8_t byte = memory->Read(addr);
    bool old_carry = flags[Flags::carry];
    flags[Flags::negative] = old_carry;
    flags[Flags::carry] = (byte & 0x1);
    byte >>= 1;
    byte |= (old_carry << 7);
    flags[Flags::zero] = (byte == 0);
    memory->Write(addr, byte);
    pc += 1;
}

void Cpu::AddMemToAccWithCarry(const uint16_t addr) {
    uint8_t val = memory->Read(addr);
    uint16_t result = A + val + flags[Flags::carry];
    flags[Flags::overflow] = (((A ^ result) & (val ^ result)) & 0x80);
    A = static_cast<uint8_t>(result);
    flags[Flags::negative] = (A >> 7);
    flags[Flags::zero] = (A == 0);
    flags[Flags::carry] = (result >> 8);
    pc += 1;
}

void Cpu::SubMemFromAccWithBorrow(const uint16_t addr) {
    uint8_t val = ~(memory->Read(addr));
    uint16_t result = A + val + flags[Flags::carry];
    flags[Flags::overflow] = (((A ^ result) & (val ^ result)) & 0x80);
    A = static_cast<uint8_t>(result);
    flags[Flags::negative] = (A >> 7);
    flags[Flags::zero] = (A == 0);
    flags[Flags::carry] = (result >> 8);
    pc += 1;
}

void Cpu::CompareWithMemory(const uint8_t byte, const uint16_t addr) {
    uint8_t val = memory->Read(addr);
    flags[Flags::negative] = ((byte - val) >> 7);
    flags[Flags::zero] = (byte == val);
    flags[Flags::carry] = (byte >= val);
    pc += 1;
}

void Cpu::Interpreter(const uint8_t instr) {
    bool increment_pc = true;
    if (log_instr) {  // TODO: C++20 will have an easy way to simplify this
        char instr_executed[5];
        sprintf_s(&instr_executed[0], sizeof(instr_executed), "0x%X", memory->Read(pc));
        char pc_char[7];
        sprintf_s(&pc_char[0], sizeof(pc_char), "0x%X", pc);
        log_helper.AddLog("\nExecuting instruction " + std::string(instr_executed) + " at address " + std::string(pc_char));
    }

    switch (instr) {
    case 0x00: {  // BRK -I
        // log_helper.AddLog("BRK\n");
        pc += 1;
        memory->Write(sp + 0x100, pc >> 8);
        --sp;
        memory->Write(sp + 0x100, pc & 0xFF);
        --sp;

        flags[Flags::interrupt] = 1;
        flags[Flags::breakpoint] = 1;
        memory->Write(sp + 0x100, static_cast<uint8_t>(flags.to_ulong()));
        --sp;
        flags[Flags::breakpoint] = 0;

        pc = (memory->Read(0xFFFF) << 8) | memory->Read(0xFFFE);
        increment_pc = false;
        break;
    }
    case 0x01: {  // ORA (ind_X) -NZ
        uint16_t addr = memory->Read(pc + 1);
        A = A | memory->Read((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x02: break;
    case 0x03: break;
    case 0x04: break; */
    case 0x05: {  // ORA (zpg) -NZ
        A |= memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x06: {  // ASL (zpg) -NZC
        ShiftLeftWithFlags(memory->Read(pc + 1));
        break;
    }
    // case 0x07: break;
    case 0x08: {  // PHP --
        flags[Flags::breakpoint] = true;
        Push(static_cast<uint8_t>(flags.to_ulong()));
        flags[Flags::breakpoint] = false;
        break;
    }
    case 0x09: {  // ORA (imm) -NZ
        A |= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x0a: {  // ASL (acc) -NZC
        flags[Flags::carry] = (A >> 7);
        A <<= 1;
        A = static_cast<uint8_t>(A);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    /* case 0x0b: break;
    case 0x0c: break; */
    case 0x0d: {  // ORA (abs) -NZ
        A |= memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x0e: {  // ASL (abs) -NZC
        ShiftLeftWithFlags(GetImmediateAddress());
        pc += 1;
        break;
    }
    //case 0x0f: break;
    case 0x10: {  // BPL (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::negative]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0x11: {  // ORA (ind_Y) -NZ
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        A |= memory->Read(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x12: break;
    case 0x13: break;
    case 0x14: break; */
    case 0x15: {  // ORA (zpg_X) -NZ
        A |= memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x16: {  // ASL (zpg_X) -NZC
        ShiftLeftWithFlags((memory->Read(pc + 1) + X) % 256);
        break;
    }
    //case 0x17: break;
    case 0x18: {  // CLC -C
        flags[Flags::carry] = false;
        break;
    }
    case 0x19: {  // ORA (abs_Y) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        A |= memory->Read(abs_y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x1a: break;
    case 0x1b: break;
    case 0x1c: break; */
    case 0x1d: {  // ORA (abs_X) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        A |= memory->Read(abs_x);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x1e: {  // ASL (abs_X) -NZC
        ShiftLeftWithFlags(GetImmediateAddress() + X);
        pc += 1;
        break;
    }
    //case 0x1f: break;
    case 0x20: {  // JSR --
        Push((pc + 2) >> 8);
        Push((pc + 2) & 0xFF);
        pc = GetImmediateAddress();
        increment_pc = false;
        break;
    }
    case 0x21: {  // AND (ind_X) -NZ
        uint16_t addr = memory->Read(pc + 1);
        A &= memory->Read((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x22: break;
    case 0x23: break; */
    case 0x24: {  // BIT (zpg) -NZV
        uint8_t value = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (value >> 7);
        flags[Flags::zero] = (value & A) == 0;
        flags[Flags::overflow] = (value >> 6) & 1;
        pc += 1;
        break;
    }
    case 0x25: {  // AND (zpg) -NZ
        A &= memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x26: {  // ROL (zpg) -NZC
        RotateLeftWithFlags(memory->Read(pc + 1));
        break;
    }
    /* case 0x27: break; */
    case 0x28: {  // PLP -NZCIDV
        flags = Pop();
        flags[Flags::breakpoint] = false;
        break;
    }
    case 0x29: {  // AND (imm) -NZ
        A &= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x2a: {  // ROL (acc) -NZC
        bool old_carry = flags[Flags::carry];
        flags[Flags::carry] = (A >> 7);
        A <<= 1;
        A |= static_cast<uint8_t>(old_carry);
        A = static_cast<uint8_t>(A);
        flags[Flags::zero] = (A == 0);
        flags[Flags::negative] = (A >> 7);
        break;
    }
    /* case 0x2b: break; */
    case 0x2c: {  // BIT (abs) -NZV
        uint8_t value = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = value >> 7;
        flags[Flags::zero] = (value & A) == 0;
        flags[Flags::overflow] = (value >> 6) & 1;
        pc += 2;
        break;
    }
    case 0x2d: {  // AND (abs) -NZ
        A &= memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x2e: {  // ROL (abs) -NZC
        RotateLeftWithFlags(GetImmediateAddress());
        pc += 1;
        break;
    }
    /* case 0x2f: break; */
    case 0x30: {  // BMI (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::negative]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0x31: {  // AND (ind_Y) -NZ
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        A &= memory->Read(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x32: break;
    case 0x33: break;
    case 0x34: break; */
    case 0x35: {  // AND (zpg_X) -NZ
        A &= memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x36: {  // ROL (zpg_X) -NZC
        RotateLeftWithFlags((memory->Read(pc + 1) + X) % 256);
        break;
    }
    /* case 0x37: break; */
    case 0x38: {  // SEC -C
        flags[Flags::carry] = true;
        break;
    }
    case 0x39: {  // AND (abs_Y) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        A &= memory->Read(abs_y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x3a: break;
    case 0x3b: break;
    case 0x3c: break; */
    case 0x3d: {  // AND (abs_X) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        A &= memory->Read(abs_x);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x3e: {  // ROL (abs_X) -NZC
        RotateLeftWithFlags(GetImmediateAddress() + X);
        pc += 1;
        break;
    }
    /* case 0x3f: break; */
    case 0x40: {  // RTI -NZCIDV
        flags = Pop();
        flags[Flags::breakpoint] = false;
        pc = Pop() | (Pop() << 8);
        increment_pc = false;
        break;
    }
    case 0x41: {  // EOR (ind_X) -NZ
        uint16_t addr = memory->Read(pc + 1);
        A ^= memory->Read((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x42: break;
    case 0x43: break;
    case 0x44: break; */
    case 0x45: {  // EOR (zpg) -NZ
        A ^= memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x46: {  // LSR (zpg) -ZC
        ShiftRightWithFlags(memory->Read(pc + 1));
        break;
    }
    // case 0x47: break;
    case 0x48: { // PHA
        Push(A);
        break;
    }
    case 0x49: {  // EOR (imm) -NZ
        A ^= memory->Read(pc + 1);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x4a: {  // LSR (acc) -ZC
        flags[Flags::carry] = (A & 0x1);
        A >>= 1;
        flags[Flags::zero] = (A == 0);
        flags[Flags::negative] = false;
        break;
    }
    /* case 0x4b: break; */
    case 0x4c: {  // JMP (abs) --
        pc = GetImmediateAddress();
        increment_pc = false;
        break;
    }
    case 0x4d: {  // EOR (abs) -NZ
        A ^= memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x4e: {  // LSR (abs) -ZC
        ShiftRightWithFlags(GetImmediateAddress());
        pc += 1;
        break;
    }
    /* case 0x4f: break; */
    case 0x50: {  // BVC (rel) --
        increment_pc = false;
        pc += 2;
        if (!flags[Flags::overflow]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0x51: {  // EOR (ind_Y) -NZ
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        A ^= memory->Read(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0x52: break;
    case 0x53: break;
    case 0x54: break; */
    case 0x55: {  // EOR (zpg_X) -NZ
        A ^= memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0x56: {  // LSR (zpg_X) -ZC
        ShiftRightWithFlags((memory->Read(pc + 1) + X) % 256);
        break;
    }
    /* case 0x57: break;
    case 0x58: break; */
    case 0x59: {  // EOR (abs_Y) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        A ^= memory->Read(abs_y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    /* case 0x5a: break;
    case 0x5b: break;
    case 0x5c: break; */
    case 0x5d: {  // EOR (abs_X) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        A ^= memory->Read(abs_x);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0x5e: {  // LSR (abs_X) -ZC
        ShiftRightWithFlags(GetImmediateAddress() + X);
        pc += 1;
        break;
    }
    /* case 0x5f: break; */
    case 0x60: {  // RTS --
        pc = Pop() | (Pop() << 8);
        break;
    }
    case 0x61: {  // ADC (ind_X) -NZCV
        uint16_t addr = memory->Read(pc + 1);
        AddMemToAccWithCarry((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        break;
    }
    /* case 0x62: break;
    case 0x63: break;
    case 0x64: break; */
    case 0x65: {  // ADC (zpg) -NZCV
        AddMemToAccWithCarry(memory->Read(pc + 1));
        break;
    }
    case 0x66: {  // ROR (zpg) -NZC
        RotateRightWithFlags(memory->Read(pc + 1));
        break;
    }
    /* case 0x67: break; */
    case 0x68: {  // PLA -NZ
        A = Pop();
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    case 0x69: {  // ADC (imm) -NZCV
        AddMemToAccWithCarry(pc + 1);
        break;
    }
    case 0x6a: {  // ROR (acc) -NZC
        bool old_carry = flags[Flags::carry];
        flags[Flags::negative] = old_carry;
        flags[Flags::carry] = (A & 0x1);
        A >>= 1;
        A |= (old_carry << 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    /* case 0x6b: break; */
    case 0x6c: {  // JMP (ind) --
        uint16_t addr = GetImmediateAddress();
        uint8_t low = (addr & 0xFF);
        low += 1;
        low = static_cast<uint8_t>(low);
        uint8_t high = (addr >> 8);
        uint16_t new_addr = (high << 8) | low;
        pc = (memory->Read(new_addr) << 8) | memory->Read(addr);
        increment_pc = false;
        break;
    }
    case 0x6d: {  // ADC (abs) -NZCV
        AddMemToAccWithCarry(GetImmediateAddress());
        pc += 1;
        break;
    }
    case 0x6e: {  // ROR (abs) -NZC
        RotateRightWithFlags(GetImmediateAddress());
        pc += 1;
        break;
    }
    /* case 0x6f: break; */
    case 0x70: {  // BVS (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::overflow]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0x71: {  // ADC (ind_Y) -NZCV
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        AddMemToAccWithCarry(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        break;
    }
    /* case 0x72: break;
    case 0x73: break;
    case 0x74: break; */
    case 0x75: {  // ADC (zpg_X) -NZCV
        AddMemToAccWithCarry((memory->Read(pc + 1) + X) % 256);
        break;
    }
    case 0x76: {  // ROR (zpg_X) -NZC
        RotateRightWithFlags((memory->Read(pc + 1) + X) % 256);
        break;
    }
    /* case 0x77: break; */
    case 0x78: {  // SEI -I
        flags[Flags::interrupt] = true;
        break;
    }
    case 0x79: {  // ADC (abs_Y) -NZCV
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        AddMemToAccWithCarry(abs_y);
        pc += 1;
        break;
    }
    /* case 0x7a: break;
    case 0x7b: break;
    case 0x7c: break; */
    case 0x7d: {  // ADC (abs_X) -NZCV
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        AddMemToAccWithCarry(abs_x);
        pc += 1;
        break;
    }
    case 0x7e: {  // ROR (abs_X) -NZC
        RotateRightWithFlags(GetImmediateAddress() + X);
        pc += 1;
        break;
    }
    /* case 0x7f: break;
    case 0x80: break; */
    case 0x81: {  // STA (ind_X) --
        uint16_t addr = memory->Read(pc + 1);
        memory->Write((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256), A);
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
    /* case 0x8b: break; */
    case 0x8c: {  // STY (abs) --
        memory->Write(GetImmediateAddress(), Y);
        pc += 2;
        break;
    }
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
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0x91: {  // STA (ind_Y) --
        uint16_t addr = memory->Read(pc + 1);
        memory->Write(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y, A);
        pc += 1;
        break;
    }
    /* case 0x92: break;
    case 0x93: break; */
    case 0x94: {  // STY (zpg_X) --
        memory->Write((memory->Read(pc + 1) + X) % 256, Y);
        pc += 1;
        break;
    }
    case 0x95: {  // STA (zpg_X) --
        memory->Write((memory->Read(pc + 1) + X) % 256, A);
        pc += 1;
        break;
    }
    case 0x96: {  // STX (zpg_Y) --
        memory->Write((memory->Read(pc + 1) + Y) % 256, X);
        pc += 1;
        break;
    }
    /* case 0x97: break; */
    case 0x98: {  // TYA -NZ
        A = Y;
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        break;
    }
    case 0x99: {  // STA (abs_Y) --
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
    case 0x9d: {  // STA (abs_X) --
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
        uint16_t addr = memory->Read(pc + 1);
        A = memory->Read((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
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
    /* case 0xa3: break; */
    case 0xa4: {  // LDY (zpg) -NZ
        Y = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        pc += 1;
        break;
    }
    case 0xa5: {  // LDA (zpg) -NZ
        A = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0xa6: {  // LDX (zpg) -NZ
        X = memory->Read(memory->Read(pc + 1));
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        pc += 1;
        break;
    }
    /* case 0xa7: break; */
    case 0xa8: {  // TAY -NZ
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
    case 0xaa: {  // TAX -NZ
        X = A;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    /* case 0xab: break; */
    case 0xac: {  // LDY (abs) -NZ
        Y = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        pc += 2;
        break;
    }
    case 0xad: {  // LDA (abs) -NZ
        A = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0xae: {  // LDX (abs) -NZ
        X = memory->Read(GetImmediateAddress());
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        pc += 2;
        break;
    }
    /* case 0xaf: break; */
    case 0xb0: {  // BCS (rel) --
        increment_pc = false;
        pc += 2;
        if (flags[Flags::carry]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0xb1: {  // LDA (ind_Y) -NZ
        uint8_t zpg_addr = memory->Read(pc + 1);
        uint16_t addr = ((memory->Read((zpg_addr + 1) % 256) << 8) | memory->Read(zpg_addr)) + Y;
        cycles += ((addr & 0xFF00) != ((addr - Y) & 0xFF00));
        A = memory->Read(addr);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    /* case 0xb2: break;
    case 0xb3: break; */
    case 0xb4: {  // LDY (zpg_X) -NZ
        Y = memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        pc += 1;
        break;
    }
    case 0xb5: {  // LDA (zpg_X) -NZ
        A = memory->Read((memory->Read(pc + 1) + X) % 256);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 1;
        break;
    }
    case 0xb6: {  // LDX (zpg_Y) -NZ
        X = memory->Read((memory->Read(pc + 1) + Y) % 256);
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        pc += 1;
        break;
    }
    /* case 0xb7: break; */
    case 0xb8: {  // CLV -V
        flags[Flags::overflow] = false;
        break;
    }
    case 0xb9: {  // LDA (abs_Y) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        A = memory->Read(abs_y);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0xba: {  // TSX -NZ
        X = sp;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    /* case 0xbb: break; */
    case 0xbc: {  // LDY (abs_X) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        Y = memory->Read(abs_x);
        flags[Flags::negative] = (Y >> 7);
        flags[Flags::zero] = (Y == 0);
        pc += 2;
        break;
    }
    case 0xbd: {  // LDA (abs_X) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        A = memory->Read(abs_x);
        flags[Flags::negative] = (A >> 7);
        flags[Flags::zero] = (A == 0);
        pc += 2;
        break;
    }
    case 0xbe: {  // LDX (abs_Y) -NZ
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        X = memory->Read(abs_y);
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        pc += 2;
        break;
    }
    /* case 0xbf: break; */
    case 0xc0: {  // CPY (imm) -NZC
        CompareWithMemory(Y, pc + 1);
        break;
    }
    case 0xc1: {  // CMP (ind_X) -NZC
        uint16_t addr = memory->Read(pc + 1);
        CompareWithMemory(A, (memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        break;
    }
    /* case 0xc2: break;
    case 0xc3: break; */
    case 0xc4: {  // CPY (zpg) -NZC
        CompareWithMemory(Y, memory->Read(pc + 1));
        break;
    }
    case 0xc5: {  // CMP (zpg) -NZC
        CompareWithMemory(A, memory->Read(pc + 1));
        break;
    }
    case 0xc6: {  // DEC (zpg) -NZ
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
        break;
    }
    case 0xca: {  // DEX -NZ
        X -= 1;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    /* case 0xcb: break; */
    case 0xcc: {  // CPY (abs) -NZC
        CompareWithMemory(Y, GetImmediateAddress());
        pc += 1;
        break;
    }
    case 0xcd: {  // CMP (abs) -NZC
        CompareWithMemory(A, GetImmediateAddress());
        pc += 1;
        break;
    }
    case 0xce: {  // DEC (abs) -NZ
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
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0xd1: {  // CMP (ind_Y) -NZC
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        CompareWithMemory(A, ((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        break;
    }
    /* case 0xd2: break;
    case 0xd3: break;
    case 0xd4: break; */
    case 0xd5: {  // CMP (zpg_X) -NZC
        CompareWithMemory(A, (memory->Read(pc + 1) + X) % 256);
        break;
    }
    case 0xd6: {  // DEC (zpg_X) -NZ
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
    case 0xd9: {  // CMP (abs_Y) -NZC
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        CompareWithMemory(A, abs_y);
        pc += 1;
        break;
    }
    /* case 0xda: break;
    case 0xdb: break;
    case 0xdc: break; */
    case 0xdd: {  // CMP (abs_X) -NZC
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        CompareWithMemory(A, abs_x);
        pc += 1;
        break;
    }
    case 0xde: {  // DEC (abs_X) -NZ
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
    /* case 0xdf: break; */
    case 0xe0: {  // CPX (imm) -NZC
        CompareWithMemory(X, pc + 1);
        break;
    }
    case 0xe1: {  // SBC (ind_X) -NZCV
        uint16_t addr = memory->Read(pc + 1);
        SubMemFromAccWithBorrow((memory->Read((addr + X + 1) % 256) << 8) | memory->Read((addr + X) % 256));
        break;
    }
    /* case 0xe2: break;
    case 0xe3: break; */
    case 0xe4: {  // CPX (zpg) -NZC
        CompareWithMemory(X, memory->Read(pc + 1));
        break;
    }
    case 0xe5: {  // SBC (zpg) -NZCV
        SubMemFromAccWithBorrow(memory->Read(pc + 1));
        break;
    }
    case 0xe6: {  // INC (zpg) -NZ
        uint16_t addr = memory->Read(pc + 1);
        uint8_t byte = memory->Read(addr);
        byte += 1;
        flags[Flags::negative] = (byte >> 7);
        flags[Flags::zero] = (byte == 0);
        memory->Write(addr, byte);
        pc += 1;
        break;
    }
    /* case 0xe7: break; */
    case 0xe8: {  // INX -NZ
        X += 1;
        flags[Flags::negative] = (X >> 7);
        flags[Flags::zero] = (X == 0);
        break;
    }
    case 0xe9: {  // SBC (imm) -NZCV
        SubMemFromAccWithBorrow(pc + 1);
        break;
    }
    case 0xea: {  // NOP --
        break;
    }
    /* case 0xeb: break; */
    case 0xec: {  // CPX (abs) -NZC
        CompareWithMemory(X, GetImmediateAddress());
        pc += 1;
        break;
    }
    case 0xed: {  // SBC (abs) -NZCV
        SubMemFromAccWithBorrow(GetImmediateAddress());
        pc += 1;
        break;
    }
    case 0xee: {  // INC (abs) -NZ
        uint16_t addr = GetImmediateAddress();
        uint8_t byte = memory->Read(addr);
        byte += 1;
        flags[Flags::negative] = (byte >> 7);
        flags[Flags::zero] = (byte == 0);
        memory->Write(addr, byte);
        pc += 2;
        break;
    }
    /* case 0xef: break; */
    case 0xf0: {  // BEQ (rel)--
        increment_pc = false;
        pc += 2;
        if (flags[Flags::zero]) {
            ++cycles;
            uint16_t abs = pc + static_cast<int8_t>(memory->Read(pc - 1));
            cycles += ((abs & 0xFF00) != (pc & 0xFF00));
            pc = abs;
            break;
        }
        break;
    }
    case 0xf1: {  // SBC (ind_Y) -NZCV
        uint16_t addr = memory->Read(pc + 1);
        cycles += ((addr + 1) > 0xFF);
        SubMemFromAccWithBorrow(((memory->Read((addr + 1) % 256) << 8) | memory->Read(addr)) + Y);
        break;
    }
    /* case 0xf2: break;
    case 0xf3: break;
    case 0xf4: break; */
    case 0xf5: {  // SBC (zpg_X) -NZCV
        SubMemFromAccWithBorrow((memory->Read(pc + 1) + X) % 256);
        break;
    }
    case 0xf6: {  // INC (zpg_X) -NZ
        uint16_t addr = ((memory->Read(pc + 1) + X) % 256);
        uint8_t byte = memory->Read(addr);
        byte += 1;
        flags[Flags::negative] = (byte >> 7);
        flags[Flags::zero] = (byte == 0);
        memory->Write(addr, byte);
        pc += 1;
        break;
    }
    /* case 0xf7: break; */
    case 0xf8: {  //  SED -D
        flags[Flags::decimal] = true;
        break;
    }
    case 0xf9: {  // SBC (abs_Y) -NZCV
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_y = abs + Y;
        cycles += ((abs & 0xFF00) != (abs_y & 0xFF00));
        SubMemFromAccWithBorrow(abs_y);
        pc += 1;
        break;
    }
    /* case 0xfa: break;
    case 0xfb: break;
    case 0xfc: break; */
    case 0xfd: {  // SBC (abs_X) -NZCV
        uint16_t abs = GetImmediateAddress();
        uint16_t abs_x = abs + X;
        cycles += ((abs & 0xFF00) != (abs_x & 0xFF00));
        SubMemFromAccWithBorrow(abs_x);
        pc += 1;
        break;
    }
    case 0xfe: {  // INC (abs_X) -NZ
        uint16_t addr = (GetImmediateAddress() + X);
        uint8_t byte = memory->Read(addr);
        byte += 1;
        flags[Flags::negative] = (byte >> 7);
        flags[Flags::zero] = (byte == 0);
        memory->Write(addr, byte);
        pc += 2;
        break;
    }
    /* case 0xff: break; */
    default:
        increment_pc = false;
        char unimpl_instr[5];
        sprintf_s(&unimpl_instr[0], sizeof(unimpl_instr), "0x%X", memory->Read(pc));
        std::string error_message = "\nUnimplemented instruction " + std::string(unimpl_instr);
        if (log_helper.GetLastMessage() != error_message) {
            log_helper.AddLog(error_message);
        }
    }

    cycles += cycle_lut[instr];

    if (increment_pc) pc += 1;
}
