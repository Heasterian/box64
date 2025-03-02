#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "dynablock_private.h"
#include "arm64_printer.h"
#include "../tools/bridge_private.h"
#include "custommem.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int s, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    uint8_t ret = x2;
    uint8_t scratch = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xRAX) scratch = hint;
    int absmin = 0;
    if(s) absmin=-absmax;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<absmin) || (tmp>absmax) || (tmp&mask))) {
                        MOV64x(scratch, tmp);
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        LSLx(ret, xRAX+sib_reg, (sib>>6));
                        *fixaddress = tmp;
                    }
                } else {
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            uint64_t tmp = F32S64;
            if((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask)) {
                GETIP(addr+delta);
                ret = xRIP;
                *fixaddress = tmp;
            } else if(tmp<0x1000) {
                GETIP(addr+delta);
                ADDx_U12(ret, xRIP, tmp);
            } else if(tmp+addr+delta<0x1000000000000LL) {  // 3 opcodes to load immediate is cheap enough
                tmp += addr+delta;
                MOV64x(ret, tmp);
            } else {
                MOV64x(ret, tmp);
                GETIP(addr+delta);
                ADDx_REG(ret, ret, xRIP);
            }
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        if(nextop&0x80)
            i64 = F32S;
        else 
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask))) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
        } else {
            int64_t sub = (i64<0)?1:0;
            if(sub) i64 = -i64;
            if(i64<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDx_REG_LSL(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                    } else {
                        scratch = xRAX+(sib&0x07)+(rex.b<<3);
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                if(sub) {
                    SUBx_U12(ret, scratch, i64);
                } else {
                    ADDx_U12(ret, scratch, i64);
                }
            } else {
                MOV64x(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBx_REG(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                        } else {
                            ADDx_REG(scratch, scratch, xRAX+(sib&0x07)+(rex.b<<3));
                        }
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07)+(rex.b<<3));
                        if(sub) {
                            SUBx_REG(ret, tmp, scratch);
                        } else {
                            ADDx_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    if(sub) {
                        SUBx_REG(ret, tmp, scratch);
                    } else {
                        ADDx_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted32(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int s, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    uint8_t ret = x2;
    uint8_t scratch = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xRAX) scratch = hint;
    int absmin = 0;
    if(s) absmin=-absmax;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<absmin) || (tmp>absmax) || (tmp&mask))) {
                        MOV64x(scratch, tmp);
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        LSLw(ret, xRAX+sib_reg, (sib>>6));
                        *fixaddress = tmp;
                    }
                } else {
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32w(ret, tmp);
            GETIP(addr+delta);
            ADDw_REG(ret, ret, xRIP);
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
            if(ret==hint) {
                MOVw_REG(hint, ret);    //to clear upper part
            }
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        if(nextop&0x80)
            i64 = F32S;
        else 
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask))) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else {
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
            }
        } else {
            int64_t sub = (i64<0)?1:0;
            if(sub) i64 = -i64;
            if(i64<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDw_REG_LSL(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                    } else {
                        scratch = xRAX+(sib&0x07)+(rex.b<<3);
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                if(sub) {
                    SUBw_U12(ret, scratch, i64);
                } else {
                    ADDw_U12(ret, scratch, i64);
                }
            } else {
                MOV32w(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBw_REG(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                        } else {
                            ADDw_REG(scratch, scratch, xRAX+(sib&0x07)+(rex.b<<3));
                        }
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07)+(rex.b<<3));
                        if(sub) {
                            SUBw_REG(ret, tmp, scratch);
                        } else {
                            ADDw_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    if(sub) {
                        SUBw_REG(ret, tmp, scratch);
                    } else {
                        ADDw_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, r3 as scratch also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, int s)
{
    MAYUSE(dyn); MAYUSE(ninst);

    uint8_t ret = x2;
    uint8_t scratch = x3;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(scratch==ret) scratch = x2;
    MAYUSE(scratch);
    uint32_t m = nextop&0xC7;
    uint32_t n = (m>>6)&3;
    int64_t offset = 0;
    int absmin = 0;
    if(s) absmin = -absmax;
    if(!n && m==6) {
        offset = F16;
        MOVZw(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (offset>absmax || offset<absmin || (offset&mask))) {
            *fixaddress = offset;
            offset = 0;
        }
        switch(m&7) {
            case 0: //R_BX + R_SI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 1: //R_BX + R_DI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 2: //R_BP + R_SI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 3: //R_BP + R_DI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 4: //R_SI
                UXTHx(ret, xRSI);
                break;
            case 5: //R_DI
                UXTHx(ret, xRDI);
                break;
            case 6: //R_BP
                UXTHx(ret, xRBP);
                break;
            case 7: //R_BX
                UXTHx(ret, xRBX);
                break;
        }
        if(offset) {
            if(offset<0 && offset>-0x1000) {
                SUBx_U12(ret, ret, -offset);
            } else if(offset>0 && offset<0x1000) {
                ADDx_U12(ret, ret, offset);
            } else {
                MOV64x(scratch, offset);
                ADDx_REG(ret, ret, scratch);
            }
        }
    }

    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if(reg) {
        if(reg!=xRIP) {
            MOVx_REG(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    TABLE64(x2, (uintptr_t)arm64_epilog);
    BR(x2);
}

void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xRIP) {
            MOVx_REG(xRIP, reg);
        }
        uintptr_t tbl = getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x2, tbl);
        UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        UBFXx(x3, xRIP, 0, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x3, x2, x3);
    } else {
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x2, p);
        GETIP_(ip);
        LDRx_U12(x3, x2, 0);
    }
    if(reg!=x1) {
        MOVx_REG(x1, xRIP);
    }
    #ifdef HAVE_TRACE
    //MOVx(x2, 15);    no access to PC reg 
    #endif
    BLR(x3); // save LR...
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1(xRIP);
    uintptr_t tbl = getJumpTable64();
    MOV64x(x2, tbl);
    UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 0, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    MOVx_REG(x1, xRIP);
    BLR(x2); // save LR
}

void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    POP1(xRIP);
    if(n>0xfff) {
        MOV32w(w1, n);
        ADDx_REG(xRSP, xRSP, x1);
    } else {
        ADDx_U12(xRSP, xRSP, n);
    }
    uintptr_t tbl = getJumpTable64();
    MOV64x(x2, tbl);
    UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 0, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    MOVx_REG(x1, xRIP);
    BLR(x2); // save LR
}

