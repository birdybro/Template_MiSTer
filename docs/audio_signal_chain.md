# MiSTer Audio Signal Chain — Full Analysis

## 1. Audio Source (Core Domain — `clk_sys`)

The `emu` module (Template.sv:103-106) exposes four audio signals to the framework:

| Port | Width | Description |
|---|---|---|
| `AUDIO_L` | 16-bit | Left channel samples |
| `AUDIO_R` | 16-bit | Right channel samples |
| `AUDIO_S` | 1-bit | 0 = unsigned samples, 1 = signed |
| `AUDIO_MIX` | 2-bit | Stereo mix: 0=none, 1=25%, 2=50%, 3=100% (mono) |

These are generated in the core's clock domain (`clk_sys`) and wired through `sys_top.v` (lines 1667-1670, 1806-1810) into the `audio_out` module.

The template demo core outputs silence (`AUDIO_L = 0, AUDIO_R = 0`). A real core would produce samples at whatever rate it naturally generates them.

## 2. Clock Domain Crossing into `clk_audio`

`sys_top.v` instantiates a dedicated audio PLL (line 1560-1565):
```
pll_audio (refclk=FPGA_CLK3_50) → clk_audio (24.576 MHz)
```

The `audio_out` module runs entirely in `clk_audio`. The core samples (`core_l`, `core_r`) arrive from `clk_sys` and are synchronized at `audio_out.v:143-153` using a simple stability-check synchronizer:
```verilog
cl1 <= core_l; cl2 <= cl1;
if(cl2 == cl1) cl <= cl2;    // only accept when stable for 2 cycles
```
This is a basic CDC approach — the `audio_cdc` branch work aims to replace this with a proper async FIFO.

## 3. IIR Low-Pass Filter (audio_out.v:180-200)

The synchronized samples feed a **3-tap IIR biquad filter** (`iir_filter.v`). This is a configurable digital low-pass filter used for anti-aliasing/reconstruction.

- **Coefficients** are loaded at runtime by the HPS ARM CPU via IO commands (`sys_top.v:465-479`), allowing the Linux side to set filter characteristics per-core
- Default coefficients (`sys_top.v:338-345`): `aflt_rate=7056000`, with pre-calculated IIR taps
- The filter runs at `flt_rate` (configurable, default ~7 MHz) and outputs at the sample rate (48 or 96 kHz based on `audio_96k` config bit)
- Unsigned-to-signed conversion happens at the filter input: `{~is_signed ^ cl[15], cl[14:0]}`
- Output is clamped to 16-bit signed range

The filter operates in time-multiplexed stereo mode (left/right processed on alternating `ce` cycles).

## 4. DC Blocker (audio_out.v:202-222)

Post-filter, each channel passes through a **DC_blocker** module (`iir_filter.v:189-213`). This is a simplified 1-tap IIR high-pass filter that removes any DC offset from the audio signal. It has:
- Rate-dependent coefficients (different for 48 kHz vs 96 kHz)
- A startup mute period (`a_en2` flag, ~8192-16384 sample warmup via `dly2` counter) to suppress transients during filter settling

## 5. Stereo Mixer and Attenuator (audio_out.v:224-254, `aud_mix_top`)

Each channel goes through `aud_mix_top` which performs three operations:

### a) ALSA Linux audio mixing
Core audio is summed with ALSA audio from the HPS (sign-extended to 17-bit to prevent overflow):
```verilog
a2 <= a1 + {linux_audio[15], linux_audio};
```

### b) Cross-channel stereo mixing
Based on `AUDIO_MIX`:

| mix | Effect |
|---|---|
| 0 | No mix (pure stereo) |
| 1 | 25% cross-mix: 87.5% own + 12.5% opposite channel |
| 2 | 50% cross-mix: 75% own + 25% opposite |
| 3 | 100% mono: 50% left + 50% right |

Note the cross-feedback: left mixer's `pre_in` connects to right mixer's `pre_out` and vice versa.

### c) Volume attenuation
`vol_att` is a 5-bit value set by HPS via IO command `0x26`:
- Bit 4 = global mute (output forced to 0)
- Bits 3:0 = right-shift amount (0-15 dB attenuation in ~6 dB steps)
- Output is clamped to prevent signed overflow

## 6. Output Stage — Three Parallel Outputs

The final mixed samples (`al`, `ar`) feed three output encoders simultaneously, all running in `clk_audio`:

