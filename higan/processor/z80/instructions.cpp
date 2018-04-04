//legend:
//  a   = register A
//  c   = condition
//  e   = relative operand
//  in  = (operand)
//  inn = (operand-word)
//  irr = (register-word)
//  o   = opcode bits
//  n   = operand
//  nn  = operand-word
//  r   = register

auto Z80::instructionADC_a_irr(uint16& x) -> void {
  A = ADD(A, read(displace(x)), CF);
}

auto Z80::instructionADC_a_n() -> void {
  A = ADD(A, operand(), CF);
}

auto Z80::instructionADC_a_r(uint8& x) -> void {
  A = ADD(A, x, CF);
}

auto Z80::instructionADC_hl_rr(uint16& x) -> void {
  wait(4);
  auto lo = ADD(HL >> 0, x >> 0, CF);
  wait(3);
  auto hi = ADD(HL >> 8, x >> 8, CF);
  HL = hi << 8 | lo << 0;
  ZF = HL == 0;
}

auto Z80::instructionADD_a_irr(uint16& x) -> void {
  A = ADD(A, read(displace(x)));
}

auto Z80::instructionADD_a_n() -> void {
  A = ADD(A, operand());
}

auto Z80::instructionADD_a_r(uint8& x) -> void {
  A = ADD(A, x);
}

auto Z80::instructionADD_hl_rr(uint16& x) -> void {
  bool vf = VF, zf = ZF, sf = SF;
  wait(4);
  auto lo = ADD(HL >> 0, x >> 0);
  wait(3);
  auto hi = ADD(HL >> 8, x >> 8, CF);
  HL = hi << 8 | lo << 0;
  VF = vf, ZF = zf, SF = sf;  //restore unaffected flags
}

auto Z80::instructionAND_a_irr(uint16& x) -> void {
  A = AND(A, read(displace(x)));
}

auto Z80::instructionAND_a_n() -> void {
  A = AND(A, operand());
}

auto Z80::instructionAND_a_r(uint8& x) -> void {
  A = AND(A, x);
}

auto Z80::instructionBIT_o_irr(uint3 bit, uint16& addr) -> void {
  BIT(bit, read(addr));
}

auto Z80::instructionBIT_o_irr_r(uint3 bit, uint16& addr, uint8& x) -> void {
  x = BIT(bit, read(addr));
}

auto Z80::instructionBIT_o_r(uint3 bit, uint8& x) -> void {
  BIT(bit, x);
}

auto Z80::instructionCALL_c_nn(bool c) -> void {
  auto addr = operands();
  if(!c) return;
  wait(1);
  push(PC);
  PC = addr;
}

auto Z80::instructionCALL_nn() -> void {
  auto addr = operands();
  wait(1);
  push(PC);
  PC = addr;
}

auto Z80::instructionCCF() -> void {
  CF = !CF;
  NF = 0;
  HF = !CF;
}

auto Z80::instructionCP_a_irr(uint16& x) -> void {
  SUB(A, read(displace(x)));
}

auto Z80::instructionCP_a_n() -> void {
  SUB(A, operand());
}

auto Z80::instructionCP_a_r(uint8& x) -> void {
  SUB(A, x);
}

auto Z80::instructionCPD() -> void {
  bool cf = CF;
  auto data = read(_HL--);
  wait(5);
  SUB(A, data);
  VF = --BC != 0;
  CF = cf;  //restore unaffected flag
}

