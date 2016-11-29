///////////////////////////////////////////////////////////////
//
// rtc: Real-Time Clock
//
//
//   06.09.2010 13:39 - created
//

#include "kdef.h"
#include "hw.h"
#include "rtc.h"


EXTERN_C_BEGIN


#define RTC_ALARM_ANY       0xC0    // 11****** -> any sec|min|hour will match
#define RTC_HOUR_PM         0x80    // pm bit (binary and bcd)
// rtc addresses
#define RTC_CURRENT_SECOND  0x00    // 0..59
#define RTC_ALARM_SECOND    0x01    // second or RTC_ALARM_ANY for any second
#define RTC_CURRENT_MINUTE  0x02    // 0..59
#define RTC_ALARM_MINUTE    0x03    // minute or RTC_ALARM_ANY for any minute
#define RTC_CURRENT_HOUR    0x04    // 1..12 / 0..23
#define RTC_ALARM_HOUR      0x05    // hour or RTC_ALARM_ANY for any hour
#define RTC_CURRENT_WEEKDAY 0x06    // 1..7, 1 = sunday
#define RTC_CURRENT_DAY     0x07    // 1..31
#define RTC_CURRENT_MONTH   0x08    // 1..12
#define RTC_CURRENT_YEAR    0x09    // last two digits
#define RTC_REGISTER_A      0x0A    // r/w, except bit7(uip)
#define RTC_REGISTER_B      0x0B    // r/w
#define RTC_REGISTER_C      0x0C    // r
#define RTC_REGISTER_D      0x0D    // r
// extension addresses
#define RTC_CENTURY         0x32
#define RTC_CENTURY_PS2     0x37    // ?  mca?
// ports
#define RTC_PORT_ADDR       0x70
#define RTC_PORT_DATA       0x71

/* refer to mc146818a specification for details
_REGISTER_A: (UIP is read-only)
7:UIP 6-4:DV 3-0:RS
UIP: =0:date/time can be read, =1:update in progress
DV: divider control: 32768 Hz, 1048576 Hz, 4194304 Hz
RS: rate select for square wave & periodic interrupt
  rate = 32768 / (2 << rs)      rs = 1000(8), rate = 256 Hz
  rs = 0000 -> rate = none 
  32768 Hz: rs = 0001,0010 -> rate = 256,128 Hz

_REGISTER_B:
7:SET PIE 5:AIE UIE SQWE 2:DM 24/12 0:DSE
SET: =0: normal mode, =1: software is initializing date/time
PIE: =0: disable, =1: enable periodic interrupt
AIE: =0: disable, =1: enable alarm interrupt
UIE: =0: disable, =1: enable update-ended interrupt
SQWE: =0: disable, =1: enable square-wave signal on SQW pin
DM: data mode: =0: binary-coded-decimal, =1: binary data
24/12: =0: 12-hour mode (hour.bit7=0: am, =1: pm), =1: 24-hour mode
DSE: =0: disable, =1: enable daylight savings
  last sunday in april: 01:59:59 -> 03:00:00 (am)
  last sunday in october: 01:59:59 -> 01:00:00 (am)

_REGISTER_C: (read-only) (irq source detection)
7:IRQF PF 5:AF UF 3-0:0
IRQF: interrupt request flag, =1 if PF=1 or AF=1 or UF=1
PF: periodic interrupt flag, =1 if periodic int. is requested
AF: alarm flag, =1 if current time matches alarm time
UF: update-ended flag, =1 if update cycle is finished

_REGISTER_D: (read only)
7:VRT 6-0:0
VRT: =0: power sense pin is low, =1: ram and time are valid
*/

union RTCREGA {     // rtc register a
    struct {
        BYTE RateSelect:4;  // bits3..0: rate select for square wave & periodic interrupt
        BYTE Divider:3;     // bits6..4: divider control
        BYTE Updating:1;    // bit7 (read-only): =1: date/time update in progress
    };
    BYTE Register;          // BYTE: whole register
};

union RTCREGB {     // rtc register b
    struct {
        BYTE DaylightSaving:1;      // bit0: =1: enable daylight savings
        BYTE Hour24Mode:1;          // bit1: =1: 24-hour mode, =0: 12-hour mode
        BYTE BinaryDataMode:1;      // bit2: =1: binary, =0: binary-coded-decimal
        BYTE EnableSquareWave:1;    // bit3: =1: enable square-wave signal on SQW pin
        BYTE UpdateInterrupt:1;     // bit4: =1: enable update-ended interrupt
        BYTE AlarmInterrupt:1;      // bit5: =1: enable alarm interrupt
        BYTE PeriodicInterrupt:1;   // bit6: =1: enable periodic interrupt
        BYTE SetDateTimeMode:1;     // bit7: =1: do not update time/date
    };
    BYTE Register;          // BYTE: whole register
};

