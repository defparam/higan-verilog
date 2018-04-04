auto APU::serialize(serializer& s) -> void {
  Thread::serialize(s);

  s.integer(this->s.sweepClock);
  s.integer(r.waveBase);
  s.integer(r.speakerEnable);
  s.integer(r.speakerShift);
  s.integer(r.headphoneEnable);

  s.integer(dma.s.clock);
  s.integer(dma.s.source);
  s.integer(dma.s.length);
  s.integer(dma.r.source);
  s.integer(dma.r.length);
  s.integer(dma.r.rate);
  s.integer(dma.r.unknown);
  s.integer(dma.r.loop);
  s.integer(dma.r.target);
  s.integer(dma.r.direction);
  s.integer(dma.r.enable);

  s.integer(channel1.o.left);
  s.integer(channel1.o.right);
  s.integer(channel1.s.period);
  s.integer(channel1.s.sampleOffset);
  s.integer(channel1.r.pitch);
  s.integer(channel1.r.volumeLeft);
  s.integer(channel1.r.volumeRight);
  s.integer(channel1.r.enable);

  s.integer(channel2.o.left);
  s.integer(channel2.o.right);
  s.integer(channel2.s.period);
  s.integer(channel2.s.sampleOffset);
  s.integer(channel2.r.pitch);
  s.integer(channel2.r.volumeLeft);
  s.integer(channel2.r.volumeRight);
  s.integer(channel2.r.enable);
  s.integer(channel2.r.voice);
  s.integer(channel2.r.voiceEnableLeft);
  s.integer(channel2.r.voiceEnableRight);

  s.integer(channel3.o.left);
  s.integer(channel3.o.right);
  s.integer(channel3.s.period);
  s.integer(channel3.s.sampleOffset);
  s.integer(channel3.s.sweepCounter);
  s.integer(channel3.r.pitch);
  s.integer(channel3.r.volumeLeft);
  s.integer(channel3.r.volumeRight);
  s.integer(channel3.r.sweepValue);
  s.integer(channel3.r.sweepTime);
  s.integer(channel3.r.enable);
  s.integer(channel3.r.sweep);

  s.integer(channel4.o.left);
  s.integer(channel4.o.right);
  s.integer(channel4.s.period);
  s.integer(channel4.s.sampleOffset);
  s.integer(channel4.s.noiseOutput);
  s.integer(channel4.s.noiseLFSR);
  s.integer(channel4.r.pitch);
  s.integer(channel4.r.volumeLeft);
  s.integer(channel4.r.volumeRight);
  s.integer(channel4.r.noiseMode);
  s.integer(channel4.r.noiseReset);
  s.integer(channel4.r.noiseUpdate);
  s.integer(channel4.r.enable);
  s.integer(channel4.r.noise);

  s.integer(channel5.o.left);
  s.integer(channel5.o.right);
  s.integer(channel5.s.clock);
  s.integer(channel5.s.data);
  s.integer(channel5.r.volume);
  s.integer(channel5.r.scale);
  s.integer(channel5.r.speed);
  s.integer(channel5.r.enable);
  s.integer(channel5.r.unknown);
  s.integer(channel5.r.leftEnable);
  s.integer(channel5.r.rightEnable);
}
