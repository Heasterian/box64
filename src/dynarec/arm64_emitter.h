#ifndef __ARM64_EMITTER_H__
#define __ARM64_EMITTER_H__
/*
    ARM64 Emitter

*/

// x86 Register mapping
#define xRAX    10
#define xRCX    11
#define xRDX    12
#define xRBX    13
#define xRSP    14
#define xRBP    15
#define xRSI    16
#define xRDI    17
#define xR8     18
#define xR9     19
#define xR10    20
#define xR11    21
#define xR12    22
#define xR13    23
#define xR14    24
#define xR15    25
#define xFlags  26
#define xRIP    27
// 32bits version
#define wEAX    xRAX
#define wECX    xRCX
#define wEDX    xRDX
#define wEBX    xRBX
#define wESP    xRSP
#define wEBP    xRBP
#define wESI    xRSI
#define wEDI    xRDI
#define wR8     xR8
#define wR9     xR9
#define wR10    xR10
#define wR11    xR11
#define wR12    xR12
#define wR13    xR13
#define wR14    xR14
#define wR15    xR15
#define wFlags  xFlags
// scratch registers
#define x1      1
#define x2      2
#define x3      3
#define x4      4
#define x5      5
#define x6      6
#define x7      7
// 32bits version of scratch
#define w1      x1
#define w2      x2
#define w3      x3
#define w4      x4
#define w5      x5
#define w6      x6
#define w7      x7
// emu is r0
#define xEmu    0
// ARM64 LR
#define xLR     30
// ARM64 SP is r31 but is a special register
#define xSP     31      
// xZR regs is 31
#define xZR     31
#define wZR     xZR

// conditions
#define cEQ 0b0000
#define cNE 0b0001
#define cCS 0b0010
#define cHS cCS
#define cCC 0b0011
#define cLO cCC
#define cMI 0b0100
#define cPL 0b0101
#define cVS 0b0110
#define cVC 0b0111
#define cHI 0b1000
#define cLS 0b1001
#define cGE 0b1010
#define cLT 0b1011
#define cGT 0b1100
#define cLE 0b1101
#define c__ 0b1110

#define invCond(cond)   ((cond)^0b0001)

// MOVZ
#define MOVZ_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b10<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVZx(Rd, imm16)                    EMIT(MOVZ_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZx_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZw(Rd, imm16)                    EMIT(MOVZ_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZw_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

// MOVN
#define MOVN_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b00<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVNx(Rd, imm16)                    EMIT(MOVN_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNx_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNw(Rd, imm16)                    EMIT(MOVN_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNw_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

// MOVK
#define MOVK_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b11<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVKx(Rd, imm16)                    EMIT(MOVK_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKx_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKw(Rd, imm16)                    EMIT(MOVK_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKw_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

// This macro will give a -Wsign-compare warning, probably bug #38341
#define MOV32w(Rd, imm32) \
    if(~((uint32_t)(imm32))<0xffffu) {                                      \
        MOVNw(Rd, (~(uint32_t)(imm32))&0xffff);                             \
    } else {                                                                \
        MOVZw(Rd, (imm32)&0xffff);                                          \
        if((imm32)&0xffff0000) {MOVKw_LSL(Rd, ((imm32)>>16)&0xffff, 16);}   \
    }
#define MOV64x(Rd, imm64) \
    if(~((uint64_t)(imm64))<0xffff) {                                                                       \
        MOVNx(Rd, (~(uint64_t)(imm64))&0xffff);                                                             \
    } else {                                                                                                \
        MOVZx(Rd, ((uint64_t)(imm64))&0xffff);                                                              \
        if(((uint64_t)(imm64))&0xffff0000) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>16)&0xffff, 16);}           \
        if(((uint64_t)(imm64))&0xffff00000000LL) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>32)&0xffff, 32);}      \
        if(((uint64_t)(imm64))&0xffff000000000000LL) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>48)&0xffff, 48);}  \
    }

#define MOV64xw(Rd, imm64)   if(rex.w) {MOV64x(Rd, imm64);} else {MOV32w(Rd, imm64);}


// ADD / SUB
#define ADDSUB_REG_gen(sf, op, S, shift, Rm, imm6, Rn, Rd) ((sf)<<31 | (op)<<30 | (S)<<29 | 0b01011<<24 | (shift)<<22 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ADDx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDw_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSxw_REG(Rd, Rn, Rm)              EMIT(ADDSUB_REG_gen(rex.w, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDxw_REG_LSR(Rd, Rn, Rm, lsr)      EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b01, Rm, lsr, Rn, Rd))

#define ADDSUB_IMM_gen(sf, op, S, shift, imm12, Rn, Rd)    ((sf)<<31 | (op)<<30 | (S)<<29 | 0b10001<<24 | (shift)<<22 | (imm12)<<10 | (Rn)<<5 | (Rd))
#define ADDx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSxw_U12(Rd, Rn, imm12)   EMIT(ADDSUB_IMM_gen(rex.w, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))

#define SUBx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBw_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBSw_REG_LSL(Rd, Rn, Rm, lsl)      EMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, lsl, Rn, Rd))
#define SUBxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSxw_REG(Rd, Rn, Rm)              EMIT(ADDSUB_REG_gen(rex.w, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define CMPSx_REG(Rn, Rm)                   SUBSx_REG(xZR, Rn, Rm)
#define CMPSw_REG(Rn, Rm)                   SUBSw_REG(wZR, Rn, Rm)
#define CMPSxw_REG(Rn, Rm)                  SUBSxw_REG(xZR, Rn, Rm)
#define NEGx_REG(Rd, Rm)                    SUBx_REG(Rd, xZR, Rm);
#define NEGw_REG(Rd, Rm)                    SUBw_REG(Rd, wZR, Rm);
#define NEGxw_REG(Rd, Rm)                   SUBxw_REG(Rd, xZR, Rm);
#define NEGSx_REG(Rd, Rm)                   SUBSx_REG(Rd, xZR, Rm);
#define NEGSw_REG(Rd, Rm)                   SUBSw_REG(Rd, wZR, Rm);
#define NEGSxw_REG(Rd, Rm)                  SUBSxw_REG(Rd, xZR, Rm);

#define SUBx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSxw_U12(Rd, Rn, imm12)   EMIT(ADDSUB_IMM_gen(rex.w, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define CMPSx_U12(Rn, imm12)        SUBSx_U12(xZR, Rn, imm12)
#define CMPSw_U12(Rn, imm12)        SUBSw_U12(wZR, Rn, imm12)
#define CMPSxw_U12(Rn, imm12)       SUBSxw_U12(xZR, Rn, imm12)

#define ADDSUBC_gen(sf, op, S, Rm, Rn, Rd)  ((sf)<<31 | (op)<<30 | (S)<<29 | 0b11010000<<21 | (Rm)<<16 | (Rn)<<5 | (Rd))
#define ADCx_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(1, 0, 0, Rm, Rn, Rd))
#define ADCw_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(0, 0, 0, Rm, Rn, Rd))
#define ADCxw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(rex.w, 0, 0, Rm, Rn, Rd))
#define SBCx_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(1, 1, 0, Rm, Rn, Rd))
#define SBCw_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(0, 1, 0, Rm, Rn, Rd))
#define SBCxw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(rex.w, 1, 0, Rm, Rn, Rd))
#define ADCSx_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(1, 0, 1, Rm, Rn, Rd))
#define ADCSw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(0, 0, 1, Rm, Rn, Rd))
#define ADCSxw_REG(Rd, Rn, Rm)      EMIT(ADDSUBC_gen(rex.w, 0, 1, Rm, Rn, Rd))
#define SBCSx_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(1, 1, 1, Rm, Rn, Rd))
#define SBCSw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(0, 1, 1, Rm, Rn, Rd))
#define SBCSxw_REG(Rd, Rn, Rm)      EMIT(ADDSUBC_gen(rex.w, 1, 1, Rm, Rn, Rd))

// LDR
#define LDR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRx_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRx_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRw_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRw_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRB_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRB_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRH_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b01, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRH_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b01, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRxw_S9_postindex(Rt, Rn, imm9)  EMIT(LDR_gen(rex.w?0b11:0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))

#define LDRS_gen(size, op1, imm9, op2, Rn, Rt)   ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b10<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_S9_postindex(Rt, Rn, imm9)  EMIT(LDRS_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRSW_S9_preindex(Rt, Rn, imm9)   EMIT(LDRS_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))

#define LD_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRx_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b11, 0b01, ((uint32_t)((imm12)>>3))&0xfff, Rn, Rt))
#define LDRw_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))
#define LDRB_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b00, 0b01, ((uint32_t)((imm12)))&0xfff, Rn, Rt))
#define LDRH_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b01, ((uint32_t)((imm12)>>1))&0xfff, Rn, Rt))
#define LDRxw_U12(Rt, Rn, imm12)          EMIT(LD_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)((imm12)>>(2+rex.w)))&0xfff, Rn, Rt))

#define LDS_gen(size, op1, imm12, Rn, Rt)       ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b10<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_U12(Rt, Rn, imm12)          EMIT(LDS_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))

#define LDR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b01<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define LDRx_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define LDRx_REG_LSL3(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define LDRx_REG_UXTW3(Rt, Rn, Rm)      EMIT(LDR_REG_gen(0b11, Rm, 0b010, 1, Rn, Rt))
#define LDRw_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define LDRw_REG_LSL2(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define LDRxw_REG(Rt, Rn, Rm)           EMIT(LDR_REG_gen(0b10+rex.w, Rm, 0b011, 0, Rn, Rt))
#define LDRB_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define LDRH_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))

#define LDRSH_gen(size, op1, opc, imm12, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSHx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b10, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b01, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b10, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b00, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))

#define LDR_PC_gen(opc, imm19, Rt)      ((opc)<<30 | 0b011<<27 | (imm19)<<5 | (Rt))
#define LDRx_literal(Rt, imm19)         EMIT(LDR_PC_gen(0b01, ((imm19)>>2)&0x7FFFF, Rt))

// STR
#define STR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define STRx_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRx_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRw_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRw_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRxw_S9_postindex(Rt, Rn, imm9)  EMIT(STR_gen(rex.w?0b11:0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRB_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRH_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b01, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))

#define ST_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define STRx_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b11, 0b01, ((uint32_t)((imm12)>>3))&0xfff, Rn, Rt))
#define STRw_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))
#define STRB_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b00, 0b01, ((uint32_t)((imm12)))&0xfff, Rn, Rt))
#define STRH_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b01, 0b01, ((uint32_t)((imm12)>>1))&0xfff, Rn, Rt))
#define STRxw_U12(Rt, Rn, imm12)          EMIT(ST_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)((imm12)>>(2+rex.w)))&0xfff, Rn, Rt))

#define STR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b00<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define STRx_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define STRx_REG_LSL3(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define STRx_REG_UXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b010, 0, Rn, Rt))
#define STRw_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define STRw_REG_LSL2(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define STRB_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define STRH_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))
#define STRxw_REG(Rt, Rn, Rm)           EMIT(STR_REG_gen(rex.w?0b11:0b10, Rm, 0b011, 0, Rn, Rt))

// LOAD/STORE PAIR
#define MEMPAIR_gen(size, L, op2, imm7, Rt2, Rn, Rt)    ((size)<<31 | 0b101<<27 | (op2)<<23 | (L)<<22 | (imm7)<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))

#define LDPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 1, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 1, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 1, 0b01, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define LDPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 1, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 1, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 1, 0b11, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define LDPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 1, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 1, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 1, 0b10, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))

#define STPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 0, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 0, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 0, 0b01, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define STPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 0, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 0, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 0, 0b11, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define STPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 0, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 0, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 0, 0b10, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))

// PUSH / POP helper
#define POP1(reg)       LDRx_S9_postindex(reg, xRSP, 8)
#define PUSH1(reg)      STRx_S9_preindex(reg, xRSP, -8)

// LOAD/STORE Acquire Exclusive
#define MEMAX_gen(size, L, Rs, Rn, Rt)      ((size)<<30 | 0b001000<<24 | (L)<<22 | (Rs)<<16 | 1<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
#define LDAXRB(Rt, Rn)                  EMIT(MEMAX_gen(0b00, 1, 31, Rn, Rt))
#define STLXRB(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b00, 0, Rs, Rn, Rt))
#define LDAXRH(Rt, Rn)                  EMIT(MEMAX_gen(0b01, 1, 31, Rn, Rt))
#define STLXRH(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b01, 0, Rs, Rn, Rt))
#define LDAXRw(Rt, Rn)                  EMIT(MEMAX_gen(0b10, 1, 31, Rn, Rt))
#define STLXRw(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b10, 0, Rs, Rn, Rt))
#define LDAXRx(Rt, Rn)                  EMIT(MEMAX_gen(0b11, 1, 31, Rn, Rt))
#define STLXRx(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b11, 0, Rs, Rn, Rt))
#define LDAXRxw(Rt, Rn)                 EMIT(MEMAX_gen(2+rex.w, 1, 31, Rn, Rt))
#define STLXRxw(Rs, Rt, Rn)             EMIT(MEMAX_gen(2+rex.w, 0, Rs, Rn, Rt))

#define MEMAX_pair(size, L, Rs, Rt2, Rn, Rt)    (1<<31 | (size)<<30 | 0b001000<<24 | (L)<<22 | 1<<21 | (Rs)<<16 | 1<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))
#define LDAXPx(Rt, Rt2, Rn)             EMIT(MEMAX_pair(1, 1, 31, Rt2, Rn, Rt))
#define LDAXPw(Rt, Rt2, Rn)             EMIT(MEMAX_pair(0, 1, 31, Rt2, Rn, Rt))
#define LDAXPxw(Rt, Rt2, Rn)            EMIT(MEMAX_pair(rex.w, 1, 31, Rt2, Rn, Rt))
#define STLXPx(Rs, Rt, Rt2, Rn)         EMIT(MEMAX_pair(1, 0, Rs, Rt2, Rn, Rt))
#define STLXPw(Rs, Rt, Rt2, Rn)         EMIT(MEMAX_pair(0, 0, Rs, Rt2, Rn, Rt))
#define STLXPxw(Rs, Rt, Rt2, Rn)        EMIT(MEMAX_pair(rex.w, 0, Rs, Rt2, Rn, Rt))

