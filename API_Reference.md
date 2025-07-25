# MiSTer Template Core API Reference

## Overview

This document provides comprehensive reference documentation for the MiSTer Template Core APIs, module interfaces, and signal specifications. It serves as the definitive guide for understanding how to interface with the MiSTer framework and implement core-specific functionality.

## Core Module Interface (emu)

The `emu` module is the primary interface between the MiSTer framework and your core implementation. All cores must implement this exact interface.

### Module Declaration

```systemverilog
module emu
(
    // Master input clock
    input         CLK_50M,
    
    // Async reset from top-level module
    input         RESET,
    
    // Must be passed to hps_io module  
    inout  [48:0] HPS_BUS,
    
    // Video interface
    output        CLK_VIDEO,
    output        CE_PIXEL,
    output [12:0] VIDEO_ARX,
    output [12:0] VIDEO_ARY,
    output  [7:0] VGA_R,
    output  [7:0] VGA_G, 
    output  [7:0] VGA_B,
    output        VGA_HS,
    output        VGA_VS,
    output        VGA_DE,
    output        VGA_F1,
    output  [1:0] VGA_SL,
    output        VGA_SCALER,
    output        VGA_DISABLE,
    
    // HDMI interface
    input  [11:0] HDMI_WIDTH,
    input  [11:0] HDMI_HEIGHT, 
    output        HDMI_FREEZE,
    output        HDMI_BLACKOUT,
    output        HDMI_BOB_DEINT,
    
    // Audio interface
    input         CLK_AUDIO,
    output [15:0] AUDIO_L,
    output [15:0] AUDIO_R,
    output        AUDIO_S,
    output  [1:0] AUDIO_MIX,
    
    // LED and button interface
    output        LED_USER,
    output  [1:0] LED_POWER,
    output  [1:0] LED_DISK,
    output  [1:0] BUTTONS,
    
    // Memory interfaces
    output        DDRAM_CLK,
    input         DDRAM_BUSY,
    output  [7:0] DDRAM_BURSTCNT,
    output [28:0] DDRAM_ADDR,
    input  [63:0] DDRAM_DOUT,
    input         DDRAM_DOUT_READY,
    output        DDRAM_RD,
    output [63:0] DDRAM_DIN,
    output  [7:0] DDRAM_BE,
    output        DDRAM_WE,
    
    output        SDRAM_CLK,
    output        SDRAM_CKE,
    output [12:0] SDRAM_A,
    output  [1:0] SDRAM_BA,
    inout  [15:0] SDRAM_DQ,
    output        SDRAM_DQML,
    output        SDRAM_DQMH,
    output        SDRAM_nCS,
    output        SDRAM_nCAS,
    output        SDRAM_nRAS,
    output        SDRAM_nWE,
    
    // I/O interfaces
    output        SD_SCK,
    output        SD_MOSI,
    input         SD_MISO,
    output        SD_CS,
    input         SD_CD,
    
    inout   [3:0] ADC_BUS,
    
    input         UART_CTS,
    output        UART_RTS,
    input         UART_RXD,
    output        UART_TXD,
    output        UART_DTR,
    input         UART_DSR,
    
    input   [6:0] USER_IN,
    output  [6:0] USER_OUT,
    
    input         OSD_STATUS
);
```

## Signal Reference

### Clock and Reset Signals

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `CLK_50M` | Input | 1 | Master 50MHz clock from board oscillator |
| `RESET` | Input | 1 | Asynchronous reset from top-level module |
| `CLK_VIDEO` | Output | 1 | Base video clock, usually equals CLK_SYS |
| `CE_PIXEL` | Output | 1 | Pixel clock enable, must be based on CLK_VIDEO |

**Usage Example:**
```systemverilog
// Generate core system clock
wire clk_sys;
pll pll(
    .refclk(CLK_50M),
    .rst(0),
    .outclk_0(clk_sys)
);

assign CLK_VIDEO = clk_sys;
assign CE_PIXEL = ce_pix;  // From core timing generator
```

### HPS Communication Bus

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `HPS_BUS` | Inout | 49 | Bidirectional bus for ARM-FPGA communication |

