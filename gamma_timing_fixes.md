# Gamma Module Timing Fixes

## Background

The `gamma_corr.sv` file contains two modules (`gamma_corr` and `gamma_fast`) that apply gamma correction LUTs to the video pipeline. Both modules had clock domain crossing (CDC) issues and architectural patterns that made timing closure harder for the Quartus fitter, causing developers to avoid enabling the GAMMA parameter.

## Commits

### 1. Pipeline RAM reads in `gamma_fast` (`67a3094`)

**File:** `sys/gamma_corr.sv` (lines 120-125)

**Problem:** Three gamma LUT reads (`gamma_curve_r/g/b[index]`) fed through a 24-bit mux directly into the `RGB_out` output register, all in a single combinational path (old line 116). On Cyclone V, this forces Quartus to either use unregistered M10K block RAM outputs or fall back to ALM-based distributed RAM — both of which create long critical paths that eat into timing slack.

**Fix:** Split RAM reads into their own `always @(posedge clk_vid)` block with dedicated output registers (`gamma_rd_r/g/b`). The reads run every `clk_vid` cycle (not gated by `ce_pix`), so data is stable well before the next pixel enable. The output mux now selects between registered values only.

**Impact:** Eliminates the longest combinational path in the gamma module. Allows Quartus to use synchronous M10K read ports with their built-in output registers.

### 2. SDC false path constraints for gamma CDC (`53d5ba6`)

**File:** `Template.sdc`

**Problem:** The gamma LUT RAM is written in `clk_sys` and read in `clk_vid`. While `sys_top.sdc` declares these clock groups as exclusive, `clk_sys` reaches the gamma module through `gamma_bus[20]` on general routing fabric — the fitter may not trace it back to `h2f_user0_clk`. Without explicit constraints, the timing analyzer attempts to close impossible cross-domain paths, wasting placement/routing effort that degrades timing on real paths.

**Fix:** Added three `set_false_path` constraints:
- `gamma_curve` RAM write-to-read path (LUT data is loaded at boot, stable before video reads)
- `gamma_curve` RAM to `gamma_rd` pipeline registers (new pipeline stage from Fix 1)
- `gamma_en` register to `RGB_out` (quasi-static control signal)

**Impact:** Fitter no longer wastes effort on uncloseable CDC paths. More routing resources available for legitimate timing-critical paths in the core.

### 3. 2-FF synchronizer for `gamma_en` (`ce12fca`)

**File:** `sys/gamma_corr.sv` (lines 24-29 in `gamma_corr`, lines 102-107 in `gamma_fast`)

**Problem:** `gamma_en` originates in `clk_sys` (written by hps_io on user menu interaction) and was used directly in `clk_vid` as the select line for the 24-bit RGB output mux. This is a textbook CDC violation — metastability on a wide mux select can cause both branches to conduct momentarily, producing glitched outputs and unpredictable fitter behavior.

**Fix:** Added a standard 2-stage flip-flop synchronizer (`gamma_en_s1` → `gamma_en_s2`) clocked by `clk_vid` in both modules. The synchronized signal `gamma_en_s2` is used for the output mux select. Since `gamma_en` only changes on user menu interaction (never mid-frame), the 2-cycle synchronization latency is invisible.

**Impact:** Eliminates metastability risk on the output mux select. The fitter only needs to close timing within `clk_vid` for this path, not across clock domains.

## Architecture Note

Fix #4 from the original analysis (clk_sys on general routing via gamma_bus) is structural to the MiSTer framework's gamma_bus interface and cannot be changed without breaking the API. The SDC constraints in Fix #2 mitigate the timing impact by preventing the fitter from attempting to close paths involving that clock.

## Summary

| Fix | Severity Addressed | Technique | Latency Added |
|-----|-------------------|-----------|---------------|
| Pipeline RAM reads | Critical | Register stage between RAM and output mux | 1 clk_vid cycle |
| SDC false paths | High | Timing constraint exclusions | None |
| gamma_en synchronizer | High | 2-FF CDC synchronizer | 2 clk_vid cycles |
