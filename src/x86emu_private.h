#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"

typedef struct zydis_dec_s zydis_dec_t;
typedef struct box86context_s box86context_t;

#define ERR_UNIMPL  1
#define ERR_DIVBY0  2
#define ERR_ILLEGAL 4

typedef struct cleanup_s cleanup_t;

typedef struct x86emu_s {
    // cpu
	reg32_t     regs[8],ip;
    int         flags[F_LAST];
	x86flags_t  packed_eflags;
    uintptr_t   old_ip;
    #ifdef HAVE_TRACE
    uintptr_t   prev2_ip, prev_ip;
    #endif
    // segments
    uint32_t    segs[6];    // only 32bits value?
    // fpu
	fpu_reg_t   fpu[9];
    fpu_ld_t    fpu_ld[9]; // for long double emulation / 80bits fld fst
    fpu_ll_t    fpu_ll[9]; // for 64bits fild / fist sequence
	fpu_p_reg_t p_regs[9];
	uint16_t    cw,cw_mask_all;
	x87flags_t  sw;
	uint32_t    top;        // top is part of sw, but it's faster to have it separatly
    int         fpu_stack;
	fpu_round_t round;
    // mmx
    mmx_regs_t  mmx[8];
    // sse
    sse_regs_t  xmm[8];
    uint32_t    mxcsr;
    // cpu helpers
    reg32_t     zero;
    reg32_t     *sbiidx[8];
    // defered flags
    defered_flags_t df;
    uint32_t    op1;
    uint32_t    op2;
    uint32_t    res;
    // emu control
    int         quit;
    int         error;
    int         fork;   // quit because need to fork
    // trace
    zydis_dec_t *dec;
    uintptr_t   trace_start, trace_end;
    // global stuffs, pointed with GS: segment
    void        *globals;
    int         *shared_global;
    // parent context
    box86context_t *context;
    // atexit and fini functions
    cleanup_t   *cleanups;
    int         clean_sz;
    int         clean_cap;
    // scratch stack, used for alignement of double and 64bits ints on arm
    uint32_t    *scratch;
    // local stack, do be deleted when emu is freed
    void*       stack;

} x86emu_t;

//#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0; emu->quit=1;}
#define INTR_RAISE_DIV0(emu) {emu->error |= ERR_DIVBY0;} // should rise a SIGFPE and not quit

#endif //__X86EMU_PRIVATE_H_