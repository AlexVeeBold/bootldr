///////////////////////////////////////////////////////////////
//
// vkern: Vienna kernel sandbox
//
//
//   21.12.2009 11:11 - created
//   24.08.2010 16:22 - working: serial port, debug interface, hardware library
//   26.08.2010 00:56 - moved into separate modules
//   29.08.2010 04:55 - reworked: (D)WBYTE, GSS, USD, SSD, etc structures
//   30.08.2010 04:14 - (D)WBYTE, GSS, USD, etc structures moved into separate modules
//   30.08.2010 07:21 - fixed: wrong image base (0x00020000), set to 0x00200000 (2MB)
//   02.09.2010 12:03 - IDT, working protected-mode interrupts
//   04.09.2010 10:55 - real-time clock query
//   06.09.2010 16:44 - pic & rtc moved into own modules, created cpp/h template
//   12.09.2010 15:12 - unicode debug interface working
//   13.09.2010 00:24 - [hosted] run-time debug added
//   14.09.2010 04:07 - reworked kstrings, added kstr_* macros, cyrillic test, pre-proc conf.
//   19.10.2010 04:40 - working: real mode int call from protected mode (hll)
//

#include "ktypes.h"
#include "kstring.h"
#include "tprintf.h"
#include "hw.h"
#include "serial.h"
#include "pic.h"
#include "rtc.h"
#include "processor.h"
#include "acpi.h"
#include "tstr.h"


#pragma comment(linker,"/NODEFAULTLIB")     // remove unused stuff
#ifdef RTDEBUG
#pragma comment(linker,"/FILEALIGN:512")
#pragma comment(linker,"/ENTRY:rtdEntry")
#pragma comment(linker,"/BASE:0x00400000")
#else //RTDEBUG
#pragma comment(linker,"/FILEALIGN:256")
#pragma comment(linker,"/ENTRY:hllEntry")
#pragma comment(linker,"/BASE:0x00200000")
#endif //RTDEBUG
//#pragma comment(linker,"/FIXED:NO")         // generate relocations
#pragma comment(linker,"/STUB:dllstub2.exe")




EXTERN_C_BEGIN      //extern "C" {


void *  __cdecl memcpy(void *, const void *, size_t);
void *  __cdecl memset(void *, int, size_t);


//[COPY]//binary debug interface (11.12.2009)
// types
#define DBG_BYTE    0x31    // print byte value:  [type] [byte] <end>
#define DBG_WORD    0x32    // print word value:  [type] [bytelo][bytehi] <end>
#define DBG_DWORD   0x34    // print dword value: [type] [bytelo][bytemedlo][bytemedhi][bytehi] <end>
#define DBG_ASTR    0x41    // print ansi string: [type] [byte][byte]...[byte] <end>
#define DBG_WSTR    0x42    // print unicode string: [type] [bytelo][bytehi]...[bytelo][bytehi] <end>
#define DBG_LF      0x0A    // print end-of-line: [type] <end>
#define DBG_END     0x00    // <end> of packet

//void KPI kdSendStr(BYTE* byString)
//{
//    kSerialPortTransferByte(DBG_ASTR);          // type
//    kSerialPortTransferBuffer(byString, 2);     // char(s) & eop
//}

void KPI kdSendLf(void)
{
    BYTE dbuf[4];
    dbuf[0] = DBG_LF;       // type
    dbuf[1] = DBG_END;      // eop
    kSerialPortTransferBuffer(dbuf, 2);
}

void KPI kdSendStr(BYTE* byString)
{
    BYTE* byBase;
    BYTE bych;
    byBase = byString;
    kSerialPortTransferByte(DBG_ASTR);          // type
    do {
        bych = *byString;
        byString++;
        kSerialPortTransferByte(bych);          // char(s) [& eop]
    } while(bych != 0);
    if(byString - 1 == byBase)      // pad packet to 3 bytes
    {
        kSerialPortTransferByte(DBG_END);       // eop
    }
}

void KPI kdSendKString(KSTR* pkString)
{
    WORD counter;
    WCHAR* pwz;
    WBYTE wbch;
    //if((kString.strLen != 0) & (kString.pwzBuffer != NULL))
    //{
        counter = pkString->strLen;
        pwz = pkString->pwzBuffer;
        kSerialPortTransferByte(DBG_WSTR);          // type
        do {
            wbch.word = *pwz;
            pwz++;
            kSerialPortTransferByte(wbch.byte0);    // char (low byte)
            kSerialPortTransferByte(wbch.byte1);    // char (high byte)
            counter--;
        } while(counter != 0);
        wbch.word = 0;
        kSerialPortTransferByte(wbch.byte0);    // send trailing zero (unicode)
        kSerialPortTransferByte(wbch.byte1);
        kSerialPortTransferByte(DBG_END);           // eop
    //}
}