union RTCREGC {    // rtc register c (read-only)
    struct {
        BYTE unused:4;          // =0
        BYTE UpdateFlag:1;      // bit4: =1: update cycle is finished
        BYTE AlarmFlag:1;       // bit5: =1: current time matches alarm time
        BYTE PeriodicFlag:1;    // bit6: =1: periodic interrupt
        BYTE IrqFlag:1;         // bit7: =1: interrupt request (check other bits for int.source)
    };
    BYTE Register;          // BYTE: whole register
};

union RTCREGD {     // rtc register d (read-only)
    struct {
        BYTE unused:7;          // =0
        BYTE ValidRam:1;        // bit7: =1: ram and time are valid
    };
    BYTE Register;          // BYTE: whole register
};


BYTE rtcRead(BYTE byRtcAddress, BOOL* pbTimeUpdated)
{
    RTCREGA regA;
    BYTE byData;
    BOOL bUpdated;
    BOOL bUpdatingNow;
    // if reading real-time clock data...
    if(byRtcAddress <= RTC_CURRENT_YEAR)
    {
        bUpdated = *pbTimeUpdated;  // i am private function, caller is responsible for crash
        // ...wait while (if) rtc is performing date/time update
        do {
            hwPortWriteByte(RTC_PORT_ADDR, RTC_REGISTER_A);
            regA.Register = hwPortReadByte(RTC_PORT_DATA);
            bUpdatingNow = (regA.Updating != 0);
            //bUpdated |= !bUpdated & bUpdatingNow;   // set (but not clear) update flag
            bUpdated |= bUpdatingNow;   // set (but not clear) update flag
        } while(bUpdatingNow != FALSE);
        // store update flag
        *pbTimeUpdated = bUpdated;  // i am private function, caller is responsible for trash
    }
    // read requested data
    hwPortWriteByte(RTC_PORT_ADDR, byRtcAddress);
    byData = hwPortReadByte(RTC_PORT_DATA);
    return byData;
}

INLINE void BinFromBcd(BYTE* pByte)
{
    // if you passed me value greater than 0x99, it's YOUR problems
    *pByte = (((*pByte & 0xF0) >> 4) * 10) + (*pByte & 0xF);
}

void KPI kRtcGetDateTime(RTC_DATETIME* pRtcDateTime)
{
    BYTE byData[8];     // century,year,month,day,hour,minute,second
    RTCREGB regB;
    BYTE byHourPM;
    BOOL bTimeUpdated;
    DWORD i;
    byHourPM = 0;
    // read settings
    regB.Register = rtcRead(RTC_REGISTER_B, NULL);
    // read time & date (no, you're wrong - it's not an endless loop)
    while(true)
    {
        bTimeUpdated = FALSE;   // forgot this & spend one day (15.09.2010) to find 
                                // wtf it hangs after kSprintf in setupIdt (prev.func) 
        byData[7] = rtcRead(RTC_CURRENT_SECOND, &bTimeUpdated);
        byData[6] = rtcRead(RTC_CURRENT_MINUTE, &bTimeUpdated);
        byData[5] = rtcRead(RTC_CURRENT_HOUR, &bTimeUpdated);
        byData[4] = rtcRead(RTC_CURRENT_WEEKDAY, &bTimeUpdated);
        if(bTimeUpdated != FALSE) continue;     // time changed: read all again
        byData[3] = rtcRead(RTC_CURRENT_DAY, &bTimeUpdated);
        byData[2] = rtcRead(RTC_CURRENT_MONTH, &bTimeUpdated);
        byData[1] = rtcRead(RTC_CURRENT_YEAR, &bTimeUpdated);
        // century is not in rtc block. possible Y2100 problem?
        byData[0] = rtcRead(RTC_CENTURY, NULL);
        if(bTimeUpdated != FALSE) continue;     // time changed: read all again
        break;  // one-pass loop (in best case)
    }
    // remove hour pm bit (if any)
    if(regB.Hour24Mode == 0)
    {
        byHourPM = byData[5] & 0x80;
        byData[5] &= 0x7F;
    }
    // convert from bcd (if any)
    if(regB.BinaryDataMode == 0)    // if data is in bcd format, convert to binary
    {
        for(i = 1; i < sizeof(byData); i++)
        {
            BinFromBcd(&byData[i]);
        }
    }
    BinFromBcd(&byData[0]);     // is century always in BCD?
    // repack data into passed structure (blame yourself for null-pointers etc)
    pRtcDateTime->Second = byData[7];
    pRtcDateTime->Minute = byData[6];
    pRtcDateTime->Hour = byData[5] | byHourPM;
    pRtcDateTime->WeekDay = byData[4];
    pRtcDateTime->Day = byData[3];
    pRtcDateTime->Month = byData[2];
    pRtcDateTime->Year = (byData[0] * 100) + byData[1];
}


EXTERN_C_END