auto Z80::instructionCPDR() -> void {
  instructionCPD();
  if(!BC) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionCPI() -> void {
  bool cf = CF;
  auto data = read(_HL++);
  wait(5);
  SUB(A, data);
  VF = --BC != 0;
  CF = cf;  //restore unaffected flag
}

auto Z80::instructionCPIR() -> void {
  instructionCPI();
  if(!BC) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionCPL() -> void {
  A = ~A;

  NF = 1;
  XF = A.bit(3);
  HF = 1;
  YF = A.bit(5);
}

auto Z80::instructionDAA() -> void {
  auto a = A;
  if(CF || (A.bits(0,7) > 0x99)) { A += NF ? -0x60 : 0x60; CF = 1; }
  if(HF || (A.bits(0,3) > 0x09)) { A += NF ? -0x06 : 0x06; }

  PF = parity(A);
  XF = A.bit(3);
  HF = uint8(A ^ a).bit(4);
  YF = A.bit(5);
  ZF = A == 0;
  SF = A.bit(7);
}

auto Z80::instructionDEC_irr(uint16& x) -> void {
  auto addr = displace(x);
  auto data = read(addr);
  wait(1);
  write(addr, DEC(data));
}

auto Z80::instructionDEC_r(uint8& x) -> void {
  x = DEC(x);
}

auto Z80::instructionDEC_rr(uint16& x) -> void {
  wait(2);
  x--;
}

auto Z80::instructionDI() -> void {
  IFF1 = 0;
  IFF2 = 0;
}

auto Z80::instructionDJNZ_e() -> void {
  wait(1);
  auto e = operand();
  if(!--B) return;
  wait(5);
  PC += (int8)e;
}

auto Z80::instructionEI() -> void {
  EI = 1;  //raise IFF1, IFF2 after the next instruction
}

auto Z80::instructionEX_irr_rr(uint16& x, uint16& y) -> void {
  uint16 z;
  z.byte(0) = read(x + 0);
  z.byte(1) = read(x + 1);
  write(x + 0, y.byte(0));
  write(x + 1, y.byte(1));
  y = z;
}

auto Z80::instructionEX_rr_rr(uint16& x, uint16& y) -> void {
  auto z = x;
  x = y;
  y = z;
}

auto Z80::instructionEXX() -> void {
  swap(BC, BC_);
  swap(DE, DE_);
  swap(_HL, HL_);
}

auto Z80::instructionHALT() -> void {
  HALT = 1;
}

auto Z80::instructionIM_o(uint2 code) -> void {
  wait(4);
  IM = code;
}

auto Z80::instructionIN_a_in() -> void {
  A = in(operand());
}

auto Z80::instructionIN_r_ic(uint8& x) -> void {
  x = in(C);
}

auto Z80::instructionINC_irr(uint16& x) -> void {
  auto addr = displace(x);
  auto data = read(addr);
  wait(1);
  write(addr, INC(data));
}

auto Z80::instructionINC_r(uint8& x) -> void {
  x = INC(x);
}

auto Z80::instructionINC_rr(uint16& x) -> void {
  wait(2);
  x++;
}

auto Z80::instructionIND() -> void {
  wait(1);
  auto data = in(C);
  write(_HL--, data);
  NF = 1;
  ZF = --B == 0;
}

auto Z80::instructionINDR() -> void {
  instructionIND();
  if(!B) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionINI() -> void {
  wait(1);
  auto data = in(C);
  write(_HL++, data);
  NF = 1;
  ZF = --B == 0;
}

auto Z80::instructionINIR() -> void {
  instructionINI();
  if(!B) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionJP_c_nn(bool c) -> void {
  auto pc = operands();
  if(c) r.pc = pc;
}

auto Z80::instructionJP_rr(uint16& x) -> void {
  PC = x;
}

auto Z80::instructionJR_c_e(bool c) -> void {
  auto e = operand();
  if(c) wait(5), r.pc += (int8)e;
}

auto Z80::instructionLD_a_inn() -> void {
  A = read(operands());
}

auto Z80::instructionLD_a_irr(uint16& x) -> void {
  A = read(displace(x));
}

auto Z80::instructionLD_inn_a() -> void {
  write(operands(), A);
}

auto Z80::instructionLD_inn_rr(uint16& x) -> void {
  auto addr = operands();
  write(addr + 0, x >> 0);
  write(addr + 1, x >> 8);
}

auto Z80::instructionLD_irr_a(uint16& x) -> void {
  write(displace(x), A);
}

auto Z80::instructionLD_irr_n(uint16& x) -> void {
  auto addr = displace(x);
  write(addr, operand());
}

auto Z80::instructionLD_irr_r(uint16& x, uint8& y) -> void {
  write(displace(x), y);
}

auto Z80::instructionLD_r_n(uint8& x) -> void {
  x = operand();
}

auto Z80::instructionLD_r_irr(uint8& x, uint16& y) -> void {
  x = read(displace(y));
}

auto Z80::instructionLD_r_r(uint8& x, uint8& y) -> void {
  x = y;
}

//LD to/from I/R requires an extra T-cycle
auto Z80::instructionLD_r_r1(uint8& x, uint8& y) -> void {
  wait(1);
  x = y;
}

//LD from I/R sets status flags
auto Z80::instructionLD_r_r2(uint8& x, uint8& y) -> void {
  wait(1);
  x = y;
  NF = 0;
  PF = IFF2;
  XF = x.bit(3);
  HF = 0;
  YF = x.bit(5);
  ZF = x == 0;
  SF = x.bit(7);
}

auto Z80::instructionLD_rr_inn(uint16& x) -> void {
  auto addr = operands();
  x.byte(0) = read(addr + 0);
  x.byte(1) = read(addr + 1);
}

auto Z80::instructionLD_rr_nn(uint16& x) -> void {
  x = operands();
}

auto Z80::instructionLD_sp_rr(uint16& x) -> void {
  wait(2);
  SP = x;
}

auto Z80::instructionLDD() -> void {
  auto data = read(_HL--);
  write(DE--, data);
  wait(2);
  NF = 0;
  VF = --BC != 0;
  HF = 0;
}

auto Z80::instructionLDDR() -> void {
  instructionLDD();
  if(!BC) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionLDI() -> void {
  auto data = read(_HL++);
  write(DE++, data);
  wait(2);
  NF = 0;
  VF = --BC != 0;
  HF = 0;
}

auto Z80::instructionLDIR() -> void {
  instructionLDI();
  if(!BC) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionNEG() -> void {
  A = SUB(0, A);
}

auto Z80::instructionNOP() -> void {
}

auto Z80::instructionOR_a_irr(uint16& x) -> void {
  A = OR(A, read(displace(x)));
}

auto Z80::instructionOR_a_n() -> void {
  A = OR(A, operand());
}

auto Z80::instructionOR_a_r(uint8& x) -> void {
  A = OR(A, x);
}

auto Z80::instructionOTDR() -> void {
  instructionOUTD();
  if(!B) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionOTIR() -> void {
  instructionOUTI();
  if(!B) return;
  wait(5);
  PC -= 2;
}

auto Z80::instructionOUT_ic_r(uint8& x) -> void {
  out(C, x);
}

auto Z80::instructionOUT_n_a() -> void {
  auto addr = operand();
  out(addr, A);
}

auto Z80::instructionOUTD() -> void {
  wait(1);
  auto data = read(_HL--);
  out(C, data);
  NF = 1;
  ZF = --B == 0;
}

auto Z80::instructionOUTI() -> void {
  wait(1);
  auto data = read(_HL++);
  out(C, data);
  NF = 1;
  ZF = --B == 0;
}

auto Z80::instructionPOP_rr(uint16& x) -> void {
  x = pop();
}

auto Z80::instructionPUSH_rr(uint16& x) -> void {
  wait(1);
  push(x);
}

auto Z80::instructionRES_o_irr(uint3 bit, uint16& addr) -> void {
  write(addr, RES(bit, read(addr)));
}

auto Z80::instructionRES_o_irr_r(uint3 bit, uint16& addr, uint8& x) -> void {
  write(addr, x = RES(bit, read(addr)));
}

auto Z80::instructionRES_o_r(uint3 bit, uint8& x) -> void {
  x = RES(bit, x);
}

auto Z80::instructionRET() -> void {
  wait(1);
  PC = pop();
}

auto Z80::instructionRET_c(bool c) -> void {
  wait(1);
  if(!c) return;
  PC = pop();
}

auto Z80::instructionRETI() -> void {
  PC = pop();
  IFF1 = IFF2;
}

auto Z80::instructionRETN() -> void {
  PC = pop();
  IFF1 = IFF2;
}

auto Z80::instructionRL_irr(uint16& addr) -> void {
  write(addr, RL(read(addr)));
}

auto Z80::instructionRL_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = RL(read(addr)));
}

auto Z80::instructionRL_r(uint8& x) -> void {
  x = RL(x);
}

auto Z80::instructionRLA() -> void {
  bool c = A.bit(7);
  A = A << 1 | CF;

  CF = c;
  NF = 0;
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
}

auto Z80::instructionRLC_irr(uint16& addr) -> void {
  write(addr, RLC(read(addr)));
}

auto Z80::instructionRLC_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = RLC(read(addr)));
}

auto Z80::instructionRLC_r(uint8& x) -> void {
  x = RLC(x);
}

auto Z80::instructionRLCA() -> void {
  bool c = A.bit(7);
  A = A << 1 | c;

  CF = c;
  NF = 0;
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
}

auto Z80::instructionRLD() -> void {
  auto data = read(HL);
  wait(1);
  write(HL, (data << 4) | (A & 0x0f));
  wait(3);
  A = (A & 0xf0) | (data >> 4);

  NF = 0;
  PF = parity(A);
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
  ZF = A == 0;
  SF = A.bit(7);
}

auto Z80::instructionRR_irr(uint16& addr) -> void {
  write(addr, RR(read(addr)));
}

auto Z80::instructionRR_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = RR(read(addr)));
}

auto Z80::instructionRR_r(uint8& x) -> void {
  x = RR(x);
}

auto Z80::instructionRRA() -> void {
  bool c = A.bit(0);
  A = CF << 7 | A >> 1;

  CF = c;
  NF = 0;
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
}

auto Z80::instructionRRC_irr(uint16& addr) -> void {
  write(addr, RRC(read(addr)));
}

auto Z80::instructionRRC_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = RRC(read(addr)));
}

