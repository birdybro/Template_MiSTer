// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vaudio_cdc_fifo__pch.h"

//============================================================
// Constructors

Vaudio_cdc_fifo::Vaudio_cdc_fifo(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vaudio_cdc_fifo__Syms(contextp(), _vcname__, this)}
    , wr_clk{vlSymsp->TOP.wr_clk}
    , wr_rst{vlSymsp->TOP.wr_rst}
    , rd_clk{vlSymsp->TOP.rd_clk}
    , rd_rst{vlSymsp->TOP.rd_rst}
    , wr_en{vlSymsp->TOP.wr_en}
    , wr_full{vlSymsp->TOP.wr_full}
    , rd_en{vlSymsp->TOP.rd_en}
    , rd_empty{vlSymsp->TOP.rd_empty}
    , wr_data{vlSymsp->TOP.wr_data}
    , rd_data{vlSymsp->TOP.rd_data}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vaudio_cdc_fifo::Vaudio_cdc_fifo(const char* _vcname__)
    : Vaudio_cdc_fifo(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vaudio_cdc_fifo::~Vaudio_cdc_fifo() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vaudio_cdc_fifo___024root___eval_debug_assertions(Vaudio_cdc_fifo___024root* vlSelf);
#endif  // VL_DEBUG
void Vaudio_cdc_fifo___024root___eval_static(Vaudio_cdc_fifo___024root* vlSelf);
void Vaudio_cdc_fifo___024root___eval_initial(Vaudio_cdc_fifo___024root* vlSelf);
void Vaudio_cdc_fifo___024root___eval_settle(Vaudio_cdc_fifo___024root* vlSelf);
void Vaudio_cdc_fifo___024root___eval(Vaudio_cdc_fifo___024root* vlSelf);

void Vaudio_cdc_fifo::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vaudio_cdc_fifo::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vaudio_cdc_fifo___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vaudio_cdc_fifo___024root___eval_static(&(vlSymsp->TOP));
        Vaudio_cdc_fifo___024root___eval_initial(&(vlSymsp->TOP));
        Vaudio_cdc_fifo___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vaudio_cdc_fifo___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vaudio_cdc_fifo::eventsPending() { return false; }

uint64_t Vaudio_cdc_fifo::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vaudio_cdc_fifo::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vaudio_cdc_fifo___024root___eval_final(Vaudio_cdc_fifo___024root* vlSelf);

VL_ATTR_COLD void Vaudio_cdc_fifo::final() {
    Vaudio_cdc_fifo___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vaudio_cdc_fifo::hierName() const { return vlSymsp->name(); }
const char* Vaudio_cdc_fifo::modelName() const { return "Vaudio_cdc_fifo"; }
unsigned Vaudio_cdc_fifo::threads() const { return 1; }
void Vaudio_cdc_fifo::prepareClone() const { contextp()->prepareClone(); }
void Vaudio_cdc_fifo::atClone() const {
    contextp()->threadPoolpOnClone();
}
