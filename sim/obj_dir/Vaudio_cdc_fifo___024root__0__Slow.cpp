// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vaudio_cdc_fifo.h for the primary calling header

#include "Vaudio_cdc_fifo__pch.h"

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_static__TOP(Vaudio_cdc_fifo___024root* vlSelf);

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_static(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_static\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    Vaudio_cdc_fifo___024root___eval_static__TOP(vlSelf);
    vlSelfRef.__Vtrigprevexpr___TOP__wr_clk__0 = vlSelfRef.wr_clk;
    vlSelfRef.__Vtrigprevexpr___TOP__wr_rst__0 = vlSelfRef.wr_rst;
    vlSelfRef.__Vtrigprevexpr___TOP__rd_clk__0 = vlSelfRef.rd_clk;
    vlSelfRef.__Vtrigprevexpr___TOP__rd_rst__0 = vlSelfRef.rd_rst;
}

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_static__TOP(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_static__TOP\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_bin = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd1 = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd2 = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr1 = 0U;
    vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2 = 0U;
}

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_initial(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_initial\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
}

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_final(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_final\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vaudio_cdc_fifo___024root___dump_triggers__stl(const VlUnpacked<QData/*63:0*/, 1> &triggers, const std::string &tag);
#endif  // VL_DEBUG
VL_ATTR_COLD bool Vaudio_cdc_fifo___024root___eval_phase__stl(Vaudio_cdc_fifo___024root* vlSelf);

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_settle(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_settle\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    IData/*31:0*/ __VstlIterCount;
    // Body
    __VstlIterCount = 0U;
    vlSelfRef.__VstlFirstIteration = 1U;
    do {
        if (VL_UNLIKELY(((0x00000064U < __VstlIterCount)))) {
#ifdef VL_DEBUG
            Vaudio_cdc_fifo___024root___dump_triggers__stl(vlSelfRef.__VstlTriggered, "stl"s);
#endif
            VL_FATAL_MT("../sys/audio_cdc_fifo.v", 20, "", "Settle region did not converge after 100 tries");
        }
        __VstlIterCount = ((IData)(1U) + __VstlIterCount);
    } while (Vaudio_cdc_fifo___024root___eval_phase__stl(vlSelf));
}

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_triggers__stl(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_triggers__stl\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.__VstlTriggered[0U] = ((0xfffffffffffffffeULL 
                                      & vlSelfRef.__VstlTriggered
                                      [0U]) | (IData)((IData)(vlSelfRef.__VstlFirstIteration)));
    vlSelfRef.__VstlFirstIteration = 0U;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vaudio_cdc_fifo___024root___dump_triggers__stl(vlSelfRef.__VstlTriggered, "stl"s);
    }
#endif
}

VL_ATTR_COLD bool Vaudio_cdc_fifo___024root___trigger_anySet__stl(const VlUnpacked<QData/*63:0*/, 1> &in);

#ifdef VL_DEBUG
VL_ATTR_COLD void Vaudio_cdc_fifo___024root___dump_triggers__stl(const VlUnpacked<QData/*63:0*/, 1> &triggers, const std::string &tag) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___dump_triggers__stl\n"); );
    // Body
    if ((1U & (~ (IData)(Vaudio_cdc_fifo___024root___trigger_anySet__stl(triggers))))) {
        VL_DBG_MSGS("         No '" + tag + "' region triggers active\n");
    }
    if ((1U & (IData)(triggers[0U]))) {
        VL_DBG_MSGS("         '" + tag + "' region trigger index 0 is active: Internal 'stl' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD bool Vaudio_cdc_fifo___024root___trigger_anySet__stl(const VlUnpacked<QData/*63:0*/, 1> &in) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___trigger_anySet__stl\n"); );
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

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___stl_sequent__TOP__0(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___stl_sequent__TOP__0\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    vlSelfRef.wr_full = ((IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray) 
                         == ((6U & ((~ ((IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2) 
                                        >> 1U)) << 1U)) 
                             | (1U & (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray_wr2))));
    vlSelfRef.rd_data = vlSelfRef.audio_cdc_fifo__DOT__mem
        [(3U & (IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_bin))];
    vlSelfRef.rd_empty = ((IData)(vlSelfRef.audio_cdc_fifo__DOT__rd_ptr_gray) 
                          == (IData)(vlSelfRef.audio_cdc_fifo__DOT__wr_ptr_gray_rd2));
}

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___eval_stl(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_stl\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    if ((1ULL & vlSelfRef.__VstlTriggered[0U])) {
        Vaudio_cdc_fifo___024root___stl_sequent__TOP__0(vlSelf);
    }
}