//!! *must* be SYNCHRONIZED with struc BTDATA (btdata.inc) !!//
struct BTDATA;
typedef void (KPI *PFNIC)(BTDATA* pBootData);
struct BTDATA {
    BYTE bt_drive;          // drive number
    BYTE bt_gdt_count;      // number of gdt entries
    WORD bt_kern_size;      // kernel size
    WORD bt_kern_seg;       // kernel address (high 16 bit of 20-bit address)
    WORD bt_kern_entry;     // kernel entry offset
    WORD bt_stk_seg;        // stack segment
    WORD bt_stk_ptr;        // stack pointer
    DWORD bt_image_addr;    // loaded image address (before relocation)
    //
    PFNIC bl_intcall_addr;  // real mode interrupt call (ic) address
    DWORD bt_hll_size;      // h.l.l. module size
    DWORD bt_hll_entry;     // h.l.l. module entry
    DWORD bt_hll_addr;      // h.l.l. module address
    //
    WORD bt_hll_codesel;    // h.l.l. code segment selector (usually 8)
    WORD bt_ptr16_code;     // 16-bit code offset
    WORD bt_seg16_code;     // 16-bit code segment base
    WORD bt_gdt16_limit;    // 16-bit gdt (limit)
    QWORD bt_gdt16_base;    // 16-bit gdt (base)
    //
    WORD bt_seg16_data;     // 16 bit data segment base
    WORD bt_seg16_extra;    // 16-bit extra segment base
    WORD bt_seg16_stack;    // 16 bit stack segment base
    WORD bt_idt16_limit;    // 16-bit idt (limit)
    QWORD bt_idt16_base;    // 16-bit idt (base)
    //
    DWORD bt_ptr32_code;    // 32-bit code offset
    WORD bt_seg32_code;     // 32-bit code segment selector
    WORD bt_gdt32_limit;    // 32-bit gdt (limit)
    QWORD bt_gdt32_base;    // 32-bit gdt (base)
    //
    WORD bt_seg32_data;     // 32 bit data segment selector
    WORD bt_seg32_extra;    // 32-bit extra segment selector
    WORD bt_seg32_stack;    // 32 bit stack segment selector
    WORD bt_idt32_limit;    // 32-bit idt (limit)
    QWORD bt_idt32_base;    // 32-bit idt (base)
    //
    DWORD bt_ptr3216_code;  // 32-to-16 code offset
    WORD bt_seg3216_code;   // 32-to-16 data segment selector
    WORD bt_seg3216_data;   // 32-to-16 data segment selector
    DWORD bl_ic_isr;        // interrupt service routine number
    WORD bl_ic_es;          // segment registers (ES)
    WORD bl_ic_ds;          // segment registers (DS)
    // 
    DWBYTE bl_ic_eax;        // generic registers (EAX)
    DWBYTE bl_ic_ecx;        // generic registers (ECX)
    DWBYTE bl_ic_edx;        // generic registers (EDX)
    DWBYTE bl_ic_ebx;        // generic registers (EBX)
    //
    DWORD bl_ic_esp;        // stack registers (ESP)
    DWORD bl_ic_ebp;        // stack registers (EBP)
    DWORD bl_ic_esi;        // data registers (ESI)
    DWORD bl_ic_edi;        // data registers (EDI)
    //
    // up to 256 bytes total
};




// memory map:
// 00000000...00000FFF (4k)   used by bios
// 00001000...00001FFF (4k)   unused ?
// 00002000...00007FFF (24k)  available  (00007C00...00007CFF: boot sector)
// 00008000...0000FFFF (32k)  available
// 00010000...0008FFFF (512k) available
// 00090000...0009FFFF (64k)  ?available?
// 000A0000...000BFFFF (128k) video memory
// 000C0000...000FFFFF (256k) bios memory

// memory usage:
// 00002000...0000EFFF (52k)  available
// 0000F000...0000F7FF (2k)   interrupt descriptors table (used: 2k, allocated: 4k)
// 00010000...0001FFFF (64k)  global descriptors table
// 00020000...0008FFFF (448k) available
// 00090000...0009FFFF (64k)  ?available?
// 000A0000...000BFFFF (128k) video memory
// 000C0000...000FFFFF (256k) bios memory

#define MEM_VBE_BASE            0x00001000
#define MEM_VBE_SIZE            0x00000200
#define MEM_VBEMODE_BASE        0x00001200
#define MEM_VBEMODE_SIZE        0x00000100
#define MEM_IDT_BASE            0x0000F000
#define MEM_IDT_LIMIT           0x000007FF      // (256 * 8) - 1
#define MEM_GDT_BASE            0x00010000 //0x00090000
#define MEM_GDT_LIMIT           0x0000FFFF      // (8192 * 8) - 1


BTDATA* pBootData;

