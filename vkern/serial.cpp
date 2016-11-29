///////////////////////////////////////////////////////////////
//
// serial: Serial port
//
//
//   26.08.2010 00:44 - created
//

#include "kdef.h"
#include "hw.h"
#include "serial.h"


EXTERN_C_BEGIN


#define SERPORT_BASE    0x03F8
#define SERPORT_BAUDL       SERPORT_BASE+0  //DLAB=1
#define SERPORT_BAUDH       SERPORT_BASE+1  //DLAB=1
#define SERPORT_DATA        SERPORT_BASE+0  //DLAB=0
#define SERPORT_INTEN       SERPORT_BASE+1  //DLAB=0
#define SERPORT_INTID       SERPORT_BASE+2
#define SERPORT_LINECTL     SERPORT_BASE+3  //bit7:DLAB (Divisor Latch Access Bit)
#define SERPORT_MDMCTL      SERPORT_BASE+4
#define SERPORT_LINESTAT    SERPORT_BASE+5
#define SERPORT_MDMSTAT     SERPORT_BASE+6
//_BAUDH, _BAUDL: baudrate divisor: 115200/baudrate
// (300 bps: 115200/300 = 384 = 180h: _BAUDH = 01h, _BAUDL = 80h)
//_INTEN: enable interrupts:
// [7:0 0 0 0 3:ModemStatus LineStatus TransferredChar 0:ReceivedChar]
// (modem status: CTS, DSR, RI, DCD)
//_INTID: interrupt identification
// [7:0 0 0 0 0 2,1:IntReason 0:PendingInts]
// IntReason: 11: line error, 10: received char, 01: transferred char, 00: line status changed
//_LINECTL:
// [7:DLAB ForceBreak SetParityBit 4:EvenParity EnableParity StopBits 1,0:DataBits]
// (data bits per chararacter = 5 + DataBits)
//_MDMCTL:
// [7:0 0 0 LoopbackTest 3:OUT2 OUT1 RTS 0:DTR]
//_LINESTAT:
// [7:0 TRsEmpty THRegEmpty LineBreak 3:FramingError ParityError RxBufferFull 0:ReceivedChar]
// (TRsEmpty = TSR & THR are empty; THRegEmpty = THR is empty)
// (TSR = Transmit Shift Register; THR = Transmit Holding Register)
// (data -> THR -> TSR -> serial line)
//_MDMSTAT:
// [7:DCD RI DSR CTS 3:DDCD DRI DDSR 0:DCTS]
// (bits 0-3 are reset when the CPU reads this register)
// (delta bits are indicating line state change since last query)

void KPI kSerialPortTransferWait(void)
{
    BYTE lineStatus;
    do {
        lineStatus = hwPortReadByte(SERPORT_LINESTAT);
    } while((lineStatus & 0x20) == 0);  // wait while THR is not empty
}

void KPI kSerialPortInit(DWORD baudrate)
{
    DWORD frequency = 115200;
    DWORD divisor;
    BYTE divLo, divHi;
    baudrate += frequency * (baudrate == 0);    // if baudrate=0 then baudrate=115200
    divisor = frequency / baudrate;
    divisor += (divisor == 0);                  // if divisor=0 then divisor=1
    divLo = (BYTE)divisor;
    divHi = (BYTE)(divisor >> 8);
    kSerialPortTransferWait();
    // DLAB=on, ForceBreak=off, Parity=off, StopBit=1, CharSize=5bit
    hwPortWriteByte(SERPORT_LINECTL, 0x80);
    // set baudrate divisor (high byte)
    hwPortWriteByte(SERPORT_BAUDH, divHi);
    // set baudrate divisor (low byte)
    hwPortWriteByte(SERPORT_BAUDL, divLo);
    // DLAB=off, ForceBreak=off, Parity=off, StopBit=1, CharSize=8bit
    hwPortWriteByte(SERPORT_LINECTL, 0x03);
    // disable all serial port interrupts
    hwPortWriteByte(SERPORT_INTEN, 0x00);
}

void KPI kSerialPortTransferByte(BYTE byData)
{
    kSerialPortTransferWait();
    hwPortWriteByte(SERPORT_DATA, byData);
}

void KPI kSerialPortTransferBuffer(BYTE* byBuffer, DWORD dwBufferLength)
{
    BYTE byValue;
    int remain;
    remain = (int)dwBufferLength;
    do {
        kSerialPortTransferWait();
        byValue = *byBuffer;
        byBuffer++;
        remain--;
        hwPortWriteByte(SERPORT_DATA, byValue);
    } while(remain > 0);
}


EXTERN_C_END

