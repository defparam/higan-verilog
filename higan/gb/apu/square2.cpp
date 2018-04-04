auto APU::Square2::dacEnable() const -> bool {
  return (envelopeVolume || envelopeDirection);
}

auto APU::Square2::run() -> void {
  if(period && --period == 0) {
    period = 2 * (2048 - frequency);
    phase++;
    switch(duty) {
    case 0: dutyOutput = (phase == 6); break;  //______-_
    case 1: dutyOutput = (phase >= 6); break;  //______--
    case 2: dutyOutput = (phase >= 4); break;  //____----
    case 3: dutyOutput = (phase <= 5); break;  //------__
    }
  }

  uint4 sample = dutyOutput ? (uint)volume : 0;
  if(!enable) sample = 0;

  output = sample;
}

auto APU::Square2::clockLength() -> void {
  if(counter) {
    if(length && --length == 0) enable = false;
  }
}

auto APU::Square2::clockEnvelope() -> void {
  if(enable && envelopeFrequency && --envelopePeriod == 0) {
    envelopePeriod = envelopeFrequency ? (uint)envelopeFrequency : 8;
    if(envelopeDirection == 0 && volume >  0) volume--;
    if(envelopeDirection == 1 && volume < 15) volume++;
  }
}

auto APU::Square2::read(uint16 addr) -> uint8 {
  if(addr == 0xff15) {  //NR20
    return 0xff;
  }

  if(addr == 0xff16) {  //NR21
    return duty << 6 | 0x3f;
  }

  if(addr == 0xff17) {  //NR22
    return envelopeVolume << 4 | envelopeDirection << 3 | envelopeFrequency;
  }

  if(addr == 0xff18) {  //NR23
    return 0xff;
  }

  if(addr == 0xff19) {  //NR24
    return 0x80 | counter << 6 | 0x3f;
  }

  return 0xff;
}

auto APU::Square2::write(uint16 addr, uint8 data) -> void {
  if(addr == 0xff16) {  //NR21
    duty = data.bits(7,6);
    length = 64 - data.bits(5,0);
  }

  if(addr == 0xff17) {  //NR22
    envelopeVolume    = data.bits(7,4);
    envelopeDirection = data.bit (3);
    envelopeFrequency = data.bits(2,0);
    if(!dacEnable()) enable = false;
  }

  if(addr == 0xff18) {  //NR23
    frequency.bits(7,0) = data;
  }

  if(addr == 0xff19) {  //NR24
    if(apu.phase.bit(0) && !counter && data.bit(6)) {
      if(length && --length == 0) enable = false;
    }

    counter = data.bit(6);
    frequency.bits(10,8) = data.bits(2,0);

    if(data.bit(7)) {
      enable = dacEnable();
      period = 2 * (2048 - frequency);
      envelopePeriod = envelopeFrequency ? (uint)envelopeFrequency : 8;
      volume = envelopeVolume;

      if(!length) {
        length = 64;
        if(apu.phase.bit(0) && counter) length--;
      }
    }
  }
}

auto APU::Square2::power(bool initializeLength) -> void {
  enable = 0;

  duty = 0;
  envelopeVolume = 0;
  envelopeDirection = 0;
  envelopeFrequency = 0;
  frequency = 0;
  counter = 0;

  output = 0;
  dutyOutput = 0;
  phase = 0;
  period = 0;
  envelopePeriod = 0;
  volume = 0;

  if(initializeLength) length = 64;
}

auto APU::Square2::serialize(serializer& s) -> void {
  s.integer(enable);

  s.integer(duty);
  s.integer(length);
  s.integer(envelopeVolume);
  s.integer(envelopeDirection);
  s.integer(envelopeFrequency);
  s.integer(frequency);
  s.integer(counter);

  s.integer(output);
  s.integer(dutyOutput);
  s.integer(phase);
  s.integer(period);
  s.integer(envelopePeriod);
  s.integer(volume);
}
