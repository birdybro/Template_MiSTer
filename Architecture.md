# MiSTer Template Core Architecture

## Overview

The MiSTer Template Core provides a standardized framework for developing FPGA cores for the MiSTer platform. This architecture implements a layered design that separates the MiSTer framework from core-specific logic, enabling developers to focus on their core implementation while leveraging the robust infrastructure provided by the framework.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        DE10-Nano Board                         │
├─────────────────────────────────────────────────────────────────┤
│                       sys_top.v                                │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                 MiSTer Framework                        │    │
│  │  ┌───────────────┐  ┌──────────────┐  ┌─────────────┐  │    │
│  │  │   hps_io.sv   │  │ video_mixer  │  │ audio_out.v │  │    │
│  │  │    (I/O)      │  │   .sv        │  │  (Audio)    │  │    │
│  │  └───────────────┘  └──────────────┘  └─────────────┘  │    │
│  │  ┌───────────────┐  ┌──────────────┐  ┌─────────────┐  │    │
│  │  │   osd.v       │  │ scandoubler  │  │  pll_*.v    │  │    │
│  │  │  (Menu/OSD)   │  │   .v         │  │  (Clocks)   │  │    │
│  │  └───────────────┘  └──────────────┘  └─────────────┘  │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                     │
│                           │ Standard Interface                  │
│                           ▼                                     │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    mycore.sv                            │    │
│  │              (Glue Logic Layer)                         │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                     │
│                           │ Core-Specific Interface             │
│                           ▼                                     │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                 Core Implementation                     │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │    │
│  │  │  mycore.v   │  │   cos.sv    │  │     lfsr.v      │  │    │
│  │  │ (Main Core) │  │ (Cosine LUT)│  │ (Random Gen)    │  │    │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │    │
│  │  ┌─────────────┐                                       │    │
│  │  │   pll.v     │         Core-Specific Modules         │    │
│  │  │ (Core PLL)  │                                       │    │
│  │  └─────────────┘                                       │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

## Architectural Layers

### 1. Hardware Abstraction Layer (sys_top.v)

The `sys_top.v` module provides the top-level hardware abstraction, interfacing directly with the DE10-Nano FPGA board pins and resources:

- **Pin Mapping**: Direct connection to FPGA pins (HDMI, VGA, SDRAM, etc.)
- **Clock Management**: Primary clock distribution from board oscillators
- **Hardware Interface**: SDRAM controllers, HDMI transmitters, audio DACs

**Key Responsibilities:**
- Board-specific pin assignments
- Primary PLL instantiation
- Hardware resource management
- Platform-specific conditional compilation

### 2. MiSTer Framework Layer (sys/ directory)

The framework layer provides standardized services and interfaces:

#### Core Framework Modules:

**hps_io.sv** - ARM-FPGA Communication Hub
- Handles all communication between ARM processor and FPGA
- Implements menu system (OSD) interface
- Manages file I/O operations
- Provides joystick/keyboard input processing
- Configuration string parsing

**video_mixer.sv** - Video Processing Pipeline
- Implements scandoubling and line doubling
- Provides gamma correction
- Handles video freezing for HDMI
- Color depth conversion and processing

**audio_out.v** - Audio Output Management
- Digital audio processing and filtering
- Multiple output format support (I2S, SPDIF)
- Volume control and mixing

**Additional Framework Components:**
- `osd.v`: On-Screen Display implementation
- `scandoubler.v`: Video line doubling
- `ascal.vhd`: Advanced scaling algorithms
- `arcade_video.v`: Arcade-specific video timing
- Various PLL configurations for different timing requirements

### 3. Glue Logic Layer (mycore.sv)

The glue logic layer serves as the interface between the MiSTer framework and the core implementation:

```systemverilog
module emu(
    // Framework interface signals
    input         CLK_50M,           // Board master clock
    input         RESET,             // System reset
    inout  [48:0] HPS_BUS,          // ARM communication bus
    
    // Video output interface
    output        CLK_VIDEO,         // Video timing clock
    output        CE_PIXEL,          // Pixel clock enable
    output [7:0]  VGA_R, VGA_G, VGA_B, // RGB video signals
    output        VGA_HS, VGA_VS,    // Sync signals
    
    // Audio interface
    output [15:0] AUDIO_L, AUDIO_R,  // Stereo audio output
    
    // Memory interfaces
    output [12:0] SDRAM_A,           // SDRAM address
    inout  [15:0] SDRAM_DQ,          // SDRAM data
    // ... additional SDRAM signals
    
    // Optional interfaces (conditional compilation)
    // Framebuffer, dual SDRAM, etc.
);
```

**Key Functions:**
- Signal mapping between framework and core
- Configuration string definition (menu structure)
- Status bit interpretation
- Clock domain management
- Default signal assignments for unused features

### 4. Core Implementation Layer (rtl/ directory)

This layer contains the actual core logic and is completely customizable:

**mycore.v** - Main Core Module
- Video timing generation (NTSC/PAL)
- Pixel generation logic
- Sync signal generation
- Core-specific state machines

**Supporting Modules:**
- `lfsr.v`: Linear Feedback Shift Register for randomization
- `cos.sv`: Cosine lookup table for waveform generation
- `pll.v`: Core-specific phase-locked loop

