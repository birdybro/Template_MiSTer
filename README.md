# MiSTer Template Core

A standardized framework and development template for creating FPGA cores for the MiSTer platform. This template provides the foundation for all MiSTer cores, ensuring compatibility with the MiSTer framework while allowing developers to focus on their core-specific implementation.

## Quick Start

### For New Developers
1. **Clone this repository** as your starting point
2. **Install Intel Quartus Prime 17.0.2** (required version)
3. **Rename project files** to match your core name
4. **Follow the [Development Guide](Development_Guide.md)** for step-by-step implementation

### For Experienced Developers
```bash
# Quick setup
git clone https://github.com/MiSTer-devel/Template_MiSTer.git YourCore
cd YourCore
# Rename files: mycore.* → yourcore.*
# Edit files.qip, update PROJECT_REVISION in .qpf
# Implement your core in rtl/ directory
# Open yourcore.qpf in Quartus Prime 17.0.x and compile
```

## Prerequisites

### Required Software
- **Intel Quartus Prime 17.0.2** (Standard or Lite Edition)
  - ⚠️ **Critical**: Newer versions (18.x+) are incompatible with MiSTer DE10-Nano
  - Available from Intel/Altera legacy downloads
- **Git** for version control
- **Text editor** with Verilog/SystemVerilog support

### Hardware Requirements
- **Development**: 8GB+ RAM, multi-core CPU, SSD recommended
- **Target**: MiSTer DE10-Nano FPGA board, MicroSD card (8GB+)
- **Optional**: USB Blaster for direct FPGA programming

## Documentation Index

### Core Documentation

| Document | Description | Audience |
|----------|-------------|----------|
| **[Development Guide](Development_Guide.md)** | Complete step-by-step development workflow | All developers |
| **[API Reference](API_Reference.md)** | Detailed interface specifications and signals | Core implementers |
| **[Architecture](Architecture.md)** | System architecture and design patterns | Technical leads |
| **[Build Guide](Build_Guide.md)** | Compilation, optimization, and deployment | Build engineers |
| **[Video System](Video_System.md)** | Video processing pipeline and timing | Video developers |

### Quick Reference
- **[CLAUDE.md](CLAUDE.md)** - AI assistant guidance for this repository
- **[Readme.md](Readme.md)** - Original template documentation (legacy)

## Project Structure

```
Template_MiSTer/
├── sys/                    # MiSTer Framework (DO NOT MODIFY)
│   ├── sys_top.v          # Hardware abstraction layer
│   ├── hps_io.sv          # ARM-FPGA communication
│   ├── video_mixer.sv     # Video processing pipeline
│   └── ...                # Other framework modules
├── rtl/                   # Your Core Implementation
│   ├── mycore.v           # Main core logic
│   ├── pll.v              # Core-specific PLL (required)
│   └── ...                # Your custom modules
├── mycore.sv              # Glue logic (framework ↔ core)
├── mycore.qpf             # Quartus project file
├── mycore.qsf             # Quartus settings file
├── files.qip              # Source file list (edit manually)
├── releases/              # Compiled RBF files
└── documentation/         # Comprehensive guides
```

## Architecture Overview

The MiSTer Template Core implements a layered architecture that cleanly separates the MiSTer framework from your core-specific implementation:

```
┌─────────────────────────────────────────┐
│            MiSTer Framework             │
│  (Video, Audio, I/O, Menu System)      │
├─────────────────────────────────────────┤
│             Glue Logic                  │
│    (mycore.sv - Interface Layer)       │
├─────────────────────────────────────────┤
│          Your Core Logic                │
│     (CPU, Memory, Video, Audio)        │
└─────────────────────────────────────────┘
```

**Key Benefits:**
- **Framework Isolation**: sys/ directory is never modified, enabling easy updates
- **Standardized Interface**: Consistent APIs across all MiSTer cores
- **Modular Design**: Focus on your core logic, leverage framework services
- **Platform Integration**: Automatic menu system, video processing, and hardware abstraction

For detailed architecture information, see [Architecture.md](Architecture.md).

## Development Workflow

### 1. Initial Setup
```bash
# Clone and customize
git clone https://github.com/MiSTer-devel/Template_MiSTer.git MyCoreProject
cd MyCoreProject

# Rename core files (replace 'mycore' with your core name)
mv mycore.qpf mycoreproject.qpf
mv mycore.qsf mycoreproject.qsf
mv mycore.sv mycoreproject.sv
# Update PROJECT_REVISION in .qpf file
# Update file references in files.qip
```

### 2. Core Implementation
```systemverilog
// In mycoreproject.sv - define your core interface
module emu(
    input         CLK_50M,     // 50MHz master clock
    input         RESET,       // System reset
    inout  [48:0] HPS_BUS,     // Framework communication
    
    // Video interface
    output        CLK_VIDEO,   // Video clock domain
    output        CE_PIXEL,    // Pixel clock enable
    output [7:0]  VGA_R, VGA_G, VGA_B,  // RGB output
    output        VGA_HS, VGA_VS,       // Sync signals
    
    // Audio interface  
    output [15:0] AUDIO_L, AUDIO_R,     // Stereo audio
    
    // Additional interfaces as needed...
);
```

