///////////////////////////////////////////////////////////////
//
// hw: Hardware abstration library
//
//
//   26.08.2010 00:35 - created
//   18.10.2010 09:22 - added hwSwitchGdtr/hwSwitchIdtr
//

#include "kdef.h"
#include "hw.h"


EXTERN_C_BEGIN


BYTE NAKED KPI hwPortReadByte(WORD port)
{
    ASM {
        xor eax, eax        // clean high bits
        mov dx, [esp + 4]   // dx = port
        in al, dx           // receive byte
        retn 4              // remove 1 dword from stack
    }
}

void NAKED KPI hwPortWriteByte(WORD port, BYTE byte)
{
    ASM {
        mov dx, [esp + 4]   // dx = port
        mov al, [esp + 8]   // al = byte
        out dx, al          // send byte
        retn 8              // remove 2 dwords from stack
    }
}

void NAKED KPI hwProcessorHalt(void)
{
    ASM {
noreturn:
        //hlt
        //int 3
        xor eax, eax    // do-nothing
        jmp noreturn
    }
    // no return
}

void NAKED KPI hwDisableInterrupts(void)
{
    ASM {
        cli
        ret
    }
}

void NAKED KPI hwEnableInterrupts(void)
{
    ASM {
        sti
        ret
    }
}

void NAKED KPI hwSwitchGdtr(BYTE* pOldGdtr, BYTE* pNewGdtr)
{
    ASM {
        mov ecx, [esp + 4]  // ecx = pOldGdtr
        mov edx, [esp + 8]  // edx = pNewGdtr
        sgdt [ecx]          // store old gdtr
        lgdt [edx]          // load new gdtr
        retn 8              // remove 2 dwords from stack
    }
}

void NAKED KPI hwSwitchIdtr(BYTE* pOldIdtr, BYTE* pNewIdtr)
{
    ASM {
        mov ecx, [esp + 4]  // ecx = pOldIdtr
        mov edx, [esp + 8]  // edx = pNewIdtr
        sidt [ecx]          // store old idtr
        lidt [edx]          // load new idtr
        retn 8              // remove 2 dwords from stack
    }
}


EXTERN_C_END