### 6a. I2S Output (i2s.v)

Standard I2S serial protocol for the **HDMI transmitter chip** (ADV7513 on the DE10-Nano):
- Clock generation: `mclk_ce` at `CE_RATE = 48000 × 16 × 8 = 6,144,000 Hz` (or 12.288 MHz for 96 kHz mode), divided from 24.576 MHz
- `i2s_ce` = half of `mclk_ce` (bit clock)
- Outputs directly to `HDMI_SCLK` (bit clock), `HDMI_LRCLK` (word select), `HDMI_I2S` (data)
- `HDMI_MCLK` is assigned directly to `clk_audio` (24.576 MHz master clock)
- 16-bit samples, MSB-first, standard I2S framing

### 6b. S/PDIF Output (spdif.v)

IEC 60958 / S/PDIF biphase-mark encoded digital audio:
- Driven by `mclk_ce` (6.144 MHz = 48000 × 32 × 2 × 2)
- Full subframe structure: preambles (B/M/W), 16-bit audio in timeslots 12-27, validity/user/channel-status/parity bits
- 192-frame audio blocks with channel status indicating 48 kHz, copy-permitted, original media
- Output on wire `spdif`

### 6c. Sigma-Delta DAC (sigma_delta_dac.v)

1-bit PDM (pulse density modulation) output for analog conversion:
- Two instances: `sd_l` and `sd_r`, 16-bit input (15-bit MSBI parameter)
- Input is converted from signed to offset-binary: `{~al[15], al[14:0]}`
- First-order sigma-delta modulator running at full `clk_audio` (24.576 MHz)
- Output: single-bit `analog_l` and `analog_r` (requires external RC low-pass filter for analog reconstruction)

## 7. Physical Pin Routing (sys_top.v:1547-1662)

The three output formats are routed to physical pins with **multiplexing controlled by dip switches and the I/O board presence**:

### GPIO Header Pins (`AUDIO_L`, `AUDIO_R`, `AUDIO_SPDIF`) — Active when `!MISTER_DUAL_SDRAM`

These are directly accessible GPIO pins on the DE10-Nano, active only when not using the dual-SDRAM board (since the pins are shared). Routing depends on `SW[0]` (dip switch on the I/O board) and `mcp_en` (I/O board MCP23009 GPIO expander detected):

| Condition | `AUDIO_L` | `AUDIO_R` | `AUDIO_SPDIF` |
|---|---|---|---|
| `av_dis` (analog video disabled) | Hi-Z | Hi-Z | Hi-Z |
| `SW[0]=1` OR `mcp_en` (I/O board mode) | `HDMI_SCLK` (I2S BCLK) | `HDMI_I2S` (I2S data) | `HDMI_LRCLK` (I2S LRCLK) |
| Default (no I/O board) | `analog_l` (sigma-delta) | `analog_r` (sigma-delta) | `spdif` |

**Key insight:** When an I/O board is present (`mcp_en` or `SW[0]`), the three "audio" GPIO pins are **repurposed as I2S outputs** (BCLK, DATA, LRCLK) for an external I2S DAC on the I/O board, rather than carrying sigma-delta PDM. This is because an I2S DAC provides far better audio quality than the 1-bit sigma-delta output with an RC filter.

When no I/O board is present, `AUDIO_L` and `AUDIO_R` carry the 1-bit sigma-delta PDM output (requires an external RC low-pass filter to produce analog audio), and `AUDIO_SPDIF` carries the S/PDIF digital output.

### `SDCD_SPDIF` Pin (line 1547)

This is a dual-purpose pin (SD card detect / S/PDIF):
- When `mcp_en` (I/O board present): drives S/PDIF as active-low open-drain (`~spdif ? 1'b0 : 1'bZ`)
- Otherwise: Hi-Z (pin used for SD card detect)

### USB 3.0 User Port (`USER_IO[6:0]`) — lines 1648-1662

When `SW[1]` is set (dip switch on the I/O board), I2S audio is routed to the USB 3.0 connector pins for external DAC boards that plug into that port:

| Pin | SW[1]=1 | SW[1]=0 |
|---|---|---|
| `USER_IO[2]` | `HDMI_I2S` (data) | `user_out[2]` (core-controlled) |
| `USER_IO[4]` | `HDMI_SCLK` (BCLK) | `user_out[4]` (core-controlled) |
| `USER_IO[5]` | `HDMI_LRCLK` (LRCLK) | `user_out[5]` (core-controlled) |

