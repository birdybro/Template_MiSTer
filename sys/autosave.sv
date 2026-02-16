//============================================================================
//
//  Autosave — adaptive save-to-SD for MiSTer cores
//
//  Two operating modes selected by the SIMPLE parameter:
//
//  ── SIMPLE=0 (default): Full upload mode ──────────────────────
//  Drives the ioctl_upload_req path in hps_io.  The module manages
//  the complete upload lifecycle: requests upload, serves save data
//  byte-by-byte via ioctl_din, and tracks completion.  Use for cores
//  whose save memory is directly readable (BRAM, SDRAM, DDR3) and
//  that use the standard ioctl upload interface (e.g. NES, MegaDrive).
//
//  ── SIMPLE=1: Trigger-only mode ───────────────────────────────
//  Outputs a one-cycle save_trigger pulse when the backoff timer
//  decides it is time to save.  The core's own save infrastructure
//  handles the actual data transfer (SD block interface, DMA, etc.).
//  The module does not touch ioctl_upload_req or serve data.  Use
//  for cores that already have a save pipeline (e.g. PSX memcard).
//
//  Shared behaviour (both modes):
//    - A prescaler divides the system clock into coarser ticks.
//      All timers count in ticks, keeping registers narrow (~15 bits
//      instead of ~31 bits for a 50 MHz clock at 30 s max interval).
//    - Every INTERVAL ticks, check the dirty flag.
//    - If dirty:  initiate save, double the interval.
//    - If clean:  halve the interval (minimum MIN_TICKS).
//    - Interval is capped at MAX_TICKS.
//    - Flush is deferred while the user is actively providing input;
//      saves only fire after a period of input inactivity
//      (enable with IDLE_GUARD=1, disable with IDLE_GUARD=0).
//    - An enable input allows the core or OSD to gate autosave on/off.
//    - A saving status output can drive an OSD indicator or LED.
//
//  Timing parameters:
//    The developer chooses PRESCALE to set the tick rate, then
//    specifies all intervals in ticks.  Example for a 50 MHz clock:
//
//      PRESCALE  = 50_000   → 1 tick = 1 ms
//      MIN_TICKS = 1_000    → 1 second
//      MAX_TICKS = 30_000   → 30 seconds
//      IDLE_TICKS= 1_000    → 1 second
//
//  Result:
//    - Well-behaved saves (single burst) are captured within
//      MIN_TICKS ticks.
//    - Continuous writers quickly ramp to the cap (MAX_TICKS),
//      limiting SD writes while still capturing data regularly.
//    - Once writes stop, the interval shrinks back for fast response.
//
//  No custom SPI commands. No Linux-side changes.
//
//============================================================================
//
// MIT License
//
// Copyright (c) 2026 Kevin Coleman
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//============================================================================