void iret_to_epilog(dynarec_arm_t* dyn, int ninst, int is64bits)
{
    #warning TODO: is64bits
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "IRet to epilog\n");
    // POP IP
    POP1(xRIP);
    // POP CS
    POP1(x2);
    STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    MOVZw(x1, 0);
    STRx_U12(x1, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
    // POP EFLAGS
    POP1(xFlags);
    MOV32w(x1, 0x3F7FD7);
    ANDx_REG(xFlags, xFlags, x1);
    ORRx_mask(xFlags, xFlags, 1, 0b111111, 0);
    SET_DFNONE(x1);
    // Ret....
    MOV64x(x2, (uintptr_t)arm64_epilog);  // epilog on purpose, CS might have changed!
    BR(x2);
}

void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if(savereg==0)
        savereg = 7;
    if(saveflags) {
        STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if(ret!=-2) {
        STPx_S7_preindex(xEmu, savereg, xSP, -16);   // ARM64 stack needs to be 16byte aligned
        STPx_S7_offset(xRAX, xRCX, xEmu, offsetof(x64emu_t, regs[_AX]));    // x9..x15, x16,x17,x18 those needs to be saved by caller
        STPx_S7_offset(xRDX, xRBX, xEmu, offsetof(x64emu_t, regs[_DX]));    // but x18 is R8 wich is lost, so it's fine to not save it?
        STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
        STPx_S7_offset(xRSI, xRDI, xEmu, offsetof(x64emu_t, regs[_SI]));
        STPx_S7_offset(xR8,  xR9,  xEmu, offsetof(x64emu_t, regs[_R8]));
    }
    TABLE64(reg, (uintptr_t)fnc);
    BLR(reg);
    if(ret>=0) {
        MOVx_REG(ret, xEmu);
    }
    if(ret!=-2) {
        LDPx_S7_postindex(xEmu, savereg, xSP, 16);
        #define GO(A, B) if(ret==x##A) {                                        \
            LDRx_U12(x##B, xEmu, offsetof(x64emu_t, regs[_##B]));               \
        } else if(ret==x##B) {                                                  \
            LDRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]));               \
        } else {                                                                \
            LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]));   \
        }
        GO(RAX, RCX);
        GO(RDX, RBX);
        GO(RSP, RBP);
        GO(RSI, RDI);
        GO(R8, R9);
        #undef GO
    }
    fpu_popcache(dyn, ninst, reg, 0);
    if(saveflags) {
        LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    SET_NODF();
}

