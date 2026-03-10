# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Template_MiSTer is the reference FPGA core template for the MiSTer platform (DE10-Nano / Cyclone V). It provides the shared framework (`sys/`) that all MiSTer cores must include unmodified, plus a demo core showing how to integrate with the framework.

## Build System

- **Quartus v17.0.2** (Standard or Lite edition) — no newer versions
- No Makefile; builds are done through the Quartus IDE
- Project files: `Template.qpf` (project), `Template.qsf` (settings), `Template.sdc` (constraints)
- `files.qip` lists all core-specific source files; edit manually when adding/removing files
- `clean.bat` removes temporary build artifacts
- Output format: `<corename>_YYYYMMDD.rbf`

## Architecture

### Key Boundary: `sys/` vs Everything Else

**`sys/` is the shared framework — do not modify it for core-specific reasons.** All MiSTer cores share the same `sys/` folder. Framework updates may overwrite changes. Core-specific work goes in `rtl/` and the top-level glue file.

### Module Hierarchy

- **`sys/sys_top.v`** — Hardware abstraction layer. Manages physical I/O (HDMI, VGA, SDRAM, buttons), clock domains, video/audio pipeline orchestration, and PLL reconfiguration. ~1,940 lines.
- **`Template.sv` (`emu` module)** — Glue logic between framework and core. Developers adapt this file to connect their core's signals to the framework interface. Instantiates `hps_io`, the core PLL, and the core itself.
- **`sys/hps_io.sv`** — HPS (ARM CPU) communication: joystick/keyboard/mouse input, SD card access, ioctl firmware download, status registers, config string parsing.
- **`rtl/mycore.v`** — Demo core implementation. Replace with actual core logic.

### Clock Domains

The framework manages multiple clock domains:
- `FPGA_CLK1_50` — 50 MHz board input clock
- `clk_sys` — System clock (typically 100 MHz, from core PLL)
- `clk_vid` — Video clock (from core, assigned to `CLK_VIDEO`)
- `clk_audio` — 24.576 MHz (framework audio PLL)
- `clk_hdmi` — HDMI pixel clock (framework HDMI PLL)

Signals crossing domains require proper CDC synchronization. See the CDC patterns section below.

### Video Pipeline

Core video output → `video_mixer` → `video_cleaner` → effects (shadowmask, scanlines, hq2x) → `gamma_corr` → output (VGA/HDMI/YC)

### Audio Pipeline

Core audio → `audio_out` (async FIFO, clk_core→clk_audio) → I2S / SPDIF / ALSA

### Verilog Macros

Define these in the QSF to enable optional framework features:

| Macro | Effect |
|---|---|
| `MISTER_FB` | Enable framebuffer from core |
| `MISTER_FB_PALETTE` | Framebuffer palette mode |
| `MISTER_DEBUG_NOHDMI` | Disable HDMI modules (faster compile) |
| `MISTER_DUAL_SDRAM` | Dual SDRAM I/O board pin configuration |
| `MISTER_SMALL_VBUF` | Smaller ASCAL video buffer |
| `MISTER_DOWNSCALE_NN` | Nearest-neighbor downscaling |
| `MISTER_DISABLE_ADAPTIVE` | Disable adaptive scanlines |

## HDL Conventions

- Mix of Verilog (`.v`), SystemVerilog (`.sv`), and VHDL (`.vhd` for ASCAL)
- Core PLL must be in `rtl/pll/` with `pll.v` and `pll.qip` present
- When porting a core: copy this template, rename project files to `<corename>.*`, edit `files.qip` for your sources, and adapt `Template.sv` (`emu` module)

## CDC (Clock Domain Crossing) Patterns

When fixing or adding signals that cross clock domains, use these established patterns:

1. **2-FF synchronizer** — Single-bit signals (e.g., enables, flags)
2. **Async FIFO** (`async_fifo.v`) — Streaming data (e.g., audio samples). Parameterized DATA_WIDTH/ADDR_WIDTH
3. **Snapshot + toggle handshake** — Multi-bit status/config: write to snapshot registers, toggle a flag, 2-FF sync the toggle, capture on detected edge
4. **Deferred toggle (pend pattern)** — Like snapshot+toggle but adds 1-cycle delay to ensure data stability before the toggle fires
5. **Gray-code counters** — Small counters crossing domains (e.g., 2-bit address pointers)

Quasi-static config signals from HPS (WIDTH, HEIGHT, ARX, etc.) are intentionally left unsynchronized as they change infrequently and settle long before use.
