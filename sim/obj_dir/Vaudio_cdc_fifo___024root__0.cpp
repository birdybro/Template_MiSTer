// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vaudio_cdc_fifo.h for the primary calling header

#include "Vaudio_cdc_fifo__pch.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vaudio_cdc_fifo___024root___dump_triggers__act(const VlUnpacked<QData/*63:0*/, 1> &triggers, const std::string &tag);
#endif  // VL_DEBUG

void Vaudio_cdc_fifo___024root___eval_triggers__act(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_triggers__act\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VactTriggered[0U] = (QData)((IData)(
                                                    (((((IData)(vlSelfRef.rd_rst) 
                                                        & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__rd_rst__0))) 
                                                       << 3U) 
                                                      | (((IData)(vlSelfRef.rd_clk) 
                                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__rd_clk__0))) 
                                                         << 2U)) 
                                                     | ((((IData)(vlSelfRef.wr_rst) 
                                                          & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__wr_rst__0))) 
                                                         << 1U) 
                                                        | ((IData)(vlSelfRef.wr_clk) 
                                                           & (~ (IData)(vlSelfRef.__Vtrigprevexpr___TOP__wr_clk__0)))))));
    vlSelfRef.__Vtrigprevexpr___TOP__wr_clk__0 = vlSelfRef.wr_clk;
    vlSelfRef.__Vtrigprevexpr___TOP__wr_rst__0 = vlSelfRef.wr_rst;
    vlSelfRef.__Vtrigprevexpr___TOP__rd_clk__0 = vlSelfRef.rd_clk;
    vlSelfRef.__Vtrigprevexpr___TOP__rd_rst__0 = vlSelfRef.rd_rst;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vaudio_cdc_fifo___024root___dump_triggers__act(vlSelfRef.__VactTriggered, "act"s);
    }
#endif
}

bool Vaudio_cdc_fifo___024root___trigger_anySet__act(const VlUnpacked<QData/*63:0*/, 1> &in) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___trigger_anySet__act\n"); );
    // Locals
    IData/*31:0*/ n;
    // Body
    n = 0U;
    do {
        if (in[n]) {
            return (1U);
        }
        n = ((IData)(1U) + n);
    } while ((1U > n));
    return (0U);
}

void Vaudio_cdc_fifo___024root___nba_sequent__TOP__0(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___nba_sequent__TOP__0\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__Vdly__audio_cdc_fifo__DOT__wr_ptr_bin 
        = vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin;
    if (vlSelfRef.wr_rst) {
        vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2 = 0U;
        vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr1 = 0U;
    } else {
        vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2 
            = vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr1;
        vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr1 
            = vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray;
    }
}

void Vaudio_cdc_fifo___024root___nba_sequent__TOP__1(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___nba_sequent__TOP__1\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    CData/*2:0*/ __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin;
    __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin = 0;
    // Body
    __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin = vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin;
    if (vlSelfRef.rd_rst) {
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd2 = 0U;
        __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin = 0U;
        vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray = 0U;
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd1 = 0U;
    } else {
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd2 
            = vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd1;
        if (((IData)(vlSelfRef.rd_en) & (~ (IData)(vlSelfRef.rd_empty)))) {
            __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin 
                = (7U & ((IData)(1U) + (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin)));
            vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray 
                = (7U & (((IData)(1U) + (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin)) 
                         ^ VL_SHIFTR_III(3,3,32, (7U 
                                                  & ((IData)(1U) 
                                                     + (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin))), 1U)));
        }
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd1 
            = vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray;
    }
    vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin = __Vdly__audio_cdc_fifo__DOT__rd_ptr_bin;
    vlSelfRef.rd_empty = ((IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray) 
                          == (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd2));
}