### 3. Build and Test
```bash
# Command-line build (Linux/Windows)
quartus_map mycoreproject
quartus_fit mycoreproject
quartus_asm mycoreproject

# Output: mycoreproject.rbf in output_files/
# Deploy to MiSTer: Copy to appropriate directory
```

For complete development workflow, see [Development Guide](Development_Guide.md).

## Common Usage Patterns

### Basic Video Generation
```systemverilog
// Generate video timing and pixel data
always @(posedge clk_sys) begin
    if (ce_pix) begin
        // Update horizontal/vertical counters
        // Generate sync signals
        // Output pixel data based on position
    end
end

assign CLK_VIDEO = clk_sys;
assign CE_PIXEL = ce_pix;
assign VGA_R = video_red;
assign VGA_G = video_green; 
assign VGA_B = video_blue;
```

### Menu System Integration
```systemverilog
// Configuration string defines OSD menu
localparam CONF_STR = {
    "MyCore;;",
    "O[2],TV Mode,NTSC,PAL;",
    "O[4:3],Color,White,Red,Green,Blue;",
    "T[0],Reset;",
    "V,v",`BUILD_DATE
};

// Extract settings from status bits
wire reset = RESET | status[0] | buttons[1];
wire pal_mode = status[2];
wire [1:0] color_mode = status[4:3];
```

### Memory Interface Usage
```systemverilog
// SDRAM controller integration
sdram_controller sdram_ctrl(
    .clk(clk_sys),
    .addr(memory_addr),
    .din(memory_write_data),
    .dout(memory_read_data),
    .we(memory_write_enable),
    
    // Physical SDRAM interface
    .SDRAM_A(SDRAM_A),
    .SDRAM_DQ(SDRAM_DQ),
    // ... other SDRAM signals
);
```

## Troubleshooting

### Build Issues

**"Can't elaborate top-level user hierarchy"**
- Check that all source files are listed in `files.qip`
- Verify module names match between files

**"Timing requirements not met"**
- Review clock constraints in `.sdc` file
- Consider adding pipeline stages to critical paths
- Check [Build Guide](Build_Guide.md) for optimization strategies

**"Can't fit design in device"**
- Enable resource optimization settings
- Use `MISTER_DEBUG_NOHDMI` macro to save resources
- Review resource usage in compilation report

### Video Problems

**Flickering or unstable display**
- Verify proper clock domain crossing with `CE_PIXEL`
- Check sync signal timing and polarity
- See [Video System](Video_System.md) for detailed timing specs

**Colors incorrect or missing**
- Verify RGB signal assignments and bit widths
- Check color space conversion if using YUV
- Ensure gamma correction is properly configured

### Framework Integration

**Menu not appearing or functioning**
- Verify `HPS_BUS` connection to `hps_io` module
- Check configuration string syntax
- Ensure status bit mapping is correct

**File loading not working**
- Check `ioctl_*` signal connections
- Verify file type index handling
- Review file I/O implementation in glue logic

For comprehensive troubleshooting, consult the relevant documentation sections and community resources.

## Contributing

### Development Standards
- **Never modify** files in the `sys/` directory
- Follow the standard interface conventions in [API Reference](API_Reference.md)
- Include comprehensive documentation for new features
- Test thoroughly on actual hardware before submitting

### Community Guidelines
- **Code Quality**: Use consistent formatting and meaningful comments
- **Framework Compliance**: Follow established patterns and interfaces  
- **Testing**: Provide simulation testbenches and hardware validation
- **Documentation**: Update relevant docs for any interface changes

### Submitting Cores
1. **Open Source**: GPL license preferred for community cores
2. **Documentation**: Include complete README and usage instructions
3. **Build Instructions**: Provide clear compilation steps
4. **Testing**: Verify functionality on multiple display types
5. **Community Review**: Engage with feedback and suggestions

## Project Information

- **Platform**: MiSTer FPGA (DE10-Nano)
- **Development Environment**: Intel Quartus Prime 17.0.x
- **License**: GPL (framework and template)
- **Target Audience**: FPGA developers, retro computing enthusiasts
- **Community**: [MiSTer-devel GitHub Organization](https://github.com/MiSTer-devel)

### Version Information
- **Template Version**: Current (updates distributed with framework)
- **Framework Compatibility**: MiSTer Main branch
- **Last Updated**: Check git commit history for recent changes
- **Build Date**: Generated automatically during compilation

For the most up-to-date information, documentation, and community support, visit the [MiSTer GitHub repositories](https://github.com/MiSTer-devel) and the [MiSTer FPGA community forums](https://misterfpga.org).