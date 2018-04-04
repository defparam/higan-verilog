processors += wdc65816 spc700 arm7tdmi gsu hg51b upd96050

objects += Vmemsnoop__ALLcls Vmemsnoop__ALLsup verilated verilated_vcd_c
objects += sfc-interface sfc-system sfc-controller
objects += sfc-cartridge sfc-memory sfc-verilog-device
objects += sfc-cpu sfc-smp sfc-dsp sfc-ppu
objects += sfc-expansion sfc-satellaview sfc-21fx
objects += sfc-icd sfc-mcc sfc-nss sfc-event
objects += sfc-sa1 sfc-superfx
objects += sfc-armdsp sfc-hitachidsp sfc-necdsp
objects += sfc-epsonrtc sfc-sharprtc
objects += sfc-spc7110 sfc-sdd1
objects += sfc-obc1 sfc-msu1
objects += sfc-bsmemory sfc-sufamiturbo


obj/verilated.o:
	g++  -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf \
	-DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=1 -Wno-sign-compare -Wno-uninitialized -Wno-unused-but-set-variable \
	-Wno-unused-parameter -Wno-unused-variable -Wno-shadow -c -o ./obj/verilated.o /usr/local/share/verilator/include/verilated.cpp

obj/verilated_vcd_c.o:
	g++  -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf \
	-DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=1 -Wno-sign-compare -Wno-uninitialized -Wno-unused-but-set-variable \
	-Wno-unused-parameter -Wno-unused-variable -Wno-shadow -MMD -MP -DVL_DEBUG=1  -c -o ./obj/verilated_vcd_c.o /usr/local/share/verilator/include/verilated_vcd_c.cpp

obj/Vmemsnoop__ALLcls.o: sfc/memory/memsnoop.v
	verilator --trace -cc -Wall -Mdir ./obj -sv sfc/memory/memsnoop.v
	cd ./obj && make -f Vmemsnoop.mk

obj/sfc-interface.o:           sfc/interface/interface.cpp $(call rwildcard,sfc/interface)
obj/sfc-system.o:              sfc/system/system.cpp $(call rwildcard,sfc/system/)
obj/sfc-controller.o:          sfc/controller/controller.cpp $(call rwildcard,sfc/controller/)
obj/sfc-cartridge.o:           sfc/cartridge/cartridge.cpp $(call rwildcard,sfc/cartridge/)
obj/sfc-memory.o:              sfc/memory/memory.cpp $(call rwildcard,sfc/memory/)
obj/sfc-verilog-device.o:      sfc/memory/verilog-device.cpp $(call rwildcard,sfc/memory/)

obj/sfc-cpu.o:         sfc/cpu/cpu.cpp $(call rwildcard,sfc/cpu/)
obj/sfc-smp.o:         sfc/smp/smp.cpp $(call rwildcard,sfc/smp/)
obj/sfc-dsp.o:         sfc/dsp/dsp.cpp $(call rwildcard,sfc/dsp/)
obj/sfc-ppu.o:         sfc/ppu/ppu.cpp $(call rwildcard,sfc/ppu/)

obj/sfc-expansion.o:   sfc/expansion/expansion.cpp $(call rwildcard,sfc/expansion/)
obj/sfc-satellaview.o: sfc/expansion/satellaview/satellaview.cpp $(call rwildcard,sfc/expansion/satellaview/)
obj/sfc-21fx.o:        sfc/expansion/21fx/21fx.cpp $(call rwildcard,sfc/expansion/21fx/)

obj/sfc-icd.o:         sfc/coprocessor/icd/icd.cpp $(call rwildcard,sfc/coprocessor/icd/)
obj/sfc-mcc.o:         sfc/coprocessor/mcc/mcc.cpp $(call rwildcard,sfc/coprocessor/mcc/)
obj/sfc-nss.o:         sfc/coprocessor/nss/nss.cpp $(call rwildcard,sfc/coprocessor/nss/)
obj/sfc-event.o:       sfc/coprocessor/event/event.cpp $(call rwildcard,sfc/coprocessor/event/)

obj/sfc-sa1.o:         sfc/coprocessor/sa1/sa1.cpp $(call rwildcard,sfc/coprocessor/sa1/)
obj/sfc-superfx.o:     sfc/coprocessor/superfx/superfx.cpp $(call rwildcard,sfc/coprocessor/superfx/)

obj/sfc-armdsp.o:      sfc/coprocessor/armdsp/armdsp.cpp $(call rwildcard,sfc/coprocessor/armdsp/)
obj/sfc-hitachidsp.o:  sfc/coprocessor/hitachidsp/hitachidsp.cpp $(call rwildcard,sfc/coprocessor/hitachidsp/)
obj/sfc-necdsp.o:      sfc/coprocessor/necdsp/necdsp.cpp $(call rwildcard,sfc/coprocessor/necdsp/)

obj/sfc-epsonrtc.o:    sfc/coprocessor/epsonrtc/epsonrtc.cpp $(call rwildcard,sfc/coprocessor/epsonrtc/)
obj/sfc-sharprtc.o:    sfc/coprocessor/sharprtc/sharprtc.cpp $(call rwildcard,sfc/coprocessor/sharprtc/)

obj/sfc-spc7110.o:     sfc/coprocessor/spc7110/spc7110.cpp $(call rwildcard,sfc/coprocessor/spc7110/)
obj/sfc-sdd1.o:        sfc/coprocessor/sdd1/sdd1.cpp $(call rwildcard,sfc/coprocessor/sdd1/)
obj/sfc-obc1.o:        sfc/coprocessor/obc1/obc1.cpp $(call rwildcard,sfc/coprocessor/obc1/)

obj/sfc-msu1.o:        sfc/coprocessor/msu1/msu1.cpp $(call rwildcard,sfc/coprocessor/msu1/)

obj/sfc-bsmemory.o:    sfc/slot/bsmemory/bsmemory.cpp $(call rwildcard,sfc/slot/bsmemory/)
obj/sfc-sufamiturbo.o: sfc/slot/sufamiturbo/sufamiturbo.cpp $(call rwildcard,sfc/slot/sufamiturbo/)