**Usage:**
```systemverilog
hps_io #(.CONF_STR(CONF_STR)) hps_io
(
    .clk_sys(clk_sys),
    .HPS_BUS(HPS_BUS),
    .buttons(buttons),
    .status(status),
    .ps2_key(ps2_key)
    // ... other connections
);
```

### Video Output Signals

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `VIDEO_ARX` | Output | 13 | Video aspect ratio X (4:3 = 4, scaled if bit 12 set) |
| `VIDEO_ARY` | Output | 13 | Video aspect ratio Y (4:3 = 3, scaled if bit 12 set) |
| `VGA_R` | Output | 8 | Red video component (0-255) |
| `VGA_G` | Output | 8 | Green video component (0-255) |
| `VGA_B` | Output | 8 | Blue video component (0-255) |
| `VGA_HS` | Output | 1 | Horizontal sync (positive pulse) |
| `VGA_VS` | Output | 1 | Vertical sync (positive pulse) |
| `VGA_DE` | Output | 1 | Data enable (~(VBlank \| HBlank)) |
| `VGA_F1` | Output | 1 | Field indicator for interlaced video |
| `VGA_SL` | Output | 2 | Scanline control (0=none, 1=25%, 2=50%, 3=75%) |
| `VGA_SCALER` | Output | 1 | Force VGA scaler usage |
| `VGA_DISABLE` | Output | 1 | Disable analog video output |

**Aspect Ratio Calculation:**
```systemverilog
wire [1:0] ar = status[122:121];  // From menu selection
assign VIDEO_ARX = (!ar) ? 12'd4 : (ar - 1'd1);
assign VIDEO_ARY = (!ar) ? 12'd3 : 12'd0;
```

**Video Signal Assignment:**
```systemverilog
assign VGA_DE = ~(HBlank | VBlank);
assign VGA_HS = HSync;
assign VGA_VS = VSync;
assign VGA_R = (!col || col == 1) ? video : 8'd0;
assign VGA_G = (!col || col == 2) ? video : 8'd0;
assign VGA_B = (!col || col == 3) ? video : 8'd0;
```

### HDMI-Specific Signals

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `HDMI_WIDTH` | Input | 12 | Current HDMI resolution width |
| `HDMI_HEIGHT` | Input | 12 | Current HDMI resolution height |
| `HDMI_FREEZE` | Output | 1 | Freeze HDMI output (displays last frame) |
| `HDMI_BLACKOUT` | Output | 1 | Force HDMI output to black |
| `HDMI_BOB_DEINT` | Output | 1 | Enable bob deinterlacing |

### Audio Interface

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `CLK_AUDIO` | Input | 1 | 24.576 MHz audio master clock |
| `AUDIO_L` | Output | 16 | Left audio channel (signed/unsigned per AUDIO_S) |
| `AUDIO_R` | Output | 16 | Right audio channel (signed/unsigned per AUDIO_S) |
| `AUDIO_S` | Output | 1 | 1=signed samples, 0=unsigned samples |
| `AUDIO_MIX` | Output | 2 | Mix mode: 0=no mix, 1=25%, 2=50%, 3=100% (mono) |

**Audio Usage Example:**
```systemverilog
assign AUDIO_S = 1;      // Signed audio samples
assign AUDIO_L = audio_left_sample;
assign AUDIO_R = audio_right_sample;  
assign AUDIO_MIX = 0;    // No mixing
```

### Memory Interfaces

#### SDRAM Interface (Low Latency)

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `SDRAM_CLK` | Output | 1 | SDRAM clock |
| `SDRAM_CKE` | Output | 1 | Clock enable |
| `SDRAM_A` | Output | 13 | Address bus |
| `SDRAM_BA` | Output | 2 | Bank address |
| `SDRAM_DQ` | Inout | 16 | Data bus |
| `SDRAM_DQML` | Output | 1 | Data mask low byte |
| `SDRAM_DQMH` | Output | 1 | Data mask high byte |
| `SDRAM_nCS` | Output | 1 | Chip select (active low) |
| `SDRAM_nCAS` | Output | 1 | Column address strobe (active low) |
| `SDRAM_nRAS` | Output | 1 | Row address strobe (active low) |
| `SDRAM_nWE` | Output | 1 | Write enable (active low) |