void call_n(dynarec_arm_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x3, 1);
    // x9..x15, x16,x17,x18 those needs to be saved by caller
    // RDI, RSI, RDX, RCX, R8, R9 are used for function call
    STPx_S7_preindex(xEmu, xRBX, xSP, -16);   // ARM64 stack needs to be 16byte aligned
    STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
    // float and double args
    if(abs(w)>1) {
        MESSAGE(LOG_DUMP, "Getting %d XMM args\n", abs(w)-1);
        for(int i=0; i<abs(w)-1; ++i) {
            sse_get_reg(dyn, ninst, x7, i);
        }
    }
    if(w<0) {
        MESSAGE(LOG_DUMP, "Return in XMM0\n");
        sse_get_reg_empty(dyn, ninst, x7, 0);
    }
    // prepare regs for native call
    MOVx_REG(0, xRDI);
    MOVx_REG(x1, xRSI);
    MOVx_REG(x2, xRDX);
    MOVx_REG(x3, xRCX);
    MOVx_REG(x4, xR8);
    MOVx_REG(x5, xR9);
    // native call
    TABLE64(16, (uintptr_t)fnc);    // using x16 as scratch regs for call address
    BLR(16);
    // put return value in x86 regs
    if(w>0) {
        MOVx_REG(xRAX, 0);
        MOVx_REG(xRDX, x1);
    }
    // all done, restore all regs
    LDPx_S7_postindex(xEmu, xRBX, xSP, 16);
    #define GO(A, B) LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
    GO(RSP, RBP);
    #undef GO

    fpu_popcache(dyn, ninst, x3, 1);
    LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    SET_NODF();
}

void grab_segdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg, int segment)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment==_FS)?"FS":"GS");
    int t1 = x1, t2 = x4;
    if(reg==t1) ++t1;
    if(reg==t2) ++t2;
    LDRw_U12(t2, xEmu, offsetof(x64emu_t, segs_serial[segment]));
    LDRx_U12(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    if(segment==_GS) {
        CBNZw_MARKSEG(t2);   // fast check
    } else {
        LDRx_U12(t1, xEmu, offsetof(x64emu_t, context));
        LDRw_U12(t1, t1, offsetof(box64context_t, sel_serial));
        SUBw_REG(t1, t1, t2);
        CBZw_MARKSEG(t1);
    }
    MOVZw(x1, segment);
    call_c(dyn, ninst, GetSegmentBaseEmu, t2, reg, 1, 0);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment==_FS)?"FS":"GS");
}

// x87 stuffs
static void x87_reset(dynarec_arm_t* dyn, int ninst)
{
    (void)ninst;
#if STEP > 1
    for (int i=0; i<8; ++i)
        dyn->x87cache[i] = -1;
    dyn->x87stack = 0;
#else
    (void)dyn;
#endif
}

void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
#if STEP > 1
    MAYUSE(scratch);
    if(!dyn->x87stack)
        return;
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    // Add x87stack to emu fpu_stack
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDw_U12(scratch, scratch, a);
    } else {
        SUBw_U12(scratch, scratch, -a);
    }
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBw_U12(scratch, scratch, a);
    } else {
        ADDw_U12(scratch, scratch, -a);
    }
    ANDw_mask(scratch, scratch, 0, 2);  //mask=7
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack
    dyn->x87stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
#else
    (void)dyn; (void)ninst; (void)scratch;
#endif
}

int x87_do_push(dynarec_arm_t* dyn, int ninst)
{
    (void)ninst;
#if STEP > 1
    dyn->x87stack+=1;
    // move all regs in cache, and find a free one
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
        else if(ret==-1) {
            dyn->x87cache[i] = 0;
            ret=dyn->x87reg[i]=fpu_get_reg_x87(dyn);
        }
    return ret;
#else
    (void)dyn;
    return 0;
#endif
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    dyn->x87stack+=1;
    // move all regs in cache
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
    if(s1)
        x87_stackcount(dyn, ninst, s1);
#else
    (void)dyn; (void)ninst; (void)s1;
#endif
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst)
{
    (void)ninst;
#if STEP > 1
    dyn->x87stack-=1;
    // move all regs in cache, poping ST0
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            --dyn->x87cache[i];
            if(dyn->x87cache[i]==-1) {
                fpu_free_reg(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
            }
        }
#else
    (void)dyn;
#endif
}