// LOAD/STORE Exclusive
#define MEMX_gen(size, L, Rs, Rn, Rt)       ((size)<<30 | 0b001000<<24 | (L)<<22 | (Rs)<<16 | 0<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
#define LDXRB(Rt, Rn)                   EMIT(MEMX_gen(0b00, 1, 31, Rn, Rt))
#define STXRB(Rs, Rt, Rn)               EMIT(MEMX_gen(0b00, 0, Rs, Rn, Rt))
#define LDXRH(Rt, Rn)                   EMIT(MEMX_gen(0b01, 1, 31, Rn, Rt))
#define STXRH(Rs, Rt, Rn)               EMIT(MEMX_gen(0b01, 0, Rs, Rn, Rt))
#define LDXRw(Rt, Rn)                   EMIT(MEMX_gen(0b10, 1, 31, Rn, Rt))
#define STXRw(Rs, Rt, Rn)               EMIT(MEMX_gen(0b10, 0, Rs, Rn, Rt))
#define LDXRx(Rt, Rn)                   EMIT(MEMX_gen(0b11, 1, 31, Rn, Rt))
#define STXRx(Rs, Rt, Rn)               EMIT(MEMX_gen(0b11, 0, Rs, Rn, Rt))
#define LDXRxw(Rt, Rn)                  EMIT(MEMX_gen(2+rex.w, 1, 31, Rn, Rt))
#define STXRxw(Rs, Rt, Rn)              EMIT(MEMX_gen(2+rex.w, 0, Rs, Rn, Rt))

// Prefetch
#define PRFM_register(Rm, option, S, Rn, Rt)    (0b11<<30 | 0b111<<27 | 0b10<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | 0b10<<10 | (Rn)<<5 | (Rt))
#define PLD_L1_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00000))
#define PLD_L2_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00010))
#define PLD_L3_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00100))
#define PLD_L1_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00001))
#define PLD_L2_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00011))
#define PLD_L3_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00101))

#define PRFM_imm(imm12, Rn, Rt)         (0b11<<30 | 0b111<<27 | 0b01<<24 | 0b10<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define PLD_L1_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00000))
#define PLD_L2_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00010))
#define PLD_L3_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00100))
#define PLD_L1_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00001))
#define PLD_L2_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00011))
#define PLD_L3_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00101))

#define PST_L1_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b01001))


// BR and Branches
#define BR_gen(Z, op, A, M, Rn, Rm)       (0b1101011<<25 | (Z)<<24 | (op)<<21 | 0b11111<<16 | (A)<<11 | (M)<<10 | (Rn)<<5 | (Rm))
#define BR(Rn)                            EMIT(BR_gen(0, 0b00, 0, 0, Rn, 0))
#define BLR(Rn)                           EMIT(BR_gen(0, 0b01, 0, 0, Rn, 0))

#define CB_gen(sf, op, imm19, Rt)       ((sf)<<31 | 0b011010<<25 | (op)<<24 | (imm19)<<5 | (Rt))
#define CBNZx(Rt, imm19)                EMIT(CB_gen(1, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZw(Rt, imm19)                EMIT(CB_gen(0, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZxw(Rt, imm19)               EMIT(CB_gen(rex.w, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZx(Rt, imm19)                 EMIT(CB_gen(1, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZw(Rt, imm19)                 EMIT(CB_gen(0, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZxw(Rt, imm19)                EMIT(CB_gen(rex.w, 0, ((imm19)>>2)&0x7FFFF, Rt))

#define TB_gen(b5, op, b40, imm14, Rt)  ((b5)<<31 | 0b011011<<25 | (op)<<24  | (b40)<<19 | (imm14)<<5 | (Rt))
#define TBZ(Rt, bit, imm16)             EMIT(TB_gen(((bit)>>5)&1, 0, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))
#define TBNZ(Rt, bit, imm16)            EMIT(TB_gen(((bit)>>5)&1, 1, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))

#define Bcond_gen(imm19, cond)          (0b0101010<<25 | (imm19)<<5 | (cond))
#define Bcond(cond, imm19)              EMIT(Bcond_gen(((imm19)>>2)&0x7FFFF, cond))

#define B_gen(imm26)                    (0b000101<<26 | (imm26))
#define B(imm26)                        EMIT(B_gen(((imm26)>>2)&0x3ffffff))

#define BL_gen(imm26)                   (0b100101<<26 | (imm26))
#define BL(imm26)                       EMIT(BL_gen(((imm26)>>2)&0x3ffffff))

#define NOP                             EMIT(0b11010101000000110010000000011111)

#define CSINC_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define CSINCx(Rd, Rn, Rm, cond)            EMIT(CSINC_gen(1, Rm, cond, Rn, Rd))
#define CSINCw(Rd, Rn, Rm, cond)            EMIT(CSINC_gen(0, Rm, cond, Rn, Rd))
#define CSINCxw(Rd, Rn, Rm, cond)           EMIT(CSINC_gen(rex.w, Rm, cond, Rn, Rd))
#define CSETx(Rd, cond)                     CSINCx(Rd, xZR, xZR, invCond(cond))
#define CSETw(Rd, cond)                     CSINCw(Rd, xZR, xZR, invCond(cond))
#define CSETxw(Rd, cond)                    CSINCxw(Rd, xZR, xZR, invCond(cond))

#define CSINV_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 1<<30 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | (Rd))
#define CSINVx(Rd, Rn, Rm, cond)            EMIT(CSINV_gen(1, Rm, cond, Rn, Rd))
#define CSINVw(Rd, Rn, Rm, cond)            EMIT(CSINV_gen(0, Rm, cond, Rn, Rd))
#define CSINVxw(Rd, Rn, Rm, cond)           EMIT(CSINV_gen(rex.w?, Rm, cond, Rn, Rd))
#define CINVx(Rd, Rn, cond)                 CSINVx(Rd, Rn, Rn, invertCond(cond))
#define CINVw(Rd, Rn, cond)                 CSINVw(Rd, Rn, Rn, invertCond(cond))
#define CINVxw(Rd, Rn, cond)                CSINVxw(Rd, Rn, Rn, invertCond(cond))
#define CSETMx(Rd, cond)                    CSINVx(Rd, xZR, xZR, invCond(cond))
#define CSETMw(Rd, cond)                    CSINVw(Rd, xZR, xZR, invCond(cond))
#define CSETMxw(Rd, cond)                   CSINVxw(Rd, xZR, xZR, invCond(cond))

#define CSEL_gen(sf, Rm, cond, Rn, Rd)      ((sf<<31) | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | Rd)
#define CSELx(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(1, Rm, cond, Rn, Rd))
#define CSELw(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(0, Rm, cond, Rn, Rd))
#define CSELxw(Rd, Rn, Rm, cond)            EMIT(CSEL_gen(rex.w, Rm, cond, Rn, Rd))

#define CSNEG_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 1<<30 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define CSNEGx(Rd, Rn, Rm, cond)            EMIT(CSNEG_gen(1, Rm, cond, Rn, Rd))
#define CSNEGw(Rd, Rn, Rm, cond)            EMIT(CSNEG_gen(0, Rm, cond, Rn, Rd))
#define CSNEGxw(Rd, Rn, Rm, cond)           EMIT(CSNEG_gen(rex.w, Rm, cond, Rn, Rd))
#define CNEGx(Rd, Rn, cond)                 CSNEGx(Rn, Rn, Rn, invCond(cond))
#define CNEGw(Rd, Rn, cond)                 CSNEGw(Rn, Rn, Rn, invCond(cond))
#define CNEGxw(Rd, Rn, cond)                CSNEGxw(Rn, Rn, Rn, invCond(cond))

// AND / ORR
#define LOGIC_gen(sf, opc, N, immr, imms, Rn, Rd)  ((sf)<<31 | (opc)<<29 | 0b100100<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | Rd)
// logic to get the mask is ... convoluted... list of possible value there: https://gist.github.com/dinfuehr/51a01ac58c0b23e4de9aac313ed6a06a
#define ANDx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b00, N, immr, imms, Rn, Rd))
#define ANDw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b00, 0, immr, imms, Rn, Rd))
#define ANDSx_mask(Rd, Rn, N, immr, imms)   EMIT(LOGIC_gen(1, 0b11, N, immr, imms, Rn, Rd))
#define ANDSw_mask(Rd, Rn, immr, imms)      EMIT(LOGIC_gen(0, 0b11, 0, immr, imms, Rn, Rd))
#define ORRx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b01, N, immr, imms, Rn, Rd))
#define ORRw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define EORx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b10, N, immr, imms, Rn, Rd))
#define EORw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b10, 0, immr, imms, Rn, Rd))
#define TSTx_mask(Rn, N, immr, imms)        ANDSx_mask(xZR, Rn, N, immr, imms)
#define TSTw_mask(Rn, immr, imms)           ANDSw_mask(wZR, Rn, immr, imms)

#define LOGIC_REG_gen(sf, opc, shift, N, Rm, imm6, Rn, Rd)    ((sf)<<31 | (opc)<<29 | 0b01010<<24 | (shift)<<22 | (N)<<21 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ANDx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSx_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(1, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(0, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSxw_REG(Rd, Rn, Rm)          EMIT(LOGIC_REG_gen(rex.w, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORNx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, lsl, Rn, Rd))
#define EORx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define MOVx_REG(Rd, Rm)                ORRx_REG(Rd, xZR, Rm)
#define MOVw_REG(Rd, Rm)                ORRw_REG(Rd, xZR, Rm)
#define MOVxw_REG(Rd, Rm)               ORRxw_REG(Rd, xZR, Rm)
#define LSLw_IMM(Rd, Rm, lsl)           ORRw_REG_LSL(Rd, xZR, Rm, lsl)
#define LSLx_IMM(Rd, Rm, lsl)           ORRx_REG_LSL(Rd, xZR, Rm, lsl)
#define LSLxw_IMM(Rd, Rm, lsl)          ORRxw_REG_LSL(Rd, xZR, Rm, lsl)
#define LSRw_IMM(Rd, Rm, lsr)           ORRw_REG_LSR(Rd, xZR, Rm, lsr)
#define LSRx_IMM(Rd, Rm, lsr)           ORRx_REG_LSR(Rd, xZR, Rm, lsr)
#define LSRxw_IMM(Rd, Rm, lsr)          ORRxw_REG_LSR(Rd, xZR, Rm, lsr)
#define MVNx_REG(Rd, Rm)                ORNx_REG(Rd, xZR, Rm)
#define MVNx_REG_LSL(Rd, Rm, lsl)       ORNx_REG_LSL(Rd, xZR, Rm, lsl)
#define MVNw_REG(Rd, Rm)                ORNw_REG(Rd, xZR, Rm)
#define MVNxw_REG(Rd, Rm)               ORNxw_REG(Rd, xZR, Rm)
#define MOV_frmSP(Rd)                   ADDx_U12(Rd, xSP, 0)
#define MOV_toSP(Rm)                    ADDx_U12(xSP, Rm, 0)
#define BICx(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw_LSL(Rd, Rn, Rm, lsl)       EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICSx(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSw(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICxw(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSxw(Rd, Rn, Rm)              EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICx_REG    BICx
#define BICw_REG    BICw
#define BICxw_REG   BICxw
#define TSTx_REG(Rn, Rm)                ANDSx_REG(xZR, Rn, Rm)
#define TSTw_REG(Rn, Rm)                ANDSw_REG(wZR, Rn, Rm)
#define TSTxw_REG(Rn, Rm)               ANDSxw_REG(xZR, Rn, Rm)

// ASRV
#define ASRV_gen(sf, Rm, Rn, Rd)        ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define ASRx_REG(Rd, Rn, Rm)            EMIT(ASRV_gen(1, Rm, Rn, Rd))
#define ASRw_REG(Rd, Rn, Rm)            EMIT(ASRV_gen(0, Rm, Rn, Rd))
#define ASRxw_REG(Rd, Rn, Rm)           EMIT(ASRV_gen(rex.w, Rm, Rn, Rd))

