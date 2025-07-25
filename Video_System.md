# MiSTer Template Core Video System

## Overview

The MiSTer video system provides a comprehensive pipeline for generating, processing, and outputting video signals. This document details the video generation architecture, timing requirements, signal processing pipeline, and display subsystems used in the MiSTer Template Core.

## Video System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Video Generation Pipeline                    │
├─────────────────────────────────────────────────────────────────────┤
│  Core Video Logic (mycore.v)                                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────────┐  │
│  │   Timing    │  │   Pixel     │  │      Sync Generation        │  │
│  │  Generator  │  │ Generation  │  │   (HSync/VSync/Blanking)    │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────────┘  │
│         │               │                        │                  │
│         ▼               ▼                        ▼                  │
├─────────────────────────────────────────────────────────────────────┤
│  Glue Logic Layer (mycore.sv)                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │              Signal Format Conversion                       │    │
│  │    (Color mapping, sync polarity, aspect ratio)            │    │
│  └─────────────────────────────────────────────────────────────┘    │
│         │                                                           │
│         ▼                                                           │
├─────────────────────────────────────────────────────────────────────┤
│  Framework Video Processing (sys/)                                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────────┐  │
│  │ video_mixer │  │scandoubler  │  │       Output Drivers        │  │
│  │    .sv      │  │    .v       │  │    (VGA/HDMI/Composite)    │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────────┘  │
│         │               │                        │                  │
│         ▼               ▼                        ▼                  │
├─────────────────────────────────────────────────────────────────────┤
│  Physical Outputs                                                   │
│  VGA Connector    HDMI Port    Composite/Component (via I/O board)  │
└─────────────────────────────────────────────────────────────────────┘
```

## Video Timing Generation

### Core Timing Generator (mycore.v)

The core timing generator is responsible for creating the basic video timing signals required for both NTSC and PAL standards.

```systemverilog
module mycore
(
    input         clk,          // Core system clock
    input         reset,        // System reset
    input         pal,          // PAL mode selection
    input         scandouble,   // Scandoubler enable
    
    output reg    ce_pix,       // Pixel clock enable
    output reg    HBlank,       // Horizontal blanking
    output reg    HSync,        // Horizontal sync
    output reg    VBlank,       // Vertical blanking  
    output reg    VSync,        // Vertical sync
    output  [7:0] video         // Video pixel data
);
```

### Timing Counter Implementation

```systemverilog
reg   [9:0] hc;    // Horizontal counter (0-637)
reg   [9:0] vc;    // Vertical counter (0-261 NTSC, 0-311 PAL)
reg   [9:0] vvc;   // Vertical animation counter

always @(posedge clk) begin
    if(scandouble) 
        ce_pix <= 1;           // Always enabled for VGA mode
    else 
        ce_pix <= ~ce_pix;     // Toggle for 15kHz mode

    if(reset) begin
        hc <= 0;
        vc <= 0;
    end
    else if(ce_pix) begin
        if(hc == 637) begin                    // End of line
            hc <= 0;
            if(vc == (pal ? (scandouble ? 623 : 311) : 
                           (scandouble ? 523 : 261))) begin
                vc <= 0;                       // End of frame
                vvc <= vvc + 9'd6;             // Animation counter
            end else begin
                vc <= vc + 1'd1;
            end
        end else begin
            hc <= hc + 1'd1;
        end
    end
end
```

### Video Timing Standards

#### NTSC Timing (525 lines, 59.94Hz)

| Parameter | Progressive | Interlaced | Description |
|-----------|-------------|------------|-------------|
| Total Lines | 525 | 262.5 | Lines per frame/field |
| Active Lines | 480 | 240 | Visible scanlines |
| Total Pixels | 638 | 638 | Pixels per line |
| Active Pixels | 529 | 529 | Visible pixels per line |
| H-Sync Start | 544 | 544 | Horizontal sync start |
| H-Sync End | 590 | 590 | Horizontal sync end |
| H-Blank Start | 529 | 529 | Horizontal blanking start |
| V-Sync Start | 490/245 | 245 | Vertical sync start |
| V-Sync End | 496/248 | 248 | Vertical sync end |
| V-Blank Start | 480/240 | 240 | Vertical blanking start |

#### PAL Timing (625 lines, 50Hz)

| Parameter | Progressive | Interlaced | Description |
|-----------|-------------|------------|-------------|
| Total Lines | 625 | 312.5 | Lines per frame/field |
| Active Lines | 576 | 288 | Visible scanlines |
| Total Pixels | 638 | 638 | Pixels per line |
| Active Pixels | 529 | 529 | Visible pixels per line |
| H-Sync Start | 544 | 544 | Horizontal sync start |
| H-Sync End | 590 | 590 | Horizontal sync end |
| H-Blank Start | 529 | 529 | Horizontal blanking start |
| V-Sync Start | 609/304 | 304 | Vertical sync start |
| V-Sync End | 617/308 | 308 | Vertical sync end |
| V-Blank Start | 601/300 | 300 | Vertical blanking start |

### Sync Signal Generation

```systemverilog
always @(posedge clk) begin
    // Horizontal blanking
    if (hc == 529) HBlank <= 1;
    else if (hc == 0) HBlank <= 0;

    // Horizontal sync and vertical timing
    if (hc == 544) begin
        HSync <= 1;
        
        if(pal) begin
            if(vc == (scandouble ? 609 : 304)) VSync <= 1;
            else if (vc == (scandouble ? 617 : 308)) VSync <= 0;
            
            if(vc == (scandouble ? 601 : 300)) VBlank <= 1;
            else if (vc == 0) VBlank <= 0;
        end
        else begin  // NTSC
            if(vc == (scandouble ? 490 : 245)) VSync <= 1;
            else if (vc == (scandouble ? 496 : 248)) VSync <= 0;
            
            if(vc == (scandouble ? 480 : 240)) VBlank <= 1;
            else if (vc == 0) VBlank <= 0;
        end
    end
    
    if (hc == 590) HSync <= 0;
