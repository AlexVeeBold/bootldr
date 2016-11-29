///////////////////////////////////////////////////////////////
//
// acpi: Advanced Configuration and Power Interface
//
//
//   18.09.2010 21:20 - created
//

#include "ktypes.h"
#include "kstring.h"
#include "tprintf.h"
#include "hw.h"
#include "acpi.h"


EXTERN_C_BEGIN


void KPI kdSendKString(KSTR* pkString);


void amlGetPackageLength(BYTE* aml)
{
    // Package length encoding:
    // len <= 0x0000003F ( 6 bits, 1 byte ): <00 len0-5>
    // len <= 0x00000FFF (12 bits, 2 bytes): <01 00 len0-3> <len4-11>
    // len <= 0x000FFFFF (20 bits, 3 bytes): <10 00 len0-3> <len4-11> <len12-19>
    // len <= 0x0FFFFFFF (28 bits, 4 bytes): <11 00 len0-3> <len4-11> <len12-19> <len20-27>
    DWBYTE pkgLen;
    DWORD byteCount;
    DWORD byteIndex;
    BYTE lowByteMask;

    pkgLen.dword = 0;
    lowByteMask = 0x3F;
    byteCount = (aml[0] & 0xC0) >> 6;
    if(byteCount != 0)
    {
        lowByteMask = 0x0F;
        for(byteIndex = 1; byteIndex < byteCount; byteIndex++)
        {
            pkgLen.byte[byteIndex] = aml[byteIndex];
        }
        pkgLen.dword >>= 4;
    }
    pkgLen.dword |= aml[0] & lowByteMask;
}



#define RSDP_FIRST      0xE0000
#define RSDP_LAST       0xFFFF0

void printSdtHdr(SDTHDR* phdr)
{
    KSTR_VAR(ksBuf, 255);
    KSTR ksFmt;
    KSTR_SET(ksFmt, "%08X: sign='%c', length=%08X, rev=%02X, check=%02X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(phdr), ADWORD(phdr->signature.dword), 
        ADWORD(phdr->length), ABYTE(phdr->revision), ABYTE(phdr->checksum));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    oemId='%w', oemTbl='%c', oemRev=%02X, crId='%c', crRev=%02X\r\n");
    kSprintf(&ksBuf, &ksFmt, AANSI(&phdr->oemId[0], sizeof(phdr->oemId)), 
        AQWORD(phdr->oemTableId.qword), ABYTE(phdr->oemRevision), 
        ADWORD(phdr->creatorId.dword), ABYTE(phdr->creatorRev));
    kdSendKString(&ksBuf);
}

