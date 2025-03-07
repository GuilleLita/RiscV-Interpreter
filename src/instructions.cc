#include <instructions.hh>
#include <memory.hh>

using namespace instrs;
using namespace mem;


// ToDo use the type instead of RISC-V funct3 field?
// assume little endian
// LW 0b010
template<>
void instrs::execute_load<0b010>(memory& mem, processor & proc,
    address_t addr, uint8_t rd)
{
  assert((addr & 0b11) == 0); // ensure alignment
  proc.write_reg(rd, mem.read<uint32_t>(addr));
}

// LBU 100
template<>
void instrs::execute_load<0b100>(memory& mem, processor & proc,
    address_t addr, uint8_t rd)
{
  // no sign extension
  uint32_t val = mem.read<uint8_t>(addr);
  proc.write_reg(rd, val);
}

// LB 000
template<>
void instrs::execute_load<0b000>(memory& mem, processor & proc,
    address_t addr, uint8_t rd)
{
  uint32_t val = sign_extend<uint32_t, sizeof(uint8_t)*8>(mem.read<uint8_t>(addr));
  proc.write_reg(rd, val);
}

// LH
template<>
void instrs::execute_load<0b001>(memory& mem, processor & proc,
    address_t addr, uint8_t rd)
{
  assert((addr & 0b1) == 0); // ensure alignment
  // perform sign extension
  uint32_t val = sign_extend<uint32_t,
           sizeof(uint16_t)*8>(mem.read<uint16_t>(addr));
  proc.write_reg(rd, val);
}

// LHU
template<>
void instrs::execute_load<0b101>(memory& mem, processor & proc,
    address_t addr, uint8_t rd)
{
  assert((addr & 0b1) == 0); // ensure alignment
  // perform sign extension
  uint32_t val = mem.read<uint16_t>(addr);
  proc.write_reg(rd, val);
}


uint32_t instrs::load(memory& mem, processor & proc, uint32_t bitstream) {

  i_instruction ii{bitstream};

  // compute src address
  address_t src = proc.read_reg(ii.rs1()) + ii.imm12();

  // ToDo refactor with templates
  switch(ii.funct3()) {
    case 0b010: execute_load<0b010>(mem, proc, src, ii.rd()); break;
    case 0b000: execute_load<0b000>(mem, proc, src, ii.rd()); break;
    case 0b001: execute_load<0b001>(mem, proc, src, ii.rd()); break;
    case 0b100: execute_load<0b100>(mem, proc, src, ii.rd()); break;
    case 0b101: execute_load<0b101>(mem, proc, src, ii.rd()); break;
  }
  // return next instruction
  return proc.next_pc();
}

// SB
template<>
void instrs::execute_store<0b000>(memory& mem, processor& proc,
    address_t addr, uint8_t rs2)
{
  mem.write<uint8_t>(addr, static_cast<uint8_t>(proc.read_reg(rs2)));
}

// SH
template<>
void instrs::execute_store<0b001>(memory& mem, processor& proc,
    address_t addr, uint8_t rs2)
{
  mem.write<uint16_t>(addr, static_cast<uint16_t>(proc.read_reg(rs2)));
}

// SW
template<>
void instrs::execute_store<0b010>(memory& mem, processor& proc,
    address_t addr, uint8_t rs2)
{
  mem.write<uint32_t>(addr, proc.read_reg(rs2));
}

uint32_t instrs::store(memory& mem, processor & proc, uint32_t bitstream) {

  s_instruction si{bitstream};

  uint32_t imm = si.imm12();
  uint32_t rs1 = proc.read_reg(si.rs1());
  
  // compute dst address
  address_t src = rs1 + imm;

  // ToDo refactor with templates
  switch(si.funct3()) {
    case 0b000: execute_store<0b000>(mem, proc, src, si.rs2()); break; // SB
    case 0b001: execute_store<0b001>(mem, proc, src, si.rs2()); break; // SH
    case 0b010: execute_store<0b010>(mem, proc, src, si.rs2()); break; // SW
  }
  // return next instruction
  return proc.next_pc();
}

template<>
void instrs::execute_arithmetic_i<0b000>( processor& proc, uint8_t rs1, uint8_t rd, uint32_t imm) {
  proc.write_reg(rd, proc.read_reg(rs1) + imm);
}

template<>
void instrs::execute_arithmetic_i<0b001>( processor& proc, uint8_t rs1, uint8_t rd, uint32_t imm) {
  proc.write_reg(rd, proc.read_reg(rs1) << imm);
}


