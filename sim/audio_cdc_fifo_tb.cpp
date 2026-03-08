#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <queue>
#include "Vaudio_cdc_fifo.h"

static int test_num = 0;
static int fail_count = 0;

#define CHECK(cond, msg) do { \
	if (!(cond)) { \
		printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
		fail_count++; \
	} \
} while(0)

struct Testbench {
	Vaudio_cdc_fifo *dut;
	uint64_t tick_count;
	int wr_half_period;  // sim ticks per half-period of wr_clk
	int rd_half_period;  // sim ticks per half-period of rd_clk
	uint64_t wr_accum;
	uint64_t rd_accum;

	Testbench(int wr_hp, int rd_hp) : wr_half_period(wr_hp), rd_half_period(rd_hp) {
		dut = new Vaudio_cdc_fifo;
		tick_count = 0;
		wr_accum = 0;
		rd_accum = 0;
		dut->wr_clk = 0;
		dut->rd_clk = 0;
		prev_wr_clk = 0;
		prev_rd_clk = 0;
		dut->wr_rst = 0;
		dut->rd_rst = 0;
		dut->wr_en = 0;
		dut->wr_data = 0;
		dut->rd_en = 0;
		dut->eval();
	}

	~Testbench() { delete dut; }

	uint8_t prev_wr_clk;
	uint8_t prev_rd_clk;
	uint32_t pre_edge_rd_data; // rd_data captured before clock edges

	// Advance one sim tick — toggle clocks at their respective rates
	// Returns flags: bit 0 = wr_clk rose, bit 1 = rd_clk rose
	int tick() {
		prev_wr_clk = dut->wr_clk;
		prev_rd_clk = dut->rd_clk;
		pre_edge_rd_data = dut->rd_data;

		tick_count++;
		wr_accum++;
		rd_accum++;

		if (wr_accum >= (uint64_t)wr_half_period) {
			dut->wr_clk = !dut->wr_clk;
			wr_accum = 0;
		}
		if (rd_accum >= (uint64_t)rd_half_period) {
			dut->rd_clk = !dut->rd_clk;
			rd_accum = 0;
		}
		dut->eval();

		int flags = 0;
		if (!prev_wr_clk && dut->wr_clk) flags |= 1;
		if (!prev_rd_clk && dut->rd_clk) flags |= 2;
		return flags;
	}

	// Run N sim ticks
	void run(int n) {
		for (int i = 0; i < n; i++) tick();
	}

	// Pulse reset for several clock cycles
	void reset() {
		dut->wr_rst = 1;
		dut->rd_rst = 1;
		dut->wr_en = 0;
		dut->rd_en = 0;
		run(wr_half_period * 10 + rd_half_period * 10);
		dut->wr_rst = 0;
		dut->rd_rst = 0;
		run(wr_half_period * 4 + rd_half_period * 4);
	}

	// Wait for a rising edge on wr_clk
	void wait_wr_posedge() {
		// Get to low first
		while (dut->wr_clk) tick();
		// Wait for rising edge
		while (!dut->wr_clk) tick();
	}

	// Wait for a rising edge on rd_clk
	void wait_rd_posedge() {
		while (dut->rd_clk) tick();
		while (!dut->rd_clk) tick();
	}

	// Write one entry (waits for wr_clk posedge, holds wr_en for one cycle)
	bool write(uint32_t data) {
		wait_wr_posedge();
		if (dut->wr_full) return false;
		dut->wr_en = 1;
		dut->wr_data = data;
		wait_wr_posedge();
		dut->wr_en = 0;
		return true;
	}

	// Read one entry (waits for rd_clk posedge, holds rd_en for one cycle)
	bool read(uint32_t &out) {
		wait_rd_posedge();
		if (dut->rd_empty) return false;
		dut->rd_en = 1;
		dut->eval();
		out = dut->rd_data;
		wait_rd_posedge();
		dut->rd_en = 0;
		return true;
	}

	// Let synchronizers propagate (need several cycles on both clocks)
	void sync_settle() {
		for (int i = 0; i < 6; i++) {
			wait_wr_posedge();
			wait_rd_posedge();
		}
	}
};

//----------------------------------------------------------------------
// Test 1: Reset state
//----------------------------------------------------------------------
void test_reset() {
	printf("Test %d: Reset state\n", ++test_num);
	Testbench tb(7, 10);
	tb.reset();

	CHECK(tb.dut->rd_empty == 1, "rd_empty should be 1 after reset");
	CHECK(tb.dut->wr_full == 0, "wr_full should be 0 after reset");
	printf("  PASS\n");
}

//----------------------------------------------------------------------
// Test 2: Single write then read
//----------------------------------------------------------------------
void test_single_write_read() {
	printf("Test %d: Single write/read\n", ++test_num);
	Testbench tb(7, 10);
	tb.reset();

	uint32_t test_val = 0xDEADBEEF;
	bool ok = tb.write(test_val);
	CHECK(ok, "write should succeed on empty FIFO");

	// Wait for synchronizers to propagate write pointer to read domain
	tb.sync_settle();

	CHECK(tb.dut->rd_empty == 0, "rd_empty should be 0 after write + sync");

	uint32_t rd_val = 0;
	ok = tb.read(rd_val);
	CHECK(ok, "read should succeed");
	CHECK(rd_val == test_val, "read data should match written data");

	printf("  PASS\n");
}

