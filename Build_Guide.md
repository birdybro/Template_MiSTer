# MiSTer Template Core Build Guide

## Overview

This guide provides comprehensive instructions for building, configuring, and deploying MiSTer cores using the Template Core framework. It covers the complete build process from source code to deployed RBF files, including optimization strategies and troubleshooting procedures.

## Prerequisites

### Software Requirements

**Intel Quartus Prime 17.0.x (Required)**
- **Version**: Must use v17.0.2 Standard Edition (recommended) or Lite Edition
- **Compatibility**: Newer versions (18.x+) are not compatible with MiSTer DE10-Nano
- **License**: Free Lite Edition sufficient for most cores
- **Download**: Available from Intel/Altera legacy downloads

**Additional Tools:**
- Git for version control
- Text editor with Verilog/SystemVerilog syntax highlighting
- USB Blaster driver (for direct FPGA programming)

### Hardware Requirements

**Development System:**
- **RAM**: Minimum 8GB, 16GB+ recommended for complex cores
- **CPU**: Multi-core processor recommended (compilation uses all cores)
- **Storage**: SSD recommended (10GB+ free space for Quartus and build artifacts)
- **OS**: Windows 10/11, Linux (Ubuntu/Debian/CentOS), or macOS

**Target Hardware:**
- MiSTer DE10-Nano FPGA board
- MicroSD card (8GB+ recommended)
- HDMI display or VGA monitor
- USB keyboard (for OSD navigation)
- Optional: USB Blaster for direct programming

## Project Structure and Files

### Core Project Files

```
Template_MiSTer/
├── mycore.qpf              # Quartus Project File
├── mycore.qsf              # Quartus Settings File  
├── mycore.sdc              # Timing Constraints (optional)
├── mycore.srf              # Warning Suppression (optional)
├── mycore.sv               # Glue Logic Layer
├── files.qip               # Source File List
├── clean.bat               # Windows Cleanup Script
├── .gitignore              # Version Control Exclusions
└── releases/               # Compiled RBF Output Directory
```

### File Descriptions

**mycore.qpf (Project File)**
```
PROJECT_REVISION = "mycore"
```
- Defines project name and revision
- Copy from template and modify PROJECT_REVISION line only

**files.qip (Source File List)**
```
set_global_assignment -name VERILOG_FILE rtl/lfsr.v
set_global_assignment -name SYSTEMVERILOG_FILE rtl/cos.sv
set_global_assignment -name SYSTEMVERILOG_FILE rtl/mycore.v
set_global_assignment -name SDC_FILE mycore.sdc
set_global_assignment -name SYSTEMVERILOG_FILE mycore.sv
```
- Lists all source files for compilation
- Edit manually to add/remove source files
- **Never add files through Quartus GUI** (they go to .qsf instead)

**mycore.qsf (Settings File)**
- Contains all Quartus project settings and optimizations
- Automatically managed by Quartus
- **Warning**: May grow large if Quartus "spills" settings from other files
- Revert to clean version if file becomes oversized

## Build Process

### Command Line Build

**Windows:**
```batch
REM Set Quartus environment
set QUARTUS_ROOTDIR=C:\intelFPGA\17.0\quartus
set PATH=%QUARTUS_ROOTDIR%\bin64;%PATH%

REM Clean previous build
del /Q output_files\*.*

REM Compile project
quartus_map mycore
quartus_fit mycore  
quartus_asm mycore
quartus_sta mycore
```

**Linux/macOS:**
```bash
#!/bin/bash
# Set Quartus environment
export QUARTUS_ROOTDIR=/opt/intelFPGA/17.0/quartus
export PATH=$QUARTUS_ROOTDIR/bin:$PATH

# Clean previous build
rm -rf output_files/*

# Compile project
quartus_map mycore
quartus_fit mycore
quartus_asm mycore
quartus_sta mycore
```

### GUI Build Process

1. **Open Project**
   - Launch Quartus Prime 17.0.2
   - File → Open Project
   - Select `mycore.qpf`

