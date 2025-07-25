# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is a template core for the MiSTer FPGA platform - an open-source implementation designed for developers to create retro computer/console cores. The project is built using Intel Quartus Prime 17.0.x and targets the DE10-Nano FPGA board.

## Build Commands

This project uses Intel Quartus Prime for compilation and synthesis:

- **Build the project**: Open `mycore.qpf` in Quartus Prime and compile, or use Quartus command-line tools
- **Clean project**: `clean.bat` (Windows) - removes all temporary build files and directories
- **Generate RBF file**: The build process automatically generates `mycore.rbf` in `output_files/` directory

**Important**: Always use Quartus v17.0.x (preferably v17.0.2). Newer versions will introduce incompatibilities.

## Architecture

### Core Structure
- **`sys/`** - MiSTer framework (never modify these files - updates will overwrite changes)
- **`rtl/`** - Actual core implementation and source files
- **`mycore.sv`** - Main glue logic between framework and core-specific signals
- **`files.qip`** - Manual file list for project (edit this instead of adding files through Quartus IDE)

### Key Files
- **`mycore.qpf`** - Quartus project file (modify PROJECT_REVISION line for core name)
- **`mycore.qsf`** - Quartus settings file (minimal changes only, watch for Quartus auto-modifications)
- **`mycore.sdc`** - Optional timing constraints
- **`mycore.srf`** - Optional file to disable safe warnings

### Framework Integration
The `emu` module in `mycore.sv` interfaces with the MiSTer framework via standardized ports:
- Clock management (CLK_50M, CLK_VIDEO, CE_PIXEL)
- Video output (VGA_*, VIDEO_ARX/ARY for aspect ratio)
- HPS communication bus for system integration
- Audio and input handling

### PLL Requirements
Framework requires at least one PLL in the `rtl/` folder:
- `pll.v` and `pll.qip` files must be present
- PLL settings are core-specific

## Framework Macros

Configure framework features by defining these macros in project settings:

| Macro | Effect |
|-------|--------|
| `MISTER_DEBUG_NOHDMI` | Disable HDMI modules for faster compilation |
| `MISTER_DUAL_SDRAM` | Enable dual SDRAM I/O board support |
| `MISTER_FB` | Enable framebuffer support |
| `MISTER_SMALL_VBUF` | Use smaller video buffer for ASCAL |
| `MISTER_DOWNSCALE_NN` | Enable ASCAL downscale mode |
| `MISTER_DISABLE_ADAPTIVE` | Disable adaptive scan lines |

## Development Notes

- Standard file naming: `<core_name>_YYYYMMDD.rbf` for releases
- Add/remove files in `files.qip` manually, not through Quartus IDE
- The `build_id.tcl` script auto-generates build timestamps
- Use `jtag.cdf` for programming via USB Blaster cable
- Framework handles video scaling, audio mixing, and system integration