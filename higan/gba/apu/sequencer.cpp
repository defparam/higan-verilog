auto APU::runsequencer() -> void {
  auto& r = sequencer;

  if(r.base == 0) {  //512hz
    if(r.step == 0 || r.step == 2 || r.step == 4 || r.step == 6) {  //256hz
      square1.clocklength();
      square2.clocklength();
      wave.clocklength();
      noise.clocklength();
    }
    if(r.step == 2 || r.step == 6) {  //128hz
      square1.clocksweep();
    }
    if(r.step == 7) {  //64hz
      square1.clockenvelope();
      square2.clockenvelope();
      noise.clockenvelope();
    }
    r.step++;
  }
  r.base++;

  if(square1.enable) square1.run();
  if(square2.enable) square2.run();
  if(wave.enable) wave.run();
  if(noise.enable) noise.run();
}

auto APU::Sequencer::sample() -> void {
  loutput = 0;
  routput = 0;
  if(!masterenable) return;

  if(lenable[0]) loutput += apu.square1.output;
  if(lenable[1]) loutput += apu.square2.output;
  if(lenable[2]) loutput +=    apu.wave.output;
  if(lenable[3]) loutput +=   apu.noise.output;
  loutput  *= 1 + lvolume;
  loutput <<= 1;
  loutput >>= 3 - volume;

  if(renable[0]) routput += apu.square1.output;
  if(renable[1]) routput += apu.square2.output;
  if(renable[2]) routput +=    apu.wave.output;
  if(renable[3]) routput +=   apu.noise.output;
  routput  *= 1 + rvolume;
  routput <<= 1;
  routput >>= 3 - volume;
}

auto APU::Sequencer::read(uint addr) const -> uint8 {
  switch(addr) {
  case 0: return (rvolume << 0) | (lvolume << 4);
  case 1: return (
    (renable[0] << 0)
  | (renable[1] << 1)
  | (renable[2] << 2)
  | (renable[3] << 3)
  | (lenable[0] << 4)
  | (lenable[1] << 5)
  | (lenable[2] << 6)
  | (lenable[3] << 7)
  );
  case 2: return (
    (apu.square1.enable << 0)
  | (apu.square2.enable << 1)
  | (apu.wave.enable    << 2)
  | (apu.noise.enable   << 3)
  | (masterenable       << 7)
  );
  }
}

auto APU::Sequencer::write(uint addr, uint8 byte) -> void {
  switch(addr) {
  case 0:  //NR50
    rvolume = byte >> 0;
    lvolume = byte >> 4;
    break;

  case 1:  //NR51
    renable[0] = byte >> 0;
    renable[1] = byte >> 1;
    renable[2] = byte >> 2;
    renable[3] = byte >> 3;
    lenable[0] = byte >> 4;
    lenable[1] = byte >> 5;
    lenable[2] = byte >> 6;
    lenable[3] = byte >> 7;
    break;

  case 2:  //NR52
    masterenable = byte >> 7;
    break;
  }
}

auto APU::Sequencer::power() -> void {
  lvolume = 0;
  rvolume = 0;
  for(auto& n : lenable) n = 0;
  for(auto& n : renable) n = 0;
  masterenable = 0;
  base = 0;
  step = 0;
  lsample = 0;
  rsample = 0;
}
