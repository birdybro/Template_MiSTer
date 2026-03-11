# Video Pipeline Timing Fixes

## Overview

Four fixes targeting timing closure issues across the MiSTer framework video pipeline modules. These address the same classes of problems found in `gamma_corr.sv` (CDC violations, deep combinational paths) but in other modules throughout `sys/`.

---

## Fix 1: shadowmask.sv — CDC synchronizers (`17f31d1`)

**File:** `sys/shadowmask.sv`

**Problem:** Five control signals (`mask_enable`, `mask_rotate`, `mask_2x`, `hmax`, `vmax`) are written in `clk_sys` (lines 120-134, HPS configuration commands) but used directly in the `clk` video domain (lines 47-54, 79) with no synchronization. The `mask_lut` RAM (256×11) is also written on `clk_sys` and read on `clk`.

**Fix:**
- Added 2-FF synchronizers for all five control signals into the `clk` domain
- Replaced all video-domain usages with synchronized versions (`mask_enable_s2`, `mask_rotate_s2`, `mask_2x_s2`, `hmax_s2`, `vmax_s2`)
- Added SDC false paths for `mask_lut` RAM CDC and all five control signal synchronizer inputs

**Impact:** Eliminates metastability risk on control signals that feed into the shadowmask arithmetic pipeline (lines 102-107). Fitter no longer wastes effort on cross-domain paths.

---

## Fix 2: osd.v — CDC synchronizers and false paths (`a064212`)

**File:** `sys/osd.v`

**Problem:** Multiple signals cross from `clk_sys` to `clk_video` without synchronization:
- `osd_enable` (1-bit) — controls whether OSD is rendered, used in video output path
- `info` (1-bit) — selects info display mode, feeds many muxes in video domain
- `osd_h`, `osd_t`, `osd_w` (22-bit each) — OSD dimensions
- `infox`, `infoy`, `infow`, `infoh` (up to 22-bit) — info panel position/size
- `osd_buffer` (4096+ entry RAM) — OSD pixel data

`sys_top.sdc` already covered `rot` and some `v_osd_start` signals with false paths, but missed all of these.

**Fix:**
- Added 2-FF synchronizers for `osd_enable` and `info` (1-bit signals in critical paths)
- Replaced all video-domain usages with `osd_enable_s2` and `info_s2`
- Added SDC false paths for multi-bit configuration signals and `osd_buffer` RAM

**Impact:** The two most timing-critical CDC signals (`osd_enable`, `info`) are properly synchronized. Multi-bit quasi-static signals get false paths so the fitter doesn't attempt cross-domain closure on 22-bit comparison paths.

---

## Fix 3: yc_out.sv — DSP-friendly multiplies (`ff8fb57`)

**File:** `sys/yc_out.sv`

**Problem:** Luma and chroma scaling used manual shift-and-add to implement constant multiplication:
- Line 131: `yr` = red×306 via 4 additions of shifted 8-bit values
- Line 132: `yg` = green×601 via 5 additions
- Line 133: `yb` = blue×117 via 5 additions
- Line 156: `phase[1].u` = phase[0].u×504 via **6 additions of 29-bit values**
- Line 157: `phase[1].v` = phase[0].v×898 via 4 additions of 30-bit values

The 6-input adder tree on line 156 is the worst case: the synthesizer builds 3 levels of wide adders in fabric logic, creating ~9ns of combinational delay.

**Fix:** Replaced all shift-and-add chains with explicit multiply operations:
```systemverilog
yr <= red * 21'd306;        // was: {red,8'd0} + {red,5'd0} + {red,4'd0} + {red,1'd0}
yg <= green * 21'd601;      // was: 5-term shift-and-add
yb <= blue * 21'd117;       // was: 5-term shift-and-add
phase[1].u <= 21'(phase[0].u * 21'sd504);  // was: 6-term shift-and-add
phase[1].v <= 21'(phase[0].v * 21'sd898);  // was: 4-term shift-and-add
```

**Impact:** Quartus infers DSP blocks (18×18 multipliers) instead of fabric adder trees. Uses 5 of 112 available DSP blocks on the Cyclone V. DSP multiply path is ~2ns vs ~9ns for the adder tree. Largest single timing improvement in this set.

---

## Fix 4: hq2x.sv — Register mux outputs before DiffCheck (`d30ed23`)

**File:** `sys/hq2x.sv`

**Problem:** Lines 69-70 feed 24-bit 4:1 muxes directly into DiffCheck modules, creating a deep combinational chain:

```
cyc register → 4:1 mux (~4ns) → DiffCheck: 3 subtractions + 2 additions + 3 comparisons + AND (~7ns) → pattern register
```

Total: ~11ns combinational path. At CLK_VIDEO=148.5MHz (6.7ns period), this cannot close.

**Fix:** Register the mux outputs in an always block running every `clk` edge (not gated by `ce_in`):
```systemverilog
always @(posedge clk) begin
    diff_input0 <= (cyc == 0) ? Prev0 : (cyc == 1) ? Curr0 : (cyc == 2) ? Prev2 : Next1;
    diff_input1 <= (cyc == 0) ? Prev1 : (cyc == 1) ? Next0 : (cyc == 2) ? Curr2 : Next2;
end
DiffCheck diffcheck0(Curr1, diff_input0, diff0);
DiffCheck diffcheck1(Curr1, diff_input1, diff1);
```

Since `ce_in` fires at most every 4th `clk` cycle (`CLK_VIDEO >= ce_pix*4` is required for hq2x operation), the registered mux values settle within 1 `clk` cycle, leaving 3+ cycles for DiffCheck to propagate before the next `ce_in` captures `pattern <= new_pattern`.

**Impact:** Splits the ~11ns path into two shorter paths: mux→register (~4ns) and DiffCheck→register (~7ns). The DiffCheck path runs freely at `clk` speed with multiple cycles to settle.

---

## Summary

| Fix | Module | Type | DSP Cost | Latency Added |
|-----|--------|------|----------|---------------|
| CDC synchronizers | shadowmask.sv | 2-FF sync + SDC | 0 | 2 clk cycles (invisible) |
| CDC synchronizers | osd.v | 2-FF sync + SDC | 0 | 2 clk cycles (invisible) |
| DSP multiplies | yc_out.sv | Architecture | 5 DSP blocks | None |
| Register mux outputs | hq2x.sv | Pipeline register | 0 | 1 clk cycle (absorbed) |