void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    (void)ninst;
#if STEP > 1
    MAYUSE(s1); MAYUSE(s2); MAYUSE(s3);
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge x87 Cache and Synch Stackcount (%+d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    if(a!=0) {
        // reset x87stack
        dyn->x87stack = 0;
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        if(a>0) {
            // new tag to fulls
            MOVZw(s3, 0);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, p_regs));
            for (int i=0; i<a; ++i) {
                SUBw_U12(s2, s2, 1);
                ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + st)&7
                STRw_REG_LSL2(s3, s1, s2);
            }
        } else {
            // empty tags
            MOVZw(s3, 0b11);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, p_regs));
            for (int i=0; i<-a; ++i) {
                STRw_REG_LSL2(s3, s1, s2);
                ADDw_U12(s2, s2, 1);
                ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + st)&7
            }
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    } else {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    if(ret!=0) {
        // --- set values
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->x87cache[i]!=-1) {
                ADDw_U12(s3, s2, dyn->x87cache[i]);
                ANDw_mask(s3, s3, 0, 2); //mask=7   // (emu->top + st)&7
                VSTR64_REG_LSL3(dyn->x87reg[i], s1, s3);
                fpu_free_reg(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
                dyn->x87cache[i] = -1;
            }
    }
#else
    (void)dyn; (void)s1; (void)s2; (void)s3;
#endif
}

#ifdef HAVE_TRACE
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    MAYUSE(s2); MAYUSE(s3);
    x87_stackcount(dyn, ninst, s1);
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            ADDw_U12(s3, s2, dyn->x87cache[i]);
            ANDw_mask(s3, s3, 0, 2); // mask=7   // (emu->top + i)&7
            VSTR64_REG_LSL3(dyn->x87reg[i], s1, s3);
        }
#else
    (void)dyn; (void)ninst; (void)s1; (void)s2; (void)s3;
#endif
}
#endif

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    (void)ninst;
#if STEP > 1
    MAYUSE(s1); MAYUSE(s2);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st)
            return i;
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_x87(dyn);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->x87stack;
    if(a) {
        if(a<0) {
            SUBw_U12(s2, s2, -a);
        } else {
            ADDw_U12(s2, s2, a);
        }
        ANDw_mask(s2, s2, 0, 2); //mask=7
    }
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
#else
    (void)dyn; (void)s1; (void)s2; (void)st;
    return 0;
#endif
}

int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a)
{
#if STEP > 1
    return dyn->x87reg[x87_get_cache(dyn, ninst, s1, s2, a)];
#else
    (void)dyn; (void)ninst; (void)s1; (void)s2; (void)a;
    return 0;
#endif
}


void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    MAYUSE(s2);
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    if(st) {
        ADDw_U12(s2, s2, st);
        ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    }
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
#else
    (void)dyn; (void)ninst; (void)s1; (void)s2; (void)st;
#endif
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    MAYUSE(s2);
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    if(st) {
        ADDw_U12(s2, s2, st);
        ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    }
    VSTR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg(dyn, dyn->x87reg[ret]);
    dyn->x87cache[ret] = -1;
    dyn->x87reg[ret] = -1;
#else
    (void)dyn; (void)ninst; (void)s1; (void)s2; (void)st;
#endif
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    (void)ninst;
#if STEP > 1
    MAYUSE(s1); MAYUSE(s2);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
            LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->x87stack;
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
            VLDR64_REG_LSL3(dyn->x87reg[i], s1, s2);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_x87(dyn);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->x87stack;
    if(a<0) {
        SUBw_U12(s2, s2, -a);
    } else {
        ADDw_U12(s2, s2, a);
    }
    ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
#else
    (void)dyn; (void)s1; (void)s2; (void)st;
#endif
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRH_U12(s1, xEmu, offsetof(x64emu_t, cw));
    RBITw(s2, s1);              // round is on bits 10-11 on x86,
    LSRw(s2, s2, 20);           // but we want the reverse of that
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);        // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRH_U12(s1, xEmu, offsetof(x64emu_t, mxcsr));
    RBITw(s2, s1);              // round is on bits 13-14 on x86,
    LSRw(s2, s2, 17);           // but we want the reverse of that
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);     // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1);
    MSR_fpcr(s1);               // put back fpscr
}

// MMX helpers
static void mmx_reset(dynarec_arm_t* dyn, int ninst)
{
    (void)ninst;
#if STEP > 1
    MAYUSE(dyn);
    for (int i=0; i<8; ++i)
        dyn->mmxcache[i] = -1;
#else
    (void)dyn;
#endif
}
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    (void)ninst; (void)s1;
#if STEP > 1
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    int ret = dyn->mmxcache[a] = fpu_get_reg_emm(dyn, a);
    VLDR64_U12(ret, xEmu, offsetof(x64emu_t, mmx87[a]));
    return ret;
