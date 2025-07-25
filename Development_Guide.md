# MiSTer Template Core Development Guide

## Overview

This guide provides step-by-step instructions for using the MiSTer Template Core to develop new FPGA cores. It covers the complete development workflow, from initial setup through core implementation, testing, and release preparation. This template serves as the foundation for all MiSTer cores and ensures compatibility with the MiSTer framework.

## Getting Started

### Template Setup

**1. Clone the Template Repository**
```bash
git clone https://github.com/MiSTer-devel/Template_MiSTer.git MyNewCore
cd MyNewCore
```

**2. Customize Project Files**
```bash
# Rename core files (replace 'mynewcore' with your core name)
mv mycore.qpf mynewcore.qpf
mv mycore.qsf mynewcore.qsf  
mv mycore.sdc mynewcore.sdc
mv mycore.srf mynewcore.srf
mv mycore.sv mynewcore.sv

# Update rtl/mycore.v to rtl/mynewcore.v
mv rtl/mycore.v rtl/mynewcore.v
```

**3. Update Project References**

**mynewcore.qpf:**
```
PROJECT_REVISION = "mynewcore"
```

**files.qip:**
```
set_global_assignment -name SYSTEMVERILOG_FILE rtl/mynewcore.v
set_global_assignment -name SDC_FILE mynewcore.sdc
set_global_assignment -name SYSTEMVERILOG_FILE mynewcore.sv
```

**4. Initialize Git Repository**
```bash
# Remove original git history
rm -rf .git

# Initialize new repository
git init
git add .
git commit -m "Initial commit: Template core setup for MyNewCore"
```

### Development Environment

**Directory Structure Setup**
```
MyNewCore/
├── sys/                    # Framework (DO NOT MODIFY)
├── rtl/                    # Your core implementation
│   ├── mynewcore.v         # Main core module
│   ├── pll.v               # Core-specific PLL
│   ├── cpu/                # CPU implementation (if applicable)
│   ├── video/              # Video generation modules
│   ├── audio/              # Audio synthesis modules
│   └── memory/             # Memory controllers
├── doc/                    # Documentation
├── test/                   # Test benches and verification
├── tools/                  # Development tools and scripts
└── releases/               # Compiled RBF files
```

## Core Implementation Workflow

### Phase 1: Basic Framework Integration

**1. Define Core Configuration**

Update the configuration string in `mynewcore.sv`:

```systemverilog
localparam CONF_STR = {
    "MyNewCore;;",                              // Core name
    "-;",
    "H0O[1],Aspect ratio,Original,Wide;",       // Hidden until implemented
    "O[2],TV Mode,NTSC,PAL;",                   // Standard TV mode option
    "-;",
    "P1,Hardware;",                             // Hardware settings page
    "P1O[3],CPU Speed,Normal,Turbo;",
    "P1O[5:4],RAM Size,64K,128K,256K,512K;",
    "-;",
    "P2,Video & Audio;",                        // Video/Audio settings page  
    "P2O[6],Scanlines,Off,On;",
    "P2O[8:7],Audio Filter,Off,Low,High,Both;",
    "-;",
    "R[0],Reset;",                              // Reset option
    "J1,Fire,Jump,Start,Select;",               // Joystick definition
    "V,v",`BUILD_DATE                          // Version info
};
```

**2. Implement Basic I/O Handling**

```systemverilog
// Extract configuration from status bits
wire reset = RESET | status[0] | buttons[1];
wire pal_mode = status[2];
wire cpu_turbo = status[3];
wire [1:0] ram_size = status[5:4];
wire scanlines = status[6];
wire [1:0] audio_filter = status[8:7];

// Joystick inputs
wire [31:0] joy1 = joystick_0;
wire [31:0] joy2 = joystick_1;