void setupGdt()
{
    // clear gdt buffer
    // copy asm-ldr gdt
    // fill gss for gdt
    // reload gdtr

    USD* pGdt;
    DWORD dwSize;
    pGdt = (USD*)MEM_GDT_BASE;
    // initialize with null descriptors
    dwSize = MEM_GDT_LIMIT+1;
    memset(pGdt, 0, dwSize);
    // overwrite with valid descriptors
    dwSize = pBootData->bt_gdt_count * sizeof(USD);
    memcpy(pGdt, (void*)pBootData->bt_gdt32_base, dwSize);

    // set data segment selector with full-size 4gb selector
    pBootData->bt_seg32_data = pBootData->bt_seg32_extra;

    BYTE* pGdtr32;
    BYTE* pGdtr16;
    pBootData->bt_gdt32_limit = MEM_GDT_LIMIT;
    pBootData->bt_gdt32_base = MEM_GDT_BASE;
    pGdtr32 = (BYTE*) &pBootData->bt_gdt32_limit;
    pGdtr16 = (BYTE*) &pBootData->bt_gdt16_limit;
    // interrupts should be disabled
    hwSwitchGdtr(pGdtr16, pGdtr32);
    // interrupts can be enabled

    KSTR_VAR(ksBuf, 256);
    KSTR ksFmt;

    KSTR_SET(ksFmt, "GDTR: old[%08X]=%016X:%04X, new[%08X]=%016X:%04X\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pGdtr16), 
        AQWORD(pBootData->bt_gdt16_base), 
        AWORD(pBootData->bt_gdt16_limit), 
        ADWORD(pGdtr32), 
        AQWORD(pBootData->bt_gdt32_base), 
        AWORD(pBootData->bt_gdt32_limit));
    kdSendKString(&ksBuf);

    // print segment selectors
    WORD segselCS;
    WORD segselDS;
    WORD segselES;
    WORD segselFS;
    WORD segselGS;
    WORD segselSS;
    __asm {
        mov segselCS, cs;
        mov segselDS, ds;
        mov segselES, es;
        mov segselFS, fs;
        mov segselGS, gs;
        mov segselSS, ss;
    }
    KSTR_SET(ksFmt, "CS:%04X, DS:%04X, ES:%04X, FS:%04X, GS:%04X, SS:%04X\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(segselCS), AWORD(segselDS), AWORD(segselES), AWORD(segselFS), AWORD(segselGS), AWORD(segselSS));
    kdSendKString(&ksBuf);

    //*
    DWORD i, iMax;
    USD* pusd;
    DWBYTE dwbBase;
    DWBYTE dwbLimit;
    DWBYTE dwbAttr;
    iMax = pBootData->bt_gdt_count;
    pusd = pGdt;
    KSTR_SET(ksFmt, "GDT entries: (%d):\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(iMax));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    (%04X) %08X:%08X %04X\r\n");
    for(i = 0; i < iMax; i++)
    {
        dwbBase.word0 = pusd->Base0.word;
        dwbBase.byte2 = pusd->Base2;
        dwbBase.byte3 = pusd->Base3;
        dwbLimit.word0 = pusd->Limit0.word;
        dwbLimit.byte2 = pusd->Limit2;
        dwbLimit.byte3 = 0;
        dwbAttr.byte0 = pusd->Attr1_Limit2 & 0xF0;
        dwbAttr.byte1 = pusd->Attr0;
        kSprintf(&ksBuf, &ksFmt, ADWORD(sizeof(USD) * i), ADWORD(dwbBase.dword), ADWORD(dwbLimit.dword), AWORD(dwbAttr.word0));
        kdSendKString(&ksBuf);
        pusd++;
    }//*/
}


// handle internal interrupt without error code
void NAKED KPI IntHandlerNoError(void)
{
    ASM {
        iretd
    }
}

// handle internal interrupt with error code
void NAKED KPI IntHandlerWithError(void)
{
    ASM {
        pop eax     // error code
        iretd
    }
}

/* processor interrupts (exceptions)
      errcode  restart  exception
00 #DE  none    yes     Divide-by-Zero-Error
01 #DB  none    yes     Debug
02 #NMI none    yes     Non-Maskable-Interrupt
03 #BP  none    yes     Breakpoint
04 #OF  none    yes     Overflow
05 #BR  none    yes     Bound-Range
06 #UD  none    yes     Invalid-Opcode
07 #NM  none    yes     Device-Not-Available
08 #DF  yes     no      Double-Fault
09 --- (Reserved)
10 #TS  yes     yes     Invalid-TSS
11 #NP  yes     yes     Segment-Not-Present
12 #SS  yes     yes     Stack
13 #GP  yes     yes     General-Protection
14 #PF  yes     yes     Page-Fault
15 --- (Reserved)
16 #MF  none    yes?    x87 Floating-Point Exception-Pending
17 #AC  yes     yes     Alignment-Check
18 #MC  none    no      Machine-Check
19 #XF  none    yes     SIMD Floating-Point
20-29 --- (Reserved)
30 #SX  none    yes     Security Exception
31 --- (Reserved)
*/

void NAKED KPI IntHandlerExt(void)
{
    ASM {
        push eax
        push ecx
        push edx
        push ebx
        push esi
        push edi
    }
    kPicClearBoth();
    ASM {
        pop edi
        pop esi
        pop ebx
        pop edx
        pop ecx
        pop eax
        iretd
    }
}

void setupIdt(void)
{
    // fill intgates
    // fill idt
    // rebase pic0 & pic1
    // reload idtr
    // enable ints
    
    SITGD* pIdt;
    SITGD igDeath;
    SITGD igIntNoError;
    SITGD igIntWithError;
    SITGD igExternal;
    DWBYTE dwbAddr;
    memset(&igDeath, 0, sizeof(SITGD));
    dwbAddr.dword = (DWORD)&IntHandlerNoError;
    igIntNoError.Offset0.word = dwbAddr.word0;
    igIntNoError.Offset2.word = dwbAddr.word1;
    igIntNoError.byUnused4 = 0;
    igIntNoError.P = 1;
    igIntNoError.DPL = 0;
    igIntNoError.Type = GSD_TYPE_INTGATE | GSD_SYS_32BIT;
    igIntNoError.SegSelector.Selector = pBootData->bt_hll_codesel;
    dwbAddr.dword = (DWORD)&IntHandlerWithError;
    igIntWithError = igIntNoError;
    igIntWithError.Offset0.word = dwbAddr.word0;
    igIntWithError.Offset2.word = dwbAddr.word1;
    dwbAddr.dword = (DWORD)&IntHandlerExt;
    igExternal = igIntWithError;
    igExternal.Offset0.word = dwbAddr.word0;
    igExternal.Offset2.word = dwbAddr.word1;

    // interrupt map:
    // 00..1F   processor
    // 20..2F   external
    // 30..FF   software
    DWORD i;
    pIdt = (SITGD*)MEM_IDT_BASE;
    // set cpu exceptions to dead int.gate
    for(i = 0x00; i < 0x20; i++)
    {
        memcpy(&pIdt[i], &igDeath, sizeof(SITGD));
    }
    for(i = 0x20; i < 0x30; i++)
    {
        memcpy(&pIdt[i], &igExternal, sizeof(SITGD));
    }
    for(i = 0x30; i < 0x100; i++)
    {
        memcpy(&pIdt[i], &igIntNoError, sizeof(SITGD));
    }
    kPicInit(0x20, 0x28);

    BYTE* pIdtr32;
    BYTE* pIdtr16;
    pBootData->bt_idt32_limit = MEM_IDT_LIMIT;
    pBootData->bt_idt32_base = MEM_IDT_BASE;
    pIdtr32 = (BYTE*) &pBootData->bt_idt32_limit;
    pIdtr16 = (BYTE*) &pBootData->bt_idt16_limit;
    // interrupts should be disabled
    hwSwitchIdtr(pIdtr16, pIdtr32);
    // interrupts can be enabled

    KSTR_VAR(ksBuf, 256);
    KSTR ksFmt;

    KSTR_SET(ksFmt, "IDTR: old[%08X]=%016X:%04X, new[%08X]=%016X:%04X\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pIdtr16), 
        AQWORD(pBootData->bt_idt16_base), 
        AWORD(pBootData->bt_idt16_limit), 
        ADWORD(pIdtr32), 
        AQWORD(pBootData->bt_idt32_base), 
        AWORD(pBootData->bt_idt32_limit));
    kdSendKString(&ksBuf);

    hwEnableInterrupts();

    KSTR_SET(ksFmt, "Interrupts are enabled!\r\n");
    kdSendKString(&ksFmt);

    /*DWBYTE dwbOffset;
    SITGD* pigd;
    pigd = pIdt;
    ptr = buf;
    ptr += aszcpy(ptr, "idt entries (first 64):\r\n");
    kdSendStr((BYTE*)buf);
    for(i = 0; i < 64; i++)
    {
        ptr = buf;
        ptr += aszcpy(ptr, "    [");
        dwbOffset.word0 = pigd->Offset0.word;
        dwbOffset.word1 = pigd->Offset2.word;
        ptr += aszIntToStr(ptr, i, 2, 16, TRUE, FALSE);
        ptr += aszcpy(ptr, "] ");
        ptr += aszIntToStr(ptr, pigd->SegSelector.Selector, 4, 16, TRUE, FALSE);
        ptr += aszcpy(ptr, ":");
        ptr += aszIntToStr(ptr, dwbOffset.dword, 8, 16, TRUE, FALSE);
        ptr += aszcpy(ptr, " ");
        ptr += aszIntToStr(ptr, pigd->Attr0, 2, 16, TRUE, FALSE);
        ptr += aszcpy(ptr, "\r\n");
        kdSendStr((BYTE*)buf);
        pigd++;
    }*/
}


void getTime(void)
{
    RTC_DATETIME rtd;

    kRtcGetDateTime(&rtd);

    //print time/date
    KSTR_VAR(ksBuf,128);
    KSTR ksFmt;
    KSTR ksWeekDays[7] = {KSTR_INIT("SUN"), KSTR_INIT("MON"), KSTR_INIT("TUE"), 
        KSTR_INIT("WED"), KSTR_INIT("THU"), KSTR_INIT("FRI"), KSTR_INIT("SAT")};
    KSTR_SET(ksFmt, "Real-Time Clock: %02d:%02d:%02d %s %02d.%02d.%04d\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(rtd.Hour), ABYTE(rtd.Minute), ABYTE(rtd.Second), 
        AKSTR(&ksWeekDays[rtd.WeekDay]), ABYTE(rtd.Day), ABYTE(rtd.Month), AWORD(rtd.Year));
    kdSendKString(&ksBuf);
}



// VBE FUNCTIONS
#define VBE_PREFIX          0x4F00
#define VBE_GETCTLRINFO     (VBE_PREFIX | 0x00)
#define VBE_GETMODEINFO     (VBE_PREFIX | 0x01)
#define VBE_SETMODE         (VBE_PREFIX | 0x02)
#define VBE_GETCURRMODE     (VBE_PREFIX | 0x03)
#define VBE_SRSTATE         (VBE_PREFIX | 0x04)
#define VBE_WINDOWCTL       (VBE_PREFIX | 0x05)
#define VBE_SCANLINELEN     (VBE_PREFIX | 0x06)
#define VBE_DISPLAYSTART    (VBE_PREFIX | 0x07)
#define VBE_PALETTEFMT      (VBE_PREFIX | 0x08)
#define VBE_PALETTEDATA     (VBE_PREFIX | 0x09)
#define VBE_GETPMIF         (VBE_PREFIX | 0x0A)
#define VBE_PIXELCLOCK      (VBE_PREFIX | 0x0B)

// VBE STATUS (LOW)
#define VBE_SUPPORTED       0x4F
// VBE STATUS (HIGH)
#define VBE_SUCCESSFUL      0x00
#define VBE_FAILED          0x01
#define VBE_NOTSUPPHW       0x02
#define VBE_INVALID         0x03
// VBE STATUS
#define VBE_SUCCESS         ((VBE_SUCCESSFUL << 8) | VBE_SUPPORTED)

// SIGNATURES
#define VBE_SIGN_VESA       0x41534556  // 'VESA'
#define VBE_SIGN_VBE2       0x32454256  // 'VBE2'

// CAPABILITIES
#define VIC_8BITDAC     0x00000001  // DAC width is switchable to 8 bits per primary color
#define VIC_NONVGA      0x00000002  // Controller is not VGA compatible
#define VIC_USEBLANK    0x00000004  // When programming large blocks of information to the RAMDAC, use the blank bit in Function 09h
#define VIC_STEREO      0x00000008  // Hardware stereoscopic signaling supported by controller
#define VIC_EVCSTEREO   0x00000010  // Stereo signaling supported via (=0) external VESA stereo connector / (=1) VESA EVC connector

// VBE CONTROLLER INFORMATION
struct VBEINFO {
    DWBYTE vbeSignature;        // 'VESA'
    WBYTE vbeVersion;           // 0x0300 = 3.0
    DWBYTE vpOemString;         // VbeFarPtr to OEM String
    DWORD capabilities;         // Capabilities of graphics controller (VIC_*)
    DWBYTE vpVideoModes;        // VbeFarPtr to VideoModeList
    WORD totalMemory;           // Number of 64kb memory blocks
    // VBE 2.0+
    WBYTE oemSoftwareRev;       // Implementation Software revision
    DWBYTE oemVendorName;       // VbeFarPtr to Vendor Name String
    DWBYTE oemProductName;      // VbeFarPtr to Product Name String
    DWBYTE oemProductRev;       // VbeFarPtr to Product Revision String
    BYTE reserved[222];         // Reserved for VBE implementation scratch area
    BYTE oemData[256];          // Data Area for OEM Strings
};

// VBE VIDEO MODE ATTRUBUTES
#define VMA_HWSUPPORT       0x0001  // Mode supported in hardware
/////// reserved            0x0002
#define VMA_TTYSUPPORT      0x0004  // TTY Output functions supported by BIOS
#define VMA_COLOR           0x0008  // Color mode (=0: Monochrome mode)
#define VMA_GRAPHICS        0x0010  // Graphics mode (=0: Text mode)
#define VMA_NONVGA          0x0020  // =0: VGA compatible mode
#define VMA_NONVGAWINMM     0x0040  // =0: VGA compatible windowed memory mode is available
#define VMA_LINEARFB        0x0080  // Linear frame buffer mode is available
#define VMA_DOUBLESCAN      0x0100  // Double scan mode is available
#define VMA_INTERLACED      0x0200  // Interlaced mode is available
#define VMA_HWTRIPLEBUF     0x0400  // Hardware triple buffering support
#define VMA_HWSTEREO        0x0800  // Hardware stereoscopic display support
#define VMA_DUALDISPSTART   0x1000  // Dual display start address support
/////// reserved            0x2000
/////// reserved            0x4000
/////// reserved            0x8000

// VBE VIDEO MODE WINDOW ATTRIBUTES
#define VMWA_RELOCATABLE    0x01    // Relocatable window(s) are supported
#define VMWA_READABLE       0x02    // Window is readable
#define VMWA_WRITEABLE      0x04    // Window is writeable
/////// reserved            0x08..0x80

// VBE VIDEO MODE MEMORY MODEL
#define VMEM_TEXT           0x00    // Text mode
#define VMEM_CGA            0x01    // CGA graphics
#define VMEM_HERCULES       0x02    // Hercules graphics
#define VMEM_PLANAR         0x03    // Planar
#define VMEM_PACKEDPIXEL    0x04    // Packed pixel
#define VMEM_NONCHAIN4      0x05    // Non-chain 4, 256 color
#define VMEM_DIRECTCOLOR    0x06    // Direct Color
#define VMEM_YUV            0x07    // YUV
/////// reserved            0x08..0x0F
/////// oem-defined         0x10..0xFF

// VBE VIDEO MODE DIRECT COLOR MODE INFO
#define VMDI_COLORAMPPROG   0x01    // Color ramp is programmable (=0: fixed)
#define VMDI_RESVUSABLE     0x02    // Bits in Resv field are usable by the application

// VBE VIDEO MODE INFO
struct VBEMODEINFO {
    WORD modeAttribs;           //00 mode attributes (VMA_*)
    BYTE winAAttribs;           //02 window A attributes (VMWA_*)
    BYTE winBAttribs;           //03 window B attributes (VMWA_*)
    WORD winGranularity;        //04 window granularity
    WORD winSize;               //06 window size
    WORD winASegment;           //08 window A start segment
    WORD winBSegment;           //0a window B start segment
    DWBYTE winFunc;             //0c real mode pointer to window function
    WORD bytesPerScanline;      //10 bytes per scan line
    // VBE 1.2+
    WORD resolutionX;           //12 horizontal resolution in pixels (graphics modes) or characters (text modes)
    WORD resolutionY;           //14 vertical resolution in pixels (graphics modes) or characters (text modes)
    BYTE charSizeX;             //16 character cell width in pixels
    BYTE charSizeY;             //17 character cell height in pixels
    BYTE numPlanes;             //18 number of memory planes
    BYTE bitsPerPixel;          //19 bits per pixel
    BYTE numBanks;              //1a number of banks
    BYTE memoryModel;           //1b memory model type (VMEM_*)
    BYTE bankSize;              //1c bank size in KB
    BYTE numImagePages;         //1d number of images
    BYTE reserved1E;            //1e reserved for page function
    // Direct Color fields (required for direct(6) and YUV(7) memory models)
    BYTE dcRedMaskSize;         //1f size of direct color red mask in bits
    BYTE dcRedFieldPos;         //20 bit position of lsb of red mask
    BYTE dcGreenMaskSize;       //21 size of direct color green mask in bits
    BYTE dcGreenFieldPos;       //22 bit position of lsb of green mask
    BYTE dcBlueMaskSize;        //23 size of direct color blue mask in bits
    BYTE dcBlueFieldPos;        //24 bit position of lsb of blue mask
    BYTE dcResvMaskSize;        //25 size of direct color reserved mask in bits
    BYTE dcResvFieldPos;        //26 bit position of lsb of reserved mask
    BYTE dcModeInfo;            //27 direct color mode attributes
    // VBE 2.0+
    DWORD linearFrameBuffer;    //28 physical address for flat memory frame buffer
    DWORD reserved2C;           //2c Reserved - always set to 0
    WORD reserved30;            //30 Reserved - always set to 0
    // VBE 3.0+
    WORD lineBytesPerScanline;  //32 bytes per scan line for linear modes
    BYTE bankNumImagePages;     //34 number of images for banked modes
    BYTE lineNumImagePages;     //35 number of images for linear modes
    BYTE lineRedMaskSize;       //36 size of direct color red mask (linear modes)
    BYTE lineRedFieldPos;       //37 bit position of lsb of red mask (linear modes)
    BYTE lineGreenMaskSize;     //38 size of direct color green mask (linear modes)
    BYTE lineGreenFieldPos;     //39 bit position of lsb of green mask (linear modes)
    BYTE lineBlueMaskSize;      //3a size of direct color blue mask (linear modes)
    BYTE lineBlueFieldPos;      //3b bit position of lsb of blue mask (linear modes)
    BYTE lineResvMaskSize;      //3c size of direct color reserved mask (linear modes)
    BYTE lineResvFieldPos;      //3d bit position of lsb of reserved mask (linear modes)
    DWORD maxPixelClock;        //3e maximum pixel clock (in Hz) for graphics mode
    BYTE reserved42[189];       //42 remainder of ModeInfoBlock
};

// VBE VIDEO MODE NUMBER
/////// mode number         0x07FF  // 0x1FF: Mode number; 0x600: Reserved (must be 0)
#define VMN_USECRTC         0x0800  // Use user specified CRTC values for refresh rate
/////// reserved for af     0x3000  // Reserved for VBE/AF (must be 0)
#define VMN_USELINEARFB     0x4000  // Use linear/flat frame buffer model
#define VMN_PRESERVEMEM     0x8000  // Don't clear display memory


struct DISPLAYBUFFER {
    BOOL bValid;
    DWORD* pBuffer;
    DWORD width;
    DWORD height;
};


void initVesa(DISPLAYBUFFER* pDisplay)
{
    pDisplay->bValid = FALSE;

    VBEINFO* pvi;
    pvi = (VBEINFO*) MEM_VBE_BASE;
    pvi->vbeSignature.dword = VBE_SIGN_VBE2;
    pBootData->bl_ic_isr = 0x10;
    pBootData->bl_ic_es = 0;
    pBootData->bl_ic_edi = MEM_VBE_BASE;
    pBootData->bl_ic_eax.dword = VBE_GETCTLRINFO;
    pBootData->bl_intcall_addr(pBootData);

    KSTR_VAR(ksBuf, 256);
    KSTR ksFmt;

    KSTR_SET(ksFmt, "VBE: <%04X>\r\n    sign: %c, ver: %02X.%02X, oemstr: %08X,\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(pBootData->bl_ic_eax.word0), 
        ADWORD(pvi->vbeSignature.dword), ABYTE(pvi->vbeVersion.byte1), 
        ABYTE(pvi->vbeVersion.byte0), ADWORD(pvi->vpOemString.dword));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    caps: %08X, modes: %08X, mem: %04X, oemrev: %02X.%02X (%04X),\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pvi->capabilities), ADWORD(pvi->vpVideoModes.dword), 
        AWORD(pvi->totalMemory), ABYTE(pvi->oemSoftwareRev.byte1), 
        ABYTE(pvi->oemSoftwareRev.byte0), AWORD(pvi->oemSoftwareRev.word));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    oemname: %08X, oemprod: %08X, oemprodrev: %08X\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pvi->oemVendorName.dword), 
        ADWORD(pvi->oemProductName.dword), ADWORD(pvi->oemProductRev.dword));
    kdSendKString(&ksBuf);

    BOOL bDone;
    DWORD iMode;
    WORD* pwMode;
    WORD wMode;
    WORD modeSelected;
    VBEMODEINFO* pvmi = (VBEMODEINFO*) MEM_VBEMODE_BASE;
    modeSelected = 0;
    if(pBootData->bl_ic_eax.word0 == VBE_SUCCESS)
    {
        bDone = FALSE;
        pwMode = (WORD*) ((pvi->vpVideoModes.word1 << 4) + pvi->vpVideoModes.word0);
        iMode = 0;

        KSTR_SET(ksFmt, "video modes:\r\n");
        kdSendKString(&ksFmt);

        while(bDone == FALSE)
        {
            wMode = *pwMode;
            KSTR_SET(ksFmt, "    (%2d) %04X");
            kSprintf(&ksBuf, &ksFmt, ADWORD(iMode), AWORD(wMode));
            kdSendKString(&ksBuf);
            if(wMode == 0xFFFF)
            {
                kdSendLf();
                bDone = TRUE;
                break;
            }

            pBootData->bl_ic_es = 0;
            pBootData->bl_ic_edi = MEM_VBEMODE_BASE;
            pBootData->bl_ic_eax.dword = VBE_GETMODEINFO;
            pBootData->bl_ic_ecx.dword = wMode;
            pBootData->bl_intcall_addr(pBootData);

            if((pvmi->resolutionX >= 800) && (pvmi->resolutionX <= 1280)
                && (pvmi->resolutionY >= 600) && (pvmi->resolutionY <= 768)
                && ((pvmi->modeAttribs & VMA_LINEARFB) != 0)
                && (pvmi->bitsPerPixel == 32))
            {
                modeSelected = wMode;
                pDisplay->pBuffer = (DWORD*) pvmi->linearFrameBuffer;
                pDisplay->width = pvmi->resolutionX;
                pDisplay->height = pvmi->resolutionY;
            }

            KSTR_SET(ksFmt, ":<%04X>  %4dx%4dx%2d %d:%d:%d, lfb: %08X\r\n");
            kSprintf(&ksBuf, &ksFmt, AWORD(pBootData->bl_ic_eax.word0), 
                AWORD(pvmi->resolutionX), AWORD(pvmi->resolutionY), 
                ABYTE(pvmi->bitsPerPixel), ABYTE(pvmi->dcRedMaskSize), 
                ABYTE(pvmi->dcGreenMaskSize), ABYTE(pvmi->dcBlueMaskSize), 
                ADWORD(pvmi->linearFrameBuffer));
            kdSendKString(&ksBuf);

            pwMode++;
            iMode++;
        }

        if(modeSelected != 0)
        {
            pBootData->bl_ic_eax.dword = VBE_SETMODE;
            pBootData->bl_ic_ebx.dword = VMN_USELINEARFB | modeSelected;
            pBootData->bl_intcall_addr(pBootData);

            KSTR_SET(ksFmt, "setvm(%04X): <%04X>  %4dx%4d, lfb: %08X\r\n");
            kSprintf(&ksBuf, &ksFmt, AWORD(modeSelected), 
                AWORD(pBootData->bl_ic_eax.word0), 
                ADWORD(pDisplay->width), ADWORD(pDisplay->height), 
                ADWORD(pDisplay->pBuffer));
            kdSendKString(&ksBuf);

            pDisplay->bValid = TRUE;
        }
    }
}