void Vaudio_cdc_fifo___024root___nba_sequent__TOP__2(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___nba_sequent__TOP__2\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    IData/*31:0*/ __VdlyVal__audio_cdc_fifo__DOT__mem__v0;
    __VdlyVal__audio_cdc_fifo__DOT__mem__v0 = 0;
    CData/*1:0*/ __VdlyDim0__audio_cdc_fifo__DOT__mem__v0;
    __VdlyDim0__audio_cdc_fifo__DOT__mem__v0 = 0;
    CData/*0:0*/ __VdlySet__audio_cdc_fifo__DOT__mem__v0;
    __VdlySet__audio_cdc_fifo__DOT__mem__v0 = 0;
    // Body
    __VdlySet__audio_cdc_fifo__DOT__mem__v0 = 0U;
    if (((IData)(vlSelfRef.wr_en) & (~ (IData)(vlSelfRef.wr_full)))) {
        __VdlyVal__audio_cdc_fifo__DOT__mem__v0 = vlSelfRef.wr_data;
        __VdlyDim0__audio_cdc_fifo__DOT__mem__v0 = 
            (3U & (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin));
        __VdlySet__audio_cdc_fifo__DOT__mem__v0 = 1U;
    }
    if (__VdlySet__audio_cdc_fifo__DOT__mem__v0) {
        vlSelfRef.audio_cdc_fifo__DOT__mem[__VdlyDim0__audio_cdc_fifo__DOT__mem__v0] 
            = __VdlyVal__audio_cdc_fifo__DOT__mem__v0;
    }
}

void Vaudio_cdc_fifo___024root___nba_sequent__TOP__3(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___nba_sequent__TOP__3\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if (vlSelfRef.wr_rst) {
        vlSelfRef.__Vdly__audio_cdc_fifo__DOT__wr_ptr_bin = 0U;
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray = 0U;
    } else if (((IData)(vlSelfRef.wr_en) & (~ (IData)(vlSelfRef.wr_full)))) {
        vlSelfRef.__Vdly__audio_cdc_fifo__DOT__wr_ptr_bin 
            = (7U & ((IData)(1U) + (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin)));
        vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray 
            = (7U & (((IData)(1U) + (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin)) 
                     ^ VL_SHIFTR_III(3,3,32, (7U & 
                                              ((IData)(1U) 
                                               + (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin))), 1U)));
    }
    vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin = vlSelfRef.__Vdly__audio_cdc_fifo__DOT__wr_ptr_bin;
    vlSelfRef.wr_full = ((IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray) 
                         == ((6U & ((~ ((IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2) 
                                        >> 1U)) << 1U)) 
                             | (1U & (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2))));
}

void Vaudio_cdc_fifo___024root___nba_comb__TOP__0(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___nba_comb__TOP__0\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.rd_data = vlSelfRef.audio_cdc_fifo__DOT__mem
        [(3U & (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin))];
}

void Vaudio_cdc_fifo___024root___eval_nba(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_nba\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((3ULL & vlSelfRef.__VnbaTriggered[0U])) {
        Vaudio_cdc_fifo___024root___nba_sequent__TOP__0(vlSelf);
    }
    if ((0x000000000000000cULL & vlSelfRef.__VnbaTriggered
         [0U])) {
        Vaudio_cdc_fifo___024root___nba_sequent__TOP__1(vlSelf);
    }
    if ((1ULL & vlSelfRef.__VnbaTriggered[0U])) {
        Vaudio_cdc_fifo___024root___nba_sequent__TOP__2(vlSelf);
    }
    if ((3ULL & vlSelfRef.__VnbaTriggered[0U])) {
        Vaudio_cdc_fifo___024root___nba_sequent__TOP__3(vlSelf);
    }
    if ((0x000000000000000dULL & vlSelfRef.__VnbaTriggered
         [0U])) {
        Vaudio_cdc_fifo___024root___nba_comb__TOP__0(vlSelf);
    }
}

void Vaudio_cdc_fifo___024root___trigger_orInto__act(VlUnpacked<QData/*63:0*/, 1> &out, const VlUnpacked<QData/*63:0*/, 1> &in) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___trigger_orInto__act\n"); );
    // Locals
    IData/*31:0*/ n;
    // Body
    n = 0U;
    do {
        out[n] = (out[n] | in[n]);
        n = ((IData)(1U) + n);
    } while ((1U > n));
}