2. **Verify Settings**
   - Tools → Options → General → Default to one-step compilation flow: **Disabled**
   - Processing → Start Compilation (or Ctrl+L)

3. **Monitor Progress**
   - Analysis & Synthesis: ~2-5 minutes
   - Fitter: ~10-30 minutes (depends on core complexity)
   - Assembler: ~1 minute
   - Timing Analyzer: ~2 minutes

4. **Check Results**
   - Compilation Report → Flow Summary
   - Verify timing constraints are met
   - Check resource utilization

### Build Stages

#### 1. Analysis & Synthesis (quartus_map)

**Purpose**: Analyzes HDL code and creates netlist
**Duration**: 2-5 minutes
**Key Outputs**:
- Netlist database
- Resource usage estimates
- Syntax and semantic error reports

**Common Issues**:
```
Error: Verilog HDL syntax error at mycore.v(123): expecting ";"
```
*Solution*: Fix syntax errors in source files

```
Warning: Inferred latch for variable "signal_name"
```
*Solution*: Ensure all signals have defined values in all code paths

#### 2. Fitter (quartus_fit)

**Purpose**: Places and routes logic elements on FPGA
**Duration**: 10-30 minutes (longest stage)
**Key Outputs**:
- Placed and routed design
- Timing analysis data
- Resource utilization report

**Critical Messages**:
```
Critical Warning: Timing requirements not met
```
*Action Required*: Review timing constraints and optimization settings

```
Error: Can't fit design in device
```
*Solution*: Reduce resource usage or enable optimization options

#### 3. Assembler (quartus_asm)

**Purpose**: Generates programming files
**Duration**: ~1 minute
**Key Outputs**:
- `mycore.rbf` - MiSTer-compatible binary
- `mycore.sof` - JTAG programming file

#### 4. Timing Analyzer (quartus_sta)

**Purpose**: Validates timing constraints
**Duration**: ~2 minutes
**Key Outputs**:
- Timing analysis report
- Setup/hold violation reports

## Configuration and Optimization

### Compilation Settings

**Performance Optimization (mycore.qsf)**
```tcl
# Optimization mode
set_global_assignment -name OPTIMIZATION_MODE "HIGH PERFORMANCE EFFORT"
set_global_assignment -name OPTIMIZATION_TECHNIQUE SPEED

# Physical synthesis options
set_global_assignment -name PHYSICAL_SYNTHESIS_COMBO_LOGIC ON
set_global_assignment -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON
set_global_assignment -name PHYSICAL_SYNTHESIS_REGISTER_RETIMING ON

# Fitter effort level
set_global_assignment -name FITTER_EFFORT "STANDARD FIT"
set_global_assignment -name FINAL_PLACEMENT_OPTIMIZATION ALWAYS

# Timing optimization
set_global_assignment -name ECO_OPTIMIZE_TIMING ON
set_global_assignment -name ROUTER_CLOCKING_TOPOLOGY_ANALYSIS ON
```

**Resource Optimization**
```tcl
# For resource-constrained designs
set_global_assignment -name OPTIMIZATION_MODE "BALANCED"
set_global_assignment -name PHYSICAL_SYNTHESIS_COMBO_LOGIC_FOR_AREA ON
set_global_assignment -name REMOVE_REDUNDANT_LOGIC_CELLS ON
set_global_assignment -name MUX_RESTRUCTURE ON
```

### Timing Constraints (mycore.sdc)

**Basic Clock Constraints**
```tcl
# Define input clocks
create_clock -name "CLK_50M" -period 20.000ns [get_ports {CLK_50M}]

# Derive PLL clocks automatically  
derive_pll_clocks -create_base_clocks

# Set clock groups for independent domains
set_clock_groups -asynchronous -group [get_clocks {CLK_50M}] \
                                -group [get_clocks {*|pll|*}]

# Set input/output delays
set_input_delay -clock [get_clocks {CLK_50M}] -max 3.0 [all_inputs]
set_input_delay -clock [get_clocks {CLK_50M}] -min 1.0 [all_inputs]

set_output_delay -clock [get_clocks {*video_clock*}] -max 2.0 [all_outputs]
set_output_delay -clock [get_clocks {*video_clock*}] -min 0.5 [all_outputs]
```