**SDRAM Controller Usage:**
```systemverilog
sdram_controller sdram_ctrl
(
    .clk(SDRAM_CLK),
    .addr(sdram_addr),
    .din(sdram_din),
    .dout(sdram_dout),
    .we(sdram_we),
    .req(sdram_req),
    .ack(sdram_ack),
    
    // SDRAM interface
    .SDRAM_A(SDRAM_A),
    .SDRAM_BA(SDRAM_BA),
    .SDRAM_DQ(SDRAM_DQ),
    .SDRAM_DQML(SDRAM_DQML),
    .SDRAM_DQMH(SDRAM_DQMH),
    .SDRAM_nCS(SDRAM_nCS),
    .SDRAM_nCAS(SDRAM_nCAS),
    .SDRAM_nRAS(SDRAM_nRAS),
    .SDRAM_nWE(SDRAM_nWE)
);
```

#### DDR3 Interface (High Latency)

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `DDRAM_CLK` | Output | 1 | DDR3 clock domain |  
| `DDRAM_BUSY` | Input | 1 | Controller busy signal |
| `DDRAM_BURSTCNT` | Output | 8 | Burst count (1-256) |
| `DDRAM_ADDR` | Output | 29 | Address (word-aligned) |
| `DDRAM_DOUT` | Input | 64 | Read data |
| `DDRAM_DOUT_READY` | Input | 1 | Read data valid |
| `DDRAM_RD` | Output | 1 | Read request |
| `DDRAM_DIN` | Output | 64 | Write data |
| `DDRAM_BE` | Output | 8 | Byte enable mask |
| `DDRAM_WE` | Output | 1 | Write request |

**DDR3 Read Transaction:**
```systemverilog
// Read burst from DDR3
assign DDRAM_ADDR = read_address;
assign DDRAM_BURSTCNT = 8'd4;  // 4 x 64-bit words
assign DDRAM_RD = start_read & ~DDRAM_BUSY;

always @(posedge DDRAM_CLK) begin
    if (DDRAM_DOUT_READY) begin
        read_data <= DDRAM_DOUT;
        // Process read data
    end
end
```

### LED and Status Indicators

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `LED_USER` | Output | 1 | User LED control (1=ON, 0=OFF) |
| `LED_POWER` | Output | 2 | Power LED: [1]=override, [0]=state |
| `LED_DISK` | Output | 2 | Disk LED: [1]=override, [0]=state |
| `BUTTONS` | Output | 2 | Button simulation: [1]=user, [0]=OSD |

**LED Usage:**
```systemverilog
// Activity indicator using counter
reg [26:0] act_cnt;
always @(posedge clk_sys) act_cnt <= act_cnt + 1'd1;
assign LED_USER = act_cnt[26] ? act_cnt[25:18] > act_cnt[7:0] : 
                                act_cnt[25:18] <= act_cnt[7:0];

// System controls power/disk LEDs
assign LED_POWER = 2'b00;  // System control
assign LED_DISK = 2'b00;   // System control
```

## HPS I/O Module Interface

The `hps_io` module handles communication between the ARM processor and FPGA core.

### Module Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `CONF_STR` | string | - | Configuration string defining menu structure |
| `CONF_STR_BRAM` | int | 0 | Store config string in BRAM (1) or logic (0) |
| `PS2DIV` | int | 0 | PS/2 clock divider |
| `WIDE` | int | 0 | 16-bit file I/O mode |
| `VDNUM` | int | 1 | Number of virtual disk drives (1-10) |
| `BLKSZ` | int | 2 | Block size: 0=128, 1=256, 2=512, ..., 7=16384 |

### Configuration String Format

The configuration string defines the OSD menu structure:

