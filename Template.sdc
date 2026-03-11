derive_pll_clocks
derive_clock_uncertainty

# core specific constraints

# Gamma correction CDC: LUT is written from clk_sys (HPS) at boot,
# read from clk_vid during active video. These are asynchronous domains
# and the data is stable long before it is read.
set_false_path -from [get_registers {*gamma*gamma_curve*}] -to [get_registers {*gamma*RGB_out*}]
set_false_path -from [get_registers {*gamma*gamma_curve*}] -to [get_registers {*gamma*gamma_rd*}]

# gamma_en is a quasi-static control signal from HPS (clk_sys) used in clk_vid.
# It only changes on user menu interaction, never mid-frame.
set_false_path -from [get_registers {*|gamma_en}] -to [get_registers {*gamma*RGB_out*}]

# Shadowmask CDC: LUT and control signals written from clk_sys on user
# configuration, read from video clk. All quasi-static.
set_false_path -from [get_registers {*shadowmask*mask_lut*}] -to [get_registers {*shadowmask*lut*}]
set_false_path -from [get_registers {*shadowmask*mask_enable}] -to [get_registers {*shadowmask*mask_enable_s1}]
set_false_path -from [get_registers {*shadowmask*mask_rotate}] -to [get_registers {*shadowmask*mask_rotate_s1}]
set_false_path -from [get_registers {*shadowmask*mask_2x}] -to [get_registers {*shadowmask*mask_2x_s1}]
set_false_path -from [get_registers {*shadowmask*hmax*}] -to [get_registers {*shadowmask*hmax_s1*}]
set_false_path -from [get_registers {*shadowmask*vmax*}] -to [get_registers {*shadowmask*vmax_s1*}]
