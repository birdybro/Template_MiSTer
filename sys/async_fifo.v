//
//  Asynchronous FIFO with gray-code pointers for clock domain crossing.
//  Copyright (C) Kevin Coleman 2026
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation; either version 2 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

module async_fifo
#(
	parameter DATA_WIDTH = 32,
	parameter ADDR_WIDTH = 2   // 2^2 = 4 entries
)
(
	// Write side
	input                       wr_clk,
	input                       wr_rst,
	input                       wr_en,
	input      [DATA_WIDTH-1:0] wr_data,
	output                      wr_full,

	// Read side
	input                       rd_clk,
	input                       rd_rst,
	input                       rd_en,
	output     [DATA_WIDTH-1:0] rd_data,
	output                      rd_empty
);

// Memory
(* ramstyle = "logic" *) reg [DATA_WIDTH-1:0] mem [0:(1<<ADDR_WIDTH)-1];

// Write pointer (binary + gray)
reg [ADDR_WIDTH:0] wr_ptr_bin  = 0;
reg [ADDR_WIDTH:0] wr_ptr_gray = 0;

wire [ADDR_WIDTH-1:0] wr_addr = wr_ptr_bin[ADDR_WIDTH-1:0];

always @(posedge wr_clk or posedge wr_rst) begin
	if (wr_rst) begin
		wr_ptr_bin  <= 0;
		wr_ptr_gray <= 0;
	end else if (wr_en && !wr_full) begin
		wr_ptr_bin  <= wr_ptr_bin + 1'd1;
		wr_ptr_gray <= (wr_ptr_bin + 1'd1) ^ ((wr_ptr_bin + 1'd1) >> 1);
	end
end

// Write data into memory
always @(posedge wr_clk) begin
	if (wr_en && !wr_full)
		mem[wr_addr] <= wr_data;
end

// Read pointer (binary + gray)
reg [ADDR_WIDTH:0] rd_ptr_bin  = 0;
reg [ADDR_WIDTH:0] rd_ptr_gray = 0;

wire [ADDR_WIDTH-1:0] rd_addr = rd_ptr_bin[ADDR_WIDTH-1:0];

always @(posedge rd_clk or posedge rd_rst) begin
	if (rd_rst) begin
		rd_ptr_bin  <= 0;
		rd_ptr_gray <= 0;
	end else if (rd_en && !rd_empty) begin
		rd_ptr_bin  <= rd_ptr_bin + 1'd1;
		rd_ptr_gray <= (rd_ptr_bin + 1'd1) ^ ((rd_ptr_bin + 1'd1) >> 1);
	end
end

// Combinational read
assign rd_data = mem[rd_addr];

// Synchronize write pointer (gray) into read clock domain
reg [ADDR_WIDTH:0] wr_ptr_gray_rd1 = 0, wr_ptr_gray_rd2 = 0;
always @(posedge rd_clk or posedge rd_rst) begin
	if (rd_rst) begin
		wr_ptr_gray_rd1 <= 0;
		wr_ptr_gray_rd2 <= 0;
	end else begin
		wr_ptr_gray_rd1 <= wr_ptr_gray;
		wr_ptr_gray_rd2 <= wr_ptr_gray_rd1;
	end
end

// Synchronize read pointer (gray) into write clock domain
reg [ADDR_WIDTH:0] rd_ptr_gray_wr1 = 0, rd_ptr_gray_wr2 = 0;
always @(posedge wr_clk or posedge wr_rst) begin
	if (wr_rst) begin
		rd_ptr_gray_wr1 <= 0;
		rd_ptr_gray_wr2 <= 0;
	end else begin
		rd_ptr_gray_wr1 <= rd_ptr_gray;
		rd_ptr_gray_wr2 <= rd_ptr_gray_wr1;
	end
end

// Full: gray pointers match with top 2 bits inverted
assign wr_full  = (wr_ptr_gray == {~rd_ptr_gray_wr2[ADDR_WIDTH:ADDR_WIDTH-1],
                                     rd_ptr_gray_wr2[ADDR_WIDTH-2:0]});

// Empty: gray pointers identical
assign rd_empty = (rd_ptr_gray == wr_ptr_gray_rd2);

endmodule
