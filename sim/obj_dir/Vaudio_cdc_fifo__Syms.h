// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VAUDIO_CDC_FIFO__SYMS_H_
#define VERILATED_VAUDIO_CDC_FIFO__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vaudio_cdc_fifo.h"

// INCLUDE MODULE CLASSES
#include "Vaudio_cdc_fifo___024root.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES) Vaudio_cdc_fifo__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vaudio_cdc_fifo* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vaudio_cdc_fifo___024root      TOP;

    // CONSTRUCTORS
    Vaudio_cdc_fifo__Syms(VerilatedContext* contextp, const char* namep, Vaudio_cdc_fifo* modelp);
    ~Vaudio_cdc_fifo__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