**Advanced Constraints**
```tcl
# Multicycle paths for slower operations
set_multicycle_path -from [get_registers {*slow_operation*}] \
                    -to [get_registers {*destination*}] -setup 2

# False paths for asynchronous signals
set_false_path -from [get_registers {reset_sync*}] \
               -to [get_registers {*}]

# Maximum delay constraints
set_max_delay -from [get_ports {USER_IN*}] \
              -to [get_registers {*button_sync*}] 10.0
```

### Conditional Compilation

**Feature Selection Macros**
```systemverilog
// In mycore.qsf
set_global_assignment -name VERILOG_MACRO "MISTER_DEBUG_NOHDMI=1"
set_global_assignment -name VERILOG_MACRO "MISTER_DUAL_SDRAM=1"
```

**Available Macros**:
- `MISTER_DEBUG_NOHDMI`: Disable HDMI (faster compilation, analog-only output)
- `MISTER_DUAL_SDRAM`: Enable dual SDRAM I/O board support
- `MISTER_FB`: Enable framebuffer functionality
- `MISTER_SMALL_VBUF`: Reduce video buffer size for ASCAL
- `MISTER_DOWNSCALE_NN`: Enable nearest-neighbor downscaling
- `MISTER_DISABLE_ADAPTIVE`: Disable adaptive scanlines
- `MISTER_FB_PALETTE`: Enable framebuffer palette mode

### Resource Utilization Analysis

**Logic Elements (LEs)**
```
Total logic elements: 32,070 / 41,910 ( 76% )
    -- Combinational LEs: 28,234
    -- Register-only LEs: 1,587  
    -- Combinational with register LEs: 2,249
```

**Memory Usage**
```
Total RAM bits: 1,677,312 / 5,662,720 ( 30% )
    -- M10K blocks: 164 / 432 ( 38% )
    -- MLAB ALMs: 89 / 15,820 ( 1% )
```

**DSP Blocks**
```
Total DSP Blocks: 45 / 87 ( 52% )
    -- 18-bit Multipliers: 90 / 174
    -- Variable-precision DSP: 45 / 87
```

## Deployment

### MiSTer Deployment

**1. Generate RBF File**
- Successful compilation generates `output_files/mycore.rbf`
- This is the file deployed to MiSTer systems

**2. File Naming Convention**
```
mycore_YYYYMMDD.rbf
```
- Replace `mycore` with your actual core name
- `YYYYMMDD` is the build date (e.g., `20240315`)
- Place in `releases/` directory

**3. MiSTer Installation**
```
/media/fat/
├── _Computer/
│   └── MyCore_20240315.rbf
└── games/
    └── MyCore/
        ├── core_settings.txt
        └── game_files/
```

**4. Core Settings File**
```ini
; MyCore settings
video_mode=0
scandoubler=1
aspect_ratio=0
```

### Direct FPGA Programming

**JTAG Programming (Development)**
```bash
# Program FPGA directly via USB Blaster
quartus_pgm -c "USB-Blaster [USB-0]" -m jtag -o "p;output_files/mycore.sof@1"
```

**1. Hardware Setup**
- Connect USB Blaster to DE10-Nano JTAG header
- Power on DE10-Nano
- Ensure USB Blaster drivers installed

**2. Quartus Programming**
- Tools → Programmer
- Hardware Setup → USB-Blaster
- Add File → Select `mycore.sof`
- Device: 5CSEBA6U23
- Check "Program/Configure"
- Start programming

### Version Control Integration