VL_ATTR_COLD bool Vaudio_cdc_fifo___024root___eval_phase__stl(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___eval_phase__stl\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Locals
    CData/*0:0*/ __VstlExecute;
    // Body
    Vaudio_cdc_fifo___024root___eval_triggers__stl(vlSelf);
    __VstlExecute = Vaudio_cdc_fifo___024root___trigger_anySet__stl(vlSelfRef.__VstlTriggered);
    if (__VstlExecute) {
        Vaudio_cdc_fifo___024root___eval_stl(vlSelf);
    }
    return (__VstlExecute);
}

bool Vaudio_cdc_fifo___024root___trigger_anySet__act(const VlUnpacked<QData/*63:0*/, 1> &in);

#ifdef VL_DEBUG
VL_ATTR_COLD void Vaudio_cdc_fifo___024root___dump_triggers__act(const VlUnpacked<QData/*63:0*/, 1> &triggers, const std::string &tag) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___dump_triggers__act\n"); );
    // Body
    if ((1U & (~ (IData)(Vaudio_cdc_fifo___024root___trigger_anySet__act(triggers))))) {
        VL_DBG_MSGS("         No '" + tag + "' region triggers active\n");
    }
    if ((1U & (IData)(triggers[0U]))) {
        VL_DBG_MSGS("         '" + tag + "' region trigger index 0 is active: @(posedge wr_clk)\n");
    }
    if ((1U & (IData)((triggers[0U] >> 1U)))) {
        VL_DBG_MSGS("         '" + tag + "' region trigger index 1 is active: @(posedge wr_rst)\n");
    }
    if ((1U & (IData)((triggers[0U] >> 2U)))) {
        VL_DBG_MSGS("         '" + tag + "' region trigger index 2 is active: @(posedge rd_clk)\n");
    }
    if ((1U & (IData)((triggers[0U] >> 3U)))) {
        VL_DBG_MSGS("         '" + tag + "' region trigger index 3 is active: @(posedge rd_rst)\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void Vaudio_cdc_fifo___024root___ctor_var_reset(Vaudio_cdc_fifo___024root* vlSelf) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vaudio_cdc_fifo___024root___ctor_var_reset\n"); );
    Vaudio_cdc_fifo__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    auto& vlSelfRef = std::ref(*vlSelf).get();
    // Body
    const uint64_t __VscopeHash = VL_MURMUR64_HASH(vlSelf->name());
    vlSelf->wr_clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 6739791619049129404ull);
    vlSelf->wr_rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 14146291223391494392ull);
    vlSelf->wr_en = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 7710928637576349896ull);
    vlSelf->wr_data = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 12812822527505751231ull);
    vlSelf->wr_full = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 4538020038871708192ull);
    vlSelf->rd_clk = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 3164821756025340486ull);
    vlSelf->rd_rst = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 17848191682747035081ull);
    vlSelf->rd_en = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 3814484142505630662ull);
    vlSelf->rd_data = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 17824471296722538975ull);
    vlSelf->rd_empty = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13234839125597451765ull);
    for (int __Vi0 = 0; __Vi0 < 4; ++__Vi0) {
        vlSelf->audio_cdc_fifo__DOT__mem[__Vi0] = VL_SCOPED_RAND_RESET_I(32, __VscopeHash, 5928843429478074851ull);
    }
    vlSelf->audio_cdc_fifo__DOT__wr_ptr_bin = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 5141261353347300355ull);
    vlSelf->audio_cdc_fifo__DOT__wr_ptr_gray = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 14300811144439749567ull);
    vlSelf->audio_cdc_fifo__DOT__rd_ptr_bin = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 13096666770035876520ull);
    vlSelf->audio_cdc_fifo__DOT__rd_ptr_gray = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 12919366332283906104ull);
    vlSelf->audio_cdc_fifo__DOT__wr_ptr_gray_rd1 = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 16660988304711980074ull);
    vlSelf->audio_cdc_fifo__DOT__wr_ptr_gray_rd2 = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 9933521338046324978ull);
    vlSelf->audio_cdc_fifo__DOT__rd_ptr_gray_wr1 = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 2237458122887686286ull);
    vlSelf->audio_cdc_fifo__DOT__rd_ptr_gray_wr2 = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 16456906983083588143ull);
    vlSelf->__Vdly__audio_cdc_fifo__DOT__wr_ptr_bin = VL_SCOPED_RAND_RESET_I(3, __VscopeHash, 14128108069196876049ull);
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        vlSelf->__VstlTriggered[__Vi0] = 0;
    }
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        vlSelf->__VactTriggered[__Vi0] = 0;
    }
    vlSelf->__Vtrigprevexpr___TOP__wr_clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13262410864789368243ull);
    vlSelf->__Vtrigprevexpr___TOP__wr_rst__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 11194384152875509381ull);
    vlSelf->__Vtrigprevexpr___TOP__rd_clk__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 530126968687307889ull);
    vlSelf->__Vtrigprevexpr___TOP__rd_rst__0 = VL_SCOPED_RAND_RESET_I(1, __VscopeHash, 13808226724452775995ull);
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        vlSelf->__VnbaTriggered[__Vi0] = 0;
    }
}