// Extract individual buttons
wire fire_1 = joy1[4];
wire jump_1 = joy1[5];  
wire start_1 = joy1[6];
wire select_1 = joy1[7];
```

**3. Create Basic Core Module Interface**

```systemverilog
mynewcore core
(
    .clk(clk_sys),
    .reset(reset),
    
    // Configuration
    .pal_mode(pal_mode),
    .cpu_turbo(cpu_turbo),
    .ram_size(ram_size),
    
    // Video output
    .ce_pix(ce_pix),
    .video_r(video_r),
    .video_g(video_g),
    .video_b(video_b),
    .hsync(hsync),
    .vsync(vsync),
    .hblank(hblank),
    .vblank(vblank),
    
    // Audio output
    .audio_l(audio_l),
    .audio_r(audio_r),
    
    // Input
    .joy1(joy1[7:0]),
    .joy2(joy2[7:0])
    
    // Memory interfaces (as needed)
    // SDRAM, DDR3, etc.
);
```

### Phase 2: Core Logic Implementation

**1. Video Timing Generator**

Create a video timing module appropriate for your system:

```systemverilog
module video_timing
(
    input             clk,
    input             reset,
    input             pal_mode,
    
    output reg        ce_pix,
    output reg        hsync,
    output reg        vsync, 
    output reg        hblank,
    output reg        vblank,
    output reg [8:0]  hpos,
    output reg [8:0]  vpos
);

// Timing parameters
localparam H_TOTAL_NTSC = 341;
localparam V_TOTAL_NTSC = 262;
localparam H_TOTAL_PAL = 341;  
localparam V_TOTAL_PAL = 312;

localparam H_ACTIVE = 256;
localparam V_ACTIVE_NTSC = 240;
localparam V_ACTIVE_PAL = 240;

reg [8:0] h_count, v_count;
reg [3:0] clk_div;