**Git Workflow**
```bash
# Clean build artifacts before commit
git clean -fdx output_files/
git clean -fdx incremental_db/
git clean -fdx db/

# Add source changes
git add rtl/ mycore.sv files.qip

# Commit with descriptive message
git commit -m "Add video timing improvements

- Fix PAL timing for 288p resolution
- Optimize pixel clock generation
- Update ASCAL configuration"

# Tag releases
git tag -a v1.0.0 -m "Release version 1.0.0"
```

**.gitignore Configuration**
```gitignore
# Quartus build artifacts
output_files/
incremental_db/
db/
simulation/
greybox_tmp/
*.rpt
*.summary
*.done
*.jdi
*.qws
*.sopcinfo
*.pin
*.qdf

# Programming files (keep RBF for releases)
*.sof
*.pof
*.jic

# Timing analyzer
*.sta.rpt

# Temporary files
*~
*.bak
*.orig
```

## Build Optimization Strategies

### Compilation Time Optimization

**Parallel Processing**
```tcl
# Use all CPU cores
set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL

# Enable smart recompile
set_global_assignment -name SMART_RECOMPILE ON

# Incremental compilation for faster iterations  
set_global_assignment -name INCREMENTAL_COMPILATION FULL_INCREMENTAL_COMPILATION
```

**Selective Compilation**
```bash
# Analysis & Synthesis only (fast syntax check)
quartus_map mycore

# Skip timing analysis during development  
# quartus_sta mycore  # Comment out during iteration
```

### Resource Optimization

**Memory Inference**
```systemverilog
// Prefer M10K blocks for large memories
(* ramstyle = "M10K" *) reg [15:0] large_ram [0:2047];

// Use MLABs for smaller memories  
(* ramstyle = "MLAB" *) reg [7:0] small_ram [0:31];

// Force logic implementation for very small memories
(* ramstyle = "logic" *) reg [3:0] tiny_ram [0:7];
```

**DSP Block Usage**
```systemverilog
// Encourage DSP block inference
(* multstyle = "dsp" *) wire [31:0] mult_result = a * b;

// Force logic implementation
(* multstyle = "logic" *) wire [15:0] small_mult = x * y;
```

### Timing Optimization

**Register Balancing**
```systemverilog
// Pipeline critical paths
always @(posedge clk) begin
    // Stage 1
    mult_stage1 <= input_a * input_b;
    add_stage1 <= input_c + input_d;
    
    // Stage 2  
    result <= mult_stage1 + add_stage1;
end
```

**Clock Domain Crossing**
```systemverilog
// Proper synchronizer chain
reg [2:0] sync_chain;
always @(posedge dest_clk or posedge reset) begin
    if (reset)
        sync_chain <= 3'b0;
    else
        sync_chain <= {sync_chain[1:0], async_signal};
end
assign synchronized_signal = sync_chain[2];
```

## Troubleshooting

### Common Build Errors

**Error: "Can't elaborate top-level user hierarchy"**
```
Error (12006): Node instance "emu" instantiates undefined entity "mycore"
```
*Solution*: Check that all source files are listed in `files.qip`

**Error: "Timing requirements not met"**
```
Critical Warning (332012): Synopsys Design Constraints File file mycore.sdc contains assignment to non-existent clock CLK_SYS
```
*Solution*: Update timing constraints to match actual clock names

**Error: "Can't fit design in device"**
```
Error (119013): Can't fit 42000 logic elements in device with 41910 logic elements
```
*Solutions*:
- Enable resource optimization settings
- Reduce design complexity
- Use `MISTER_DEBUG_NOHDMI` to save resources

### Build Performance Issues

**Slow Fitter Performance**
```tcl
# Reduce fitter effort for faster compilation
set_global_assignment -name FITTER_EFFORT "FAST FIT"

# Disable some optimizations
set_global_assignment -name PHYSICAL_SYNTHESIS_REGISTER_RETIMING OFF
```

**Memory Issues**
- Increase system RAM if available
- Close other applications during compilation
- Use incremental compilation mode

### Timing Analysis