module autosave #(
	parameter SIMPLE     = 0,             // 0 = full ioctl_upload mode, 1 = trigger-only mode
	parameter PRESCALE   = 50_000,        // Clock divider: 1 tick = PRESCALE clk_sys cycles
	parameter MIN_TICKS  = 1_000,         // Minimum save interval in ticks
	parameter MAX_TICKS  = 30_000,        // Maximum save interval in ticks
	parameter IDLE_TICKS = 1_000,         // Input inactivity timeout in ticks
	parameter IDLE_GUARD = 1,             // 1 = defer saves during input activity, 0 = ignore input
	parameter ADDR_W     = 27,            // Save memory address width (e.g. 13 for 8KB BRAM)
	parameter DATA_W     = 8,             // Save data width: 8 for normal, 16 for WIDE hps_io
	parameter RD_TIMEOUT = 1_000_000      // Read watchdog in raw clock cycles (0 = disabled)
) (
	input             clk_sys,
	input             reset,

	// ── Core interface ──────────────────────────────────────────
	// High enables autosave; low disables it (e.g. active-high
	// from an OSD toggle).  When disabled the module is inert:
	// no uploads are requested and the dirty flag is ignored.
	input             enable,

	// Pulse or strobe indicating a write to save memory.
	// Connect to the write-enable of your core's battery-backed
	// save RAM (BRAM, SDRAM, DDR3).  For cores that expose a
	// level signal (e.g. bk_pending), use a rising-edge detector
	// so that dirty is set once per write burst, not held.
	input             save_ram_wr,

	// Directly connect to any signal that changes on user input
	// (e.g. |joystick_0, ps2_key[10] toggle, etc.). When high or
	// toggling, saves are deferred until activity stops.
	// Ignored when IDLE_GUARD=0.
	input             user_active,

	// ── Trigger interface (SIMPLE=1 only) ───────────────────────
	// save_trigger: one-cycle pulse requesting the core to save.
	//   Always low when SIMPLE=0; use ioctl_upload_req instead.
	// save_busy: core holds high while its own save pipeline is
	//   active. Suppresses triggers and gates the countdown timer.
	//   Ignored when SIMPLE=0. Tie to 1'b0 if unused.
	output reg        save_trigger,
	input             save_busy,

	// ── Save memory read port (SIMPLE=0 only) ───────────────────
	// During upload the HPS reads data one word at a time
	// (byte for DATA_W=8, word for DATA_W=16).
	//
	// save_addr   — address presented to your save memory
	// save_rd     — one-cycle read strobe
	// save_data   — data from your memory (DATA_W bits wide)
	// save_valid  — data on save_data is valid this cycle
	//
	// For BRAM:   assign save_data = bram_q;  save_valid = 1'b1;
	// For SDRAM:  assign save_data = sdram_dout[DATA_W-1:0];
	//             assign save_valid = sdram_ack;
	// For DDR3:   assign save_data = DDRAM_DOUT[DATA_W-1:0];
	//             assign save_valid = DDRAM_DOUT_READY;
	//
	output reg [ADDR_W-1:0] save_addr,
	output reg             save_rd,
	input       [DATA_W-1:0] save_data,
	input             save_valid,

	// ── hps_io interface (SIMPLE=0 only) ────────────────────────
	// Connect these directly to the corresponding hps_io ports.
	//
	// core_wait: the core's own ioctl_wait for downloads (e.g.
	//   active while SDRAM write completes during ROM load).
	//   The module ORs it with its own upload-wait so that a
	//   single ioctl_wait wire can be shared without multi-driver
	//   conflicts.  Tie to 1'b0 if the core doesn't use ioctl_wait.
	input             core_wait,
	input             ioctl_upload,
	output reg        ioctl_upload_req,
	input             ioctl_rd,
	output reg  [DATA_W-1:0] ioctl_din,
	output            ioctl_wait,

	// ── Status ─────────────────────────────────────────────────
	// High while a save upload is in progress. Can be used to
	// drive an OSD save indicator, LED, or activity icon.
	output            saving
);

// ---------------------------------------------------------------------------
// Timer and interval sizing
//
// A shared prescaler divides clk_sys so that all interval timers count
// in ticks rather than raw clock cycles.  This keeps timer registers
// narrow (~15 bits for 30 000 ticks) instead of wide (~31 bits for
// 1.5 billion cycles at 50 MHz).
// ---------------------------------------------------------------------------
localparam SAFE_MIN  = (MIN_TICKS > 0)  ? MIN_TICKS  : 1;
localparam SAFE_PRE  = (PRESCALE  > 1)  ? PRESCALE   : 2;
localparam SAFE_IDLE = (IDLE_TICKS > 0) ? IDLE_TICKS : 1;

localparam PRE_W    = $clog2(SAFE_PRE);
localparam TMR_W    = $clog2(MAX_TICKS + 1);
localparam IDLE_W   = $clog2(SAFE_IDLE + 1);
localparam RD_TMO_W = (RD_TIMEOUT > 0) ? $clog2(RD_TIMEOUT + 1) : 1;

// ---------------------------------------------------------------------------
// Prescaler — shared tick for all timers
// ---------------------------------------------------------------------------
reg [PRE_W-1:0] prescale_cnt;
wire            tick = (prescale_cnt == 0);

always @(posedge clk_sys) begin
	if (reset || prescale_cnt == 0)
		prescale_cnt <= SAFE_PRE[PRE_W-1:0] - 1'd1;
	else
		prescale_cnt <= prescale_cnt - 1'd1;