//----------------------------------------------------------------------
// Test 3: Fill to full
//----------------------------------------------------------------------
void test_fill_to_full() {
	printf("Test %d: Fill to full\n", ++test_num);
	Testbench tb(7, 10);
	tb.reset();

	uint32_t vals[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};

	for (int i = 0; i < 4; i++) {
		bool ok = tb.write(vals[i]);
		CHECK(ok, "write should succeed");
		// Let sync propagate between writes so full flag updates
		tb.sync_settle();
	}

	// After 4 writes, FIFO should be full
	tb.sync_settle();
	CHECK(tb.dut->wr_full == 1, "wr_full should be 1 after 4 writes");

	// Verify write is blocked when full
	tb.wait_wr_posedge();
	CHECK(tb.dut->wr_full == 1, "wr_full still asserted");

	printf("  PASS\n");
}

//----------------------------------------------------------------------
// Test 4: Drain to empty
//----------------------------------------------------------------------
void test_drain_to_empty() {
	printf("Test %d: Fill then drain\n", ++test_num);
	Testbench tb(7, 10);
	tb.reset();

	uint32_t vals[4] = {0xAAAA0001, 0xBBBB0002, 0xCCCC0003, 0xDDDD0004};

	// Fill
	for (int i = 0; i < 4; i++) {
		tb.write(vals[i]);
		tb.sync_settle();
	}

	tb.sync_settle();

	// Drain and verify order
	for (int i = 0; i < 4; i++) {
		uint32_t rd_val = 0;
		bool ok = tb.read(rd_val);
		CHECK(ok, "read should succeed");
		if (rd_val != vals[i]) {
			printf("  FAIL: expected 0x%08X, got 0x%08X at index %d\n", vals[i], rd_val, i);
			fail_count++;
		}
		tb.sync_settle();
	}

	tb.sync_settle();
	CHECK(tb.dut->rd_empty == 1, "rd_empty should be 1 after draining all entries");

	printf("  PASS\n");
}

//----------------------------------------------------------------------
// Test 5: Continuous streaming with data integrity check
//----------------------------------------------------------------------
void test_continuous_streaming(int wr_hp, int rd_hp, int wr_gap, int count) {
	printf("Test %d: Streaming (%d samples, wr_hp=%d rd_hp=%d wr_gap=%d)\n",
	       ++test_num, count, wr_hp, rd_hp, wr_gap);
	Testbench tb(wr_hp, rd_hp);
	tb.reset();

	std::queue<uint32_t> expected;
	int written = 0, read_count = 0;
	int wr_cooldown = 0;
	int rd_cooldown = 0;
	int rd_gap = wr_gap * 2;  // read slower than write
	bool rd_was_active = false; // rd_en was asserted before this edge

	for (uint64_t t = 0; t < (uint64_t)count * 400; t++) {
		int edges = tb.tick();
		bool wr_rose = edges & 1;
		bool rd_rose = edges & 2;

		// Write side: on wr_clk rising edge, the FIFO sampled wr_en
		// from BEFORE this tick. Now set up for the next edge.
		if (wr_rose) {
			tb.dut->wr_en = 0;
			if (wr_cooldown > 0) {
				wr_cooldown--;
			} else if (written < count && !tb.dut->wr_full) {
				uint32_t val = 0xA0000000 | written;
				tb.dut->wr_en = 1;
				tb.dut->wr_data = val;
				expected.push(val);
				written++;
				wr_cooldown = wr_gap;
			}
		}

		// Read side: on rd_clk rising edge, if rd_en was asserted
		// before this edge, the FIFO consumed an entry. Use
		// pre_edge_rd_data which was captured before the pointer
		// advanced.
		if (rd_rose) {
			if (rd_was_active) {
				uint32_t rd_val = tb.pre_edge_rd_data;
				if (expected.empty()) {
					printf("  FAIL: read data but expected queue empty\n");
					fail_count++;
				} else {
					uint32_t exp = expected.front();
					expected.pop();
					if (rd_val != exp) {
						printf("  FAIL: expected 0x%08X, got 0x%08X (sample %d)\n",
						       exp, rd_val, read_count);
						fail_count++;
						return;
					}
				}
				read_count++;
				rd_was_active = false;
			}

			// Set up next read
			tb.dut->rd_en = 0;
			if (rd_cooldown > 0) {
				rd_cooldown--;
			} else if (!tb.dut->rd_empty) {
				tb.dut->rd_en = 1;
				rd_was_active = true;
				rd_cooldown = rd_gap;
			}
		}

		if (read_count >= count) break;
	}

	CHECK(read_count == count, "should have read all samples");
	printf("  read %d/%d samples OK\n", read_count, count);
	printf("  PASS\n");
}

//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);

	printf("=== audio_cdc_fifo testbench ===\n\n");

	test_reset();
	test_single_write_read();
	test_fill_to_full();
	test_drain_to_empty();

	// Clock ratio sweep: wr_clk faster, equal, slower
	test_continuous_streaming(5, 13, 3, 500);   // wr ~2.6x faster
	test_continuous_streaming(7, 7, 4, 500);     // equal
	test_continuous_streaming(11, 5, 2, 500);    // wr ~2.2x slower
	test_continuous_streaming(3, 11, 5, 1000);   // wr ~3.7x faster, more samples

	printf("\n=== Results: %d tests, %d failures ===\n",
	       test_num, fail_count);

	return fail_count ? 1 : 0;
}