uint32_t instrs::arithmetic_i(memory& , processor & proc, uint32_t bitstream) {
  
  i_instruction ii{bitstream};

  uint32_t imm = ii.imm12();
  // ToDo refactor with templates
  switch(ii.funct3()) {
    case 0b000: execute_arithmetic_i<0b000>(proc, ii.rs1(), ii.rd(), imm); break;
    case 0b001: execute_arithmetic_i<0b001>(proc, ii.rs1(), ii.rd(), imm); break;
  }
  // return next instruction
  return proc.next_pc();
}

template<>
void instrs::execute_arithmetic_r<0b000>( processor& proc, uint8_t rs1, uint8_t rs2, uint8_t rd, uint32_t funct7) {
  if(funct7 == 0) {
    proc.write_reg(rd, proc.read_reg(rs1) + proc.read_reg(rs2));
  } else {
    proc.write_reg(rd, proc.read_reg(rs1) - proc.read_reg(rs2));
  }
}

template<>
void instrs::execute_arithmetic_r<0b001>( processor& proc, uint8_t rs1, uint8_t rs2, uint8_t rd, uint32_t) {
  proc.write_reg(rd, proc.read_reg(rs1) << proc.read_reg(rs2));
}

uint32_t instrs::arithmetic_r(memory& , processor & proc, uint32_t bitstream) {
  
  r_instruction ri{bitstream};

  // ToDo refactor with templates
  switch(ri.funct3()) {
    case 0b000: execute_arithmetic_r<0b000>(proc, ri.rs1(), ri.rs2(), ri.rd(), ri.funct7()); break;
    case 0b001: execute_arithmetic_r<0b001>(proc, ri.rs1(), ri.rs2(), ri.rd(), ri.funct7()); break;
  }
  // return next instruction
  return proc.next_pc();
}

uint32_t instrs::jal(memory&, processor & proc, uint32_t bitstream) {

  j_instruction ji{bitstream};

  // save the return address
  if(ji.rd() != 0) {
    proc.write_reg(ji.rd(), proc.read_pc());
  }

  address_t target = proc.read_pc() + sign_extend<uint32_t, 21>(ji.imm());
  // jump to the new address
  proc.write_pc(target);
  return target;
}

// BEQ
template<>
uint32_t instrs::execute_branch<0b000>(processor& proc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
  if(rs1 == rs2) {
    address_t target = proc.read_pc() + imm;
    proc.write_pc(target);
    return target;
  }
  return proc.next_pc();
}

// BNE
template<>
uint32_t instrs::execute_branch<0b001>(processor& proc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
  if(rs1 != rs2) {
    address_t target = proc.read_pc() + imm;
    proc.write_pc(target);
    return target;
  }
  return proc.next_pc();
}

// BLT
template<>
uint32_t instrs::execute_branch<0b100>(processor& proc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
  if(static_cast<int32_t>(rs1) < static_cast<int32_t>(rs2)) {
    address_t target = proc.read_pc() + imm;
    proc.write_pc(target);
    return target;
  }
  return proc.next_pc();
}


// BGE
template<>
uint32_t instrs::execute_branch<0b101>(processor& proc, uint32_t rs1, uint32_t rs2, uint32_t imm) {
  if(rs1 >= rs2) {
    address_t target = proc.read_pc() + imm;
    proc.write_pc(target);
    return target;
  }
  return proc.next_pc();
}

uint32_t instrs::branch(memory&, processor& proc, uint32_t bitstream){

  b_instruction bi{bitstream};

  uint32_t imm = bi.imm13();
  uint32_t rs1 = proc.read_reg(bi.rs1());
  uint32_t rs2 = proc.read_reg(bi.rs2());

  switch(bi.funct3()) {
    case 0b000: return execute_branch<0b000>(proc, rs1, rs2, imm); // BEQ
    case 0b001: return execute_branch<0b001>(proc, rs1, rs2, imm); // BNE
    case 0b100: return execute_branch<0b100>(proc, rs1, rs2, imm); // BLT
    case 0b101: return execute_branch<0b101>(proc, rs1, rs2, imm); // BGE
  }
  return proc.next_pc();
}

uint32_t instrs::lui(memory& , processor & proc, uint32_t bitstream) {
  u_instruction ui{bitstream};

  uint32_t imm = ui.imm();
  proc.write_reg(ui.rd(), imm);
  return proc.next_pc();
}

