# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MiSTer FPGA core template for the DE10-Nano (Cyclone V 5CSEBA6U23I7). Developers use this as a starting point to build new cores or port existing ones to the MiSTer platform. The framework (`sys/`) handles all hardware interfacing; developers implement their core logic in `rtl/` and wire it through the top-level glue module.

## Build System

- **Toolchain**: Intel Quartus Prime v17.0.x (v17.0.2 recommended). Newer versions are incompatible.
- **Compilation**: Open `Template.qpf` in Quartus and run full compilation. Output is an `.rbf` file.
- **Build ID**: `sys/build_id.tcl` runs as a pre-flow script to generate `build_id.v` (timestamp) and `jtag.cdf`.
- **Variant builds**: `Template_Q13.qsf` exists for Quartus v13 compatibility.
- **Clean**: `clean.bat` removes Quartus temporaries (Windows only).
- No test infrastructure or CI exists in this repository.

## Architecture

### Two-Layer Design

1. **Framework layer (`sys/`)**: ~40 HDL modules providing HPS communication, video scaling/output (HDMI, VGA, OSD), audio mixing/output (I2S, S/PDIF), SDRAM/DDR3 interfaces, SD card, and input handling. **Do not modify files in `sys/`** — framework updates will overwrite changes.

2. **Core layer (`rtl/`)**: Developer-owned source files. The template includes a demo core (`mycore.v`) generating a test pattern with noise (via `lfsr.v`) and cosine lookup (`cos.sv`).

### Signal Flow

`sys_top.v` (actual FPGA top) → `emu` module (`Template.sv`, the glue logic) → `hps_io` (HPS communication) + `mycore` (core logic)

- `hps_io` decodes the HPS bus to provide joystick inputs, button presses, OSD status/config, and SD card/download interfaces
- The PLL in `rtl/pll/` generates `clk_sys` from the 50MHz input
- The core produces video and audio signals; the framework handles scaling, mixing, and output

### Key Files to Customize When Building a New Core

| File | Purpose |
|------|---------|
| `Template.sv` | Glue logic: config strings, hps_io instantiation, core instantiation, video/audio routing |
| `rtl/*.v` / `rtl/*.sv` | Core implementation (replace mycore.v with your own) |
| `files.qip` | Must manually list all core source files |
| `Template.qsf` | Quartus settings, optional feature macros |
| `Template.sdc` | Timing constraints for your core's clocks |

### Optional Feature Macros (defined in .qsf)

Enable by uncommenting `set_global_assignment -name VERILOG_MACRO` lines in `Template.qsf`:

- `MISTER_FB` — Framebuffer support (direct video to DDR3)
- `MISTER_FB_PALETTE` — Framebuffer with 8-bit palette mode
- `MISTER_DUAL_SDRAM` — Dual SDRAM I/O board support
- `MISTER_DEBUG_NOHDMI` — Disable HDMI for debugging
- `MISTER_SMALL_VBUF` — Smaller video buffer
- `MISTER_DOWNSCALE_NN` — Nearest-neighbor downscaling
- `MISTER_DISABLE_ADAPTIVE` — Disable adaptive scanlines

## Timing and CDC Patterns

The framework has two main clock domains: `clk_sys` (HPS/ARM side, 100MHz) and the video clock (`CLK_VIDEO` / `clk_vid` / `clk`, derived from core PLL). Several framework modules cross between them. When modifying `sys/` or adding features that touch these domains:

### Clock Domain Crossings

- **Control signals** (1-bit, quasi-static): Use a 2-FF synchronizer into the destination domain. Examples: `gamma_en`, `osd_enable`, `mask_enable`.
- **Multi-bit configuration** (dimensions, positions): These change only on user menu interaction. Use SDC `set_false_path` constraints rather than synchronizers, since individual bits may arrive at different times but the values are stable for thousands of cycles before use.
- **Dual-clock RAMs** (LUTs written on `clk_sys`, read on video clk): Use `(* ramstyle="no_rw_check" *)` and add SDC false paths from the RAM write registers to the read-side registers. Data is loaded at boot/config time, stable before reads.
- **`gamma_bus[20]` carries `clk_sys` on general routing** (not a dedicated clock network). SDC false paths on gamma write paths compensate for the fitter not tracing this back to `h2f_user0_clk`.

### Combinational Path Guidelines

- **Prefer explicit multiplies over shift-and-add chains** for constant multiplication. Quartus maps explicit multiplies to DSP blocks (~2ns) instead of building fabric adder trees (~9ns for 6-input chains). The Cyclone V has 112 DSP blocks.
- **Register RAM read outputs** in a separate always block before using them in output muxes. Combinational RAM reads through muxes prevent Quartus from using M10K synchronous read ports.
- **When `ce_pix`/`ce_in` gates logic**, combinational paths between registers still must meet full `CLK_VIDEO` timing. Register wide mux outputs on every `clk` edge (ungated) so downstream combinational logic has multiple cycles to settle before the next `ce_in`.

### SDC Constraints

- `sys_top.sdc`: Framework-level constraints (clock groups, pin false paths, OSD/scaler relaxations). Declares `clk_sys` and video PLL clocks as exclusive groups.
- `Template.sdc`: Core-specific constraints. Add false paths here for any new CDC paths your core introduces (gamma, shadowmask, OSD config signals all have entries).

## Conventions

- Release binaries go in `releases/` with format `<core_name>_YYYYMMDD.rbf`
- The top-level Quartus entity is always `sys_top` (defined in sys, not user code)
- The user module is named `emu` in the top-level `.sv` file
- Config strings in `Template.sv` define the OSD menu structure for MiSTer