auto Z80::instructionRRC_r(uint8& x) -> void {
  x = RRC(x);
}

auto Z80::instructionRRCA() -> void {
  bool c = A.bit(0);
  A = c << 7 | A >> 1;

  CF = c;
  NF = 0;
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
}

auto Z80::instructionRRD() -> void {
  auto data = read(HL);
  wait(1);
  write(HL, (data >> 4) | (A << 4));
  wait(3);
  A = (A & 0xf0) | (data & 0x0f);

  NF = 0;
  PF = parity(A);
  XF = A.bit(3);
  HF = 0;
  YF = A.bit(5);
  ZF = A == 0;
  SF = A.bit(7);
}

auto Z80::instructionRST_o(uint3 vector) -> void {
  wait(1);
  push(PC);
  PC = vector << 3;
}

auto Z80::instructionSBC_a_irr(uint16& x) -> void {
  A = SUB(A, read(displace(x)), CF);
}

auto Z80::instructionSBC_a_n() -> void {
  A = SUB(A, operand(), CF);
}

auto Z80::instructionSBC_a_r(uint8& x) -> void {
  A = SUB(A, x, CF);
}

auto Z80::instructionSBC_hl_rr(uint16& x) -> void {
  wait(4);
  auto lo = SUB(HL >> 0, x >> 0, CF);
  wait(3);
  auto hi = SUB(HL >> 8, x >> 8, CF);
  HL = hi << 8 | lo << 0;
  ZF = HL == 0;
}