/*void drawQuad(BYTE* pFrameBuf, DWORD frameWidth, DWORD idx)
{
    DWBYTE clr;
    DWORD row;
    DWORD col;
    clr.byte0 = (BYTE) idx;
    clr.byte1 = (BYTE) idx;
    clr.byte2 = (BYTE) idx;
    clr.byte3 = (BYTE) idx;
    row = frameWidth * ((idx & 0xF8) >> 3) * 4;
    col = (idx & 0x07) * 4;
    pFrameBuf = pFrameBuf + row + col;
    *(DWORD*)pFrameBuf = clr.dword;
    pFrameBuf += frameWidth;
    *(DWORD*)pFrameBuf = clr.dword;
    pFrameBuf += frameWidth;
    *(DWORD*)pFrameBuf = clr.dword;
    pFrameBuf += frameWidth;
    *(DWORD*)pFrameBuf = clr.dword;
}

void drawQuad32rg(DWORD* pFrameBuf, DWORD frameWidth, DWORD x, DWORD ix, DWORD iy)
{
    DWBYTE clr;
    DWORD row;
    DWORD col;
    clr.byte0 = (BYTE) 0;   // b
    clr.byte1 = (BYTE) ix;  // g
    clr.byte2 = (BYTE) iy;  // r
    clr.byte3 = (BYTE) 0;   // a
    row = frameWidth * iy;
    col = x + ix;
    pFrameBuf = pFrameBuf + row + col;
    *pFrameBuf = clr.dword;
}*/

