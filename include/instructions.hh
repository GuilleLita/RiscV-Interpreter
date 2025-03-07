#pragma once

#include <cstdint>
#include <functional>

#include <memory.hh>
#include <processor.hh>

// extracted from the riscv-spec-20191213.pdf

namespace instrs {

enum class type {base, r, i, s, b, u, j};

template<typename T, unsigned B>
inline T sign_extend(const T x) {
    struct { T x:B; } s;
    return s.x = x;
}  

class instruction {
    protected:
        instrs::type _type;
        uint32_t _bitstream;
    public:
        // check Figure 2.4 from the manual
        constexpr uint32_t bits(size_t lsb, size_t len) const
        {
            return (_bitstream >> lsb) & ((static_cast<uint32_t>(1) << len)-1);
        }
        instruction(uint32_t bitstream, instrs::type type=type::base) : _type(type),
        _bitstream(bitstream) {}

        constexpr instrs::type type() const { return _type; }
        constexpr uint8_t opcode() const { return bits(0,7); }
};

class r_instruction : public instruction {
    public:
        r_instruction(uint32_t bitstream) :
            instruction(bitstream, type::r) {}
        constexpr uint8_t rd() const { return bits(7, 5); }
        constexpr uint8_t funct3() const { return bits(12, 3); }
        constexpr uint8_t rs1() const { return bits(15, 5); }
        constexpr uint8_t rs2() const { return bits(20, 5); }
        constexpr uint32_t funct7() const { return bits(25, 7); }
};

class i_instruction : public instruction {
    public:
        i_instruction(uint32_t bitstream) :
            instruction(bitstream, type::i) {}
        constexpr uint8_t rd() const { return bits(7, 5); }
        constexpr uint8_t funct3() const { return bits(12, 3); }
        constexpr uint8_t rs1() const { return bits(15, 5); }
        constexpr uint32_t imm() const { return bits(20, 12); }
        uint32_t imm12() const { return static_cast<uint32_t>(sign_extend<int32_t, 12>(imm()));}
};

class s_instruction : public instruction {
    public:
        s_instruction(uint32_t bitstream) :
            instruction(bitstream, type::s) {}
        constexpr uint8_t funct3() const { return bits(12, 3); }
        constexpr uint8_t rs1() const { return bits(15, 5); }
        constexpr uint8_t rs2() const { return bits(20, 5); }
        constexpr uint32_t imm() const { return (bits(7, 5)) | (bits(25, 7) << 5); }
        uint32_t imm12() const { return static_cast<uint32_t>(sign_extend<int32_t, 12>(imm()));}
};

class b_instruction : public instruction {
    public:
        b_instruction(uint32_t bitstream) :
            instruction(bitstream, type::b) {}
        constexpr uint8_t funct3() const { return bits(12, 3); }
        constexpr uint8_t rs1() const { return bits(15, 5); }
        constexpr uint8_t rs2() const { return bits(20, 5); }
        constexpr uint32_t imm() const { return (bits(8, 4) << 1) | (bits(25, 6) << 5) | (bits(7, 1) << 11) | (bits(31, 1) << 12); }
        uint32_t imm13() const { return static_cast<uint32_t>(sign_extend<int32_t, 13>(imm()));}

};

class u_instruction : public instruction {
    public:
        u_instruction(uint32_t bitstream) :
            instruction(bitstream, type::u) {}
        constexpr uint8_t rd() const { return bits(7, 5); }
        constexpr uint32_t imm() const { return bits(12, 20) << 12; }

};

class j_instruction : public instruction {
    public:
        j_instruction(uint32_t bitstream) :
            instruction(bitstream, type::j) {}
        constexpr uint8_t rd() const { return bits(7, 5); }
        constexpr uint32_t imm() const { return (bits(21, 10) << 1) | (bits(20, 1) << 11) | (bits(12, 8) << 12) | (bits(31, 1) << 20); }
};

uint32_t load(mem::memory& mem, processor& proc, uint32_t bitstream);

uint32_t store(mem::memory& mem, processor & proc, uint32_t bitstream);

uint32_t arithmetic_i(mem::memory&, processor & proc, uint32_t bitstream);

uint32_t arithmetic_r(mem::memory&, processor & proc, uint32_t bitstream);

uint32_t jal(mem::memory&, processor & proc, uint32_t bitstream);

uint32_t branch(mem::memory&, processor & proc, uint32_t bitstream);


uint32_t lui(mem::memory&, processor & proc, uint32_t bitstream);

template<uint8_t funct3>
void execute_arithmetic_i(processor& proc,
    uint8_t rs1, uint8_t rd, uint32_t imm);

template<uint8_t funct3>
void execute_arithmetic_r(processor& proc,
    uint8_t rs1, uint8_t rs2, uint8_t rd, uint32_t funct7);

template<uint8_t funct3>
void execute_load(mem::memory& mem, processor& proc,
    mem::address_t addr, uint8_t rd);

template<uint8_t funct3>
void execute_store(mem::memory& mem, processor& proc,
    mem::address_t addr, uint8_t rs2);

template<uint8_t funct3>
uint32_t execute_branch(processor& proc,
    uint32_t rs1, uint32_t rs2, uint32_t imm);

using instr_emulation = std::function<uint32_t(mem::memory& mem, processor& proc, uint32_t)>;



} // namespace instrs
