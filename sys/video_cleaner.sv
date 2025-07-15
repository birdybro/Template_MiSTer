//
//
// Copyright (c) 2018 Sorgelig
//
// This program is GPL Licensed. See COPYING for the full license.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module video_cleaner
(
	input            clk_vid,
	input            ce_pix,

	input      [7:0] R,
	input      [7:0] G,
	input      [7:0] B,

	input            HSync,
	input            VSync,
	input            HBlank,
	input            VBlank,

	//optional de
	input            DE_in,

	//optional interlace support
	input            interlace,
	input            f1,

	// video output signals
	output reg [7:0] VGA_R,
	output reg [7:0] VGA_G,
	output reg [7:0] VGA_B,
	output reg       VGA_VS,
	output reg       VGA_HS,
	output           VGA_DE,
	
	// optional aligned blank
	output reg       HBlank_out,
	output reg       VBlank_out,
	
	// optional aligned de
	output reg       DE_out
);

wire hs, vs;
s_fix_dual sync_dual(clk_vid, HSync, VSync, hs, vs);

wire hbl = hs | HBlank;
wire vbl = vs | VBlank;

assign VGA_DE = ~(HBlank_out | VBlank_out);

always @(posedge clk_vid) begin
	if(ce_pix) begin
		HBlank_out <= hbl;

		VGA_HS <= hs;

		VGA_R  <= R;
		VGA_G  <= G;
		VGA_B  <= B;
		DE_out <= DE_in;

		if (interlace & f1) begin
			VGA_VS <= vs;
			VBlank_out <= vbl;
		end else begin
			if(~VGA_HS & hs) VGA_VS <= vs;
			if(HBlank_out & ~hbl) VBlank_out <= vbl;
		end
	end
end

endmodule

// Optimized merged module
module s_fix_dual (
    input clk,
    input sync_h_in,
    input sync_v_in,
    output sync_h_out,
    output sync_v_out
);
    assign sync_h_out = sync_h_in ^ pol_h;
    assign sync_v_out = sync_v_in ^ pol_v;
    
    reg pol_h, pol_v;
    always @(posedge clk) begin
        integer pos_h = 0, neg_h = 0, cnt_h = 0;
        integer pos_v = 0, neg_v = 0, cnt_v = 0;
        reg s1_h, s2_h, s1_v, s2_v;
        
        // Shared pipeline registers
        s1_h <= sync_h_in; s2_h <= s1_h;
        s1_v <= sync_v_in; s2_v <= s1_v;
        
        // H sync processing
        if(~s2_h & s1_h) neg_h <= cnt_h;
        if(s2_h & ~s1_h) pos_h <= cnt_h;
        cnt_h <= cnt_h + 1;
        if(s2_h != s1_h) cnt_h <= 0;
        pol_h <= pos_h > neg_h;
        
        // V sync processing (shared increment logic)
        if(~s2_v & s1_v) neg_v <= cnt_v;
        if(s2_v & ~s1_v) pos_v <= cnt_v;
        cnt_v <= cnt_v + 1;
        if(s2_v != s1_v) cnt_v <= 0;
        pol_v <= pos_v > neg_v;
    end
endmodule
