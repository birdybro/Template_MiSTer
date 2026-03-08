VERILOG_SRC = ../sys/audio_cdc_fifo.v
TB_SRC = audio_cdc_fifo_tb.cpp
TOP = audio_cdc_fifo

.PHONY: all run clean

all: run

run: obj_dir/V$(TOP)
	obj_dir/V$(TOP)

obj_dir/V$(TOP): $(VERILOG_SRC) $(TB_SRC)
	verilator --cc --exe --build -Wall \
		-Wno-PROCASSINIT \
		--top-module $(TOP) \
		$(VERILOG_SRC) $(TB_SRC)

clean:
	rm -rf obj_dir