// BFI
#define BFM_gen(sf, opc, N, immr, imms, Rn, Rd) ((sf)<<31 | (opc)<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define BFMx(Rd, Rn, immr, imms)        EMIT(BFM_gen(1, 0b01, 1, immr, imms, Rn, Rd))
#define BFMw(Rd, Rn, immr, imms)        EMIT(BFM_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define BFMxw(Rd, Rn, immr, imms)       EMIT(BFM_gen(rex.w, 0b01, rex.w, immr, imms, Rn, Rd))
#define BFIx(Rd, Rn, lsb, width)        BFMx(Rd, Rn, ((-lsb)%64)&0x3f, (width)-1)
#define BFIw(Rd, Rn, lsb, width)        BFMw(Rd, Rn, ((-lsb)%32)&0x1f, (width)-1)
#define BFIxw(Rd, Rn, lsb, width)       if(rex.w) {BFIx(Rd, Rn, lsb, width);} else {BFIw(Rd, Rn, lsb, width);}
#define BFCx(Rd, lsb, width)            BFMx(Rd, xZR, ((-lsb)%64)&0x3f, (width)-1)
#define BFCw(Rd, lsb, width)            BFMw(Rd, xZR, ((-lsb)%32)&0x1f, (width)-1)
#define BFCxw(Rd, lsb, width)           BFMxw(Rd, xZR, rex.w?(((-lsb)%64)&0x3f):(((-lsb)%32)&0x1f), (width)-1)
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILx(Rd, Rn, lsb, width)      EMIT(BFM_gen(1, 0b01, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILw(Rd, Rn, lsb, width)      EMIT(BFM_gen(0, 0b01, 0, (lsb), (lsb)+(width)-1, Rn, Rd))
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILxw(Rd, Rn, lsb, width)     EMIT(BFM_gen(rex.w, 0b01, rex.w, (lsb), (lsb)+(width)-1, Rn, Rd))

// UBFX
#define UBFM_gen(sf, N, immr, imms, Rn, Rd)    ((sf)<<31 | 0b10<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define UBFMx(Rd, Rn, immr, imms)       EMIT(UBFM_gen(1, 1, immr, imms, Rn, Rd))
#define UBFMw(Rd, Rn, immr, imms)       EMIT(UBFM_gen(0, 0, immr, imms, Rn, Rd))
#define UBFMxw(Rd, Rn, immr, imms)      EMIT(UBFM_gen(rex.w, rex.w, immr, imms, Rn, Rd))
#define UBFXx(Rd, Rn, lsb, width)       EMIT(UBFM_gen(1, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UBFXw(Rd, Rn, lsb, width)       EMIT(UBFM_gen(0, 0, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UBFXxw(Rd, Rn, lsb, width)      EMIT(UBFM_gen(rex.w, rex.w, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UXTBx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 7, Rn, Rd))
#define UXTBw(Rd, Rn)                   EMIT(UBFM_gen(0, 0, 0, 7, Rn, Rd))
#define UXTBxw(Rd, Rn)                  EMIT(UBFM_gen(rex.w, rex.w, 0, 7, Rn, Rd))
#define UXTHx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 15, Rn, Rd))
#define UXTHw(Rd, Rn)                   EMIT(UBFM_gen(0, 0, 0, 15, Rn, Rd))
#define LSRx(Rd, Rn, shift)             EMIT(UBFM_gen(1, 1, shift, 63, Rn, Rd))
#define LSRw(Rd, Rn, shift)             EMIT(UBFM_gen(0, 0, shift, 31, Rn, Rd))
#define LSRxw(Rd, Rn, shift)            EMIT(UBFM_gen(rex.w, rex.w, shift, (rex.w)?63:31, Rn, Rd))
#define LSLx(Rd, Rn, lsl)               UBFMx(Rd, Rn, ((-(lsl))%64)&63, 63-(lsl))
#define LSLw(Rd, Rn, lsl)               UBFMw(Rd, Rn, ((-(lsl))%32)&31, 31-(lsl))
#define LSLxw(Rd, Rn, lsl)              UBFMxw(Rd, Rn, rex.w?(((-(lsl))%64)&63):(((-(lsl))%32)&31), (rex.w?63:31)-(lsl))
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZx(Rd, Rn, lsb, width)      UBFMx(Rd, Rn, ((-(lsb))%64)&63, width-1)
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZw(Rd, Rn, lsb, width)      UBFMw(Rd, Rn, ((-(lsb))%32)&31, width-1)
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZxw(Rd, Rn, lsb, width)     UBFMxw(Rd, Rn, rex.w?(((-(lsb))%64)&63):(((-(lsb))%32)&31), width-1)

// SBFM
#define SBFM_gen(sf, N, immr, imms, Rn, Rd)    ((sf)<<31 | 0b00<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define SBFMx(Rd, Rn, immr, imms)       EMIT(SBFM_gen(1, 1, immr, imms, Rn, Rd))
#define SBFMw(Rd, Rn, immr, imms)       EMIT(SBFM_gen(0, 0, immr, imms, Rn, Rd))
#define SBFMxw(Rd, Rn, immr, imms)      EMIT(SBFM_gen(rex.w, rex.w, immr, imms, Rn, Rd))
#define SBFXx(Rd, Rn, lsb, width)       SBFMx(Rd, Rn, lsb, lsb+width-1)
#define SBFXw(Rd, Rn, lsb, width)       SBFMw(Rd, Rn, lsb, lsb+width-1)
#define SBFXxw(Rd, Rn, lsb, width)      SBFMxw(Rd, Rn, lsb, lsb+width-1)
#define SXTBx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 7)
#define SXTBw(Rd, Rn)                   SBFMw(Rd, Rn, 0, 7)
#define SXTHx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 15)
#define SXTHw(Rd, Rn)                   SBFMw(Rd, Rn, 0, 15)
#define SXTHxw(Rd, Rn)                  SBFMxw(Rd, Rn, 0, 15)
#define SXTWx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 31)
#define ASRx(Rd, Rn, shift)             SBFMx(Rd, Rn, shift, 63)
#define ASRw(Rd, Rn, shift)             SBFMw(Rd, Rn, shift, 31)
#define ASRxw(Rd, Rn, shift)            SBFMxw(Rd, Rn, shift, rex.w?63:31)
#define SBFIZx(Rd, Rn, lsb, width)      SFBFMx(Rd, Rn, ((-(lsb))%64), (width)-1)
#define SBFIZw(Rd, Rn, lsb, width)      SFBFMw(Rd, Rn, ((-(lsb))%32), (width)-1)
#define SBFIZxw(Rd, Rn, lsb, width)     SFBFMxw(Rd, Rn, ((-(lsb))%(rex.w?64:32)), (width)-1)

// EXTR
#define EXTR_gen(sf, N, Rm, imms, Rn, Rd)   ((sf)<<31 | 0b00<<29 | 0b100111<<23 | (N)<<22 | (Rm)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define EXTRx(Rd, Rn, Rm, lsb)          EMIT(EXTR_gen(1, 1, Rm, lsb, Rn, Rd))
#define EXTRw(Rd, Rn, Rm, lsb)          EMIT(EXTR_gen(0, 0, Rm, lsb, Rn, Rd))
#define EXTRxw(Rd, Rn, Rm, lsb)         EMIT(EXTR_gen(rex.w, rex.w, Rm, lsb, Rn, Rd))
#define RORx(Rd, Rn, lsb)               EMIT(EXTR_gen(1, 1, Rn, lsb, Rn, Rd))
#define RORw(Rd, Rn, lsb)               EMIT(EXTR_gen(0, 0, Rn, lsb, Rn, Rd))
#define RORxw(Rd, Rn, lsb)              EMIT(EXTR_gen(rex.w, rex.w, Rn, lsb, Rn, Rd))

// RORV
#define RORV_gen(sf, Rm, Rn, Rd)        ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | 0b11<<10 | (Rn)<<5 | (Rd))
#define RORx_REG(Rd, Rn, Rm)            EMIT(RORV_gen(1, Rm, Rn, Rd))
#define RORw_REG(Rd, Rn, Rm)            EMIT(RORV_gen(0, Rm, Rn, Rd))
#define RORxw_REG(Rd, Rn, Rm)           EMIT(RORV_gen(rex.w, Rm, Rn, Rd))


// LSRV / LSLV
#define LS_V_gen(sf, Rm, op2, Rn, Rd)   ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | (op2)<<10 | (Rn)<<5 | (Rd))
#define LSRx_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(1, Rm, 0b01, Rn, Rd))
#define LSRw_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(0, Rm, 0b01, Rn, Rd))
#define LSRxw_REG(Rd, Rn, Rm)           EMIT(LS_V_gen(rex.w, Rm, 0b01, Rn, Rd))

#define LSLx_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(1, Rm, 0b00, Rn, Rd))
#define LSLw_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(0, Rm, 0b00, Rn, Rd))
#define LSLxw_REG(Rd, Rn, Rm)           EMIT(LS_V_gen(rex.w, Rm, 0b00, Rn, Rd))

// UMULL / SMULL
#define MADDL_gen(U, Rm, o0, Ra, Rn, Rd)    (1<<31 | 0b11011<<24 | (U)<<23 | 0b01<<21 | (Rm)<<16 | (o0)<<15 | (Ra)<<10 | (Rn)<<5 | (Rd))
#define UMADDL(Xd, Wn, Wm, Xa)          EMIT(MADDL_gen(1, Wm, 0, Xa, Wn, Xd))
#define UMULL(Xd, Wn, Wm)               UMADDL(Xd, Wn, Wm, xZR)
#define SMADDL(Xd, Wn, Wm, Xa)          EMIT(MADDL_gen(0, Wm, 0, Xa, Wn, Xd))
#define SMULL(Xd, Wn, Wm)               SMADDL(Xd, Wn, Wm, xZR)

#define MULH_gen(U, Rm, Rn, Rd)         (1<<31 | 0b11011<<24 | (U)<<23 | 0b10<<21 | (Rm)<<16 | 0b11111<<10 | (Rn)<<5 | (Rd))
#define UMULH(Xd, Xn, Xm)               EMIT(MULH_gen(1, Xm, Xn, Xd))
#define SMULH(Xd, Xn, Xm)               EMIT(MULH_gen(0, Xm, Xn, Xd))

#define MADD_gen(sf, Rm, o0, Ra, Rn, Rd)    ((sf)<<31 | 0b11011<<24 | (Rm)<<16 | (o0)<<15 | (Ra)<<10 | (Rn)<<5 | (Rd))
#define MADDx(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(1, Rm, 0, Ra, Rn, Rd))
#define MADDw(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(0, Rm, 0, Ra, Rn, Rd))
#define MADDxw(Rd, Rn, Rm, Ra)          EMIT(MADD_gen(rex.w, Rm, 0, Ra, Rn, Rd))
#define MULx(Rd, Rn, Rm)                MADDx(Rd, Rn, Rm, xZR)
#define MULw(Rd, Rn, Rm)                MADDw(Rd, Rn, Rm, xZR)
#define MULxw(Rd, Rn, Rm)               MADDxw(Rd, Rn, Rm, xZR)
#define MSUBx(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(1, Rm, 1, Ra, Rn, Rd))
#define MSUBw(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(0, Rm, 1, Ra, Rn, Rd))
#define MSUBxw(Rd, Rn, Rm, Ra)          EMIT(MADD_gen(rex.w, Rm, 1, Ra, Rn, Rd))
#define MNEGx(Rd, Rn, Rm)               EMIT(MADD_gen(1, Rm, 1, xZR, Rn, Rd))
#define MNEGw(Rd, Rn, Rm)               EMIT(MADD_gen(0, Rm, 1, xZR, Rn, Rd))
#define MNEGxw(Rd, Rn, Rm)              EMIT(MADD_gen(rex.w, Rm, 1, xZR, Rn, Rd))


// DIV
#define DIV_gen(sf, Rm, o1, Rn, Rd)     ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b00001<<11 | (o1)<<10 | (Rn)<<5 | (Rd))
#define UDIVw(Wd, Wn, Wm)               EMIT(DIV_gen(0, Wm, 0, Wn, Wd))
#define UDIVx(Xd, Xn, Xm)               EMIT(DIV_gen(1, Xm, 0, Xn, Xd))
#define SDIVw(Wd, Wn, Wm)               EMIT(DIV_gen(0, Wm, 1, Wn, Wd))
#define SDIVx(Xd, Xn, Xm)               EMIT(DIV_gen(1, Xm, 1, Xn, Xd))

// CLZ
#define CL_gen(sf, op, Rn, Rd)          ((sf)<<31 | 1<<30 | 0b11010110<<21 | 0b00010<<11 | (op)<<10 | (Rn)<<5 | (Rd))
#define CLZx(Rd, Rn)                    EMIT(CL_gen(1, 0, Rn, Rd))
#define CLZw(Rd, Rn)                    EMIT(CL_gen(0, 0, Rn, Rd))
#define CLZxw(Rd, Rn)                   EMIT(CL_gen(rex.w, 0, Rn, Rd))
#define CLSx(Rd, Rn)                    EMIT(CL_gen(1, 1, Rn, Rd))
#define CLSw(Rd, Rn)                    EMIT(CL_gen(0, 1, Rn, Rd))
#define CLSxw(Rd, Rn)                   EMIT(CL_gen(rex.w, 1, Rn, Rd))

// RBIT
#define RBIT_gen(sf, Rn, Rd)            ((sf)<<31 | 1<<30 | 0b11010110<<21 | (Rn)<<5 | (Rd))
#define RBITx(Rd, Rn)                   EMIT(RBIT_gen(1, Rn, Rd))
#define RBITw(Rd, Rn)                   EMIT(RBIT_gen(0, Rn, Rd))
#define RBITxw(Rd, Rn)                  EMIT(RBIT_gen(rex.w, Rn, Rd))

// REV
#define REV_gen(sf, opc, Rn, Rd)        ((sf)<<31 | 1<<30 | 0b11010110<<21 | (opc)<<10 | (Rn)<<5 | (Rd))
#define REVx(Rd, Rn)                    EMIT(REV_gen(1, 0b11, Rn, Rd))
#define REVw(Rd, Rn)                    EMIT(REV_gen(0, 0b10, Rn, Rd))
#define REVxw(Rd, Rn)                   EMIT(REV_gen(rex.w, 0b10|rex.w, Rn, Rd))

// MRS
#define MRS_gen(L, o0, op1, CRn, CRm, op2, Rt)  (0b1101010100<<22 | (L)<<21 | 1<<20 | (o0)<<19 | (op1)<<16 | (CRn)<<12 | (CRm)<<8 | (op2)<<5 | (Rt))
// mrs    x0, nzcv : 1101010100 1 1 1 011 0100 0010 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0
// MRS : from System register
#define MRS_nzvc(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 2, 0, Rt))
// MSR : to System register
#define MSR_nzvc(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 2, 0, Rt))
// mrs    x0, fpcr : 1101010100 1 1 1 011 0100 0100 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=0
#define MRS_fpcr(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 4, 0, Rt))
#define MSR_fpcr(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 4, 0, Rt))
// mrs    x0, fpsr : 1101010100 1 1 1 011 0100 0100 001 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=1
#define MRS_fpsr(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 4, 1, Rt))
#define MSR_fpsr(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 4, 1, Rt))
// NEON Saturation Bit
#define FPSR_QC 27
// NEON Input Denormal Cumulative
#define FPSR_IDC    7
// NEON IneXact Cumulative
#define FPSR_IXC    4
// NEON Underflow Cumulative
#define FPSR_UFC    3
// NEON Overflow Cumulative
#define FPSR_OFC    2
// NEON Divide by 0 Cumulative
#define FPSR_DZC    1
// NEON Invalid Operation Cumulative
#define FPSR_IOC    0
                     
// FCSEL
#define FCSEL_scalar(type, Rm, cond, Rn, Rd)    (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | (cond)<<12 | 0b11<<10 | (Rn)<<5 | (Rd))
#define FCSELS(Sd, Sn, Sm, cond)        EMIT(FCSEL_scalar(0b00, Sm, cond, Sn, Sd))
#define FCSELD(Dd, Dn, Dm, cond)        EMIT(FCSEL_scalar(0b01, Dm, cond, Dn, Dd))
 
// VLDR
#define VMEM_gen(size, opc, imm12, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | 0b01<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
// imm14 must be 3-aligned
#define VLDR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b01, ((uint32_t)((imm14)>>2))&0xfff, Rn, Dt))
// imm15 must be 3-aligned
#define VLDR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b01, ((uint32_t)((imm15)>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VLDR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b11, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))
// (imm14) must be 3-aligned
#define VSTR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b00, ((uint32_t)(imm14>>2))&0xfff, Rn, Dt))
// (imm15) must be 3-aligned
#define VSTR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b00, ((uint32_t)(imm15>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VSTR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b10, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))
// (imm14) must be 1-aligned
#define VSTR16_U12(Ht, Rn, imm14)           EMIT(VMEM_gen(0b01, 0b00, ((uint32_t)(imm14>>1))&0xfff, Rn, Ht))

#define VMEMUR_vector(size, opc, imm9, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | (imm9)<<12 | (Rn)<<5 | (Rt))
// signed offset, no alignement!
#define VLDR8_I9(Vt, Rn, imm9)              EMIT(VMEMUR(0b00, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR16_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b01, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR32_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b10, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR64_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b11, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR128_I9(Vt, Rn, imm9)            EMIT(VMEMUR(0b00, 0b11, (imm9)&0b111111111, Rn, Vt))
// signed offset, no alignement!
#define VSTR8_I9(Vt, Rn, imm9)              EMIT(VMEMUR(0b00, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR16_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b01, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR32_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b10, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR64_I9(Vt, Rn, imm9)             EMIT(VMEMUR(0b11, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR128_I9(Vt, Rn, imm9)            EMIT(VMEMUR(0b00, 0b10, (imm9)&0b111111111, Rn, Vt))

#define VMEMW_gen(size, opc, imm9, op2, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | (imm9)<<12 | (op2)<<10 | 0b01<<10 | (Rn)<<5 | (Rt))
#define VLDR64_S9_postindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b01, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VLDR64_S9_preindex(Rt, Rn, imm9)    EMIT(VMEMW_gen(0b11, 0b01, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VLDR128_S9_postindex(Rt, Rn, imm9)  EMIT(VMEMW_gen(0b11, 0b11, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VLDR128_S9_preindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b11, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VSTR64_S9_postindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VSTR64_S9_preindex(Rt, Rn, imm9)    EMIT(VMEMW_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VSTR128_S9_postindex(Rt, Rn, imm9)  EMIT(VMEMW_gen(0b11, 0b10, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VSTR128_S9_preindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b10, (imm9)&0x1ff, 0b11, Rn, Rt))

#define VMEM_REG_gen(size, opc, Rm, option, S, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | 0b10<<10 | (Rn)<<5 | (Rt))

#define VLDR32_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR32_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 0, Rn, Dt))
#define VLDR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 1, Rn, Dt))

#define VSTR32_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR32_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 0, Rn, Dt))
#define VSTR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 1, Rn, Dt))

#define VLDR_PC_gen(opc, imm19, Rt)         ((opc)<<30 | 0b011<<27 | 1<<26 | (imm19)<<5 | (Rt))
#define VLDR32_literal(Vt, imm19)           EMIT(VLDR_PC_gen(0b00, ((imm19)>>2)&0x7FFFF, Vt))
#define VLDR64_literal(Vt, imm19)           EMIT(VLDR_PC_gen(0b01, ((imm19)>>2)&0x7FFFF, Vt))
#define VLDR128_literal(Vt, imm19)          EMIT(VLDR_PC_gen(0b10, ((imm19)>>2)&0x7FFFF, Vt))


#define LD1R_gen(Q, size, Rn, Rt)           ((Q)<<30 | 0b0011010<<23 | 1<<22 | 0<<21 | 0b110<<13 | (size)<<10 | (Rn)<<5 | (Rt))
#define VLDQ1R_8(Vt, Rn)                    EMIT(LD1R_gen(1, 0b00, Rn, Vt))
#define VLDQ1R_16(Vt, Rn)                   EMIT(LD1R_gen(1, 0b01, Rn, Vt))
#define VLDQ1R_32(Vt, Rn)                   EMIT(LD1R_gen(1, 0b10, Rn, Vt))
#define VLDQ1R_64(Vt, Rn)                   EMIT(LD1R_gen(1, 0b11, Rn, Vt))
#define VLD1R_8(Vt, Rn)                     EMIT(LD1R_gen(0, 0b00, Rn, Vt))
#define VLD1R_16(Vt, Rn)                    EMIT(LD1R_gen(0, 0b01, Rn, Vt))
#define VLD1R_32(Vt, Rn)                    EMIT(LD1R_gen(0, 0b10, Rn, Vt))

#define LD1_single(Q, opcode, S, size, Rn, Rt)  ((Q)<<30 | 0b0011010<<23 | 1<<22 | 0<<21 | (opcode)<<13 | (S)<<12 | (size)<<10 | (Rn)<<5 | (Rt))
#define VLD1_8(Vt, index, Rn)               EMIT(LD1_single(((index)>>3)&1, 0b000, ((index)>>2)&1, (index)&3, Rn, Vt))
#define VLD1_16(Vt, index, Rn)              EMIT(LD1_single(((index)>>2)&1, 0b010, ((index)>>1)&1, ((index)&1)<<1, Rn, Vt))
#define VLD1_32(Vt, index, Rn)              EMIT(LD1_single(((index)>>1)&1, 0b100, ((index))&1, 0b00, Rn, Vt))
#define VLD1_64(Vt, index, Rn)              EMIT(LD1_single(((index))&1, 0b100, 0, 0b01, Rn, Vt))

#define ST1_single(Q, opcode, S, size, Rn, Rt)  ((Q)<<30 | 0b0011010<<23 | 0<<22 | 0<<21 | (opcode)<<13 | (S)<<12 | (size)<<10 | (Rn)<<5 | (Rt))
#define VST1_8(Vt, index, Rn)               EMIT(ST1_single(((index)>>3)&1, 0b000, ((index)>>2)&1, (index)&3, Rn, Vt))
#define VST1_16(Vt, index, Rn)              EMIT(ST1_single(((index)>>2)&1, 0b010, ((index)>>1)&1, ((index)&1)<<1, Rn, Vt))
#define VST1_32(Vt, index, Rn)              EMIT(ST1_single(((index)>>1)&1, 0b100, ((index))&1, 0b00, Rn, Vt))
#define VST1_64(Vt, index, Rn)              EMIT(ST1_single(((index))&1, 0b100, 0, 0b01, Rn, Vt))

// LOGIC
#define VLOGIC_gen(Q, opc2, Rm, Rn, Rd)     ((Q)<<30 | 1<<29 | 0b01110<<24 | (opc2)<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VEORQ(Vd, Vn, Vm)                   EMIT(VLOGIC_gen(1, 0b00, Vm, Vn, Vd))
#define VEOR(Vd, Vn, Vm)                    EMIT(VLOGIC_gen(0, 0b00, Vm, Vn, Vd))

#define VLOGIC_immediate(Q, op, abc, cmade, defgh, Rd)  ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | (cmode)<<12 | 1<<10 | (defgh)<<5 | (Rd))
//#define V

#define SHL_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VSHLQ_8(Vd, Vn, shift)              EMIT(SHL_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSHLQ_16(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b0010 | (((shift)>>3)&1), (shift)&7, Vn, Vd))
#define VSHLQ_32(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSHLQ_64(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSHL_8(Vd, Vn, shift)               EMIT(SHL_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSHL_16(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0010 | (((shift)>>3)&1), (shift)&7, Vn, Vd))
#define VSHL_32(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

#define SHL_scalar(U, size, Rm, R, S, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b010<<13 | (R)<<12 | (S)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHL_R_64(Vd, Vn, Vm)               EMIT(SHL_scalar(0, 0b11, Vm, 0, 0, Vn, Vd))
#define USHL_R_64(Vd, Vn, Vm)               EMIT(SHL_scalar(1, 0b11, Vm, 0, 0, Vn, Vd))

#define SHL_scalar_imm(U, immh, immb, Rn, Rd)   (0b01<<30 | 0b111110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SHL_64(Vd, Vn, shift)               EMIT(SHL_scalar_imm(0, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))

#define SHL_vector_vector(Q, U, size, Rm, R, S, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b010<<13 | (R)<<12 | (S)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHL_8(Vd, Vn, Vm)                  EMIT(SHL_vector_vector(0, 0, 0b00, Vm, 0, 0, Vn, Vd))
#define SSHL_16(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 0, 0b01, Vm, 0, 0, Vn, Vd))
#define SSHL_32(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 0, 0b10, Vm, 0, 0, Vn, Vd))
#define SSHLQ_8(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(1, 0, 0b00, Vm, 0, 0, Vn, Vd))
#define SSHLQ_16(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b01, Vm, 0, 0, Vn, Vd))
#define SSHLQ_32(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b10, Vm, 0, 0, Vn, Vd))
#define SSHLQ_64(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b11, Vm, 0, 0, Vn, Vd))
#define USHL_8(Vd, Vn, Vm)                  EMIT(SHL_vector_vector(0, 1, 0b00, Vm, 0, 0, Vn, Vd))
#define USHL_16(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 1, 0b01, Vm, 0, 0, Vn, Vd))
#define USHL_32(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 1, 0b10, Vm, 0, 0, Vn, Vd))
#define USHLQ_8(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(1, 1, 0b00, Vm, 0, 0, Vn, Vd))
#define USHLQ_16(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b01, Vm, 0, 0, Vn, Vd))
#define USHLQ_32(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b10, Vm, 0, 0, Vn, Vd))
#define USHLQ_64(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b11, Vm, 0, 0, Vn, Vd))

#define SHR_vector(Q, U, immh, immb, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b00000<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VSHRQ_8(Vd, Vn, shift)              EMIT(SHR_vector(1, 1, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSHRQ_16(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSHRQ_32(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSHRQ_64(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, Vn, Vd))
#define VSHR_8(Vd, Vn, shift)               EMIT(SHR_vector(0, 1, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSHR_16(Vd, Vn, shift)              EMIT(SHR_vector(0, 1, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSHR_32(Vd, Vn, shift)              EMIT(SHR_vector(0, 1, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSSHRQ_8(Vd, Vn, shift)             EMIT(SHR_vector(1, 0, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSSHRQ_16(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSSHRQ_32(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSSHRQ_64(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, Vn, Vd))
#define VSSHR_8(Vd, Vn, shift)              EMIT(SHR_vector(0, 0, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSSHR_16(Vd, Vn, shift)             EMIT(SHR_vector(0, 0, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSSHR_32(Vd, Vn, shift)             EMIT(SHR_vector(0, 0, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))

#define SHR_scalar_imm(U, immh, immb, o1, o0, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b111110<<23 | (immh)<<19 | (immb)<<16 | (o1)<<13 | (o0)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHR_64(Vd, Vn, shift)              EMIT(SHR_scalar_imm(0, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, 0, 0, Vn, Vd))
#define USHR_64(Vd, Vn, shift)              EMIT(SHR_scalar_imm(1, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, 0, 0, Vn, Vd))

#define EXT_vector(Q, Rm, imm4, Rn, Rd)     ((Q)<<30 | 0b101110<<24 | (Rm)<<16 | (imm4)<<11 | (Rn)<<5 | (Rd))
#define VEXT_8(Rd, Rn, Rm, index)           EMIT(EXT_vector(0, Rm, index, Rn, Rd))
#define VEXTQ_8(Rd, Rn, Rm, index)          EMIT(EXT_vector(1, Rm, index, Rn, Rd))

// Shift Left and Insert (not touching lower part of dest)
#define SLI_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<1 | 1<<10 | (Rn)<<5 | (Rd))
#define VSLIQ_8(Vd, Vn, shift)              EMIT(VSLI_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSLIQ_16(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSLIQ_32(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSLIQ_64(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSLI_8(Vd, Vn, shift)               EMIT(VSLI_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSLI_16(Vd, Vn, shift)              EMIT(VSLI_vector(0, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSLI_32(Vd, Vn, shift)              EMIT(VSLI_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

// Shift Right and Insert (not touching higher part of dest)
#define SRI_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01000<<1 | 1<<10 | (Rn)<<5 | (Rd))
#define VSRIQ_8(Vd, Vn, shift)              EMIT(VSRI_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSRIQ_16(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSRIQ_32(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSRIQ_64(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSRI_8(Vd, Vn, shift)               EMIT(VSRI_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSRI_16(Vd, Vn, shift)              EMIT(VSRI_vector(0, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSRI_32(Vd, Vn, shift)              EMIT(VSRI_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

// Integer MATH
#define ADDSUB_vector(Q, U, size, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10000<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VADDQ_8(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(1, 0, 0b00, Vm, Vn, Vd))
#define VADDQ_16(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b01, Vm, Vn, Vd))
#define VADDQ_32(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b10, Vm, Vn, Vd))
#define VADDQ_64(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b11, Vm, Vn, Vd))
#define VADD_8(Vd, Vn, Vm)                  EMIT(ADDSUB_vector(0, 0, 0b00, Vm, Vn, Vd))
#define VADD_16(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 0, 0b01, Vm, Vn, Vd))
#define VADD_32(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 0, 0b10, Vm, Vn, Vd))
#define VSUBQ_8(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(1, 1, 0b00, Vm, Vn, Vd))
#define VSUBQ_16(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b01, Vm, Vn, Vd))
#define VSUBQ_32(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b10, Vm, Vn, Vd))
#define VSUBQ_64(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b11, Vm, Vn, Vd))
#define VSUB_8(Vd, Vn, Vm)                  EMIT(ADDSUB_vector(0, 1, 0b00, Vm, Vn, Vd))
#define VSUB_16(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 1, 0b01, Vm, Vn, Vd))
#define VSUB_32(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 1, 0b10, Vm, Vn, Vd))

#define NEGABS_vector(Q, U, size, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b01011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define NEG_8(Vd, Vn)                       EMIT(NEGABS_vector(0, 1, 0b00, Vn, Vd))
#define NEG_16(Vd, Vn)                      EMIT(NEGABS_vector(0, 1, 0b01, Vn, Vd))
#define NEG_32(Vd, Vn)                      EMIT(NEGABS_vector(0, 1, 0b10, Vn, Vd))
#define NEGQ_8(Vd, Vn)                      EMIT(NEGABS_vector(1, 1, 0b00, Vn, Vd))
#define NEGQ_16(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b01, Vn, Vd))
#define NEGQ_32(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b10, Vn, Vd))
#define NEGQ_64(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b11, Vn, Vd))
#define ABS_8(Vd, Vn)                       EMIT(NEGABS_vector(0, 0, 0b00, Vn, Vd))
#define ABS_16(Vd, Vn)                      EMIT(NEGABS_vector(0, 0, 0b01, Vn, Vd))
#define ABS_32(Vd, Vn)                      EMIT(NEGABS_vector(0, 0, 0b10, Vn, Vd))
#define ABSQ_8(Vd, Vn)                      EMIT(NEGABS_vector(1, 0, 0b00, Vn, Vd))
#define ABSQ_16(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b01, Vn, Vd))
#define ABSQ_32(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b10, Vn, Vd))
#define ABSQ_64(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b11, Vn, Vd))

#define NEGABS_vector_scalar(U, size, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 0b10000<<17 | 0b01011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define NEG_64(Vd, Vn)                     EMIT(NEGABS_vector_scalar(1, 0b11, Vn, Vd))
#define ABS_64(Vd, Vn)                     EMIT(NEGABS_vector_scalar(0, 0b11, Vn, Vd))

// FMOV
#define FMOV_general(sf, type, mode, opcode, Rn, Rd)    ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (mode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
// 32-bit to single-precision
#define FMOVSw(Sd, Wn)                      EMIT(FMOV_general(0, 0b00, 0b00, 0b111, Wn, Sd))
// Single-precision to 32-bit
#define FMOVwS(Wd, Sn)                      EMIT(FMOV_general(0, 0b00, 0b00, 0b110, Sn, Wd))
// 64-bit to double-precision
#define FMOVDx(Dd, Xn)                      EMIT(FMOV_general(1, 0b01, 0b00, 0b111, Xn, Dd))
// 64-bit to top half of 128-bit
#define FMOVD1x(Vd, Xn)                     EMIT(FMOV_general(1, 0b10, 0b01, 0b111, Xn, Vd))
// Double-precision to 64-bit
#define FMOVxD(Xd, Dn)                      EMIT(FMOV_general(1, 0b01, 0b00, 0b110, Dn, Xd))
// Top half of 128-bit to 64-bit
#define FMOVxD1(Xd, Vn)                     EMIT(FMOV_general(1, 0b10, 0b01, ob110, Vn, Xd))

#define FMOV_register(type, Rn, Rd)         (0b11110<<24 | (type)<<22 | 1<<21 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FMOVS(Sd, Sn)                       EMIT(FMOV_register(0b00, Sn, Sd))
#define FMOVD(Dd, Dn)                       EMIT(FMOV_register(0b01, Dn, Dd))

#define FMOV_vector_imm(Q, op, abc, defgh, Rd)  ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | 0b1111<<12 | 1<<10 | (defgh)<<5 | (Rd))
#define VFMOVS_8(Vd, u8)                    EMIT(FMOV_vector_imm(0, 0, ((u8)>>5)&0b111, (u8)&0b11111, Vd))
#define VFMOVSQ_8(Vd, u8)                   EMIT(FMOV_vector_imm(1, 0, ((u8)>>5)&0b111, (u8)&0b11111, Vd))
#define VFMOVDQ_8(Vd, u8)                   EMIT(FMOV_vector_imm(1, 1, ((u8)>>5)&0b111, (u8)&0b11111, Vd))

#define FMOV_scalar_imm(type, imm8, Rd)     (0b11110<<24 | (type)<<22 | 1<<21 | (imm8)<<13 | 0b100<<10 | (Rd))
#define FMOVS_8(Sd, u8)                     EMIT(FMOV_scalar_imm(0b00, u8, Sd))
#define FMOVD_8(Dd, u8)                     EMIT(FMOV_scalar_imm(0b01, u8, Dd))

// VMOV
#define VMOV_element(imm5, imm4, Rn, Rd)    (1<<30 | 1<<29 | 0b01110000<<21 | (imm5)<<16 | (imm4)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVeB(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<1) | 1, (i2), Vn, Vd))
#define VMOVeH(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<2) | 2, (i2)<<1, Vn, Vd))
#define VMOVeS(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<3) | 4, (i2)<<2, Vn, Vd))
#define VMOVeD(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<4) | 8, (i2)<<3, Vn, Vd))

#define VMOV_from(imm5, Rn, Rd)     (1<<30 | 0<<29 | 0b01110000<<21 | (imm5)<<16 | 0b0011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVQBfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<1) | 1, Wn, Vd))
#define VMOVQHfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<2) | 2, Wn, Vd))
#define VMOVQSfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<3) | 4, Wn, Vd))
#define VMOVQDfrom(Vd, index, Xn)    EMIT(VMOV_from(((index)<<4) | 8, Xn, Vd))

#define UMOV_gen(Q, imm5, Rn, Rd)   ((Q)<<30 | 0b01110000<<21 | (imm5)<<16 | 0b01<<13 | 1<<12 | 1<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVQDto(Xd, Vn, index)     EMIT(UMOV_gen(1, ((index)<<4) | 8, Vn, Xd))
#define VMOVBto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<1) | 1, Vn, Wd))
#define VMOVHto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<2) | 2, Vn, Wd))
#define VMOVSto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<3) | 4, Vn, Wd))

#define MVN_vector(Q, Rn, Rd)       ((Q)<<30 | 1<<29 | 0b01110<<24 | 0b10000<<17 | 0b00101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VMVNQ(Rd, Rn)               EMIT(MVN_vector(1, Rn, Rd))

// VORR
#define ORR_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b10<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VORRQ(Vd, Vn, Vm)           EMIT(ORR_vector(1, Vm, Vn, Vd))
#define VORR(Dd, Dn, Dm)            EMIT(ORR_vector(0, Dm, Dn, Dd))
#define VMOVQ(Vd, Vn)               EMIT(ORR_vector(1, Vn, Vn, Vd))
#define VMOV(Dd, Dn)                EMIT(ORR_vector(0, Dn, Dn, Dd))

// VAND
#define AND_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b00<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VANDQ(Vd, Vn, Vm)           EMIT(AND_vector(1, Vm, Vn, Vd))
#define VAND(Dd, Dn, Dm)            EMIT(AND_vector(0, Dm, Dn, Dd))

// VBIC
#define BIC_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b01<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VBICQ(Vd, Vn, Vm)           EMIT(BIC_vector(1, Vm, Vn, Vd))
#define VBIC(Dd, Dn, Dm)            EMIT(BIC_vector(0, Dm, Dn, Dd))

// VORN
#define ORN_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b11<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VORNQ(Vd, Vn, Vm)           EMIT(ORN_vector(1, Vm, Vn, Vd))
#define VORN(Dd, Dn, Dm)            EMIT(ORN_vector(0, Dm, Dn, Dd))

// ADD / SUB
#define FADDSUB_vector(Q, U, op, sz, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (op)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFADDQS(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 0, 0, Vm, Vn, Vd))
#define VFADDQD(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 0, 1, Vm, Vn, Vd))
#define VFADDS(Dd, Dn, Dm)          EMIT(FADDSUB_vector(0, 0, 0, 0, Dm, Dn, Dd))

#define VFSUBQS(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 1, 0, Vm, Vn, Vd))
#define VFSUBQD(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 1, 1, Vm, Vn, Vd))
#define VFSUBS(Dd, Dn, Dm)          EMIT(FADDSUB_vector(0, 0, 1, 0, Dm, Dn, Dd))

#define FADDSUB_scalar(type, Rm, op, Rn, Rd)    (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b001<<13 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FADDS(Sd, Sn, Sm)           EMIT(FADDSUB_scalar(0b00, Sm, 0, Sn, Sd))
#define FADDD(Dd, Dn, Dm)           EMIT(FADDSUB_scalar(0b01, Dm, 0, Dn, Dd))

#define FSUBS(Sd, Sn, Sm)           EMIT(FADDSUB_scalar(0b00, Sm, 1, Sn, Sd))
#define FSUBD(Dd, Dn, Dm)           EMIT(FADDSUB_scalar(0b01, Dm, 1, Dn, Dd))

// ADD Pair
#define ADDP_vector(Q, size, Rm, Rn, Rd)    ((Q)<<30 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VADDPQ_8(Vd, Vn, Vm)        EMIT(ADDP_vector(1, 0b00, Vm, Vn, Vd))
#define VADDPQ_16(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b01, Vm, Vn, Vd))
#define VADDPQ_32(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b10, Vm, Vn, Vd))
#define VADDPQ_64(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b11, Vm, Vn, Vd))
#define VADDP_8(Vd, Vn, Vm)         EMIT(ADDP_vector(0, 0b00, Vm, Vn, Vd))
#define VADDP_16(Vd, Vn, Vm)        EMIT(ADDP_vector(0, 0b01, Vm, Vn, Vd))
#define VADDP_32(Vd, Vn, Vm)        EMIT(ADDP_vector(0, 0b10, Vm, Vn, Vd))

#define FADDP_vector(Q, sz, Rm, Rn, Rd) ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFADDPQS(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 0, Vm, Vn, Vd))
#define VFADDPQD(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 1, Vm, Vn, Vd))

// NEG / ABS
#define FNEGABS_scalar(type, opc, Rn, Rd)  (0b11110<<24 | (type)<<22 | 1<<21 | (opc)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FNEGS(Sd, Sn)               EMIT(FNEGABS_scalar(0b00, 0b10, Sn, Sd))
#define FNEGD(Dd, Dn)               EMIT(FNEGABS_scalar(0b01, 0b10, Dn, Dd))

#define FABSS(Sd, Sn)               EMIT(FNEGABS_scalar(0b00, 0b01, Sn, Sd))
#define FABSD(Dd, Dn)               EMIT(FNEGABS_scalar(0b01, 0b01, Dn, Dd))


// MUL
#define FMUL_vector(Q, sz, Rm, Rn, Rd)  ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFMULS(Sd, Sn, Sm)          EMIT(FMUL_vector(0, 0, Sm, Sn, Sd))
#define VFMULQS(Sd, Sn, Sm)         EMIT(FMUL_vector(1, 0, Sm, Sn, Sd))
#define VFMULQD(Sd, Sn, Sm)         EMIT(FMUL_vector(1, 1, Sm, Sn, Sd))

#define FMUL_scalar(type, Rm, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b10<<10 | (Rn)<<5 | Rd)
#define FMULS(Sd, Sn, Sm)           EMIT(FMUL_scalar(0b00, Sm, Sn, Sd))
#define FMULD(Dd, Dn, Dm)           EMIT(FMUL_scalar(0b01, Dm, Dn, Dd))

// DIV
#define FDIV_vector(Q, sz, Rm, Rn, Rd)  ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFDIVS(Sd, Sn, Sm)          EMIT(FDIV_vector(0, 0, Sm, Sn, Sd))
#define VFDIVQS(Sd, Sn, Sm)         EMIT(FDIV_vector(1, 0, Sm, Sn, Sd))
#define VFDIVQD(Sd, Sn, Sm)         EMIT(FDIV_vector(1, 1, Sm, Sn, Sd))

#define FDIV_scalar(type, Rm, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b0001<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FDIVS(Sd, Sn, Sm)           EMIT(FDIV_scalar(0b00, Sm, Sn, Sd))
#define FDIVD(Dd, Dn, Dm)           EMIT(FDIV_scalar(0b01, Dm, Dn, Dd))

#define FRECPE_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRECPES(Vd, Vn)           EMIT(FRECPE_vector(0, 0, Vn, Vd))
#define VFRECPEQS(Vd, Vn)          EMIT(FRECPE_vector(1, 0, Vn, Vd))
#define VFRECPEQD(Vd, Vn)          EMIT(FRECPE_vector(1, 0, Vn, Vd))

#define FRECPS_vector(Q, sz, Rm, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 0<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFRECPSS(Vd, Vn, Vm)       EMIT(FRECPS_vector(0, 0, Vm, Vn, Vd))
#define VFRECPSQS(Vd, Vn, Vm)      EMIT(FRECPS_vector(1, 0, Vm, Vn, Vd))
#define VFRECPSQD(Vd, Vn, Vm)      EMIT(FRECPS_vector(1, 0, Vm, Vn, Vd))

// SQRT
#define FSQRT_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 1<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFSQRTS(Sd, Sn)             EMIT(FSQRT_vector(0, 0, Sn, Sd))
#define VFSQRTQS(Sd, Sn)            EMIT(FSQRT_vector(1, 0, Sn, Sd))
#define VFSQRTQD(Sd, Sn)            EMIT(FSQRT_vector(1, 1, Sn, Sd))

#define FSQRT_scalar(type, Rn, Rd)      (0b11110<<24 | (type)<<22 | 1<<21 | 0b11<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FSQRTS(Sd, Sn)              EMIT(FSQRT_scalar(0b00, Sn, Sd))
#define FSQRTD(Dd, Dn)              EMIT(FSQRT_scalar(0b01, Dn, Dd))

#define FRSQRTE_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRSQRTES(Vd, Vn)           EMIT(FRSQRTE_vector(0, 0, Vn, Vd))
#define VFRSQRTEQS(Vd, Vn)          EMIT(FRSQRTE_vector(1, 0, Vn, Vd))
#define VFRSQRTEQD(Vd, Vn)          EMIT(FRSQRTE_vector(1, 0, Vn, Vd))

#define FRSQRTS_vector(Q, sz, Rm, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFRSQRTSS(Vd, Vn, Vm)       EMIT(FRSQRTS_vector(0, 0, Vm, Vn, Vd))
#define VFRSQRTSQS(Vd, Vn, Vm)      EMIT(FRSQRTS_vector(1, 0, Vm, Vn, Vd))
#define VFRSQRTSQD(Vd, Vn, Vm)      EMIT(FRSQRTS_vector(1, 0, Vm, Vn, Vd))

// CMP
#define FCMP_scalar(type, Rn, Rm, opc)  (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b1000<<10 | (Rn)<<5 | (opc)<<3)
#define FCMPS(Sn, Sm)               EMIT(FCMP_scalar(0b00, Sn, Sm, 0b00))
#define FCMPD(Dn, Dm)               EMIT(FCMP_scalar(0b01, Dn, Dm, 0b00))
#define FCMPS_0(Sn)                 EMIT(FCMP_scalar(0b00, 0, Sn, 0b01))
#define FCMPD_0(Dn)                 EMIT(FCMP_scalar(0b01, 0, Dn, 0b01))

// CVT
#define FCVT_scalar(sf, type, rmode, opcode, Rn, Rd)    ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (rmode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
// Floating-point Convert to Signed integer, rounding to nearest with ties to Away
#define FCVTASwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b100, Sn, Wd))
#define FCVTASxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b100, Sn, Xd))
#define FCVTASwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b100, Dn, Wd))
#define FCVTASxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b100, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding to nearest with ties to Away
#define FCVTAUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b101, Sn, Wd))
#define FCVTAUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b101, Sn, Xd))
#define FCVTAUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b101, Dn, Wd))
#define FCVTAUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b101, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Minus infinity
#define FCVTMSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b10, 0b100, Sn, Wd))
#define FCVTMSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b10, 0b100, Sn, Xd))
#define FCVTMSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b10, 0b100, Sn, Xd))
#define FCVTMSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b100, Dn, Wd))
#define FCVTMSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b100, Dn, Xd))
#define FCVTMSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b10, 0b100, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Minus infinity
#define FCVTMUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b10, 0b101, Sn, Wd))
#define FCVTMUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b10, 0b101, Sn, Xd))
#define FCVTMUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b10, 0b101, Sn, Xd))
#define FCVTMUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b101, Dn, Wd))
#define FCVTMUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b101, Dn, Xd))
#define FCVTMUxwD(Xd, Dn)           EMIT(FCVT_scalar(rfex.w, 0b01, 0b10, 0b101, Dn, Xd))
// Floating-point Convert to Signed integer, rounding to nearest with ties to even
#define FCVTNSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b000, Sn, Wd))
#define FCVTNSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b000, Sn, Xd))
#define FCVTNSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b00, 0b000, Sn, Xd))
#define FCVTNSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b000, Dn, Wd))
#define FCVTNSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b000, Dn, Xd))
#define FCVTNSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b00, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding to nearest with ties to even
#define FCVTNUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b001, Sn, Wd))
#define FCVTNUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b001, Sn, Xd))
#define FCVTNUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b00, 0b001, Sn, Xd))
#define FCVTNUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b001, Dn, Wd))
#define FCVTNUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b001, Dn, Xd))
#define FCVTNUxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b00, 0b001, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Plus infinity
#define FCVTPSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b01, 0b000, Sn, Wd))
#define FCVTPSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b01, 0b000, Sn, Xd))
#define FCVTPSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b01, 0b000, Sn, Xd))
#define FCVTPSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b01, 0b000, Dn, Wd))
#define FCVTPSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b01, 0b000, Dn, Xd))
#define FCVTPSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b01, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Plus infinity
#define FCVTPUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b01, 0b001, Sn, Wd))
#define FCVTPUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b01, 0b001, Sn, Xd))
#define FCVTPUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b01, 0b001, Dn, Wd))
#define FCVTPUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b01, 0b001, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Zero
#define FCVTZSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b11, 0b000, Sn, Wd))
#define FCVTZSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b11, 0b000, Sn, Xd))
#define FCVTZSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b11, 0b000, Sn, Xd))
#define FCVTZSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b11, 0b000, Dn, Wd))
#define FCVTZSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b11, 0b000, Dn, Xd))
#define FCVTZSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b11, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Zero
#define FCVTZUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b11, 0b001, Sn, Wd))
#define FCVTZUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b11, 0b001, Sn, Xd))
#define FCVTZUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b11, 0b001, Sn, Xd))
#define FCVTZUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b11, 0b001, Dn, Wd))
#define FCVTZUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b11, 0b001, Dn, Xd))
#define FCVTZUxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b11, 0b001, Dn, Xd))

#define FCVT_vector_scalar(U, o2, sz, o1, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1110<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to Away
#define VFCVTASs(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 0, 0, Vn, Vd))
#define VFCVTASd(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 1, 0, Vn, Vd))
#define VFCVTAUs(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 0, 0, Vn, Vd))
#define VFCVTAUd(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Minus infinity
#define VFCVTMSs(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 0, 1, Vn, Vd))
#define VFCVTMSd(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 1, 1, Vn, Vd))
#define VFCVTMUs(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 0, 1, Vn, Vd))
#define VFCVTMUd(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 1, 1, Vn, Vd))

#define FCVT2_vector_scalar(U, o2, sz, o1, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1101<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to even
#define VFCVTNSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 0, 0, 0, Vn, Vd))
#define VFCVTNSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 0, 1, 0, Vn, Vd))
#define VFCVTNUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 0, 0, 0, Vn, Vd))
#define VFCVTNUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Plus infinity
#define VFCVTPSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 0, 0, Vn, Vd))
#define VFCVTPSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 1, 0, Vn, Vd))
#define VFCVTPUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 0, 0, Vn, Vd))
#define VFCVTPUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Zero
#define VFCVTZSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 0, 1, Vn, Vd))
#define VFCVTZSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 1, 1, Vn, Vd))
#define VFCVTZUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 0, 1, Vn, Vd))
#define VFCVTZUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 1, 1, Vn, Vd))

#define FCVT_vector(Q, U, o2, sz, o1, Rn, Rd)       ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1110<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to Away
#define VFCVTASS(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 0, 0, Vn, Vd))
#define VFCVTASD(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 1, 0, Vn, Vd))
#define VFCVTASQS(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 0, 0, Vn, Vd))
#define VFCVTASQD(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 1, 0, Vn, Vd))
#define VFCVTAUS(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 0, 0, Vn, Vd))
#define VFCVTAUD(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 1, 0, Vn, Vd))
#define VFCVTAUQS(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 0, 0, Vn, Vd))
#define VFCVTAUQD(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Minus infinity
#define VFCVTMSS(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 0, 1, Vn, Vd))
#define VFCVTMSD(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 1, 1, Vn, Vd))
#define VFCVTMSQS(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 0, 1, Vn, Vd))
#define VFCVTMSQD(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 1, 1, Vn, Vd))
#define VFCVTMUS(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 0, 1, Vn, Vd))
#define VFCVTMUD(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 1, 1, Vn, Vd))
#define VFCVTMUQS(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 0, 1, Vn, Vd))
#define VFCVTMUQD(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 1, 1, Vn, Vd))

#define FCVT2_vector(Q, U, o2, sz, o1, Rn, Rd)       ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1101<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to even
#define VFCVTNSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 0, 0, 0, Vn, Vd))
#define VFCVTNSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 0, 1, 0, Vn, Vd))
#define VFCVTNSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 0, 0, 0, Vn, Vd))
#define VFCVTNSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 0, 1, 0, Vn, Vd))
#define VFCVTNUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 0, 0, 0, Vn, Vd))
#define VFCVTNUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 0, 1, 0, Vn, Vd))
#define VFCVTNUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 0, 0, 0, Vn, Vd))
#define VFCVTNUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Plus infinity
#define VFCVTPSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 0, 0, Vn, Vd))
#define VFCVTPSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 1, 0, Vn, Vd))
#define VFCVTPSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 0, 0, Vn, Vd))
#define VFCVTPSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 1, 0, Vn, Vd))
#define VFCVTPUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 0, 0, Vn, Vd))
#define VFCVTPUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 1, 0, Vn, Vd))
#define VFCVTPUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 0, 0, Vn, Vd))
#define VFCVTPUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Zero
#define VFCVTZSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 0, 1, Vn, Vd))
#define VFCVTZSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 1, 1, Vn, Vd))
#define VFCVTZSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 0, 1, Vn, Vd))
#define VFCVTZSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 1, 1, Vn, Vd))
#define VFCVTZUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 0, 1, Vn, Vd))
#define VFCVTZUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 1, 1, Vn, Vd))
#define VFCVTZUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 0, 1, Vn, Vd))
#define VFCVTZUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 1, 1, Vn, Vd))

#define FCVT_precision(type, opc, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | 0b0001<<17 | (opc)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FCVT_D_S(Dd, Sn)            EMIT(FCVT_precision(0b00, 0b01, Sn, Dd))
#define FCVT_S_D(Sd, Dn)            EMIT(FCVT_precision(0b01, 0b00, Dn, Sd))

#define FCVTXN_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10110<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert Vn from 2*Double to lower Vd as 2*float and clears the upper half
#define FCVTXN(Vd, Vn)              EMIT(FCVTXN_vector(0, 1, Vn, Vd))
// Convert Vn from 2*Double to higher Vd as 2*float
#define FCVTXN2(Vd, Vn)             EMIT(FCVTXN_vector(1, 1, Vn, Vd))

#define FCVTL_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 0<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert lower Vn from 2*float to Vd as 2*double
#define FCVTL(Vd, Vn)               EMIT(FCVTL_vector(0, 1, Vn, Vd))
// Convert higher Vn from 2*float to Vd as 2*double
#define FCVTL2(Vd, Vn)              EMIT(FCVTL_vector(1, 1, Vn, Vd))

#define SCVTF_scalar(sf, type, rmode, opcode, Rn, Rd)   ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (rmode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
#define SCVTFSw(Sd, Wn)             EMIT(SCVTF_scalar(0, 0b00, 0b00, 0b010, Wn, Sd))
#define SCVTFDw(Dd, Wn)             EMIT(SCVTF_scalar(0, 0b01, 0b00, 0b010, Wn, Dd))
#define SCVTFSx(Sd, Xn)             EMIT(SCVTF_scalar(1, 0b00, 0b00, 0b010, Xn, Sd))
#define SCVTFDx(Dd, Xn)             EMIT(SCVTF_scalar(1, 0b01, 0b00, 0b010, Xn, Dd))

#define SCVTF_vector_scalar(U, sz, Rn, Rd)    (1<<30 | (U)<<29 | 0b11110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SCVTFSS(Vd, Vn)             EMIT(SCVTF_vector_scalar(0, 0, Vn, Vd))
#define SCVTFDD(Vd, Vn)             EMIT(SCVTF_vector_scalar(0, 1, Vn, Vd))

#define SCVTF_vector(Q, U, sz, Rn, Rd)      ((Q)<<30 | (U)<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SCVTFS(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 0, Vn, Vd))
#define SCVTFD(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 1, Vn, Vd))
#define SCVTQFS(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 0, Vn, Vd))
#define SCVTQFD(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 1, Vn, Vd))

// FRINTI Floating-point Round to Integral, using current rounding mode from FPCR (vector).
#define FRINT_vector(Q, U, o2, sz, o1, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1100<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRINTIS(Vd,Vn)             EMIT(FRINT_vector(0, 1, 1, 0, 1, Vn, Vd))
#define VFRINTISQ(Vd,Vn)            EMIT(FRINT_vector(1, 1, 1, 0, 1, Vn, Vd))
#define VFRINTIDQ(Vd,Vn)            EMIT(FRINT_vector(1, 1, 1, 1, 1, Vn, Vd))

#define FRINTxx_scalar(type, op, Rn, Rd)  (0b11110<<24 | (type)<<22 | 1<<21 | 0b0100<<17 | (op)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FRINT32ZS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b00, Sn, Sd))
#define FRINT32ZD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b00, Dn, Dd))
#define FRINT32XS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b01, Sn, Sd))
#define FRINT32XD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b01, Dn, Dd))
#define FRINT64ZS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b10, Sn, Sd))
#define FRINT64ZD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b10, Dn, Dd))
#define FRINT64XS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b11, Sn, Sd))
#define FRINT64XD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b11, Dn, Dd))

#define FRINT_scalar(type, rmode, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | 0b001<<18 | (rmode)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
// round toward 0 (truncate)
#define FRINTZS(Sd, Sn)             EMIT(FRINT_scalar(0b00, 0b011, Sn, Sd))
// round toward 0 (truncate)
#define FRINTZD(Sd, Sn)             EMIT(FRINT_scalar(0b01, 0b011, Sn, Sd))
// round with current FPCR mode
#define FRINTXS(Sd, Sn)             EMIT(FRINT_scalar(0b00, 0b110, Sn, Sd))
// round with current FPCR mode
#define FRINTXD(Sd, Sn)             EMIT(FRINT_scalar(0b01, 0b110, Sn, Sd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define FRINTRRS(Sd, Sn, mode)      EMIT(FRINT_scalar(0b00, ((mode)&3), Sn, Sd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define FRINTRRD(Dd, Dn, mode)      EMIT(FRINT_scalar(0b01, ((mode)&3), Dn, Dd))

// FMAX / FMIN
#define FMINMAX_vector(Q, U, o1, sz, Rm, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o1)<<23 | (sz)<<22 | 0b1<<21 | (Rm)<<16 | 0b11110<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFMINS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 1, Vm, Vn, Vd))
#define VFMAXQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 1, Vm, Vn, Vd))

#define FMINMAX_scalar(type, Rm, op, Rn, Rd)        (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b01<<14 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FMINS(Sd, Sn, Sm)           EMIT(FMINMAX_scalar(0b00, Sm, 0b01, Sn, Sd))
#define FMIND(Dd, Dn, Dm)           EMIT(FMINMAX_scalar(0b01, Dm, 0b01, Dn, Dd))
#define FMAXS(Sd, Sn, Sm)           EMIT(FMINMAX_scalar(0b00, Sm, 0b00, Sn, Sd))
#define FMAXD(Dd, Dn, Dm)           EMIT(FMINMAX_scalar(0b01, Dm, 0b00, Dn, Dd))
// FMINNM NaN vs Number: number is picked
#define FMINNMS(Sd, Sn, Sm)         EMIT(FMINMAX_scalar(0b00, Sm, 0b11, Sn, Sd))
// FMINNM NaN vs Number: number is picked
#define FMINNMD(Dd, Dn, Dm)         EMIT(FMINMAX_scalar(0b01, Dm, 0b11, Dn, Dd))
// FMAXNM NaN vs Number: number is picked
#define FMAXNMS(Sd, Sn, Sm)         EMIT(FMINMAX_scalar(0b00, Sm, 0b10, Sn, Sd))
// FMAXNM NaN vs Number: number is picked
#define FMAXNMD(Dd, Dn, Dm)         EMIT(FMINMAX_scalar(0b01, Dm, 0b10, Dn, Dd))

// ZIP / UZP
#define ZIP_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b11<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VZIP1Q_8(Rt, Rn, Rm)        EMIT(ZIP_gen(1, 0b00, Rm, 0, Rn, Rt))
#define VZIP2Q_8(Rt, Rn, Rm)        EMIT(ZIP_gen(1, 0b00, Rm, 1, Rn, Rt))
#define VZIP1_8(Rt, Rn, Rm)         EMIT(ZIP_gen(0, 0b00, Rm, 0, Rn, Rt))
#define VZIP2_8(Rt, Rn, Rm)         EMIT(ZIP_gen(0, 0b00, Rm, 1, Rn, Rt))
#define VZIP1Q_16(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b01, Rm, 0, Rn, Rt))
#define VZIP2Q_16(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b01, Rm, 1, Rn, Rt))
#define VZIP1_16(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b01, Rm, 0, Rn, Rt))
#define VZIP2_16(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b01, Rm, 1, Rn, Rt))
#define VZIP1Q_32(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b10, Rm, 0, Rn, Rt))
#define VZIP2Q_32(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b10, Rm, 1, Rn, Rt))
#define VZIP1_32(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b10, Rm, 0, Rn, Rt))
#define VZIP2_32(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b10, Rm, 1, Rn, Rt))
#define VZIP1Q_64(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b11, Rm, 0, Rn, Rt))
#define VZIP2Q_64(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b11, Rm, 1, Rn, Rt))

#define UZP_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b01<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VUZP1Q_8(Rt, Rn, Rm)        EMIT(UZP_gen(1, 0b00, Rm, 0, Rn, Rt))
#define VUZP2Q_8(Rt, Rn, Rm)        EMIT(UZP_gen(1, 0b00, Rm, 1, Rn, Rt))
#define VUZP1_8(Rt, Rn, Rm)         EMIT(UZP_gen(0, 0b00, Rm, 0, Rn, Rt))
#define VUZP2_8(Rt, Rn, Rm)         EMIT(UZP_gen(0, 0b00, Rm, 1, Rn, Rt))
#define VUZP1Q_16(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b01, Rm, 0, Rn, Rt))
#define VUZP2Q_16(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b01, Rm, 1, Rn, Rt))
#define VUZP1_16(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b01, Rm, 0, Rn, Rt))
#define VUZP2_16(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b01, Rm, 1, Rn, Rt))
#define VUZP1Q_32(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b10, Rm, 0, Rn, Rt))
#define VUZP2Q_32(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b10, Rm, 1, Rn, Rt))
#define VUZP1_32(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b10, Rm, 0, Rn, Rt))
#define VUZP2_32(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b10, Rm, 1, Rn, Rt))
#define VUZP1Q_64(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b11, Rm, 0, Rn, Rt))
#define VUZP2Q_64(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b11, Rm, 1, Rn, Rt))

// TBL
#define TBL_gen(Q, Rm, len, op, Rn, Rd) ((Q)<<30 | 0b001110<<24 | (Rm)<<16 | (len)<<13 | (op)<<12 | (Rn)<<5 | (Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element gets 0
#define VTBLQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 0, Rn, Rd))
#define VTBL1_8(Rd, Rn, Rm)         EMIT(TBL_gen(0, Rm, 0b00, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element gets 0
#define VTBLQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element gets 0
#define VTBLQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element gets 0
#define VTBLQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 0, Rn, Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element stay untouched
#define VTBXQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 0, Rn, Rd))

// TRN
#define TRN_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b10<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VTRNQ1_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 0, Vn, Vd))
#define VTRNQ1_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 0, Vn, Vd))
#define VTRNQ1_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 0, Vn, Vd))
#define VTRNQ1_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 0, Vn, Vd))
#define VSWP(Vd, Vn)                VTRNQ1_64(Vd, Vn, Vn)
#define VTRNQ2_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 1, Vn, Vd))
#define VTRNQ2_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 1, Vn, Vd))
#define VTRNQ2_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 1, Vn, Vd))
#define VTRNQ2_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 1, Vn, Vd))

// QXTN / QXTN2
#define QXTN_scalar(U, size, Rn, Rd)        (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 0b10000<<17 | 0b10100<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Narrow, from D to S
#define SQXTN_S_D(Sd, Dn)           EMIT(QXTN_scalar(0, 0b10, Dn, Sd))
// Signed saturating extract Narrow, from S to H
#define SQXTN_H_S(Hd, Sn)           EMIT(QXTN_scalar(0, 0b01, Sn, Hd))
// Signed saturating extract Narrow, from H to B
#define SQXTN_B_H(Bd, Hn)           EMIT(QXTN_scalar(0, 0b00, Hn, Bd))

#define QXTN_vector(Q, U, size, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b10100<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 64->32 with Signed saturation and fit lower part of Rd
#define SQXTN_32(Rd, Rn)            EMIT(QXTN_vector(0, 0, 0b10, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 64->32 with Signed saturation and fit higher part of Rd
#define SQXTN2_32(Rd, Rn)           EMIT(QXTN_vector(1, 0, 0b10, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 32->16 with Signed saturation and fit lower part of Rd
#define SQXTN_16(Rd, Rn)            EMIT(QXTN_vector(0, 0, 0b01, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 32->16 with Signed saturation and fit higher part of Rd
#define SQXTN2_16(Rd, Rn)           EMIT(QXTN_vector(1, 0, 0b01, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 16->8 with Signed saturation and fit lower part of Rd
#define SQXTN_8(Rd, Rn)             EMIT(QXTN_vector(0, 0, 0b00, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 16->8 with Signed saturation and fit higher part of Rd
#define SQXTN2_8(Rd, Rn)            EMIT(QXTN_vector(1, 0, 0b00, Rn, Rd))
// Unsigned saturating Extract Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit lower part of Rd
#define UQXTN_32(Rd, Rn)            EMIT(QXTN_vector(0, 1, 0b10, Rn, Rd))
// Unsigned saturating Extract Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_32(Rd, Rn)           EMIT(QXTN_vector(1, 1, 0b10, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit lower part of Rd
#define UQXTN_16(Rd, Rn)            EMIT(QXTN_vector(0, 1, 0b01, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_16(Rd, Rn)           EMIT(QXTN_vector(1, 1, 0b01, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit lower part of Rd
#define UQXTN_8(Rd, Rn)             EMIT(QXTN_vector(0, 1, 0b00, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_8(Rd, Rn)            EMIT(QXTN_vector(1, 1, 0b00, Rn, Rd))

#define QXTUN_vector(Q, U, size, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b10010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_32(Rd, Rn)           EMIT(QXTUN_vector(0, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_32(Rd, Rn)          EMIT(QXTUN_vector(1, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_16(Rd, Rn)           EMIT(QXTUN_vector(0, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_16(Rd, Rn)          EMIT(QXTUN_vector(1, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_8(Rd, Rn)            EMIT(QXTUN_vector(0, 1, 0b00, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_8(Rd, Rn)           EMIT(QXTUN_vector(1, 1, 0b00, Rn, Rd))

// Integer CMP
// EQual
#define CMEQ_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10001<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VCMEQ_8(Rd, Rn, Rm)         EMIT(CMEQ_vector(0, 1, 0b00, Rm, Rn, Rd))
#define VCMEQ_16(Rd, Rn, Rm)        EMIT(CMEQ_vector(0, 1, 0b01, Rm, Rn, Rd))
#define VCMEQ_32(Rd, Rn, Rm)        EMIT(CMEQ_vector(0, 1, 0b10, Rm, Rn, Rd))
#define VCMEQQ_8(Rd, Rn, Rm)        EMIT(CMEQ_vector(1, 1, 0b00, Rm, Rn, Rd))
#define VCMEQQ_16(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b01, Rm, Rn, Rd))
#define VCMEQQ_32(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b10, Rm, Rn, Rd))
#define VCMEQQ_64(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b11, Rm, Rn, Rd))
// Greater test
#define CMG_vector(Q, U, size, eq, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0011<<12 | (eq)<<11 | 1<<10 | (Rn)<<5 | (Rd))
// Signed Greater or Equal
#define VCMGEQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 0, 0b00, 1, Rm, Rn, Rd))
#define VCMGEQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b01, 1, Rm, Rn, Rd))
#define VCMGEQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b10, 1, Rm, Rn, Rd))
#define VCMGEQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b11, 1, Rm, Rn, Rd))
// Unsigned Higher or Same
#define VCMHSQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 1, 0b00, 1, Rm, Rn, Rd))
#define VCMHSQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b01, 1, Rm, Rn, Rd))
#define VCMHSQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b10, 1, Rm, Rn, Rd))
#define VCMHSQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b11, 1, Rm, Rn, Rd))
// Signed Greater Than
#define VCMGTQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 0, 0b00, 0, Rm, Rn, Rd))
#define VCMGTQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b01, 0, Rm, Rn, Rd))
#define VCMGTQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b10, 0, Rm, Rn, Rd))
#define VCMGTQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b11, 0, Rm, Rn, Rd))
#define VCMGT_8(Rd, Rn, Rm)         EMIT(CMG_vector(0, 0, 0b00, 0, Rm, Rn, Rd))
#define VCMGT_16(Rd, Rn, Rm)        EMIT(CMG_vector(0, 0, 0b01, 0, Rm, Rn, Rd))
#define VCMGT_32(Rd, Rn, Rm)        EMIT(CMG_vector(0, 0, 0b10, 0, Rm, Rn, Rd))
// Unsigned Higher
#define VCHIQQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 1, 0b00, 0, Rm, Rn, Rd))
#define VCHIQQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b01, 0, Rm, Rn, Rd))
#define VCHIQQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b10, 0, Rm, Rn, Rd))
#define VCHIQQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b11, 0, Rm, Rn, Rd))

// Vector Float CMP
// EQual
#define FCMP_vector(Q, U, E, sz, Rm, ac, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (E)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b1110<<12 | (ac)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define FCMEQQD(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 0, 0, 1, Rm, 0, Rn, Rd))
#define FCMEQQS(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 0, 0, 0, Rm, 0, Rn, Rd))
// Greater or Equal
#define FCMGEQD(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 1, 0, 1, Rm, 0, Rn, Rd))
#define FCMGEQS(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 1, 0, 0, Rm, 0, Rn, Rd))
#define FCMGEQD_ABS(Rd, Rn, Rm)      EMIT(FCMP_vector(1, 1, 0, 1, Rm, 1, Rn, Rd))
#define FCMGEQS_ABS(Rd, Rn, Rm)      EMIT(FCMP_vector(1, 1, 0, 0, Rm, 1, Rn, Rd))
// Greater Than
#define FCMGTQD(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 1, 1, 1, Rm, 0, Rn, Rd))
#define FCMGTQS(Rd, Rn, Rm)          EMIT(FCMP_vector(1, 1, 1, 0, Rm, 0, Rn, Rd))
#define FCMGTQD_ABS(Rd, Rn, Rm)      EMIT(FCMP_vector(1, 1, 1, 1, Rm, 1, Rn, Rd))
#define FCMGTQS_ABS(Rd, Rn, Rm)      EMIT(FCMP_vector(1, 1, 1, 0, Rm, 1, Rn, Rd))

// UMULL / SMULL
#define MULL_vector(Q, U, size, Rm, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b1100<<12 |(Rn)<<5 |(Rd))
#define VUMULL_8(Rd, Rn, Rm)        EMIT(MULL_vector(0, 1, 0b00, Rm, Rn, Rd))
#define VUMULL_16(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b01, Rm, Rn, Rd))
#define VUMULL_32(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b10, Rm, Rn, Rd))
#define VUMULL2_8(Rd, Rn, Rm)       EMIT(MULL_vector(1, 1, 0b00, Rm, Rn, Rd))
#define VUMULL2_16(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b01, Rm, Rn, Rd))
#define VUMULL2_32(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b10, Rm, Rn, Rd))
#define VSMULL_8(Rd, Rn, Rm)        EMIT(MULL_vector(0, 0, 0b00, Rm, Rn, Rd))
#define VSMULL_16(Rd, Rn, Rm)       EMIT(MULL_vector(0, 0, 0b01, Rm, Rn, Rd))
#define VSMULL_32(Rd, Rn, Rm)       EMIT(MULL_vector(0, 0, 0b10, Rm, Rn, Rd))
#define VSMULL2_8(Rd, Rn, Rm)       EMIT(MULL_vector(1, 0, 0b00, Rm, Rn, Rd))
#define VSMULL2_16(Rd, Rn, Rm)      EMIT(MULL_vector(1, 0, 0b01, Rm, Rn, Rd))
#define VSMULL2_32(Rd, Rn, Rm)      EMIT(MULL_vector(1, 0, 0b10, Rm, Rn, Rd))

// MUL
#define MUL_vector(Q, size, Rm, Rn, Rd)     ((Q)<<30 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMUL_8(Vd, Vn, Vm)          EMIT(MUL_vector(0, 0b00, Vm, Vn, Vd))
#define VMUL_16(Vd, Vn, Vm)         EMIT(MUL_vector(0, 0b01, Vm, Vn, Vd))
#define VMUL_32(Vd, Vn, Vm)         EMIT(MUL_vector(0, 0b10, Vm, Vn, Vd))
#define VMULQ_8(Vd, Vn, Vm)         EMIT(MUL_vector(1, 0b00, Vm, Vn, Vd))
#define VMULQ_16(Vd, Vn, Vm)        EMIT(MUL_vector(1, 0b01, Vm, Vn, Vd))
#define VMULQ_32(Vd, Vn, Vm)        EMIT(MUL_vector(1, 0b10, Vm, Vn, Vd))

// (S/Q)ADD
#define QADD_vector(Q, U, size, Rm, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00001<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SQADDQ_8(Vd, Vn, Vm)        EMIT(QADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQADDQ_16(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQADDQ_32(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define SQADDQ_64(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b11, Vm, Vn, Vd))
#define UQADDQ_8(Vd, Vn, Vm)        EMIT(QADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UQADDQ_16(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UQADDQ_32(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b10, Vm, Vn, Vd))
#define UQADDQ_64(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b11, Vm, Vn, Vd))
#define SQADD_8(Vd, Vn, Vm)         EMIT(QADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQADD_16(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQADD_32(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQADD_64(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b11, Vm, Vn, Vd))
#define UQADD_8(Vd, Vn, Vm)         EMIT(QADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UQADD_16(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UQADD_32(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UQADD_64(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b11, Vm, Vn, Vd))

// Absolute Difference
#define AD_vector(Q, U, size, Rm, ac, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0111<<12 | (ac)<<11 | 1<<10 | (Rn)<<5 | (Rd))
// Signed Absolute Difference and accumulate
#define SABAQ_8(Rd, Rn, Rm)         EMIT(AD_vector(1, 0, 0b00, Rm, 1, Rn, Rd))
#define SABAQ_16(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b01, Rm, 1, Rn, Rd))
#define SABAQ_32(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b10, Rm, 1, Rn, Rd))
#define SABA_8(Rd, Rn, Rm)          EMIT(AD_vector(0, 0, 0b00, Rm, 1, Rn, Rd))
#define SABA_16(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b01, Rm, 1, Rn, Rd))
#define SABA_32(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b10, Rm, 1, Rn, Rd))
// Signed Absolute Difference
#define SABDQ_8(Rd, Rn, Rm)         EMIT(AD_vector(1, 0, 0b00, Rm, 0, Rn, Rd))
#define SABDQ_16(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b01, Rm, 0, Rn, Rd))
#define SABDQ_32(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b10, Rm, 0, Rn, Rd))
#define SABD_8(Rd, Rn, Rm)          EMIT(AD_vector(0, 0, 0b00, Rm, 0, Rn, Rd))
#define SABD_16(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b01, Rm, 0, Rn, Rd))
#define SABD_32(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b10, Rm, 0, Rn, Rd))

#define ADL_vector(Q, U, size, Rm, op, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b01<<14 | (op)<<13 | 1<<12 | (Rn)<<5 | (Rd))
#define SABAL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 0, 0b00, Rm, 0, Rn, Rd))
#define SABAL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 0, 0b00, Rm, 0, Rn, Rd))
#define SABAL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b01, Rm, 0, Rn, Rd))
#define SABAL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b01, Rm, 0, Rn, Rd))
#define SABAL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b10, Rm, 0, Rn, Rd))
#define SABAL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b10, Rm, 0, Rn, Rd))
#define UABAL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 1, 0b00, Rm, 0, Rn, Rd))
#define UABAL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 1, 0b00, Rm, 0, Rn, Rd))
#define UABAL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b01, Rm, 0, Rn, Rd))
#define UABAL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b01, Rm, 0, Rn, Rd))
#define UABAL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b10, Rm, 0, Rn, Rd))
#define UABAL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b10, Rm, 0, Rn, Rd))
#define SABDL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 0, 0b00, Rm, 1, Rn, Rd))
#define SABDL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 0, 0b00, Rm, 1, Rn, Rd))
#define SABDL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b01, Rm, 1, Rn, Rd))
#define SABDL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b01, Rm, 1, Rn, Rd))
#define SABDL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b10, Rm, 1, Rn, Rd))
#define SABDL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b10, Rm, 1, Rn, Rd))
#define UABDL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 1, 0b00, Rm, 1, Rn, Rd))
#define UABDL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 1, 0b00, Rm, 1, Rn, Rd))
#define UABDL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b01, Rm, 1, Rn, Rd))
#define UABDL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b01, Rm, 1, Rn, Rd))
#define UABDL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b10, Rm, 1, Rn, Rd))
#define UABDL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b10, Rm, 1, Rn, Rd))

// Add Pairwise
#define ADDLP_vector(Q, U, size, op, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (op)<<14 | 0b10<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SADDLPQ_8(Rd, Rn)           EMIT(ADDLP_vector(1, 0, 0b00, 0, Rn, Rd))
#define SADDLPQ_16(Rd, Rn)          EMIT(ADDLP_vector(1, 0, 0b01, 0, Rn, Rd))
#define SADDLPQ_32(Rd, Rn)          EMIT(ADDLP_vector(1, 0, 0b10, 0, Rn, Rd))
#define SADDLP_8(Rd, Rn)            EMIT(ADDLP_vector(0, 0, 0b00, 0, Rn, Rd))
#define SADDLP_16(Rd, Rn)           EMIT(ADDLP_vector(0, 0, 0b01, 0, Rn, Rd))
#define SADDLP_32(Rd, Rn)           EMIT(ADDLP_vector(0, 0, 0b10, 0, Rn, Rd))
#define UADDLPQ_8(Rd, Rn)           EMIT(ADDLP_vector(1, 1, 0b00, 0, Rn, Rd))
#define UADDLPQ_16(Rd, Rn)          EMIT(ADDLP_vector(1, 1, 0b01, 0, Rn, Rd))
#define UADDLPQ_32(Rd, Rn)          EMIT(ADDLP_vector(1, 1, 0b10, 0, Rn, Rd))
#define UADDLP_8(Rd, Rn)            EMIT(ADDLP_vector(0, 1, 0b00, 0, Rn, Rd))
#define UADDLP_16(Rd, Rn)           EMIT(ADDLP_vector(0, 1, 0b01, 0, Rn, Rd))
#define UADDLP_32(Rd, Rn)           EMIT(ADDLP_vector(0, 1, 0b10, 0, Rn, Rd))

// Add accros vector
#define ADDLV_vector(Q, U, size, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b11000<<17 | 0b00011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SADDLVQ_8(Rd, Rn)           EMIT(ADDLV_vector(1, 0, 0b00, Rn, Rd))
#define SADDLVQ_16(Rd, Rn)          EMIT(ADDLV_vector(1, 0, 0b01, Rn, Rd))
#define SADDLVQ_32(Rd, Rn)          EMIT(ADDLV_vector(1, 0, 0b10, Rn, Rd))
#define SADDLV_8(Rd, Rn)            EMIT(ADDLV_vector(0, 0, 0b00, Rn, Rd))
#define SADDLV_16(Rd, Rn)           EMIT(ADDLV_vector(0, 0, 0b01, Rn, Rd))
#define SADDLV_32(Rd, Rn)           EMIT(ADDLV_vector(0, 0, 0b10, Rn, Rd))
#define UADDLVQ_8(Rd, Rn)           EMIT(ADDLV_vector(1, 1, 0b00, Rn, Rd))
#define UADDLVQ_16(Rd, Rn)          EMIT(ADDLV_vector(1, 1, 0b01, Rn, Rd))
#define UADDLVQ_32(Rd, Rn)          EMIT(ADDLV_vector(1, 1, 0b10, Rn, Rd))
#define UADDLV_8(Rd, Rn)            EMIT(ADDLV_vector(0, 1, 0b00, Rn, Rd))
#define UADDLV_16(Rd, Rn)           EMIT(ADDLV_vector(0, 1, 0b01, Rn, Rd))
#define UADDLV_32(Rd, Rn)           EMIT(ADDLV_vector(0, 1, 0b10, Rn, Rd))

// MOV Immediate
#define MOVI_vector(Q, op, abc, cmode, defgh, Rd)   ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | (cmode)<<12 | 1<<10 | (defgh)<<5 | (Rd))
#define MOVIQ_8(Rd, imm8)           EMIT(MOVI_vector(1, 0, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))
#define MOVI_8(Rd, imm8)            EMIT(MOVI_vector(0, 0, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))

// SHLL and eXtend Long
#define SHLL_vector(Q, U, immh, immb, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b10100<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define USHLL2_8(Vd, Vn, imm)       EMIT(SHLL_vector(1, 1, 0b0001, (imm)&0x7, Vn, Vd))
#define USHLL_8(Vd, Vn, imm)        EMIT(SHLL_vector(0, 1, 0b0001, (imm)&0x7, Vn, Vd))
#define SSHLL2_8(Vd, Vn, imm)       EMIT(SHLL_vector(1, 0, 0b0001, (imm)&0x7, Vn, Vd))
#define SSHLL_8(Vd, Vn, imm)        EMIT(SHLL_vector(0, 0, 0b0001, (imm)&0x7, Vn, Vd))
#define USHLL2_16(Vd, Vn, imm)      EMIT(SHLL_vector(1, 1, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define USHLL_16(Vd, Vn, imm)       EMIT(SHLL_vector(0, 1, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define SSHLL2_16(Vd, Vn, imm)      EMIT(SHLL_vector(1, 0, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define SSHLL_16(Vd, Vn, imm)       EMIT(SHLL_vector(0, 0, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define USHLL2_32(Vd, Vn, imm)      EMIT(SHLL_vector(1, 1, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define USHLL_32(Vd, Vn, imm)       EMIT(SHLL_vector(0, 1, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define SSHLL2_32(Vd, Vn, imm)      EMIT(SHLL_vector(1, 0, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define SSHLL_32(Vd, Vn, imm)       EMIT(SHLL_vector(0, 0, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))

#define UXTL_8(Vd, Vn)              USHLL_8(Vd, Vn, 0)
#define UXTL2_8(Vd, Vn)             USHLL2_8(Vd, Vn, 0)
#define UXTL_16(Vd, Vn)             USHLL_16(Vd, Vn, 0)
#define UXTL2_16(Vd, Vn)            USHLL2_16(Vd, Vn, 0)
#define UXTL_32(Vd, Vn)             USHLL_32(Vd, Vn, 0)
#define UXTL2_32(Vd, Vn)            USHLL2_32(Vd, Vn, 0)

#define SXTL_8(Vd, Vn)              SSHLL_8(Vd, Vn, 0)
#define SXTL2_8(Vd, Vn)             SSHLL2_8(Vd, Vn, 0)
#define SXTL_16(Vd, Vn)             SSHLL_16(Vd, Vn, 0)
#define SXTL2_16(Vd, Vn)            SSHLL2_16(Vd, Vn, 0)
#define SXTL_32(Vd, Vn)             SSHLL_32(Vd, Vn, 0)
#define SXTL2_32(Vd, Vn)            SSHLL2_32(Vd, Vn, 0)

// SHRN
#define QSHRN_vector(Q, U, immh, immb, op, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b1001<<12 | (op)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define UQSHRN_8(Vd, Vn, imm)       EMIT(QSHRN_vector(0, 1, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_8(Vd, Vn, imm)      EMIT(QSHRN_vector(1, 1, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_8(Vd, Vn, imm)       EMIT(QSHRN_vector(0, 0, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_8(Vd, Vn, imm)      EMIT(QSHRN_vector(1, 0, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN_16(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 1, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_16(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 1, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_16(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 0, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_16(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 0, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN_32(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 1, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_32(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 1, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_32(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 0, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_32(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 0, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))

// UQSUB
#define QSUB_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00101<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define UQSUB_8(Vd, Vn, Vm)         EMIT(QSUB_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UQSUB_16(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UQSUB_32(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UQSUB_64(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b11, Vm, Vn, Vd))
#define SQSUB_8(Vd, Vn, Vm)         EMIT(QSUB_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQSUB_16(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQSUB_32(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQSUB_64(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b11, Vm, Vn, Vd))
#define UQSUBQ_8(Vd, Vn, Vm)        EMIT(QSUB_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UQSUBQ_16(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UQSUBQ_32(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b10, Vm, Vn, Vd))
#define UQSUBQ_64(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b11, Vm, Vn, Vd))
#define SQSUBQ_8(Vd, Vn, Vm)        EMIT(QSUB_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQSUBQ_16(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQSUBQ_32(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b10, Vm, Vn, Vd))
#define SQSUBQ_64(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b11, Vm, Vn, Vd))

// MAX/MIN vector
#define MINMAX_vector(Q, U, size, Rm, op, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0110<<12 | (op)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SMAX_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 0, 0b00, Vm, 0, Vn, Vd))
#define SMAX_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b01, Vm, 0, Vn, Vd))
#define SMAX_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b10, Vm, 0, Vn, Vd))
#define SMAX_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b11, Vm, 0, Vn, Vd))
#define UMAX_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 1, 0b00, Vm, 0, Vn, Vd))
#define UMAX_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b01, Vm, 0, Vn, Vd))
#define UMAX_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b10, Vm, 0, Vn, Vd))
#define UMAX_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b11, Vm, 0, Vn, Vd))
#define SMIN_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 0, 0b00, Vm, 1, Vn, Vd))
#define SMIN_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b01, Vm, 1, Vn, Vd))
#define SMIN_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b10, Vm, 1, Vn, Vd))
#define SMIN_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b11, Vm, 1, Vn, Vd))
#define UMIN_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 1, 0b00, Vm, 1, Vn, Vd))
#define UMIN_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b01, Vm, 1, Vn, Vd))
#define UMIN_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b10, Vm, 1, Vn, Vd))
#define UMIN_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b11, Vm, 1, Vn, Vd))
#define SMAXQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 0, 0b00, Vm, 0, Vn, Vd))
#define SMAXQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b01, Vm, 0, Vn, Vd))
#define SMAXQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b10, Vm, 0, Vn, Vd))
#define SMAXQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b11, Vm, 0, Vn, Vd))
#define UMAXQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 1, 0b00, Vm, 0, Vn, Vd))
#define UMAXQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b01, Vm, 0, Vn, Vd))
#define UMAXQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b10, Vm, 0, Vn, Vd))
#define UMAXQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b11, Vm, 0, Vn, Vd))
#define SMINQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 0, 0b00, Vm, 1, Vn, Vd))
#define SMINQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b01, Vm, 1, Vn, Vd))
#define SMINQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b10, Vm, 1, Vn, Vd))
#define SMINQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b11, Vm, 1, Vn, Vd))
#define UMINQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 1, 0b00, Vm, 1, Vn, Vd))
#define UMINQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b01, Vm, 1, Vn, Vd))
#define UMINQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b10, Vm, 1, Vn, Vd))
#define UMINQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b11, Vm, 1, Vn, Vd))

// HADD vector
#define HADD_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 1<<10 | (Rn)<<5 | (Rd))
#define SHADD_8(Vd, Vn, Vm)         EMIT(HADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SHADD_16(Vd, Vn, Vm)        EMIT(HADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SHADD_32(Vd, Vn, Vm)        EMIT(HADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SHADDQ_8(Vd, Vn, Vm)        EMIT(HADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SHADDQ_16(Vd, Vn, Vm)       EMIT(HADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SHADDQ_32(Vd, Vn, Vm)       EMIT(HADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define UHADD_8(Vd, Vn, Vm)         EMIT(HADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UHADD_16(Vd, Vn, Vm)        EMIT(HADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UHADD_32(Vd, Vn, Vm)        EMIT(HADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UHADDQ_8(Vd, Vn, Vm)        EMIT(HADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UHADDQ_16(Vd, Vn, Vm)       EMIT(HADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UHADDQ_32(Vd, Vn, Vm)       EMIT(HADD_vector(1, 1, 0b10, Vm, Vn, Vd))

#define RHADD_vector(Q, U, size, Rm, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SRHADD_8(Vd, Vn, Vm)        EMIT(RHADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SRHADD_16(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SRHADD_32(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SRHADDQ_8(Vd, Vn, Vm)       EMIT(RHADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SRHADDQ_16(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SRHADDQ_32(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define URHADD_8(Vd, Vn, Vm)        EMIT(RHADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define URHADD_16(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define URHADD_32(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define URHADDQ_8(Vd, Vn, Vm)       EMIT(RHADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define URHADDQ_16(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define URHADDQ_32(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 1, 0b10, Vm, Vn, Vd))

// QRDMULH Signed saturating (Rounding) Doubling Multiply returning High half
#define QDMULH_vector(Q, U, size, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10110<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SQRDMULH_8(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 1, 0b00, Vm, Vn, Vd))
#define SQRDMULH_16(Vd, Vn, Vm)     EMIT(QDMULH_vector(0, 1, 0b01, Vm, Vn, Vd))
#define SQRDMULH_32(Vd, Vn, Vm)     EMIT(QDMULH_vector(0, 1, 0b10, Vm, Vn, Vd))
#define SQRDMULHQ_8(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 1, 0b00, Vm, Vn, Vd))
#define SQRDMULHQ_16(Vd, Vn, Vm)    EMIT(QDMULH_vector(1, 1, 0b01, Vm, Vn, Vd))
#define SQRDMULHQ_32(Vd, Vn, Vm)    EMIT(QDMULH_vector(1, 1, 0b10, Vm, Vn, Vd))
#define SQDMULH_8(Vd, Vn, Vm)       EMIT(QDMULH_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQDMULH_16(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQDMULH_32(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQDMULHQ_8(Vd, Vn, Vm)      EMIT(QDMULH_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQDMULHQ_16(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQDMULHQ_32(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 0, 0b10, Vm, Vn, Vd))

#endif  //__ARM64_EMITTER_H__
