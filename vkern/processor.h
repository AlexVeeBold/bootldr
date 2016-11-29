///////////////////////////////////////////////////////////////
//
// processor: Central processor
//
//
//   30.08.2010 04:14 - created
//

#ifndef _HW_PROCESSOR_H_INC_
#define _HW_PROCESSOR_H_INC_


EXTERN_C_BEGIN


// GLOBAL DESCRIPTOR-TABLE REGISTER

struct GIDTREG {    // Global Descriptor-Table Register, Interrupt Descriptor-Table Register
    WORD Limit;         // Descriptor-Table Limit (last valid byte offset)
    QWBYTE Address;     // Descriptor-Table Base Address
};


// (GENERIC) SEGMENT SELECTOR

union GSS {         // segment selector
    struct {
        WORD RPL:2;     // bits 0..1: Requestor Privilege Level
        WORD Local:1;   // bit 2: Table Indicator (0 = GDT, 1 = LDT)
        WORD Index:13;  // bits 3..15: Selector Index
    };
    // ignoring .RPL and .Local values, .Selector = byte offset in GDT/LDT
    WORD Selector;      // WORD: whole selector (SI, TI, RPL)
};


// SEGMENT DESCRIPTORS

/* types:
s=1: code: 1,C,R,A  (Conforming, Readable(as data), Accessed(loaded to CS))
s=1: data: 0,E,W,A  (Expand-Down, Writable, Accessed(loaded to DS,FS,SS,etc))
s=0: type    descriptor
  0000  0  reserved
  0001  1  TSS, 16-bit, available
  0010  2  LDT
  0011  3  TSS, 16-bit, busy
  0100  4  Call Gate, 16-bit
  0101  5  Task Gate
  0110  6  Interrupt Gate, 16-bit
_ 0111  7  Trap Gate, 16-bit
  1000  8  reserved
  1001  9  TSS, 32-bit, available
  1010  A  reserved
  1011  B  TSS, 32-bit, busy
  1100  C  Call Gate 32-bit
  1101  D  reserved
  1110  E  Interrupt Gate, 32-bit
  1111  F  Trap Gate, 32-bit
*/
// type flags
#define GSD_CODE_ACCESSED   0x01    // ---!!**1
#define GSD_CODE_READABLE   0x02    // ---!!*1*
#define GSD_CODE_CONFORMING 0x04    // ---!!1**
#define GSD_DATA_ACCESSED   0x01    // ---!!**1
#define GSD_DATA_WRITABLE   0x02    // ---!!*1*
#define GSD_DATA_EXPANDDOWN 0x04    // ---!!1**
#define GSD_TSS_AVAILABLE   0x00    // ---!*!0!  (pseudo-flag)
#define GSD_TSS_BUSY        0x02    // ---!*!1!
#define GSD_SYS_16BIT       0x00    // ---!0!!!  (pseudo-flag)
#define GSD_SYS_32BIT       0x08    // ---!1!!!
// types
#define GSD_TYPE_CODE       0x18    // ---11***  + GSD_CODE_*
#define GSD_TYPE_DATA       0x10    // ---10***  + GSD_DATA_*
#define GSD_TYPE_LDT        0x02    // ---00010  (no flags)
#define GSD_TYPE_TASKGATE   0x05    // ---00101  (no flags)
#define GSD_TYPE_TSS        0x01    // ---0*0*1  + GSD_SYS_* + GSD_TSS_*
#define GSD_TYPE_CALLGATE   0x04    // ---0*100  + GSD_SYS_*
#define GSD_TYPE_INTGATE    0x06    // ---0*110  + GSD_SYS_*
#define GSD_TYPE_TRAPGATE   0x07    // ---0*111  + GSD_SYS_*
/* "system" types
  0010  2  LDT
  0101  5  Task Gate
  0001  1  TSS, 16-bit, available
  0011  3  TSS, 16-bit, busy
  1001  9  TSS, 32-bit, available
  1011  B  TSS, 32-bit, busy
  0100  4  Call Gate, 16-bit
  1100  C  Call Gate 32-bit
  0110  6  Interrupt Gate, 16-bit
  1110  E  Interrupt Gate, 32-bit
  0111  7  Trap Gate, 16-bit
  1111  F  Trap Gate, 32-bit
*/

/*  possible segment descriptors locations:
  GDT
   +- CODE
   +- DATA
   +- CALLGATE
   +- TSS
   +- TASKGATE
   +- LDT
       +- CODE
       +- DATA
       +- CALLGATE
       +- TASKGATE
  IDT
   +- INTGATE
   +- TRAPGATE
   +- TASKGATE
*/