// Pixel clock generation
always @(posedge clk) begin
    clk_div <= clk_div + 1'd1;
    ce_pix <= (clk_div == 4'd0);  // Divide by 16 example
end

// Horizontal timing
wire h_total = pal_mode ? H_TOTAL_PAL : H_TOTAL_NTSC;
always @(posedge clk) begin
    if (reset) begin
        h_count <= 0;
    end else if (ce_pix) begin
        if (h_count == h_total - 1) begin
            h_count <= 0;
        end else begin
            h_count <= h_count + 1'd1;
        end
    end
end

// Vertical timing  
wire v_total = pal_mode ? V_TOTAL_PAL : V_TOTAL_NTSC;
wire v_active = pal_mode ? V_ACTIVE_PAL : V_ACTIVE_NTSC;

always @(posedge clk) begin
    if (reset) begin
        v_count <= 0;
    end else if (ce_pix && h_count == h_total - 1) begin
        if (v_count == v_total - 1) begin
            v_count <= 0;
        end else begin
            v_count <= v_count + 1'd1;
        end
    end
end

// Generate sync and blanking signals
always @(posedge clk) begin
    hpos <= h_count;
    vpos <= v_count;
    
    hblank <= (h_count >= H_ACTIVE);
    vblank <= (v_count >= v_active);
    
    // Sync signal timing (adjust for your system)
    hsync <= (h_count >= H_ACTIVE + 16) && (h_count < H_ACTIVE + 16 + 32);
    vsync <= (v_count >= v_active + 8) && (v_count < v_active + 8 + 4);
end

endmodule
```

**2. CPU Core Integration**

For systems with CPUs, integrate existing CPU cores or implement custom ones:

```systemverilog
// Example: 6502 CPU integration
cpu_6502 cpu
(
    .clk(clk_sys),
    .reset(reset),
    .enable(cpu_enable),
    
    // Memory interface
    .addr(cpu_addr),
    .din(cpu_din),
    .dout(cpu_dout),
    .we(cpu_we),
    
    // Interrupt handling
    .irq(cpu_irq),
    .nmi(cpu_nmi)
);

// CPU clock enable for speed control
reg [3:0] cpu_div;
always @(posedge clk_sys) begin
    cpu_div <= cpu_div + 1'd1;
    cpu_enable <= cpu_turbo ? (cpu_div[1:0] == 2'b00) :  // 4x speed
                              (cpu_div[3:0] == 4'b0000);  // Normal speed
end
```

**3. Memory System Implementation**

Implement memory controllers and address decoding:

```systemverilog
module memory_controller
(
    input             clk,
    input             reset,
    input [1:0]       ram_size,
    
    // CPU interface
    input [15:0]      cpu_addr,
    input [7:0]       cpu_dout,
    output reg [7:0]  cpu_din,
    input             cpu_we,
    
    // External RAM interface (SDRAM)
    output reg [12:0] ram_addr,
    output reg [15:0] ram_dout,
    input [15:0]      ram_din,
    output reg        ram_we,
    output reg        ram_req,
    input             ram_ack
);

// Address decoding
wire ram_cs = (cpu_addr < ram_limit);
wire rom_cs = (cpu_addr >= 16'hF000);

// RAM size selection
reg [15:0] ram_limit;
always @(*) begin
    case (ram_size)
        2'b00: ram_limit = 16'h1000;  // 64K
        2'b01: ram_limit = 16'h2000;  // 128K
        2'b10: ram_limit = 16'h4000;  // 256K  
        2'b11: ram_limit = 16'h8000;  // 512K
    endcase
end

// Memory access logic
always @(posedge clk) begin
    if (reset) begin
        ram_req <= 0;
    end else begin
        if (ram_cs && (cpu_we || !ram_req)) begin
            ram_addr <= cpu_addr[12:0];
            ram_dout <= {cpu_dout, cpu_dout};  // 16-bit from 8-bit
            ram_we <= cpu_we;
            ram_req <= 1;
        end else if (ram_ack) begin
            ram_req <= 0;
            if (!cpu_we) begin
                cpu_din <= ram_din[7:0];  // 8-bit from 16-bit
            end
        end
    end
end

endmodule
```

### Phase 3: System Integration

**1. SDRAM Controller Integration**

For cores requiring external memory:

```systemverilog
// SDRAM controller instantiation
sdram_controller #(
    .CLK_FREQ(50_000_000),  // 50MHz system clock
    .REFRESH_MS(64)         // 64ms refresh period
) sdram_ctrl (
    .clk(clk_sys),
    .reset(reset),
    
    // Internal interface
    .addr(sdram_addr),
    .din(sdram_din), 
    .dout(sdram_dout),
    .we(sdram_we),
    .req(sdram_req),
    .ack(sdram_ack),
    
    // External SDRAM pins
    .SDRAM_CLK(SDRAM_CLK),
    .SDRAM_CKE(SDRAM_CKE),
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

**2. File Loading System**

Implement ROM/cartridge loading:

```systemverilog
// File download handling
reg [24:0] download_addr;
reg [7:0] download_data;
reg download_wr;

always @(posedge clk_sys) begin
    download_wr <= 0;
    
    if (ioctl_download && ioctl_wr) begin
        if (ioctl_index == 8'd0) begin  // ROM file
            download_addr <= ioctl_addr[24:0];
            download_data <= ioctl_dout;
            download_wr <= 1;
        end
    end
end

// ROM storage (using BRAM for small ROMs)
(* ram_init_file = "boot.mif" *) reg [7:0] boot_rom [0:4095];
always @(posedge clk_sys) begin
    if (download_wr && download_addr < 16'h1000) begin
        boot_rom[download_addr[11:0]] <= download_data;
    end
end

wire [7:0] rom_data = boot_rom[cpu_addr[11:0]];
```

**3. Audio System Implementation**

Create audio synthesis modules:

```systemverilog
module audio_system
(
    input             clk,
    input             reset,
    input [1:0]       filter_mode,
    
    // Audio sources
    input [15:0]      psg_audio,
    input [15:0]      pcm_audio,
    
    // Mixed output
    output reg [15:0] audio_l,
    output reg [15:0] audio_r
);

// Audio mixing
wire [16:0] mixed_audio = psg_audio + pcm_audio;
wire [15:0] mixed_clipped = (mixed_audio[16]) ? 16'h8000 : 
                           (mixed_audio[15]) ? 16'h7FFF : 
                           mixed_audio[15:0];

// Digital filter implementation
reg [15:0] filter_state;
wire [15:0] filtered_audio;

iir_filter audio_filter
(
    .clk(clk),
    .reset(reset),
    .enable(filter_mode != 2'b00),
    .cutoff(filter_mode),
    .input_sample(mixed_clipped),
    .output_sample(filtered_audio)
);

always @(posedge clk) begin
    audio_l <= filtered_audio;
    audio_r <= filtered_audio;  // Mono output
end

endmodule
```

## Testing and Validation

### Simulation Testing

**1. Create Test Bench**

```systemverilog
module mynewcore_tb;

reg clk, reset;
reg [31:0] joystick_0;
wire [7:0] vga_r, vga_g, vga_b;
wire vga_hs, vga_vs;

// Instantiate core
mynewcore dut
(
    .CLK_50M(clk),
    .RESET(reset),
    .HPS_BUS(),  // Leave unconnected for simulation
    .VGA_R(vga_r),
    .VGA_G(vga_g), 
    .VGA_B(vga_b),
    .VGA_HS(vga_hs),
    .VGA_VS(vga_vs)
    // ... other connections
);

// Clock generation
initial begin
    clk = 0;
    forever #10 clk = ~clk;  // 50MHz
end

// Test sequence
initial begin
    reset = 1;
    joystick_0 = 32'h0;
    
    #1000;
    reset = 0;
    
    // Test joystick input
    #10000;
    joystick_0[4] = 1;  // Press fire button
    
    #50000;
    joystick_0[4] = 0;  // Release fire button
    
    #1000000;
    $finish;
end

// Monitor video timing
always @(posedge vga_vs) begin
    $display("VSYNC at time %t", $time);
end

endmodule
```

**2. Functional Verification**

```bash
# Compile and run simulation (ModelSim)
vlog *.sv rtl/*.v
vsim -c mynewcore_tb -do "run -all; quit"

# Review simulation output
grep "ERROR\|WARNING" transcript
```

### Hardware Testing

**1. SignalTap Integration**

```systemverilog
// Add debug signals to your core
(* keep *) wire [7:0] debug_cpu_state;
(* keep *) wire [15:0] debug_cpu_addr;
(* keep *) wire debug_cpu_we;

// Connect to your CPU/core signals
assign debug_cpu_state = cpu.state;
assign debug_cpu_addr = cpu.addr;
assign debug_cpu_we = cpu.we;
```

Add SignalTap file (debug.stp) to capture these signals during runtime.

**2. Progressive Testing Strategy**

```
Phase 1: Basic video output
- Verify sync signal generation
- Test color output (solid colors)
- Confirm timing meets specifications

Phase 2: Input handling  
- Test joystick input registration
- Verify menu system integration
- Check reset functionality

Phase 3: Core functionality
- Test CPU execution (if applicable)
- Verify memory access patterns
- Check audio output

Phase 4: System integration
- Test file loading
- Verify save/load functionality
- Performance optimization
```

## Advanced Features

### Save State Implementation

```systemverilog
// Save state interface
reg [15:0] ss_addr;
reg [7:0] ss_din, ss_dout;
reg ss_we, ss_req;
wire ss_ack;

// State save/load logic
always @(posedge clk_sys) begin
    ss_req <= 0;
    
    if (ioctl_download && ioctl_index == 8'd2) begin  // Save state
        if (ioctl_wr) begin
            // Load state data
            case (ioctl_addr)
                16'h0000: cpu_reg_a <= ioctl_dout;
                16'h0001: cpu_reg_x <= ioctl_dout;
                16'h0002: cpu_reg_y <= ioctl_dout;
                // ... other CPU registers
            endcase
        end
    end
end
```

### Cheats and Game Genie Support

```systemverilog
// Cheat code application
reg [15:0] cheat_addr [0:15];
reg [7:0] cheat_data [0:15];
reg [15:0] cheat_enable;

always @(posedge clk_sys) begin
    // Apply active cheats during memory read
    for (int i = 0; i < 16; i++) begin
        if (cheat_enable[i] && cpu_addr == cheat_addr[i] && !cpu_we) begin
            cpu_din <= cheat_data[i];
        end
    end
end
```

### Multi-region Support

```systemverilog
// Region detection and configuration
wire region_ntsc = status[10];
wire region_60hz = status[11];

// Adjust timing based on region
wire pixel_clock_freq = region_60hz ? FREQ_NTSC : FREQ_PAL;
wire lines_per_frame = region_ntsc ? LINES_NTSC : LINES_PAL;
```

## Performance Optimization

### Resource Usage Analysis

**1. Monitor Resource Usage**
```bash
# Extract resource usage from compilation report
grep -A 20 "Logic utilization" output_files/mynewcore.fit.rpt
grep -A 10 "Total RAM" output_files/mynewcore.fit.rpt
```

**2. Optimize Critical Paths**
```systemverilog
// Pipeline complex operations
always @(posedge clk) begin
    // Stage 1: Address calculation
    addr_stage1 <= base_addr + offset;
    
    // Stage 2: Memory access
    mem_data_stage2 <= memory[addr_stage1];
    
    // Stage 3: Data processing  
    result <= process_data(mem_data_stage2);
end
```

### Memory Optimization

**1. Efficient Memory Usage**
```systemverilog
// Use appropriate memory types
(* ramstyle = "M10K" *) reg [7:0] large_buffer [0:8191];   // Use M10K
(* ramstyle = "MLAB" *) reg [7:0] small_buffer [0:63];     // Use MLAB
(* ramstyle = "logic" *) reg [7:0] tiny_buffer [0:7];      // Use logic
```

**2. Memory Access Optimization**
```systemverilog
// Burst memory access for better SDRAM efficiency
reg [2:0] burst_count;
always @(posedge clk) begin
    if (start_burst) begin
        burst_count <= 3'd0;
        sdram_addr <= start_addr;
        sdram_req <= 1;
    end else if (sdram_ack && burst_count < 3'd7) begin
        burst_count <= burst_count + 1'd1;
        sdram_addr <= sdram_addr + 1'd1;
        // Process received data
    end
end
```

## Release Preparation

### Documentation

**1. Core-Specific Documentation**
Create comprehensive documentation including:
- Core overview and features
- Supported file formats
- Control mapping
- Known issues and limitations
- Version history

**2. README.md Template**
```markdown
# MyNewCore for MiSTer

## Overview
Brief description of the original system and core features.

## Features
- List of implemented features
- Accuracy notes
- Special capabilities

## Installation
1. Copy `MyNewCore_YYYYMMDD.rbf` to `/media/fat/_Computer/`
2. Create folder `/media/fat/games/MyNewCore/`
3. Copy ROM files to the games directory

## Controls
| Original | MiSTer |
|----------|--------|
| Fire     | A      |
| Jump     | B      |

## File Formats
- .ROM - Standard ROM format
- .SAV - Save state files

## Known Issues
- List any known problems
- Performance limitations

## Version History
### v1.0 (2024-03-15)
- Initial release
```

### Quality Assurance

**1. Final Testing Checklist**
```
□ Video output correct on HDMI and VGA
□ Audio output levels appropriate
□ All menu options functional
□ File loading works correctly
□ Save states function properly
□ Reset behavior correct
□ Resource usage within limits
□ Timing constraints met
□ No critical warnings in compilation
```

**2. Compatibility Testing**
```
□ Test on different MiSTer hardware revisions
□ Verify with various displays/monitors
□ Check with different file sizes
□ Test edge cases and error conditions
```

### Release Package

**1. File Organization**
```
releases/
├── MyNewCore_20240315.rbf          # Core binary
├── MyNewCore.mra                   # MRA file (if arcade)
├── README.md                       # Documentation
├── CHANGELOG.md                    # Version history
└── screenshots/                    # Screenshots
    ├── title_screen.png
    └── gameplay.png
```

**2. Version Tagging**
```bash
# Tag release in git
git tag -a v1.0.0 -m "Release version 1.0.0 - Initial public release"
git push origin v1.0.0

# Create GitHub release with binaries
gh release create v1.0.0 releases/MyNewCore_20240315.rbf \
    --title "MyNewCore v1.0.0" \
    --notes-file CHANGELOG.md
```

## Community Guidelines

### Development Standards

**1. Code Quality**
- Use consistent indentation and formatting
- Include meaningful comments
- Follow Verilog/SystemVerilog best practices
- Use descriptive signal and module names

**2. Framework Compliance**
- Never modify files in `sys/` directory
- Follow standard interface conventions
- Use approved configuration string format
- Implement standard menu options where applicable

**3. Testing Requirements**
- Provide simulation test benches
- Document test procedures
- Include performance benchmarks
- Test with multiple file types/sizes

### Collaboration

**1. Contributing to Existing Cores**
- Fork the repository
- Create feature branches
- Submit pull requests with clear descriptions
- Follow the existing code style

**2. Sharing New Cores**
- Open source preferred (GPL license)
- Provide complete documentation
- Include build instructions
- Respond to community feedback

### Support and Maintenance

**1. Issue Tracking**
- Use GitHub issues for bug reports
- Provide clear reproduction steps
- Include system information and logs
- Respond promptly to user reports

**2. Updates and Fixes**
- Follow semantic versioning (major.minor.patch)
- Document all changes in CHANGELOG.md
- Test thoroughly before releases
- Maintain backward compatibility when possible

This development guide provides a comprehensive framework for creating new MiSTer cores using the template. Following these guidelines ensures compatibility with the MiSTer platform and maintainability of your core over time.