void printFadt(FADT* pfadt)
{
    KSTR_VAR(ksBuf, 255);
    KSTR ksFmt;
    KSTR_SET(ksFmt, "    facs=%08X, dsdt=%08X, pmProf=%02X, sciVect=%02X,\r\n"
        L"    smiPort=%08X, acpiEn=%02X, acpiDis=%02X, s4Bios=%02X, pstateCnt=%02X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pfadt->facsAddress), ADWORD(pfadt->dsdtAddress), 
        ABYTE(pfadt->preferredPmProfile), AWORD(pfadt->sciIntVector), 
        ADWORD(pfadt->smiCmdPortAddress), ABYTE(pfadt->acpiEnableValue), 
        ABYTE(pfadt->acpiDisableValue), ABYTE(pfadt->s4BiosValue), ABYTE(pfadt->pstateCnt));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    pm1aErb=%08X, pm1bErb=%08X, pm1aCrb=%08X, pm1bCrb=%08X,\r\n"
        L"    pm2Crb=%08X, pmTmrCrb=%08X, gpe0rb=%08X, gpe1rb=%08X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ADWORD(pfadt->pm1aEvtAddress), ADWORD(pfadt->pm1bEvtAddress), 
        ADWORD(pfadt->pm1aCtlAddress), ADWORD(pfadt->pm1bCtlAddress), 
        ADWORD(pfadt->pm2CtlAddress), ADWORD(pfadt->pmTmrCtlAddress), 
        ADWORD(pfadt->gpe0rbAddress), ADWORD(pfadt->gpe1rbAddress));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    pm1EvtLen=%02X, pm1CtlLen=%02X, pm2CtlLen=%02X, pmTmrLen=%02X, gpe0Len=%02X,\r\n"
        L"    gpe1Len=%02X, gpe1Base=%02X, cstCnt=%02X, lvl2Lat=%d, lvl3Lat=%d,\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(pfadt->pm1EventLength), ABYTE(pfadt->pm1ControlLength), 
        ABYTE(pfadt->pm2ControlLength), ABYTE(pfadt->pmTimerLength), 
        ABYTE(pfadt->gpEvent0Length), ABYTE(pfadt->gpEvent1Length), 
        ABYTE(pfadt->gpEvent1Base), ABYTE(pfadt->cstCnt), 
        AWORD(pfadt->level2Latency), AWORD(pfadt->level3Latency));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    flSize=%04X, flStrd=%04X, dutyOffset=%02X, dutyWidth=%02X,\r\n"
        L"    rtcDayAl=%02X, rtcMoAl=%02X, rtcCent=%02X, iaBoot=%04X, fixedFeat=%08X,\r\n");
    kSprintf(&ksBuf, &ksFmt, AWORD(pfadt->flushSize), AWORD(pfadt->flushStride), 
        ABYTE(pfadt->dutyOffset), ABYTE(pfadt->dutyWidth), ABYTE(pfadt->rtcDayAlarmIndex), 
        ABYTE(pfadt->rtcMonthAlarmIndex), ABYTE(pfadt->rtcCenturyIndex), 
        AWORD(pfadt->iapcBootArchFlags), ADWORD(pfadt->fixedFeatFlags));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    resetReg=%02X:%02X:%02X:%02X:%016X, resetVal=%02X,\r\n"
        L"    facsExt=%016X, dsdtExt=%016X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(pfadt->resetRegAddressGA.addrSpaceId), 
        ABYTE(pfadt->resetRegAddressGA.regBitWidth), 
        ABYTE(pfadt->resetRegAddressGA.regBitOffset), 
        ABYTE(pfadt->resetRegAddressGA.accessSize), 
        AQWORD(pfadt->resetRegAddressGA.address.system.qword), 
        ABYTE(pfadt->resetValue), AQWORD(pfadt->facsAddressExt), 
        AQWORD(pfadt->dsdtAddressExt));
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    pm1aEvtEx=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    pm1bEvtEx=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    pm1aCtlEx=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    pm1bCtlEx=%02X:%02X:%02X:%02X:%016X,\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(pfadt->pm1aEvtAddressGA.addrSpaceId), 
        ABYTE(pfadt->pm1aEvtAddressGA.regBitWidth), 
        ABYTE(pfadt->pm1aEvtAddressGA.regBitOffset), 
        ABYTE(pfadt->pm1aEvtAddressGA.accessSize), 
        AQWORD(pfadt->pm1aEvtAddressGA.address.system.qword), 
        ABYTE(pfadt->pm1bEvtAddressGA.addrSpaceId), 
        ABYTE(pfadt->pm1bEvtAddressGA.regBitWidth), 
        ABYTE(pfadt->pm1bEvtAddressGA.regBitOffset), 
        ABYTE(pfadt->pm1bEvtAddressGA.accessSize), 
        AQWORD(pfadt->pm1bEvtAddressGA.address.system.qword), 
        ABYTE(pfadt->pm1aCtlAddressGA.addrSpaceId), 
        ABYTE(pfadt->pm1aCtlAddressGA.regBitWidth), 
        ABYTE(pfadt->pm1aCtlAddressGA.regBitOffset), 
        ABYTE(pfadt->pm1aCtlAddressGA.accessSize), 
        AQWORD(pfadt->pm1aCtlAddressGA.address.system.qword), 
        ABYTE(pfadt->pm1bCtlAddressGA.addrSpaceId), 
        ABYTE(pfadt->pm1bCtlAddressGA.regBitWidth), 
        ABYTE(pfadt->pm1bCtlAddressGA.regBitOffset), 
        ABYTE(pfadt->pm1bCtlAddressGA.accessSize), 
        AQWORD(pfadt->pm1bCtlAddressGA.address.system.qword)
        );
/*
    GAS pm2CtlAddressEx;    // extended system port address of the PM2 Control Register Block
    GAS pmTmrCtlAddressEx;  // extended system port address of the Power Management Timer Control Register Block
    GAS gpe0rbAddressEx;    // extended system port address of General-Purpose Event 0 Register Block
    GAS gpe1rbAddressEx;    // extended system port address of General-Purpose Event 1 Register Block
    */
    kdSendKString(&ksBuf);
    KSTR_SET(ksFmt, "    pm2CtlEx=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    pmTmrCtlEx=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    gpe0Ex=%02X:%02X:%02X:%02X:%016X,\r\n"
        L"    gpe1Ex=%02X:%02X:%02X:%02X:%016X\r\n");
    kSprintf(&ksBuf, &ksFmt, ABYTE(pfadt->pm2CtlAddressGA.addrSpaceId), 
        ABYTE(pfadt->pm2CtlAddressGA.regBitWidth), 
        ABYTE(pfadt->pm2CtlAddressGA.regBitOffset), 
        ABYTE(pfadt->pm2CtlAddressGA.accessSize), 
        AQWORD(pfadt->pm2CtlAddressGA.address.system.qword), 
        ABYTE(pfadt->pmTmrCtlAddressGA.addrSpaceId), 
        ABYTE(pfadt->pmTmrCtlAddressGA.regBitWidth), 
        ABYTE(pfadt->pmTmrCtlAddressGA.regBitOffset), 
        ABYTE(pfadt->pmTmrCtlAddressGA.accessSize), 
        AQWORD(pfadt->pmTmrCtlAddressGA.address.system.qword), 
        ABYTE(pfadt->gpe0rbAddressGA.addrSpaceId), 
        ABYTE(pfadt->gpe0rbAddressGA.regBitWidth), 
        ABYTE(pfadt->gpe0rbAddressGA.regBitOffset), 
        ABYTE(pfadt->gpe0rbAddressGA.accessSize), 
        AQWORD(pfadt->gpe0rbAddressGA.address.system.qword), 
        ABYTE(pfadt->gpe1rbAddressGA.addrSpaceId), 
        ABYTE(pfadt->gpe1rbAddressGA.regBitWidth), 
        ABYTE(pfadt->gpe1rbAddressGA.regBitOffset), 
        ABYTE(pfadt->gpe1rbAddressGA.accessSize), 
        AQWORD(pfadt->gpe1rbAddressGA.address.system.qword)
        );
    kdSendKString(&ksBuf);
}