auto Z80::instructionSCF() -> void {
  CF = 1;
  NF = 0;
  HF = 0;
}

auto Z80::instructionSET_o_irr(uint3 bit, uint16& addr) -> void {
  write(addr, SET(bit, read(addr)));
}

auto Z80::instructionSET_o_irr_r(uint3 bit, uint16& addr, uint8& x) -> void {
  write(addr, x = SET(bit, read(addr)));
}

auto Z80::instructionSET_o_r(uint3 bit, uint8& x) -> void {
  x = SET(bit, x);
}

auto Z80::instructionSLA_irr(uint16& addr) -> void {
  write(addr, SLA(read(addr)));
}

auto Z80::instructionSLA_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = SLA(read(addr)));
}

auto Z80::instructionSLA_r(uint8& x) -> void {
  x = SLA(x);
}

auto Z80::instructionSLL_irr(uint16& addr) -> void {
  write(addr, SLL(read(addr)));
}

auto Z80::instructionSLL_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = SLL(read(addr)));
}

auto Z80::instructionSLL_r(uint8& x) -> void {
  x = SLL(x);
}

auto Z80::instructionSRA_irr(uint16& addr) -> void {
  write(addr, SRA(read(addr)));
}

auto Z80::instructionSRA_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = SRA(read(addr)));
}

auto Z80::instructionSRA_r(uint8& x) -> void {
  x = SRA(x);
}

auto Z80::instructionSRL_irr(uint16& addr) -> void {
  write(addr, SRL(read(addr)));
}

auto Z80::instructionSRL_irr_r(uint16& addr, uint8& x) -> void {
  write(addr, x = SRL(read(addr)));
}

auto Z80::instructionSRL_r(uint8& x) -> void {
  x = SRL(x);
}

auto Z80::instructionSUB_a_irr(uint16& x) -> void {
  A = SUB(A, read(displace(x)));
}

auto Z80::instructionSUB_a_n() -> void {
  A = SUB(A, operand());
}

auto Z80::instructionSUB_a_r(uint8& x) -> void {
  A = SUB(A, x);
}

auto Z80::instructionXOR_a_irr(uint16& x) -> void {
  A = XOR(A, read(displace(x)));
}

auto Z80::instructionXOR_a_n() -> void {
  A = XOR(A, operand());
}

auto Z80::instructionXOR_a_r(uint8& x) -> void {
  A = XOR(A, x);
}
