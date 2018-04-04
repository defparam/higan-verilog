auto CPU::dmaCounter() const -> uint { return clockCounter & 7; }
auto CPU::joypadCounter() const -> uint { return clockCounter & 255; }

auto CPU::step(uint clocks) -> void {
  status.irqLock = false;
  uint ticks = clocks >> 1;
  while(ticks--) {
    clockCounter += 2;
    tick();
    if(hcounter() & 2) pollInterrupts();
    if(joypadCounter() == 0) joypadEdge();
  }

  Thread::step(clocks);
  for(auto peripheral : peripherals) synchronize(*peripheral);

  if(!status.dramRefreshed && hcounter() >= status.dramRefreshPosition) {
    status.dramRefreshed = true;
    for(auto _ : range(5)) {
      step(8);
      aluEdge();
    }
  }

  #if defined(DEBUGGER)
  synchronizeSMP();
  synchronizePPU();
  synchronizeCoprocessors();
  #endif
}

//called by ppu.tick() when Hcounter=0
auto CPU::scanline() -> void {
  status.lineClocks = lineclocks();

  //forcefully sync S-CPU to other processors, in case chips are not communicating
  synchronize(smp);
  synchronize(ppu);
  for(auto coprocessor : coprocessors) synchronize(*coprocessor);

  if(vcounter() == 0) {
    //HDMA init triggers once every frame
    status.hdmaInitPosition = (version == 1 ? 12 + 8 - dmaCounter() : 12 + dmaCounter());
    status.hdmaInitTriggered = false;

    status.autoJoypadCounter = 0;
  }

  //DRAM refresh occurs once every scanline
  if(version == 2) status.dramRefreshPosition = 530 + 8 - dmaCounter();
  status.dramRefreshed = false;

  //HDMA triggers once every visible scanline
  if(vcounter() < ppu.vdisp()) {
    status.hdmaPosition = 1104;
    status.hdmaTriggered = false;
  }
}

auto CPU::aluEdge() -> void {
  if(alu.mpyctr) {
    alu.mpyctr--;
    if(io.rddiv & 1) io.rdmpy += alu.shift;
    io.rddiv >>= 1;
    alu.shift <<= 1;
  }

  if(alu.divctr) {
    alu.divctr--;
    io.rddiv <<= 1;
    alu.shift >>= 1;
    if(io.rdmpy >= alu.shift) {
      io.rdmpy -= alu.shift;
      io.rddiv |= 1;
    }
  }
}

auto CPU::dmaEdge() -> void {
  //H/DMA pending && DMA inactive?
  //.. Run one full CPU cycle
  //.. HDMA pending && HDMA enabled ? DMA sync + HDMA run
  //.. DMA pending && DMA enabled ? DMA sync + DMA run
  //.... HDMA during DMA && HDMA enabled ? DMA sync + HDMA run
  //.. Run one bus CPU cycle
  //.. CPU sync

  if(status.dmaActive) {
    if(status.hdmaPending) {
      status.hdmaPending = false;
      if(hdmaEnabledChannels()) {
        if(!dmaEnabledChannels()) {
          dmaStep(8 - dmaCounter());
        }
        status.hdmaMode == 0 ? hdmaInit() : hdmaRun();
        if(!dmaEnabledChannels()) {
          step(status.clockCount - (status.dmaClocks % status.clockCount));
          status.dmaActive = false;
        }
      }
    }

    if(status.dmaPending) {
      status.dmaPending = false;
      if(dmaEnabledChannels()) {
        dmaStep(8 - dmaCounter());
        dmaRun();
        step(status.clockCount - (status.dmaClocks % status.clockCount));
        status.dmaActive = false;
      }
    }
  }

  if(!status.hdmaInitTriggered && hcounter() >= status.hdmaInitPosition) {
    status.hdmaInitTriggered = true;
    hdmaInitReset();
    if(hdmaEnabledChannels()) {
      status.hdmaPending = true;
      status.hdmaMode = 0;
    }
  }

  if(!status.hdmaTriggered && hcounter() >= status.hdmaPosition) {
    status.hdmaTriggered = true;
    if(hdmaActiveChannels()) {
      status.hdmaPending = true;
      status.hdmaMode = 1;
    }
  }

  if(!status.dmaActive) {
    if(status.dmaPending || status.hdmaPending) {
      status.dmaClocks = 0;
      status.dmaActive = true;
    }
  }
}

//called every 256 clocks; see CPU::step()
auto CPU::joypadEdge() -> void {
  if(vcounter() >= ppu.vdisp()) {
    //cache enable state at first iteration
    if(status.autoJoypadCounter == 0) status.autoJoypadLatch = io.autoJoypadPoll;
    status.autoJoypadActive = status.autoJoypadCounter <= 15;

    if(status.autoJoypadActive && status.autoJoypadLatch) {
      if(status.autoJoypadCounter == 0) {
        controllerPort1.device->latch(1);
        controllerPort2.device->latch(1);
        controllerPort1.device->latch(0);
        controllerPort2.device->latch(0);

        //shift registers are cleared at start of auto joypad polling
        io.joy1 = 0;
        io.joy2 = 0;
        io.joy3 = 0;
        io.joy4 = 0;
      }

      uint2 port0 = controllerPort1.device->data();
      uint2 port1 = controllerPort2.device->data();

      io.joy1 = io.joy1 << 1 | port0.bit(0);
      io.joy2 = io.joy2 << 1 | port1.bit(0);
      io.joy3 = io.joy3 << 1 | port0.bit(1);
      io.joy4 = io.joy4 << 1 | port1.bit(1);
    }

    status.autoJoypadCounter++;
  }
}

//used to test for NMI/IRQ, which can trigger on the edge of every opcode.
//test one cycle early to simulate two-stage pipeline of x816 CPU.
//
//status.irq_lock is used to simulate hardware delay before interrupts can
//trigger during certain events (immediately after DMA, writes to $4200, etc)
auto CPU::lastCycle() -> void {
  if(!status.irqLock) {
    status.nmiPending |= nmiTest();
    status.irqPending |= irqTest();
    status.interruptPending = (status.nmiPending || status.irqPending);
  }
}