void initAcpi(void)
{
    BYTE* ptr;
    RSDPTR* prsdp;
    BOOL bFound;
    KSTR ksMsg;
    bFound = FALSE;
    KSTR_SET(ksMsg, "Searching RSDPtr...\r\n");
    kdSendKString(&ksMsg);
    ptr = (BYTE*)RSDP_FIRST;
    do {
        prsdp = (RSDPTR*) ptr;
        if(prsdp->signature.qword == ACPI_RSDP_SIGN)
        {
            bFound = TRUE;
            break;
        }
        ptr += 16;
    } while(ptr <= (BYTE*)RSDP_LAST);
    KSTR_VAR(ksBuf, 255);
    KSTR ksFmt;
    DWORD i;
    DWORD numEntries;
    DWORD* prsdtEntry;
    QWORD* pxsdtEntry;
    RSDT* prsdt;
    XSDT* pxsdt;
    DWORD* psign;
    if(bFound != FALSE)
    {
        KSTR_SET(ksFmt, "Found RSDPtr (at %08X):\r\n");
        kSprintf(&ksBuf, &ksFmt, ADWORD(ptr));
        kdSendKString(&ksBuf);
        KSTR_SET(ksFmt, "    sign='%c', check=%02X, oem='%w', rev=%02X, rsdt=%08X,\r\n");
        kSprintf(&ksBuf, &ksFmt, AQWORD(prsdp->signature.qword), 
            ABYTE(prsdp->checksum), AANSI(&prsdp->oemId[0], sizeof(prsdp->oemId)), 
            ABYTE(prsdp->revision), ADWORD(prsdp->rsdtAddress));
        kdSendKString(&ksBuf);
        KSTR_SET(ksFmt, "    len=%d, xsdt=%016X, extchk=%02X\r\n");
        kSprintf(&ksBuf, &ksFmt, ADWORD(prsdp->length), AQWORD(prsdp->xsdtAddress), 
            ABYTE(prsdp->extChecksum));
        kdSendKString(&ksBuf);

        prsdt = (RSDT*) prsdp->rsdtAddress;
        pxsdt = (XSDT*) prsdp->xsdtAddress;

        prsdtEntry = &prsdt->addressEntry[0];
        numEntries = NUM_RSDT_ENTRIES(prsdt->header.length); //(phdr->length);
        printSdtHdr(&prsdt->header);
        KSTR_SET(ksFmt, "entries (%d):\r\n");
        kSprintf(&ksBuf, &ksFmt, ADWORD(numEntries));
        kdSendKString(&ksBuf);
        KSTR_SET(ksFmt, "    %08X -> '%c'\r\n");
        for(i = 0; i < numEntries; i++)
        {
            psign = (DWORD*) prsdtEntry[i];
            kSprintf(&ksBuf, &ksFmt, ADWORD(prsdtEntry[i]), ADWORD(*psign));
            kdSendKString(&ksBuf);
            //if(*psign == ACPI_FADT_SIGN)      // RSDT -> *old* FADT
            //{
            //    printSdtHdr((SDTHDR*)psign);
            //    printFadt((FADT*)psign);
            //}
        }

        pxsdtEntry = &pxsdt->addressEntry[0];
        numEntries = NUM_XSDT_ENTRIES(pxsdt->header.length); //(phdr->length);
        printSdtHdr(&pxsdt->header);
        KSTR_SET(ksFmt, "entries (%d):\r\n");
        kSprintf(&ksBuf, &ksFmt, ADWORD(numEntries));
        kdSendKString(&ksBuf);
        KSTR_SET(ksFmt, "    %016X -> '%c'\r\n");
        for(i = 0; i < numEntries; i++)
        {
            psign = (DWORD*) pxsdtEntry[i];
            kSprintf(&ksBuf, &ksFmt, AQWORD(pxsdtEntry[i]), ADWORD(*psign));
            kdSendKString(&ksBuf);
            if(*psign == ACPI_FADT_SIGN)
            {
                printSdtHdr((SDTHDR*)psign);
                printFadt((FADT*)psign);
            }
        }
    }
    else
    {
        KSTR_SET(ksMsg, "RSDPtr not found\r\n");
        kdSendKString(&ksMsg);
    }
}


EXTERN_C_END

