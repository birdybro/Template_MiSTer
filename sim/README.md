# audio_cdc_fifo Verilator Testbench

Validates the async FIFO (`sys/audio_cdc_fifo.v`) used for clock domain crossing between the core audio output (`clk_sys`) and the framework audio chain (`clk_audio`).

## Requirements

- Verilator (tested with v5.042)
- g++

## Running

```
cd sim
make
```

`make clean` removes build artifacts.

## How it works

Verilator is a cycle-based simulator — it cannot model true asynchronous clock domains the way an event-driven simulator (ModelSim, Icarus) can. To approximate two independent clocks, the testbench uses a fine-grained sim tick loop where `wr_clk` and `rd_clk` toggle at different intervals. Co-prime toggle periods (e.g. 7 and 10) ensure the phase relationship between clocks shifts over time, exercising various alignment scenarios.

Each test instantiates a fresh FIFO, resets it, then drives writes and reads while checking data integrity.

### Clock edge handling

The FIFO uses a combinational read output (`assign rd_data = mem[rd_addr]`). On a `rd_clk` rising edge with `rd_en` asserted, the read pointer advances via non-blocking assignment, and after Verilator's `eval()`, `rd_data` already reflects the *next* entry. To capture the correct consumed value, the testbench snapshots `rd_data` *before* each tick (`pre_edge_rd_data`) and uses that snapshot when a read-side rising edge is detected.

## Tests

### Test 1: Reset state

Asserts that after reset, `rd_empty == 1` and `wr_full == 0`.

### Test 2: Single write/read

Writes one 32-bit value (`0xDEADBEEF`), waits for the gray-code write pointer to propagate through the 2-FF synchronizer into the read clock domain (~6 cycles of both clocks), then reads and verifies the value matches.

### Test 3: Fill to full

Writes 4 entries (the full FIFO depth) with sync settle time between each write. Verifies `wr_full` asserts after the 4th write.

### Test 4: Fill then drain

Writes 4 entries with known values, then reads all 4 back. Verifies FIFO ordering (first-in, first-out) and that `rd_empty` reasserts after draining.

### Tests 5-8: Continuous streaming

Writes and reads concurrently at different rates, verifying every sample arrives in order with correct data. Each test uses different clock toggle periods to exercise various `clk_sys` : `clk_audio` frequency ratios:

| Test | wr_clk half-period | rd_clk half-period | Ratio (wr faster) | Samples |
|------|--------------------|--------------------|-------------------|---------|
| 5    | 5                  | 13                 | ~2.6x             | 500     |
| 6    | 7                  | 7                  | 1:1               | 500     |
| 7    | 11                 | 5                  | ~0.45x (rd faster) | 500     |
| 8    | 3                  | 11                 | ~3.7x             | 1000    |

Writes are gated by a cooldown counter (`wr_gap`) to simulate the core producing samples at a rate slower than `clk_sys`. Reads use a longer cooldown (`rd_gap = wr_gap * 2`) to simulate the 48kHz `sample_ce` consumption rate being slower than the write rate. A reference queue tracks the expected read order.

## Interpreting results

Passing output:

```
=== audio_cdc_fifo testbench ===

Test 1: Reset state
  PASS
Test 2: Single write/read
  PASS
...
Test 8: Streaming (1000 samples, wr_hp=3 rd_hp=11 wr_gap=5)
  read 1000/1000 samples OK
  PASS

=== Results: 8 tests, 0 failures ===
```

A failure prints the expected vs actual value and the sample index where it occurred:

```
Test 5: Streaming (500 samples, wr_hp=5 rd_hp=13 wr_gap=3)
  FAIL: expected 0xA0000003, got 0xA0000005 (sample 3)
```

This would indicate the FIFO delivered samples out of order or dropped entries. The exit code is non-zero on any failure.
