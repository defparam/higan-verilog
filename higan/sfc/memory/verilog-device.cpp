#include <sfc/sfc.hpp>

namespace SuperFamicom {

	VerilogIf ver;

	VerilogIf::VerilogIf() {
    main_time = 0;
	  memsnoop = new Vmemsnoop;
//#if VM_TRACE
    Verilated::traceEverOn(true);  // Verilator must compute traced signals
    tfp = new VerilatedVcdC;
    memsnoop->trace(tfp, 99);  // Trace 99 levels of hierarchy
    tfp->open("memsnoop_dump.vcd");  // Open the dump file
//#endif
    quiesce(1);
	}

  auto VerilogIf::read(uint24 addr, uint8 data) -> uint8 {
    data = modelread(addr,data);
		return data;
	}

	auto VerilogIf::write(uint24 addr, uint8 data) -> uint8 {
    data = modelwrite(addr,data);
		return data;
	}

  auto VerilogIf::step(int steps) -> void {
    for(int i = 0; i<steps; i++) {
      memsnoop->eval();
      main_time++;
//#if VM_TRACE
        // Dump trace data for this cycle
        if (tfp && (main_time < 50000)) tfp->dump (main_time);
//#endif
    }
  }

  auto VerilogIf::quiesce(int steps) -> void {
    memsnoop->RD_n    = 1;
    memsnoop->WR_n    = 1;
    memsnoop->data_in = 0;
    memsnoop->addr    = 0;
    step(steps);
  }

  auto VerilogIf::modelread(uint24 addr, uint8 data) -> uint8  {
    memsnoop->RD_n    = 0;
    memsnoop->WR_n    = 1;
    memsnoop->data_in = data;
    memsnoop->addr    = addr;
    step(1);
    data = memsnoop->data_out;
    quiesce(1);
    return data;
  }

  auto VerilogIf::modelwrite(uint24 addr, uint8 data) -> uint8 {
    memsnoop->RD_n    = 1;
    memsnoop->WR_n    = 0;
    memsnoop->data_in = data;
    memsnoop->addr    = addr;
    step(1);
    data = memsnoop->data_out;
    quiesce(1);
    return data;
  }

}