bool Vaudio_cdc_fifo___024root___eval_phase__act(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_phase__act\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    Vaudio_cdc_fifo___024root___eval_triggers__act(vlSelf);
    Vaudio_cdc_fifo___024root___trigger_orInto__act(vlSelfRef.__VnbaTriggered, vlSelfRef.__VactTriggered);
    return (0U);
}

void Vaudio_cdc_fifo___024root___trigger_clear__act(VlUnpacked<QData/*63:0*/, 1> &out) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___trigger_clear__act\n"); );
    // Locals
    IData/*31:0*/ n;
    // Body
    n = 0U;
    do {
        out[n] = 0ULL;
        n = ((IData)(1U) + n);
    } while ((1U > n));
}

bool Vaudio_cdc_fifo___024root___eval_phase__nba(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_phase__nba\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    CData/*0:0*/ __VnbaExecute;
    // Body
    __VnbaExecute = Vaudio_cdc_fifo___024root___trigger_anySet__act(vlSelfRef.__VnbaTriggered);
    if (__VnbaExecute) {
        Vaudio_cdc_fifo___024root___eval_nba(vlSelf);
        Vaudio_cdc_fifo___024root___trigger_clear__act(vlSelfRef.__VnbaTriggered);
    }
    return (__VnbaExecute);
}

void Vaudio_cdc_fifo___024root___eval(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    IData/*31:0*/ __VnbaIterCount;
    // Body
    __VnbaIterCount = 0U;
    do {
        if (VL_UNLIKELY(((0x00000064U < __VnbaIterCount)))) {
#ifdef VL_DEBUG
            Vaudio_cdc_fifo___024root___dump_triggers__act(vlSelfRef.__VnbaTriggered, "nba"s);
#endif
            VL_FATAL_MT("../sys/audio_cdc_fifo.v", 20, "", "NBA region did not converge after 100 tries");
        }
        __VnbaIterCount = ((IData)(1U) + __VnbaIterCount);
        vlSelfRef.__VactIterCount = 0U;
        do {
            if (VL_UNLIKELY(((0x00000064U < vlSelfRef.__VactIterCount)))) {
#ifdef VL_DEBUG
                Vaudio_cdc_fifo___024root___dump_triggers__act(vlSelfRef.__VactTriggered, "act"s);
#endif
                VL_FATAL_MT("../sys/audio_cdc_fifo.v", 20, "", "Active region did not converge after 100 tries");
            }
            vlSelfRef.__VactIterCount = ((IData)(1U) 
                                         + vlSelfRef.__VactIterCount);
        } while (Vaudio_cdc_fifo___024root___eval_phase__act(vlSelf));
    } while (Vaudio_cdc_fifo___024root___eval_phase__nba(vlSelf));
}

#ifdef VL_DEBUG
void Vaudio_cdc_fifo___024root___eval_debug_assertions(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_debug_assertions\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if (VL_UNLIKELY(((vlSelfRef.wr_clk & 0xfeU)))) {
        Verilated::overWidthError("wr_clk");
    }
    if (VL_UNLIKELY(((vlSelfRef.wr_rst & 0xfeU)))) {
        Verilated::overWidthError("wr_rst");
    }
    if (VL_UNLIKELY(((vlSelfRef.wr_en & 0xfeU)))) {
        Verilated::overWidthError("wr_en");
    }
    if (VL_UNLIKELY(((vlSelfRef.rd_clk & 0xfeU)))) {
        Verilated::overWidthError("rd_clk");
    }
    if (VL_UNLIKELY(((vlSelfRef.rd_rst & 0xfeU)))) {
        Verilated::overWidthError("rd_rst");
    }
    if (VL_UNLIKELY(((vlSelfRef.rd_en & 0xfeU)))) {
        Verilated::overWidthError("rd_en");
    }
}
#endif  // VL_DEBUG