**Setup Time Violations**
```
Critical Warning (332148): Timing requirements not met
Setup time: -0.513ns (requirement: 20.000ns)
```
*Solutions*:
- Add pipeline stages to critical paths
- Reduce clock frequency
- Optimize logic structure

**Hold Time Violations**
```
Hold time: -0.156ns (requirement: 0.000ns)  
```
*Solutions*:
- Usually resolved automatically by Quartus
- Add delay elements if persistent
- Check for clock domain crossing issues

### Debug Strategies

**SignalTap Logic Analyzer**
```tcl
# Enable SignalTap in .qsf
set_global_assignment -name ENABLE_SIGNALTAP ON
set_global_assignment -name USE_SIGNALTAP_FILE debug.stp
```

**Simulation Testing**
```systemverilog
// Testbench for functional verification
module mycore_tb;
    reg clk, reset;
    wire [7:0] video;
    
    mycore dut(
        .clk(clk),
        .reset(reset),
        .video(video)
    );
    
    initial begin
        clk = 0;
        forever #10 clk = ~clk;  // 50MHz clock
    end
    
    initial begin
        reset = 1;
        #100 reset = 0;
        #1000000;
        $finish;
    end
endmodule
```

## Continuous Integration

### Automated Build Script

```bash
#!/bin/bash
# build.sh - Automated build script

set -e  # Exit on error

CORE_NAME="mycore"
BUILD_DATE=$(date +%Y%m%d)
OUTPUT_DIR="releases"

echo "Building $CORE_NAME for date $BUILD_DATE"

# Clean previous build
echo "Cleaning previous build..."
rm -rf output_files/*
rm -rf incremental_db/*

# Run compilation
echo "Starting compilation..."
quartus_map $CORE_NAME
quartus_fit $CORE_NAME
quartus_asm $CORE_NAME
quartus_sta $CORE_NAME

# Check if RBF was generated
if [ ! -f "output_files/$CORE_NAME.rbf" ]; then
    echo "ERROR: RBF file not generated!"
    exit 1
fi

# Copy to releases directory
mkdir -p $OUTPUT_DIR
cp "output_files/$CORE_NAME.rbf" "$OUTPUT_DIR/${CORE_NAME}_${BUILD_DATE}.rbf"

echo "Build completed successfully!"
echo "Output: $OUTPUT_DIR/${CORE_NAME}_${BUILD_DATE}.rbf"

# Display resource usage
echo "Resource Utilization:"
grep -A 10 "Logic utilization" output_files/$CORE_NAME.fit.rpt || true
```

### Build Validation

```bash
#!/bin/bash
# validate.sh - Build validation script

CORE_NAME="mycore"
RBF_FILE="output_files/$CORE_NAME.rbf"

# Check file exists and has reasonable size
if [ ! -f "$RBF_FILE" ]; then
    echo "ERROR: RBF file not found"
    exit 1
fi

FILE_SIZE=$(stat -c%s "$RBF_FILE")
MIN_SIZE=1000000  # 1MB minimum

if [ $FILE_SIZE -lt $MIN_SIZE ]; then
    echo "ERROR: RBF file too small ($FILE_SIZE bytes)"
    exit 1
fi

# Check timing was met
if grep -q "Timing requirements not met" output_files/$CORE_NAME.sta.rpt; then
    echo "ERROR: Timing requirements not met"
    exit 1
fi

# Check resource usage is reasonable
LOGIC_USAGE=$(grep "Total logic elements" output_files/$CORE_NAME.fit.rpt | \
              sed 's/.*(\s*\([0-9]*\)%.*/\1/')

if [ "$LOGIC_USAGE" -gt "95" ]; then
    echo "WARNING: High logic utilization ($LOGIC_USAGE%)"
fi

echo "Build validation passed"
```

This build guide provides comprehensive coverage of the MiSTer core build process, from basic compilation through advanced optimization and deployment strategies. Following these procedures will ensure reliable, optimized builds suitable for MiSTer deployment.