```systemverilog
localparam CONF_STR = {
    "CoreName;;",                    // Core name and separator
    "-;",                           // Menu separator
    "O[15:14],Aspect Ratio,Original,Full Screen,[ARC1],[ARC2];",
    "O[2],TV Mode,NTSC,PAL;",
    "O[4:3],Color,White,Red,Green,Blue;",
    "T[0],Reset;",                  // Trigger (momentary)
    "R[0],Reset and close OSD;",    // Reset and close
    "P1,Page 1;",                   // Sub-page definition
    "P1O[5],Page 1 Option,Off,On;", // Option in sub-page
    "F1,BIN;",                      // File selector
    "S0,DSK;",                      // Save file
    "v,0;",                         // Config version
    "V,v",`BUILD_DATE              // Version display
};
```

**Configuration String Elements:**

| Format | Description | Example |
|--------|-------------|---------|
| `O[n:m],Name,Opt1,Opt2,...` | Options (bits n:m) | `O[2],Mode,NTSC,PAL` |
| `T[n],Name` | Trigger (momentary) | `T[0],Reset` |
| `R[n],Name` | Reset trigger | `R[0],Reset and close OSD` |  
| `F[n],EXT` | File selector | `F1,ROM,Load ROM` |
| `S[n],EXT` | Save file | `S0,SAV,Save State` |
| `P[n],Name` | Sub-page | `P1,Settings` |
| `-;` | Menu separator | `-;` |
| `H[n]` | Hide option if bit n=0 | `H0O[5],Hidden Option` |
| `d[n]` | Disable option if bit n=0 | `d0F1,ROM` |

### Status and Control Signals

```systemverilog
hps_io #(.CONF_STR(CONF_STR)) hps_io
(
    .clk_sys(clk_sys),
    .HPS_BUS(HPS_BUS),
    
    // Menu and control
    .buttons(buttons),              // [1:0] OSD and user buttons
    .status(status),                // [127:0] Status bits from menu
    .status_menumask(menumask),     // [15:0] Hide menu items
    .forced_scandoubler(forced_scandoubler),
    
    // Input devices
    .ps2_key(ps2_key),              // [10:0] PS/2 keyboard
    .joystick_0(joystick_0),        // [31:0] Joystick 0
    .joystick_1(joystick_1),        // [31:0] Joystick 1
    
    // File I/O
    .ioctl_download(ioctl_download), // File download active
    .ioctl_index(ioctl_index),       // File type index
    .ioctl_wr(ioctl_wr),            // Write enable
    .ioctl_addr(ioctl_addr),         // Write address
    .ioctl_dout(ioctl_dout)          // Write data
);
```

**Status Bit Mapping:**
```systemverilog
wire [127:0] status;
wire reset = RESET | status[0] | buttons[1];
wire pal_mode = status[2];
wire [1:0] color = status[4:3];
wire [1:0] aspect = status[122:121];
```

## Core Implementation Module Interface

Your core implementation should follow this interface pattern:

```systemverilog
module mycore
(
    input         clk,          // Core system clock
    input         reset,        // Core reset
    
    // Configuration
    input         pal,          // PAL mode (vs NTSC)
    input         scandouble,   // Scandoubler active
    
    // Video timing output
    output reg    ce_pix,       // Pixel clock enable
    output reg    HBlank,       // Horizontal blank
    output reg    HSync,        // Horizontal sync
    output reg    VBlank,       // Vertical blank  
    output reg    VSync,        // Vertical sync
    
    // Video data
    output  [7:0] video         // Video output data
);
```

### Video Timing Standards

**NTSC Timing (525 lines, 60Hz):**
- Total lines: 525 (262.5 per field for interlaced)
- Active lines: 480 (240 per field)
- Line time: 63.5μs (15.734kHz)
- Horizontal sync: 4.7μs
- Vertical sync: 3 lines

**PAL Timing (625 lines, 50Hz):**
- Total lines: 625 (312.5 per field for interlaced)  
- Active lines: 576 (288 per field)
- Line time: 64μs (15.625kHz)
- Horizontal sync: 4.7μs
- Vertical sync: 2.5 lines

**Scandoubler Mode:**
- Doubles vertical resolution for VGA output
- 31.5kHz horizontal frequency (vs 15.7kHz)
- Progressive scan (non-interlaced)

## Conditional Interface Extensions

### Framebuffer Interface (MISTER_FB)

```systemverilog
`ifdef MISTER_FB
    output        FB_EN,           // Framebuffer enable
    output  [4:0] FB_FORMAT,       // Format: [2:0]=bpp, [3]=555/565, [4]=RGB/BGR
    output [11:0] FB_WIDTH,        // Frame width in pixels
    output [11:0] FB_HEIGHT,       // Frame height in pixels  
    output [31:0] FB_BASE,         // Base address in DDR3
    output [13:0] FB_STRIDE,       // Line stride in bytes
    input         FB_VBL,          // Vertical blank from scaler
    input         FB_LL,           // Low latency mode
    output        FB_FORCE_BLANK,  // Force blank output

    `ifdef MISTER_FB_PALETTE
        output        FB_PAL_CLK,   // Palette clock
        output  [7:0] FB_PAL_ADDR,  // Palette address  
        output [23:0] FB_PAL_DOUT,  // Palette write data
        input  [23:0] FB_PAL_DIN,   // Palette read data
        output        FB_PAL_WR,    // Palette write enable
    `endif
`endif
```

**Framebuffer Formats:**
- `3'b011`: 8bpp with palette
- `3'b100`: 16bpp (RGB565 or RGB555)
- `3'b101`: 24bpp RGB
- `3'b110`: 32bpp RGBA

