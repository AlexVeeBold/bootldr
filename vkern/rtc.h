///////////////////////////////////////////////////////////////
//
// rtc: Real-Time Clock
//
//
//   06.09.2010 13:39 - created
//

#ifndef _RTC_H_INC_
#define _RTC_H_INC_


EXTERN_C_BEGIN


struct RTC_DATETIME {
    BYTE Second;
    BYTE Minute;
    BYTE Hour;          // bit7 =1: PM, =0: AM or 24-hour
    BYTE WeekDay;
    BYTE Day;
    BYTE Month;
    WORD Year;
};


void KPI kRtcGetDateTime(RTC_DATETIME* pRtcDateTime);


EXTERN_C_END


#endif //_RTC_H_INC_