end
```

## Pixel Generation Pipeline

### Demonstration Video Generator

The template core includes a demonstration video generator that creates animated patterns:

```systemverilog
// Cosine wave generator for animation
reg  [7:0] cos_out;
wire [5:0] cos_g = cos_out[7:3] + 6'd32;  // Offset and scale
cos cos_lut(vvc + {vc>>scandouble, 2'b00}, cos_out);

// Random noise generator  
reg  [63:0] rnd_reg;
wire [5:0] rnd_c = {rnd_reg[0],rnd_reg[1],rnd_reg[2],
                    rnd_reg[2],rnd_reg[2],rnd_reg[2]};
wire [63:0] rnd;
lfsr random(rnd);

// Combine cosine wave with noise
assign video = (cos_g >= rnd_c) ? {cos_g - rnd_c, 2'b00} : 8'd0;
```

### Cosine Lookup Table (cos.sv)

The cosine module provides smooth waveform generation for animation:

```systemverilog
module cos (
    input   [9:0] x,    // Input angle (0-1023)
    output  [7:0] y     // Output amplitude (0-255)
);

// 256-entry cosine lookup table
wire [7:0] qcos[0:255] = '{
    8'b01111111, 8'b01111111, 8'b01111111, ...
    // Pre-computed cosine values
};

// Quadrant handling and sign extension
wire ival = ^x[9:8];
assign y = qcos[x[7:0] ^ {8{x[8]}}] ^ {~ival,{7{ival}}};

endmodule
```

### Linear Feedback Shift Register (lfsr.v)

Provides pseudo-random number generation for noise effects:

```systemverilog
module lfsr(
    output [N-1:0] rnd
);
parameter N = 63;

// Polynomial: x^63 + x^62 + x^60 + x^59 + x^57 + x^53 + 1
lcell lc0(~(rnd[N-1] ^ rnd[N-3] ^ rnd[N-4] ^ rnd[N-6] ^ rnd[N-10]), rnd[0]);

generate 
    genvar i;
    for (i = 0; i <= N-2; i = i + 1) begin : lcn
        lcell lc(rnd[i], rnd[i+1]);
    end
endgenerate

endmodule
```

## Video Signal Processing Pipeline

### Color Channel Mapping

The glue logic layer maps core video data to RGB channels based on user settings:

```systemverilog
wire [1:0] col = status[4:3];  // Color selection from menu

assign VGA_G = (!col || col == 2) ? video : 8'd0;  // Green
assign VGA_R = (!col || col == 1) ? video : 8'd0;  // Red  
assign VGA_B = (!col || col == 3) ? video : 8'd0;  // Blue
```

**Color Modes:**
- `2'b00`: White (all channels active)
- `2'b01`: Red only
- `2'b10`: Green only  
- `2'b11`: Blue only

### Aspect Ratio Control

```systemverilog
wire [1:0] ar = status[122:121];  // Aspect ratio selection

assign VIDEO_ARX = (!ar) ? 12'd4 : (ar - 1'd1);  // 4:3 or custom
assign VIDEO_ARY = (!ar) ? 12'd3 : 12'd0;        // 3 or scaled
```

**Aspect Ratio Modes:**
- `2'b00`: Original 4:3 aspect ratio
- `2'b01`: Full screen (stretched)
- `2'b10`: User-defined aspect ratio 1  
- `2'b11`: User-defined aspect ratio 2

## Framework Video Processing

### Video Mixer (video_mixer.sv)

The video mixer handles advanced video processing including scaling, gamma correction, and output formatting:

```systemverilog
module video_mixer
#(
    parameter LINE_LENGTH  = 768,   // Display line length
    parameter HALF_DEPTH   = 0,     // 4-bit vs 8-bit color depth
    parameter GAMMA        = 0      // Gamma correction enable
)
(
    input            CLK_VIDEO,     // Video clock domain
    output reg       CE_PIXEL,      // Output pixel clock enable
    input            ce_pix,        // Input pixel clock enable
    
    input            scandoubler,   // Scandoubler active
    input            hq2x,          // High quality 2x scaling
    
    // Input video signals
    input [DWIDTH:0] R, G, B,       // Color inputs
    input            HSync, VSync,   // Sync inputs
    input            HBlank, VBlank, // Blanking inputs
    
    // Output video signals
    output reg [7:0] VGA_R, VGA_G, VGA_B,  // Color outputs
    output reg       VGA_VS, VGA_HS,       // Sync outputs
    output reg       VGA_DE                 // Data enable output
);
```

### Scandoubler (scandoubler.v)

Converts 15kHz video signals to 31kHz VGA-compatible signals:

```systemverilog
module scandoubler
(
    input            clk_sys,       // System clock
    input            ce_divider,    // Clock enable
    input            hq2x,          // High quality mode
    
    // Input video  
    input      [7:0] R_in, G_in, B_in,  // RGB input
    input            HS_in, VS_in,       // Sync input
    input            HBlank, VBlank,     // Blanking input
    
    // Output video
    output reg [7:0] R_out, G_out, B_out, // RGB output  
    output reg       HS_out, VS_out,      // Sync output
    output reg       line_out              // Line indicator
);
```

**Scandoubler Operation:**
1. **Input Buffering**: Stores incoming 15kHz video lines in line buffers
2. **Line Doubling**: Outputs each line twice at 31kHz rate
3. **Interpolation**: Optional high-quality interpolation between lines
4. **Sync Conversion**: Converts sync timing from 15kHz to 31kHz

### Advanced Scaling (ascal.vhd)

ASCAL (Advanced SCaling ALgorithm) provides sophisticated video scaling:

**Features:**
- Multiple scaling algorithms (nearest neighbor, bilinear, bicubic)
- Integer and fractional scaling ratios
- Automatic aspect ratio correction
- Configurable output resolutions
- Low-latency processing

**Configuration Macros:**
- `MISTER_SMALL_VBUF`: Reduces video buffer size for memory-constrained cores
- `MISTER_DOWNSCALE_NN`: Enables nearest-neighbor downscaling
- `MISTER_DISABLE_ADAPTIVE`: Disables adaptive scanlines

## Display Output Systems

### VGA Output

**Signal Specifications:**
- **Resolution**: Up to 1920x1080@60Hz
- **Color Depth**: 8 bits per channel (24-bit total)
- **Sync Type**: Separate H/V sync, positive polarity
- **Timing**: VESA standard timing

```systemverilog
// VGA output assignments from emu module
assign VGA_DE = ~(HBlank | VBlank);  // Data enable
assign VGA_HS = HSync;               // Horizontal sync
assign VGA_VS = VSync;               // Vertical sync
```

### HDMI Output

**Signal Processing Pipeline:**
```
RGB Data → Color Space → TMDS Encoding → HDMI Transmitter
    ↓           ↓              ↓              ↓
8-bit RGB   YUV422/444    10-bit TMDS    Differential
per chan.   conversion    serialized     pairs @ 742.5MHz
```

**HDMI Features:**
- **Resolutions**: 480p, 576p, 720p, 1080p, custom resolutions
- **Color Formats**: RGB 4:4:4, YUV 4:2:2, YUV 4:4:4
- **Audio**: Embedded I2S audio up to 192kHz/24-bit
- **Timing**: Automatic timing adjustment and scaling

### Analog Video Output

Available through I/O board expansion:

**Composite Video:**
- NTSC: 525 lines, 59.94Hz, 4.43MHz chroma
- PAL: 625 lines, 50Hz, 4.43MHz chroma
- Sync: Composite sync embedded in video

**Component Video (YPbPr):**
- Y: Luminance signal (0.7V p-p)
- Pb/Pr: Color difference signals (0.7V p-p)
- Sync: Sync on luma or separate sync

**S-Video:**
- Y: Luminance (separate)
- C: Chrominance (separate)
- Higher quality than composite

## Video Processing Features

### Scanlines

Simulates CRT scanline appearance:

```systemverilog
// Scanline control from menu
assign VGA_SL = scanline_mode;  // 0=none, 1=25%, 2=50%, 3=75%
```

**Implementation:**
- Alternating line brightness reduction
- Configurable intensity levels
- Adaptive scaling based on output resolution

### Gamma Correction

Provides accurate color reproduction:

```systemverilog
`ifdef GAMMA_CORRECTION
gamma_corr gamma_r(.clk(CLK_VIDEO), .gamma_bus(gamma_bus), 
                   .color_in(R_in), .color_out(R_corrected));
gamma_corr gamma_g(.clk(CLK_VIDEO), .gamma_bus(gamma_bus),
                   .color_in(G_in), .color_out(G_corrected));  
gamma_corr gamma_b(.clk(CLK_VIDEO), .gamma_bus(gamma_bus),
                   .color_in(B_in), .color_out(B_corrected));
`endif
```

### Shadow Mask Simulation

Emulates CRT phosphor patterns:

```systemverilog
shadowmask shadow_mask
(
    .clk_vid(CLK_VIDEO),
    .ce_pix(CE_PIXEL),
    .enable(shadow_mask_enable),
    .mask_type(mask_type),    // Aperture grille, slot mask, etc.
    .R_in(R_in), .G_in(G_in), .B_in(B_in),
    .R_out(R_masked), .G_out(G_masked), .B_out(B_masked)
);
```

## Clock Domain Management

### Video Clock Architecture

```
CLK_50M ──┐
          ├── System PLL ──┬── clk_sys (Core clock)
          │                ├── CLK_VIDEO (Video clock)  
          │                └── HDMI clocks (742.5MHz, etc.)
          │
          └── Audio PLL ──┬── CLK_AUDIO (24.576MHz)
                          └── I2S clocks
```

### Clock Enable Generation

```systemverilog
// Pixel clock enable generation
always @(posedge CLK_VIDEO) begin
    if(scandouble) 
        CE_PIXEL <= 1;                    // Always enabled for VGA
    else 
        CE_PIXEL <= pixel_counter == 0;   // Divided clock enable
end
```

### Cross-Domain Synchronization

```systemverilog
// Video to system clock domain crossing
always @(posedge clk_sys) begin
    if(video_clock_enable) begin
        video_data_sync <= video_data;
        hsync_sync <= hsync;
        vsync_sync <= vsync;
    end
end
```

## Performance Optimization

### Memory Bandwidth Management

**Line Buffer Optimization:**
- Dual-port RAM for simultaneous read/write
- Ping-pong buffering for continuous operation
- Burst transfers to minimize latency

**SDRAM Access Patterns:**
- Align video data to word boundaries
- Use page-mode access when possible
- Pipeline memory requests with video timing

### Resource Utilization

**Memory Usage:**
- Line buffers: ~1KB per active video line
- Frame buffers: Resolution × color depth
- Lookup tables: ~1KB for gamma/color correction

**Logic Resources:**
- Video timing: ~100 LEs
- Pixel processing: 200-500 LEs (depending on complexity)
- Scandoubler: ~1000 LEs
- ASCAL scaler: ~2000 LEs

## Debugging and Testing

### Video Signal Analysis

**Timing Verification:**
```systemverilog
// Add to debug build for timing analysis
wire timing_error = (hc > MAX_HC) || (vc > MAX_VC);
assign debug_led = timing_error;
```

**Signal Quality Monitoring:**
```systemverilog
// Detect sync signal integrity
reg hsync_valid, vsync_valid;
always @(posedge CLK_VIDEO) begin
    hsync_valid <= (hsync_count >= MIN_HSYNC) && (hsync_count <= MAX_HSYNC);
    vsync_valid <= (vsync_count >= MIN_VSYNC) && (vsync_count <= MAX_VSYNC);
end
```

### Test Patterns

**Color Bars:**
```systemverilog
// Generate standard color bar test pattern
wire [7:0] colorbar_r, colorbar_g, colorbar_b;
test_pattern_generator tpg
(
    .clk(CLK_VIDEO),
    .ce_pix(CE_PIXEL),
    .hc(hc), .vc(vc),
    .pattern_select(test_pattern),
    .r_out(colorbar_r),
    .g_out(colorbar_g), 
    .b_out(colorbar_b)
);
```

### Common Issues and Solutions

**Issue: Flickering Display**
- *Cause*: Clock domain crossing violations
- *Solution*: Proper synchronization with CE_PIXEL

**Issue: Color Artifacts**  
- *Cause*: Incorrect gamma correction or color space conversion
- *Solution*: Verify lookup tables and color mapping

**Issue: Sync Problems**
- *Cause*: Timing violations or incorrect sync polarity
- *Solution*: Check timing parameters and sync signal generation

**Issue: Scaling Artifacts**
- *Cause*: Insufficient line buffer depth or timing issues
- *Solution*: Increase buffer size, verify ASCAL configuration

This video system documentation provides comprehensive coverage of the MiSTer video pipeline, from basic timing generation through advanced scaling and output processing. The modular architecture allows cores to implement simple video generation while leveraging the framework's sophisticated processing capabilities.