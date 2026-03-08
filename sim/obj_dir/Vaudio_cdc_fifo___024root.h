// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vaudio_cdc_fifo.h for the primary calling header

#ifndef VERILATED_VAUDIO_CDC_FIFO___024ROOT_H_
#define VERILATED_VAUDIO_CDC_FIFO___024ROOT_H_  // guard

#include "verilated.h"


class Vaudio_cdc_fifo__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vaudio_cdc_fifo___024root final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(wr_clk,0,0);
    VL_IN8(wr_rst,0,0);
    VL_IN8(rd_clk,0,0);
    VL_IN8(rd_rst,0,0);
    VL_IN8(wr_en,0,0);
    VL_OUT8(wr_full,0,0);
    VL_IN8(rd_en,0,0);
    VL_OUT8(rd_empty,0,0);
    CData/*2:0*/ audio_cdc_fifo__DOT__wr_ptr_bin;
    CData/*2:0*/ audio_cdc_fifo__DOT__wr_ptr_gray;
    CData/*2:0*/ audio_cdc_fifo__DOT__rd_ptr_bin;
    CData/*2:0*/ audio_cdc_fifo__DOT__rd_ptr_gray;
    CData/*2:0*/ audio_cdc_fifo__DOT__wr_ptr_gray_rd1;
    CData/*2:0*/ audio_cdc_fifo__DOT__wr_ptr_gray_rd2;
    CData/*2:0*/ audio_cdc_fifo__DOT__rd_ptr_gray_wr1;
    CData/*2:0*/ audio_cdc_fifo__DOT__rd_ptr_gray_wr2;
    CData/*2:0*/ __Vdly__audio_cdc_fifo__DOT__wr_ptr_bin;
    CData/*0:0*/ __VstlFirstIteration;
    CData/*0:0*/ __Vtrigprevexpr___TOP__wr_clk__0;
    CData/*0:0*/ __Vtrigprevexpr___TOP__wr_rst__0;
    CData/*0:0*/ __Vtrigprevexpr___TOP__rd_clk__0;
    CData/*0:0*/ __Vtrigprevexpr___TOP__rd_rst__0;
    VL_IN(wr_data,31,0);
    VL_OUT(rd_data,31,0);
    IData/*31:0*/ __VactIterCount;
    VlUnpacked<IData/*31:0*/, 4> audio_cdc_fifo__DOT__mem;
    VlUnpacked<QData/*63:0*/, 1> __VstlTriggered;
    VlUnpacked<QData/*63:0*/, 1> __VactTriggered;
    VlUnpacked<QData/*63:0*/, 1> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vaudio_cdc_fifo__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vaudio_cdc_fifo___024root(Vaudio_cdc_fifo__Syms* symsp, const char* v__name);
    ~Vaudio_cdc_fifo___024root();
    VL_UNCOPYABLE(Vaudio_cdc_fifo___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