end

// ---------------------------------------------------------------------------
// User activity tracking — edge-detect any change on user_active
// When IDLE_GUARD=0 the logic is eliminated and user_idle is always 1.
// ---------------------------------------------------------------------------
wire user_idle_w;

generate if (IDLE_GUARD) begin : gen_idle
	reg              prev_user_active;
	reg [IDLE_W-1:0] idle_timer;
	reg              user_idle;

	always @(posedge clk_sys) begin
		if (reset) begin
			prev_user_active <= 0;
			idle_timer       <= 0;
			user_idle        <= 1;
		end else begin
			prev_user_active <= user_active;

			if (user_active != prev_user_active) begin
				// Activity detected — reset idle countdown
				idle_timer <= SAFE_IDLE[IDLE_W-1:0];
				user_idle  <= 0;
			end else if (idle_timer != 0) begin
				if (tick) idle_timer <= idle_timer - 1'd1;
			end else begin
				user_idle <= 1;
			end
		end
	end

	assign user_idle_w = user_idle;
end else begin : gen_no_idle
	assign user_idle_w = 1'b1;
end endgenerate

// ---------------------------------------------------------------------------
// Combined ioctl_wait — OR core's download-wait with upload save-wait
// ---------------------------------------------------------------------------
reg save_wait;
assign ioctl_wait = core_wait | save_wait;

// ---------------------------------------------------------------------------
// Status output
// ---------------------------------------------------------------------------
assign saving = SIMPLE ? save_busy : ioctl_upload;

// ---------------------------------------------------------------------------
// Dirty flag — set by core writes, cleared when a flush is initiated
// ---------------------------------------------------------------------------
reg save_dirty;
reg save_flush;        // single-cycle pulse to clear dirty

always @(posedge clk_sys) begin
	if (reset)
		save_dirty <= 0;
	else if (save_flush)
		save_dirty <= 0;
	else if (save_ram_wr)
		save_dirty <= 1;
end

// ---------------------------------------------------------------------------
// Adaptive timer with exponential backoff
//
// The interval and timer are tracked in prescaled ticks so that
// doubling and halving are just shifts — no multipliers needed.
// ---------------------------------------------------------------------------
reg  [TMR_W-1:0] save_timer;
reg  [TMR_W-1:0] interval_ticks;   // current interval in prescaled ticks
reg               save_requested;

// Next doubled / halved interval — pure shifts and compares.
wire [TMR_W-1:0] dbl_interval =
	(interval_ticks >= MAX_TICKS[TMR_W-1:0]) ? MAX_TICKS[TMR_W-1:0] :
	((interval_ticks << 1) > MAX_TICKS[TMR_W-1:0]) ? MAX_TICKS[TMR_W-1:0] :
	(interval_ticks << 1);

wire [TMR_W-1:0] half_interval =
	(interval_ticks <= SAFE_MIN[TMR_W-1:0]) ? SAFE_MIN[TMR_W-1:0] :
	(interval_ticks >> 1);

// True when the interval has reached the cap — used for max-deferral
// override (force save even if user is active).
wire at_max_interval = (interval_ticks >= MAX_TICKS[TMR_W-1:0]);