### Dual SDRAM Interface (MISTER_DUAL_SDRAM)

```systemverilog
`ifdef MISTER_DUAL_SDRAM
    input         SDRAM2_EN,       // Secondary SDRAM enable
    output        SDRAM2_CLK,      // Secondary SDRAM clock
    output [12:0] SDRAM2_A,        // Secondary SDRAM address
    output  [1:0] SDRAM2_BA,       // Secondary SDRAM bank
    inout  [15:0] SDRAM2_DQ,       // Secondary SDRAM data
    output        SDRAM2_nCS,      // Secondary SDRAM chip select
    output        SDRAM2_nCAS,     // Secondary SDRAM column strobe
    output        SDRAM2_nRAS,     // Secondary SDRAM row strobe
    output        SDRAM2_nWE,      // Secondary SDRAM write enable
`endif
```

## Error Handling and Debugging

### Common Interface Issues

1. **Clock Domain Crossing:**
```systemverilog
// Incorrect - direct connection across clock domains
assign video_out = core_video_data;

// Correct - proper clock domain crossing  
always @(posedge CLK_VIDEO) begin
    if (CE_PIXEL)
        video_out <= core_video_data;
end
```

2. **Reset Synchronization:**
```systemverilog
// Incorrect - asynchronous reset release
wire reset = RESET | status[0];

// Correct - synchronous reset release
reg reset_sync;
always @(posedge clk_sys or posedge RESET) begin
    if (RESET) reset_sync <= 1'b1;
    else reset_sync <= status[0];
end
```

3. **Unused Signal Handling:**
```systemverilog
// Always assign unused outputs to prevent floating
assign {SD_SCK, SD_MOSI, SD_CS} = 'Z;        // Tri-state
assign {UART_RTS, UART_TXD, UART_DTR} = 0;   // Logic 0
assign USER_OUT = '1;                         // Open-drain high
```

### Debugging Aids

**SignalTap Integration:**
```systemverilog
// Add to .qsf for logic analyzer
set_global_assignment -name ENABLE_SIGNALTAP ON
set_global_assignment -name USE_SIGNALTAP_FILE debug.stp
```

**LED Status Indicators:**
```systemverilog
// Multi-bit status display on single LED
assign LED_USER = error_state ? 1'b1 :           // Solid on error
                  loading ? slow_blink :          // Slow blink when loading  
                  running ? fast_blink :          // Fast blink when running
                  1'b0;                          // Off when idle
```

This API reference provides the complete interface specification for developing MiSTer cores using the template framework. All signal timings, formats, and protocols must be followed exactly to ensure compatibility with the MiSTer platform.