#else
    (void)dyn; (void)a;
    return 0;
#endif
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    (void)ninst; (void)s1;
#if STEP > 1
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    int ret = dyn->mmxcache[a] = fpu_get_reg_emm(dyn, a);
    return ret;
#else
    (void)dyn; (void)a;
    return 0;
#endif
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
    (void)ninst; (void)s1;
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge MMX Cache ------\n");
                ++old;
            }
            VSTR64_U12(dyn->mmxcache[i], xEmu, offsetof(x64emu_t, mmx87[i]));
            fpu_free_reg(dyn, dyn->mmxcache[i]);
            dyn->mmxcache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
#else
    (void)dyn;
#endif
}
#ifdef HAVE_TRACE
static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    (void) ninst; (void)s1;
#if STEP > 1
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            VLDR64_U12(dyn->mmxcache[i], xEmu, offsetof(x64emu_t, mmx87[i]));
        }
#else
    (void)dyn;
#endif
}
#endif


// SSE / SSE2 helpers
static void sse_reset(dynarec_arm_t* dyn, int ninst)
{
    (void)ninst;
#if STEP > 1
    for (int i=0; i<16; ++i)
        dyn->ssecache[i] = -1;
#else
    (void)dyn;
#endif
}
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    (void) ninst; (void)s1;
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_xmm(dyn, a);
    VLDR128_U12(ret, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
#else
    (void)dyn; (void)a;
    return 0;
#endif
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    (void) ninst; (void)s1;
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_xmm(dyn, a);
    return ret;
#else
    (void)dyn; (void)a;
    return 0;
#endif
}
// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1)
{
    (void) ninst; (void)s1;
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg(dyn, dyn->ssecache[i]);
            dyn->ssecache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
#else
    (void)dyn;
#endif
}

// purge the SSE cache only
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
    (void) ninst; (void)s1;
#if STEP > 1
    int old = -1;
    for (int i=0; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge SSE Cache ------\n");
                ++old;
            }
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg(dyn, dyn->ssecache[i]);
            dyn->ssecache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
#else
    (void)dyn;
#endif
}
#ifdef HAVE_TRACE
static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    (void) ninst; (void)s1;
#if STEP > 1
    for (int i=0; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
        }
#else
    (void)dyn;
#endif
}
#endif

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    (void) ninst; (void)s1;
#if STEP > 1
    int start = not07?8:0;
    // only SSE regs needs to be push back to xEmu
    int n=0;
    for (int i=start; i<16; i++)
        if(dyn->ssecache[i]!=-1)
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPush XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
        }
    MESSAGE(LOG_DUMP, "\t------- Push XMM Cache (%d)\n", n);
#else
    (void)dyn;
#endif
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    (void) ninst; (void)s1;
#if STEP > 1
    int start = not07?8:0;
    // only SSE regs needs to be pop back from xEmu
    int n=0;
    for (int i=start; i<16; i++)
        if(dyn->ssecache[i]!=-1)
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            VLDR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
        }
    MESSAGE(LOG_DUMP, "\t------- Pop XMM Cache (%d)\n", n);
#else
    (void)dyn;
#endif
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, s1, s2, s3);
    mmx_purgecache(dyn, ninst, s1);
    sse_purgecache(dyn, ninst, s1);
    fpu_reset_reg(dyn);
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    if(trace_emm)
       mmx_reflectcache(dyn, ninst, s1);
    if(trace_xmm)
       sse_reflectcache(dyn, ninst, s1);
}
#endif

void fpu_reset(dynarec_arm_t* dyn, int ninst)
{
    x87_reset(dyn, ninst);
    mmx_reset(dyn, ninst);
    sse_reset(dyn, ninst);
    fpu_reset_reg(dyn);
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    ANDw_mask(s3, s1, 0b011011, 0b000010); // mask=0xE0
    LSRw(s3, s3, 5);
    MOV64x(s4, (uintptr_t)GetParityTab());
    LDRw_REG_LSL2(s4, s4, s3);
    ANDw_mask(s3, s1, 0, 0b000100); //0x1f
    LSRw_REG(s4, s4, s3);
    MVNw_REG(s4, s4);
    BFIw(xFlags, s4, F_PF, 1);
}