struct USD {        // user segment descriptor (code, data)
    WBYTE Limit0;           // BYTES 0-1: Segment Limit: bits 0..15
    WBYTE Base0;            // BYTES 2-3: Base Address: bits 0..15
    BYTE Base2;             // BYTE 4: Base Address: bits 16..23
    union {
        struct {
            BYTE Type:5;    // byte 5, BITS 0..4: = GSD_TYPE_CODE + GSD_CODE_*; = GSD_TYPE_DATA + GSD_DATA_*
            BYTE DPL:2;     // byte 5, BITS 5..6: Descriptor Privilege-Level
            BYTE P:1;       // byte 5, BIT 7: Present
        };
        BYTE Attr0;         // BYTE 5: Attributes (P, DPL, S, Type) [S moved into Type]
    };
    union {
        struct {
            BYTE Limit2:4;  // byte 6, BITS 0..3: Segment Limit: bits 16..19
            BYTE AVL:1;     // byte 6, BIT 4: Available To Software bit
            BYTE unused:1;  // byte 6, BIT 5: reserved, must be zero
            BYTE D_B:1;     // byte 6, BIT 6: Default Operand Size, Stack Size (=0: 16; =1: 32)
            BYTE G:1;       // byte 6, BIT 7: Granularity (0 = limit is in bytes; 1 = limit is in 4KB blocks)
        };
        BYTE Attr1_Limit2;  // BYTE 6: Segment Limit: bits 16..19, Attributes (G, AVL)
    };
    BYTE Base3;             // BYTE 7: Base Address: bits 24..31
};

struct SSD {        // system segment descriptor (LDT, TSS)
    WBYTE Limit0;           // BYTES 0-1: Segment Limit: bits 0..15
    WBYTE Base0;            // BYTES 2-3: Base Address: bits 0..15
    BYTE Base2;             // BYTE 4: Base Address: bits 16..23
    union {
        struct {
            BYTE Type:5;    // byte 5, BITS 0..4: = GSD_TYPE_LDT; = GSD_TYPE_TSS + GSD_SYS_* + GSD_TSS_*
            BYTE DPL:2;     // byte 5, BITS 5..6: Descriptor Privilege-Level
            BYTE P:1;       // byte 5, BIT 7: Present
        };
        BYTE Attr0;         // BYTE 5: Attributes (P, DPL, S, Type) [S moved into Type]
    };
    union {
        struct {
            BYTE Limit2:4;  // byte 6, BITS 0..3: Segment Limit: bits 16..19
            BYTE AVL:1;     // byte 6, BIT 4: Available To Software bit
            BYTE unused:2;  // byte 6, BITS 5..6: reserved, must be zero
            BYTE G:1;       // byte 6, BIT 7: Granularity (0 = limit is in bytes; 1 = limit is in 4KB blocks)
        };
        BYTE Attr1_Limit2;  // BYTE 6: Segment Limit: bits 16..19, Attributes (G, AVL)
    };
    BYTE Base3;             // BYTE 7: Base Address: bits 24..31
};

struct SCGD {       // system gate descriptor (Call Gate)
    WBYTE offset0;          // BYTES 0-1: Target Code-Segment Offset: bits 0..15
    GSS SegSelector;        // BYTES 2-3: Target Code-Segment Selector
    BYTE ParamCount:5;      // byte 4, BITS 0..4: Parameter Count
    BYTE unused:3;          // byte 4, BITS 5..7: reserved, must be zero
    union {
        struct {
            BYTE Type:5;    // byte 5, BITS 0..4: = GSD_TYPE_CALLGATE + GSD_SYS_*
            BYTE DPL:2;     // byte 5, BITS 5..6: Descriptor Privilege-Level
            BYTE P:1;       // byte 5, BIT 7: Present
        };
        BYTE Attr0;         // BYTE 5: Attributes (P, DPL, S, Type) [S moved into Type]
    };
    WBYTE offset2;          // BYTES 6-7: Target Code-Segment Offset: bits 16..31
};

struct SITGD {      // system gate descriptor (Interrupt Gate & Trap Gate)
    WBYTE Offset0;          // BYTES 0-1: Target Code-Segment Offset: bits 0..15
    GSS SegSelector;        // BYTES 2-3: Target Code-Segment Selector
    BYTE byUnused4;         // BYTE 4: reserved, must be zero
    union {
        struct {
            BYTE Type:5;    // byte 5, BITS 0..4: = GSD_TYPE_INTGATE + GSD_SYS_*; = GSD_TYPE_TRAPGATE + GSD_SYS_*
            BYTE DPL:2;     // byte 5, BITS 5..6: Descriptor Privilege-Level
            BYTE P:1;       // byte 5, BIT 7: Present
        };
        BYTE Attr0;         // BYTE 5: Attributes (P, DPL, S, Type) [S moved into Type]
    };
    WBYTE Offset2;          // BYTES 6-7: Target Code-Segment Offset: bits 16..31
};

