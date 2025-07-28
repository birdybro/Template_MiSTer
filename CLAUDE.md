# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is a MiSTer FPGA core template repository using Quartus 17.0.x. MiSTer is an open-source project that uses FPGA to recreate classic computers, game consoles, and arcade machines. This template provides the standard framework (sys folder) and example implementation structure for developing new MiSTer cores.

## Build Commands

**Compilation:**
- Open the project in Quartus 17.0.x (required version - do not use newer versions)
- Open `mycore.qpf` in Quartus IDE
- Run Compilation (Processing → Start Compilation)
- Output RBF file will be in `output_files/` directory

**Cleaning:**
```bash
# Windows
clean.bat

# Linux/WSL - manually run equivalent commands:
rm -rf db incremental_db output_files simulation greybox_tmp hc_output .qsys_edit
rm -rf hps_isw_handoff sys/.qsys_edit sys/vip
rm -rf sys/*_sim rtl/*_sim
rm -f build_id.v c5_pin_model_dump.txt PLLJ_PLLSPE_INFO.txt
rm -f *.qws *.ppf *.ddb *.csv *.cmp *.sip *.spd *.bsf *.f *.sopcinfo *.xml *.cdf *.rpt
```

**Testing:**
- Load compiled RBF file to MiSTer via FTP/SSH to `/media/fat/` directory
- Or use USB Blaster cable with `jtag.cdf` file (generated after compilation)

## Architecture

### Core Structure
- `sys/` - Framework folder (DO NOT modify - updated from upstream)
- `rtl/` - Core implementation source files
  - Must include PLL files: `pll.v`, `pll.qip`, `pll/` folder
- `mycore.sv` - Top-level glue logic between framework and core
- `mycore.qsf` - Quartus settings (keep changes minimal)
- `files.qip` - List of all source files (manually maintained)

### Key Interfaces
The core implements the `emu` module (mycore.sv:19-315) which interfaces with the MiSTer framework:
- Video output: VGA_R/G/B, VGA_HS/VS, VGA_DE, CE_PIXEL
- Audio output: AUDIO_L/R (16-bit), AUDIO_S (signed flag)
- Memory interfaces: SDRAM, DDRAM
- HPS communication: HPS_BUS for menu/OSD
- User inputs: buttons, ps2_key from hps_io module

### Configuration String
The CONF_STR (mycore.sv:210-239) defines the OSD menu structure using a specific format:
- Core name and menu options
- Status bits allocation (e.g., O[122:121] for aspect ratio)
- File selectors (F1 for BIN files, S0 for DSK)
- Reset triggers (T[0], R[0])

### Video Generation
The example core (rtl/mycore.v) generates a test pattern:
- Supports NTSC/PAL modes with scandoubler option
- Generates proper video timing signals
- Uses LFSR for random noise and cosine table for pattern generation

## HDL Development Workflow

**Agent Selection Process:**
For every HDL-related request, Claude Code will first evaluate which specialized agents are needed and invoke them in the appropriate order. The available HDL agents are:

- **hdl-system-architect**: High-level FPGA system design, module hierarchies, and architectural blueprints
- **hdl-design-engineer**: RTL module implementation, digital logic optimization, and synthesizable HDL code
- **hdl-timing-analyzer**: Timing analysis, critical paths, clock domain crossings, and timing closure
- **hdl-resource-optimizer**: FPGA/ASIC resource utilization analysis, power optimization, and area efficiency
- **hdl-code-reviewer**: Code quality assurance, synthesizability checks, and HDL best practices

**Documentation Requirement:**
After completing any HDL-related request, the **hdl-docs-writer** agent will automatically create or update documentation in a separate `.md` file to record:
- Changes made to the codebase
- Design decisions and rationale
- Interface specifications
- Implementation notes
- Testing considerations

This ensures all modifications are properly documented for future reference and collaboration.

## Important Notes

- **Quartus Version**: Must use v17.0.x (17.0.2 recommended) - newer versions cause incompatibilities
- **Framework Updates**: Never modify files in sys/ folder
- **File Management**: Add new files to `files.qip` manually, not through Quartus IDE
- **Macros**: Define in mycore.qsf for features like MISTER_DUAL_SDRAM, MISTER_FB
- **Releases**: Place RBF files in releases/ folder with format: `<core_name>_YYYYMMDD.rbf`