## Design Patterns

### 1. Hierarchical Module Structure

The architecture follows a strict hierarchy:
```
sys_top (Hardware)
    └── emu (Glue Logic)
        └── mycore (Core Implementation)
            ├── cos (Cosine LUT)
            ├── lfsr (Random Generator)
            └── pll (Core PLL)
```

### 2. Interface Standardization

All cores must implement the standard `emu` module interface:
- Fixed signal names and widths
- Standardized clock and reset handling
- Consistent video output format
- Uniform memory interface

### 3. Conditional Compilation

The framework uses Verilog macros for feature selection:

```systemverilog
`ifdef MISTER_FB
    // Framebuffer signals
    output        FB_EN,
    output  [4:0] FB_FORMAT,
    // ...
`endif

`ifdef MISTER_DUAL_SDRAM  
    // Secondary SDRAM interface
    output        SDRAM2_CLK,
    // ...
`endif
```

**Available Macros:**
- `MISTER_DEBUG_NOHDMI`: Disable HDMI for faster compilation
- `MISTER_DUAL_SDRAM`: Enable dual SDRAM support
- `MISTER_FB`: Enable framebuffer functionality
- `MISTER_SMALL_VBUF`: Reduce video buffer size

### 4. Clock Domain Architecture

```
CLK_50M (Board) ──┐
                  ├── sys_pll ──── Various system clocks
                  │
                  └── core_pll ──── Core-specific clocks
                                   │
                                   ├── clk_sys (Core clock)
                                   ├── CLK_VIDEO (Video clock)
                                   └── CE_PIXEL (Pixel enable)
```

### 5. Configuration Management

The framework uses a string-based configuration system:

```systemverilog
localparam CONF_STR = {
    "MyCore;;",                                    // Core name
    "-;",                                          // Separator
    "O[122:121],Aspect ratio,Original,Full Screen,[ARC1],[ARC2];",
    "O[2],TV Mode,NTSC,PAL;",                     // Option definitions
    "O[4:3],Noise,White,Red,Green,Blue;",
    // ... menu structure
};
```

## Memory Architecture

### SDRAM Interface

```
Core ←→ SDRAM Controller ←→ Physical SDRAM
      │                   │
      │                   ├── Address: 13 bits
      │                   ├── Data: 16 bits  
      │                   ├── Bank: 2 bits
      │                   └── Control signals
      │
      └── Optional: Dual SDRAM support
```

### DDR3 Interface (High Latency)

```
Core ←→ DDR3 Controller ←→ HPS DDR3 Memory
      │                  │
      │                  ├── Burst-oriented access
      │                  ├── 64-bit data width
      │                  └── 29-bit addressing
```

## Signal Flow Architecture

### Video Pipeline

```
Core Video Logic → mycore.sv → video_mixer.sv → Output Drivers
     │                │             │              │
   Pixel Gen        Format         Process        HDMI/VGA
   Timing Gen       Convert        Scale/Filter   
   Sync Gen         Assign         Gamma Correct
```

### Audio Pipeline

```
Core Audio Logic → mycore.sv → audio_out.v → Physical Outputs  
     │               │            │            │
   Sample Gen      Format       Filter       I2S/SPDIF
   Rate Gen        Convert      Mix          Analog Out
```

### Input Processing

```
Physical Inputs → hps_io.sv → mycore.sv → Core Logic
     │              │           │           │
   Joystick       Process     Interpret   Game Logic
   Keyboard       Debounce    Map         State Update
   Menu           Buffer      Distribute
```

## File System Organization

### Standard Directory Structure

```
Template_MiSTer/
├── sys/                    # Framework (DO NOT MODIFY)
│   ├── sys_top.v          # Hardware abstraction
│   ├── hps_io.sv          # ARM communication
│   ├── video_mixer.sv     # Video processing
│   ├── audio_out.v        # Audio processing
│   └── ...                # Other framework modules
├── rtl/                   # Core implementation
│   ├── mycore.v           # Main core logic
│   ├── pll.v              # Core PLL
│   └── ...                # Core-specific modules
├── mycore.sv              # Glue logic layer
├── mycore.qsf             # Quartus project settings
├── mycore.qpf             # Quartus project file
├── files.qip              # File list for compilation
└── releases/              # Compiled RBF files
```

### Framework Immutability

**Critical Rule**: The `sys/` directory must never be modified. All framework updates are distributed by updating this directory, and any local changes will be lost.

**Rationale:**
- Ensures compatibility across all cores
- Enables centralized bug fixes and improvements
- Maintains standardized interfaces
- Simplifies core maintenance and updates

## Build System Integration

### Quartus Integration

The template integrates with Intel Quartus Prime through:

- **Project Files**: `.qpf` and `.qsf` define project structure
- **File Lists**: `files.qip` manages source file inclusion
- **Constraints**: `.sdc` files provide timing constraints
- **Build Scripts**: TCL scripts automate build processes

### Version Control Integration

- `.gitignore`: Excludes build artifacts and temporary files
- Framework separation enables easy updates without merge conflicts
- Core-specific changes isolated to `rtl/` and glue logic

This architectural foundation provides a robust, scalable platform for MiSTer core development while maintaining strict separation between framework and implementation concerns.