DWORD min256(DWORD a, DWORD b)
{
    if(a < b)
        return a;
    else
        return b;
}

DWORD max256(DWORD a, DWORD b)
{
    if(a > b)
        return a;
    else
        return b;
}

DWORD crop256(DWORD a, DWORD b)
{
    a = a + b;
    if(a > 255)
        return 255;
    else
        return a;
}

DWORD avg256(DWORD a, DWORD b)
{
    a = (a * b) / 256;
    return a;
}

void draw32quad(DISPLAYBUFFER* pDisplay)
{
    // draw 32-bit color quad:
    // yellow - red --- magenta
    // |                      |
    // green    black      blue
    // |                      |
    // cyan --- blue ---- white

    DWORD* pFrameBuf;
    DWORD frameWidth;
    DWBYTE clr;
    DWORD ix;
    DWORD iy;
    DWORD* pPixel;

    pFrameBuf = pDisplay->pBuffer;
    frameWidth = pDisplay->width;

    KSTR_VAR(ksBuf, 256);
    KSTR ksFmt;

    KSTR_SET(ksFmt, "quad: %08X, %d\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pFrameBuf), ADWORD(frameWidth));
    kdSendKString(&ksBuf);

    clr.byte0 = (BYTE) 0;   // b
    clr.byte1 = (BYTE) 0;   // g
    clr.byte2 = (BYTE) 0;   // r
    clr.byte3 = (BYTE) 0;   // a
    for(iy = 0; iy < 256; iy++)
    {
        KSTR_SET(ksFmt, " %3d");
        kSprintf(&ksBuf, &ksFmt, ADWORD(iy));
        kdSendKString(&ksBuf);
        for(ix = 0; ix < 256; ix++)
        {
            // make r-y-g color
            clr.byte0 = (BYTE) 0;   // b
            clr.byte1 = (BYTE) (255 - ix);   // g
            clr.byte2 = (BYTE) (255 - iy);   // r
            pPixel = pFrameBuf + frameWidth * iy + ix;
            *pPixel = clr.dword;
            // make r-m-b color
            clr.byte0 = (BYTE) ix;   // b
            clr.byte1 = (BYTE) 0;   // g
            clr.byte2 = (BYTE) (255 - iy);   // r
            pPixel = pFrameBuf + frameWidth * iy + (ix + 256);
            *pPixel = clr.dword;
            // make g-c-b color
            clr.byte0 = (BYTE) iy;   // b
            clr.byte1 = (BYTE) (255 - ix);   // g
            clr.byte2 = (BYTE) 0;   // r
            pPixel = pFrameBuf + frameWidth * (iy + 256) + ix;
            *pPixel = clr.dword;
            // make b-w-b color
            clr.byte0 = (BYTE) (iy + (ix * (255 - iy) / 256)); //max256(avg256(iy, 255 - ix), avg256(255 - iy, ix));
            clr.byte1 = (BYTE) avg256(ix, iy);
            clr.byte2 = (BYTE) avg256(ix, iy);
            pPixel = pFrameBuf + frameWidth * (iy + 256) + (ix + 256);
            *pPixel = clr.dword;
        }
    }
    kdSendLf();
}





void kMain(BTDATA* pbyBootData)
{
    KSTR_VAR(ksBuf, 128);
    KSTR ksFmt;
    BTDATA* pbtd = pbyBootData;

    BYTE kMsgStart[63] = "Kernel is launched.\r\n";

    kdSendLf();
    kdSendStr(kMsgStart);

    KSTR_SET(ksFmt, "Using unicode strings.\r\n");
    kdSendKString(&ksFmt);

    KSTR_SET(ksFmt, "boot data: \r\n    drive: %02X, kimg: %04X at %04X:%04X (%08X), stack: %04X:%04X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(pbtd->bt_drive), AWORD(pbtd->bt_kern_size), 
        AWORD(pbtd->bt_kern_seg), AWORD(pbtd->bt_kern_entry), ADWORD(pbtd->bt_image_addr), 
        AWORD(pbtd->bt_stk_seg), AWORD(pbtd->bt_stk_ptr));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    image: %08X at %08X, entry: %08X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pbtd->bt_hll_size), 
        ADWORD(pbtd->bt_hll_addr), ADWORD(pbtd->bt_hll_entry));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    intcall: %08X, gdt: %04X at %08X\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pbtd->bl_intcall_addr), 
        AWORD(pbtd->bt_gdt_count), ADWORD(pbtd->bt_gdt32_base));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    16: code: %04X, data: %04X, extra: %04X, stack %04X\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(pbtd->bt_seg16_code), 
        AWORD(pbtd->bt_seg16_data), AWORD(pbtd->bt_seg16_extra), 
        AWORD(pbtd->bt_seg16_stack));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    32: code: %04X, data: %04X, extra: %04X, stack %04X\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(pbtd->bt_seg32_code), 
        AWORD(pbtd->bt_seg32_data), AWORD(pbtd->bt_seg32_extra), 
        AWORD(pbtd->bt_seg32_stack));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    32->16: code: %04X, data: %04X\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(pbtd->bt_seg3216_code), 
        AWORD(pbtd->bt_seg3216_data));
    kdSendKString(&ksBuf);

    DISPLAYBUFFER display;

    setupGdt();
    setupIdt();
    getTime();
    ///initAcpi();
    initVesa(&display);
    if(display.bValid)
    {
        draw32quad(&display);
    }

    //done//
    KSTR_CONST(ksDone, "\r\n\r\nDone.\r\n");
    kdSendKString(&ksDone);
}

