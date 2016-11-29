///////////////////////////////////////////////////////////////
//
// pic: Programmable Interrupt Controller (Intel 8259A)
//
//
//   06.09.2010 13:15 - created
//

#include "kdef.h"
#include "hw.h"
#include "pic.h"


EXTERN_C_BEGIN


/*
DEVICE(*)->IRR(*)->IMR(*)->PRR(*)->ISR(*)->INT(1)
       *=multi-line, 1=single-line, IRR=interrupt request register
       IMR=interrupt mask register, PRR=priority resolver, 
       ISR=interrupt state register, INT=processor interrupt pin

Initialization Command Words command sequence:
icw1, icw2, [icw3], [icw4]
0x20  0x21   0x21   0x21
Operation Command Words can be written anytime after initialization

BIOS Initialization Sequence:
0x11->0x20,     primary pic: full sequence
0x08->0x21,     int. vector base = 0x08
0x04->0x21,     secondary pic at pin 2
0x01->0x21,     80x86 mode
0x11->0xA0,     secondary pic: full sequence
0x70->0xA1,     int. vector base = 0x70
0x02->0xA1,     master's pin = 2
0x01->0xA1,     80x86 mode
0xB8->0x21,     primary pic: enable IRQ 0, 1, 2, 6 (mask=10111000)
0xBD->0xA1.     (YUROV)secondary pic: enable IRQ 1(9), 6(14) (mask=10111101)
0x8F->0xA1.     (VBOX)secondary pic: enable IRQ 4(12), 5(13), 6(14) (mask=10111101)

icw1 -> 0x20
7-5:0 4:1 LTIM 2:=0?unused? MODE 0:ICW4
LTIM: =0:edge triggered mode, =1:level triggered mode
MODE: =0:cascade (icw3 in sequence), =1:single (no icw3)
ICW4: =0:no icw4, =1:icw4 in sequence

icw2 -> 0x21
7-0: int. vector base (2-0: always =0)

icw3 -> 0x21
master:
7-0: =0: ext.device pin, =1: slave pin
slave:
7-4:0 3-0: master's pin for slave

icw4 -> 0x21
7-5:0 4:SFNM BUF 2:M/S AEOI 0:PM
SFNM: =0: not s.f.n.m., =1: special fully nested mode
BUF: =0: non-buffered mode, =1: buffered mode
M/S: =0: buffered mode slave, =1: buffered mode master
AEOI: =0: normal end of interrupt, =1: auto eoi
PM: =0: 8080 processor, =1: 80x86 processor

ocw1 -> 0x21  (IMR)
7-0: =0: enable int. level N, =1: disable int. level N
can be read from same port

ocw2 -> 0x20
7:ROT SP 5:EOI 4-3:OCW2 2-0:IRL
OCW2: = 0 0
ROT,SP,EOI:
0 0 1: non-specific eoi
0 1 1: specific eoi (with irq level)
0 0 0: normal auto-eoi mode
1 0 0: rotating auto-eoi mode
1 0 1: rotate on non-specific eoi
1 1 1: rotate on specific eoi (with irq level)
IRL: irq level (for some operations)

ocw3 -> 0x20
7:0 ESMM SMM 4-3:OCW3 POLL RR 0:RIS
OCW3: = 0 1
ESMM,SMM:
0 0, 0 1: do nothing
1 0: reset special mask mode
1 1: set special mask mode
POLL: =0: no poll, =1: cpu->pic polling
RR,RIS:
0 0, 0 1: do-nothing
1 0: read interrupt request register (next port_0x20 read)
1 1: read interrupt state register (next port_0x20 read)
*/

#define PIC0_PORT_ICW1  0x20    // port for icw1
#define PIC0_PORT_ICWN  0x21    // port for icw2, icw3, icw4
#define PIC0_PORT_OCW1  0x21    // port for ocw1 (read/write IMR)
#define PIC0_PORT_OCWN  0x20    // port for ocw2, ocw3
#define PIC0_PORT_READ  0x20    // port for reading IRR, ISR
#define PIC1_PORT_ICW1  0xA0    // port for icw1
#define PIC1_PORT_ICWN  0xA1    // port for icw2, icw3, icw4
#define PIC1_PORT_OCW1  0xA1    // port for ocw1 (read/write IMR)
#define PIC1_PORT_OCWN  0xA0    // port for ocw2, ocw3
#define PIC1_PORT_READ  0xA0    // port for reading IRR, ISR
#define PICX_ICW1       0x11    // any pic: edge triggered mode, full init sequence
// PIC*_ICW2 = interrupt vector base
#define PIC0_ICW3       0x04    // master: slave at pin 2
#define PIC1_ICW3       0x02    // slave: master's pin = 2
#define PICX_ICW4       0x01    // any pic: not buffered, normal eoi, 80x86
#define PIC_OCW2_EOI        0x20    // write to PORT_OCWN
#define PIC_OCW3_READIRR    0x0A    // write to PORT_OCWN
#define PIC_OCW3_READISR    0x0B    // write to PORT_OCWN


void KPI kPicInit(BYTE primaryBaseVector, BYTE secondaryBaseVector)
{
    primaryBaseVector &= 0xF8;    // clear low 3 bits
    secondaryBaseVector &= 0xF8;  // clear low 3 bits
    hwPortWriteByte(PIC0_PORT_ICW1, PICX_ICW1);
    hwPortWriteByte(PIC1_PORT_ICW1, PICX_ICW1);
    hwPortWriteByte(PIC0_PORT_ICWN, primaryBaseVector);
    hwPortWriteByte(PIC1_PORT_ICWN, secondaryBaseVector);
    hwPortWriteByte(PIC0_PORT_ICWN, PIC0_ICW3);
    hwPortWriteByte(PIC1_PORT_ICWN, PIC1_ICW3);
    hwPortWriteByte(PIC0_PORT_ICWN, PICX_ICW4);
    hwPortWriteByte(PIC1_PORT_ICWN, PICX_ICW4);
}

void KPI kPicClearBoth(void)
{
    hwPortWriteByte(PIC0_PORT_OCWN, PIC_OCW3_READISR);
    if(hwPortReadByte(PIC0_PORT_READ) != 0) // are primary pic interrupt bit(s) set?
    {
        hwPortWriteByte(PIC1_PORT_OCWN, PIC_OCW3_READISR);
        if(hwPortReadByte(PIC1_PORT_READ) != 0) // are secondary pic interrupt bit(s) set?
        {
            hwPortWriteByte(PIC1_PORT_OCWN, PIC_OCW2_EOI);  // clear ints on secondary pic
        }
        hwPortWriteByte(PIC0_PORT_OCWN, PIC_OCW2_EOI);  // clear ints on primary pic
    }
}


EXTERN_C_END