struct STGD {       // system gate descriptor (Task Gate)
    WORD wUnused0;          // BYTES 0-1: reserved, must be zero
    GSS TaskSelector;       // BYTES 2-3: Target-task TSS Selector
    BYTE byUnused4;         // BYTE 4: reserved, must be zero
    union {
        struct {
            BYTE Type:5;    // byte 5, BITS 0..4: = GSD_TYPE_TASKGATE
            BYTE DPL:2;     // byte 5, BITS 5..6: Descriptor Privilege-Level
            BYTE P:1;       // byte 5, BIT 7: Present
        };
        BYTE Attr0;         // BYTE 5: Attributes (P, DPL, S, Type) [S moved into Type]
    };
    WORD wUnused6;          // BYTES 6-7: reserved, must be zero
};


// TASK-STATE SEGMENT

struct TSS {
    //// mandatory part
    // <dynamic> = modified by the processor when a task is suspended
    GSS PriorTask;      // <dynamic> Prior TSS Selector
    WORD wUnused02;     // reserved, must be zero
    DWORD Esp0;         // Privilege 0 Stack-Segment Offset
    GSS Ss0;            // Privilege 0 Stack-Segment Selector
    WORD wUnused0A;     // reserved, must be zero
    DWORD Esp1;         // Privilege 1 Stack-Segment Offset
    GSS Ss1;            // Privilege 1 Stack-Segment Selector
    WORD wUnused12;     // reserved, must be zero
    DWORD Esp2;         // Privilege 2 Stack-Segment Offset
    GSS Ss2;            // Privilege 2 Stack-Segment Selector
    WORD wUnused1A;     // reserved, must be zero
    DWORD Cr3;          // Page-Translation-Table Base-Address
    DWORD Eip;          // <dynamic> Code-Segment Offset (next instruction pointer)
    DWORD Eflags;       // <dynamic>
    DWORD Eax;          // <dynamic>
    DWORD Ecx;          // <dynamic>
    DWORD Edx;          // <dynamic>
    DWORD Ebx;          // <dynamic>
    DWORD Esp;          // <dynamic> Stack-Segment Offset
    DWORD Ebp;          // <dynamic>
    DWORD Esi;          // <dynamic>
    DWORD Edi;          // <dynamic>
    GSS Es;             // <dynamic>
    WORD wUnused4A;     // reserved, must be zero
    GSS Cs;             // <dynamic> Code-Segment Selector
    WORD wUnused4E;     // reserved, must be zero
    GSS Ss;             // <dynamic> Stack-Segment Selector
    WORD wUnused52;     // reserved, must be zero
    GSS Ds;             // <dynamic>
    WORD wUnused56;     // reserved, must be zero
    GSS Fs;             // <dynamic>
    WORD wUnused5A;     // reserved, must be zero
    GSS Gs;             // <dynamic>
    WORD wUnused5E;     // reserved, must be zero
    GSS Ldt;            // local-descriptor-table segment selector
    WORD wUnused62;     // reserved, must be zero
    WORD TrapBit:1;     // generate a debug exception (#DB) on a task switch
    WORD unused64:13;   // reserved, must be zero
    // pointer to the beginning of the I/O-permission bitmap, 
    // and the end of the interrupt-redirection bitmap
    // (should be not less than 0x68, if not using IRB, or 
    // not less than 0x88, if IRB is used)
    WORD IopbBase;
    //// operating system data
    //// ...
    //// (immediately before IOPB base) Interrupt-Redirection Bitmap (256 bits)
    //// (at IOPB base) I/O-Permission Bitmap (up to 8 Kbytes; 1 bit per port)
};


// MEDIA EXTENSIONS STATE (XMM, MMX, X87)

struct MMXREG {     // mmx/x87 register (80 bits used, 128 bits total)
    WORD reg[5];
    WORD unused[3];
};

struct XMMREG {     // xmm register (128 bits)
    WORD reg[8];
};

struct MXS {    // media and x87 processor state (saved by FXSAVE, loaded by FXRSTOR instructions)
    WORD Fcw;               // x87 control word
    WORD Fsw;               // x87 status word
    BYTE Ftw;               // x87 tag word (packed)
    BYTE byZero005;         // = 0
    WORD Fop;               // last x87 opcode
    // Pointer to the last executed non-control x87 floating-point instruction
    DWORD Eip;
    GSS Cs;
    WORD wUnused00E;        // reserved
    // Pointer to the data operand (if) referenced by the last instruction
    DWORD Dp;
    GSS Ds;
    WORD wUnused016;        // reserved
    // 128-bit media-instruction control and status register
    DWORD Mxcsr;
    DWORD MxcsrMask;        // Supported feature bits in MXCSR
    // Shared 64-bit media and x87 floating-point registers
    MMXREG St[8];           // ST(0) ... ST(7)
    // 128-bit media registers
    XMMREG Xmm[8];          // XMM0 ... XMM7
    // unused space
    DWORD dwUnused120[56];  // reserved 224 bytes
};


EXTERN_C_END


#endif //_HW_PROCESSOR_H_INC_