always @(posedge clk_sys) begin
	if (reset || !enable) begin
		// First check fires after MIN_TICKS to catch leftover dirty
		// state quickly (e.g. after a crash or re-enable).
		save_timer       <= SAFE_MIN[TMR_W-1:0];
		interval_ticks   <= SAFE_MIN[TMR_W-1:0];
		save_flush       <= 0;
		save_requested   <= 0;
		ioctl_upload_req <= 0;
		save_trigger     <= 0;
	end else begin
		save_flush   <= 0;
		save_trigger <= 0;

		if (SIMPLE) begin
			// ── SIMPLE mode: pulse save_trigger, reload immediately ──

			// Count down while no save is in progress
			if (!save_busy && tick && save_timer != 0)
				save_timer <= save_timer - 1'd1;

			// Timer expired — check dirty flag
			if (!save_busy && save_timer == 0) begin
				if (save_dirty && (user_idle_w || at_max_interval)) begin
					// Dirty and idle (or max-deferred): trigger save
					save_trigger   <= 1;
					save_flush     <= 1;
					interval_ticks <= dbl_interval;
					save_timer     <= dbl_interval;
				end else if (save_dirty) begin
					// Dirty but user active: defer, double toward cap
					interval_ticks <= dbl_interval;
					save_timer     <= dbl_interval;
				end else begin
					// Clean: halve the interval, reload timer
					interval_ticks <= half_interval;
					save_timer     <= half_interval;
				end
			end

		end else begin
			// ── COMPLEX mode: drive ioctl_upload_req, serve data ──

			// Deassert upload request once HPS starts the transfer
			if (ioctl_upload)
				ioctl_upload_req <= 0;

			// Count down to next check (prescaled)
			if (!save_requested && !ioctl_upload && tick && save_timer != 0)
				save_timer <= save_timer - 1'd1;

			// Timer expired — check dirty flag
			if (!save_requested && !ioctl_upload && save_timer == 0) begin
				if (save_dirty && (user_idle_w || at_max_interval)) begin
					// Dirty and idle (or max-deferred): flush now, double interval
					ioctl_upload_req <= 1;
					save_requested   <= 1;
					save_flush       <= 1;
					interval_ticks   <= dbl_interval;
				end else if (save_dirty) begin
					// Dirty but user active: defer, double toward cap
					interval_ticks <= dbl_interval;
					save_timer     <= dbl_interval;
				end else begin
					// Clean: halve the interval, reload timer
					interval_ticks <= half_interval;
					save_timer     <= half_interval;
				end
			end

			// Upload finished — reload timer with current interval.
			// Note: interval_ticks was already doubled when the save
			// was requested, so it reflects the post-doubling value.
			if (save_requested && !ioctl_upload && !ioctl_upload_req) begin
				save_requested <= 0;
				save_timer     <= interval_ticks;
			end
		end
	end
end

// ---------------------------------------------------------------------------
// Serve save data to HPS during upload (SIMPLE=0 only)
//
// Note: if save_ram_wr fires during an active upload, the dirty flag
// re-sets immediately.  The data being uploaded may or may not include
// the new write (depends on whether that address was already read).
// The next scheduled save will capture any missed writes.
// ---------------------------------------------------------------------------
generate if (!SIMPLE) begin : gen_data_serve
	// On ioctl_rd: issue a read to save memory, hold ioctl_wait.
	// When save_valid: latch data into ioctl_din, release ioctl_wait, advance.
	reg save_rd_pending;
	reg [RD_TMO_W-1:0] rd_watchdog;

	always @(posedge clk_sys) begin
		save_rd <= 0;

		if (!ioctl_upload) begin
			save_addr       <= 0;
			save_rd_pending <= 0;
			save_wait       <= 0;
			rd_watchdog     <= 0;
		end else if (ioctl_rd && !save_rd_pending) begin
			save_rd         <= 1;
			save_rd_pending <= 1;
			save_wait       <= 1;
			rd_watchdog     <= (RD_TIMEOUT > 0) ? RD_TIMEOUT[RD_TMO_W-1:0] : '0;
		end else if (save_rd_pending && save_valid) begin
			ioctl_din       <= save_data;
			save_addr       <= save_addr + 1'd1;
			save_rd_pending <= 0;
			save_wait       <= 0;
			rd_watchdog     <= 0;
		end else if (save_rd_pending && RD_TIMEOUT > 0 && rd_watchdog == 0) begin
			// Watchdog: memory never responded — release wait, feed zero
			ioctl_din       <= '0;
			save_addr       <= save_addr + 1'd1;
			save_rd_pending <= 0;
			save_wait       <= 0;
		end else if (save_rd_pending && RD_TIMEOUT > 0) begin
			rd_watchdog <= rd_watchdog - 1'd1;
		end
	end
end else begin : gen_simple_stub
	// SIMPLE mode: data-serving ports held inactive
	always @(posedge clk_sys) begin
		save_rd   <= 0;
		save_addr <= 0;
		save_wait <= 0;
		ioctl_din <= 0;
	end
end endgenerate

endmodule