When `SW[1]=1`, the corresponding `user_in[]` signals are forced high so the core doesn't see the I2S activity as input.

## Signal Flow Diagram

```
Core (clk_sys)                    audio_out (clk_audio = 24.576 MHz)
┌──────────┐                     ┌─────────────────────────────────────────────┐
│          │  AUDIO_L/R (16-bit) │                                             │
│  mycore  ├────────────────────►│  2-FF stability sync                        │
│          │  AUDIO_S (signed?)  │         │                                   │
│          │  AUDIO_MIX (stereo) │         ▼                                   │
└──────────┘                     │  IIR 3-tap biquad LPF                       │
                                 │  (coefficients from HPS)                    │
ALSA (HPS ARM)                   │         │                                   │
┌──────────┐                     │         ▼                                   │
│  Linux   │  SPI ──► alsa.sv    │  DC Blocker (1-tap HPF)                     │
│  audio   ├────────────────────►│         │                                   │
└──────────┘   alsa_l/r (16-bit) │         ▼                                   │
                                 │  aud_mix_top (L+R)                          │
                                 │  ├─ sum core + ALSA                         │
                                 │  ├─ stereo cross-mix (AUDIO_MIX)            │
                                 │  └─ volume attenuate + clamp                │
                                 │         │                                   │
                                 │    al/ar (final 16-bit signed)              │
                                 │    ┌────┼────────┐                          │
                                 │    │    │         │                          │
                                 │    ▼    ▼         ▼                         │
                                 │  I2S  S/PDIF  Σ-Δ DAC                      │
                                 └──┬────┬──────┬──────┘
                                    │    │      │
                    ┌───────────────┘    │      └──────────────────┐
                    │                    │                         │
              ┌─────┴─────┐      ┌──────┴──────┐          ┌───────┴───────┐
              │ HDMI_SCLK │      │   spdif     │          │ analog_l/r    │
              │ HDMI_LRCLK│      │   (wire)    │          │ (1-bit PDM)   │
              │ HDMI_I2S  │      │             │          │               │
              │ HDMI_MCLK │      │             │          │               │
              └─────┬─────┘      └──────┬──────┘          └───────┬───────┘
                    │                    │                         │
         ┌──────────┴─────┐     ┌───────┴──────┐          ┌───────┴───────┐
         │                │     │              │          │               │
    ┌────▼────┐    ┌──────▼──┐  │  ┌───────────▼──┐  ┌───▼───────────┐   │
    │HDMI chip│    │ GPIO    │  │  │ SDCD_SPDIF   │  │ GPIO          │   │
    │(ADV7513)│    │AUDIO_*  │  │  │ (I/O board)  │  │ AUDIO_L/R     │   │
    │I2S input│    │(SW[0]/  │  │  │ open-drain   │  │ (no I/O board)│   │
    └─────────┘    │mcp_en)  │  │  └──────────────┘  └───────────────┘   │
                   │→I2S DAC │  │                                        │
                   └─────────┘  └──► GPIO AUDIO_SPDIF (no I/O board)     │
                                                                         │
         ┌──────────────────────────────────────────────────┐            │
         │ USER_IO (USB 3.0 connector) — when SW[1]=1      │            │
         │  [2] = HDMI_I2S  (data)                         │◄───(I2S)───┘
         │  [4] = HDMI_SCLK (BCLK)                         │
         │  [5] = HDMI_LRCLK (LRCLK)                       │
         │  → external USB DAC board                        │
         └──────────────────────────────────────────────────┘
```

## Key Takeaways

1. **All audio processing happens in `clk_audio` (24.576 MHz)** — the core only provides raw samples and metadata.
2. **The I2S signals do triple duty** — they always drive the HDMI transmitter, and are optionally muxed onto GPIO (for I/O board DAC) or USER_IO (for USB DAC) via dip switches.
3. **The sigma-delta DAC outputs are only used when no I/O board is present** — in that case the GPIO pins carry 1-bit PDM requiring external RC filtering. With an I/O board, those same pins carry I2S instead.
4. **Filter coefficients are runtime-configurable from Linux** — the HPS sends IIR taps via IO commands, so different cores can have different anti-aliasing characteristics.
5. **The CDC for core audio samples is currently a 2-cycle stability check**, not a proper async FIFO — this is what the `audio_cdc` branch aims to fix.