void NAKED hllEntry(void)   // EBP = boot data block ptr
{
    //:STARTUP
    ASM {
        mov pBootData, ebp
    }

    //:MAIN
    kMain(pBootData);

    //:EXIT
    hwProcessorHalt();
}


#ifdef RTDEBUG

void rtdEntry(void)     // run-time-debug (win) entry
{
    RTC_DATETIME rtd;
    rtd.Year = 2010;
    rtd.Month = 9;
    rtd.Day = 0;//12;
    rtd.WeekDay = 0;
    rtd.Hour = 4;
    rtd.Minute = 25;
    rtd.Second = 42;
    QWORD rsdpsign;
    rsdpsign = ACPI_RSDP_SIGN;
  //  RSDP rsdp;
  //  rsdp.signature.qword = RSDP_SIGN;
  //  rtdFunc1();
    KSTR_VAR(ksBuf, 15);//127);
    //KSTR_CONST(ksFmt, "S: %c, RTC: %02d.%02d.%04d %02d:%02d:%02d\r\n");
    KSTR ksFmt;
    //KSTR_SET(ksFmt, "S: %c, RTC: %02d.%02d.%04d %02d:%02d:%02d\r\n");
    KSTR_SET(ksFmt, "RTC: %02d.%02d.%04d %02d:%02d:%02d\r\n");
    //kSprintf(&ksBuf, &ksFmt, AQWORD(rsdpsign), 
    kSprintf(&ksBuf, &ksFmt, 
        ABYTE(rtd.Day), ABYTE(rtd.Month), AWORD(rtd.Year), 
        ABYTE(rtd.Hour), ABYTE(rtd.Minute), ABYTE(rtd.Second));

    //done
}

#endif //RTDEBUG


EXTERN_C_END        //} //extern "C"


