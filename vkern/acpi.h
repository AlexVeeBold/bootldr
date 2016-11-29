///////////////////////////////////////////////////////////////
//
// acpi: Advanced Configuration and Power Interface
//
//
//   18.09.2010 21:20 - created
//

#ifndef _ACPI_H_INC_
#define _ACPI_H_INC_


EXTERN_C_BEGIN


//////////////////////////////// REGISTERS ////////////////////////////////

struct PM1STATUS {      // [PM1_STS] PM1 Status Registers
    BYTE timerStatus:1;     // [TMR_STS] =1 if highest p.m.timer bit switched 0->1 or 1->0
    BYTE reserved0:3;       // =0?
    BYTE busMasterStatus:1; // [BM_STS] =1 when system bus master requests the system bus
    BYTE globalStatus:1;    // [GBL_STS] =1 when: SCI generated, BIOS releasing control of the Global Lock
    BYTE reserved1:2;       // =0
    BYTE powerBtnStatus:1;  // [PWRBTN_STS] =1 when the power button is pressed
    BYTE sleepBtnStatus:1;  // [SPLBTN_STS] =1 when the sleep button is pressed
    BYTE rtcAlarmStatus:1;  // [RTC_STS] =1 when the RTC generates an alarm
    BYTE ignore0:1;         // ignored by software
    BYTE reserved2:2;       // =0
    BYTE pcieWakeStatus:1;  // [PCIEXP_WAKE_STS] =1 when system woke due to a PCI Express wakeup event
    BYTE wakeStatus:1;      // [WAK_STS] =1 when the system is in sleeping state and an enabled wake event occurs
};

struct PM1ENABLE {      // [PM1_EN] PM1 Enable Registers
    BYTE timerEnable:1;     // [TMR_EN] enable p.m.timer s.c.interrupt (with timerStatus set)
    BYTE reserved0:4;       // =0
    BYTE globalEnable:1;    // [GBL_EN] enable global SCI (with globalStatus set)
    BYTE reserved1:2;       // =0?
    BYTE powerBtnEnable:1;  // [PWRBTN_EN] enable power button SCI/wake (with powerBtnStatus set)
    BYTE sleepBtnEnable:1;  // [SLPBTN_EN] enable sleep button SCI/wake (with sleepBtnStatus set)
    BYTE rtcAlarmEnable:1;  // [RTC_EN] enable RTC alarm SCI/wake (with rtcAlarmStatus set)
    BYTE reserved2:3;       // =0
    BYTE pcieWakeDisable:1; // [PCIEXP_WAKE_DIS] disable PCI Express wakeup event (with pcieWakeStatus set)
    BYTE reserved3:1;       // =0
};

struct PM1CONTROL {     // [PM1_CNT] PM1 Control Registers
    BYTE sciEnable:1;       // [SCI_EN] =0: generate SMI; =1: generate SCI
    BYTE busMasterRld:1;//? // [BM_RLD] =1: allow transfer cpu C3->C0 by a bus master request
    BYTE globalRelease:1;   // [GBL_RLS] raise an event to BIOS to enable its ability to receive ACPI events
    BYTE reserved0:5;       // =0
    BYTE reserved1:1;       // =0 (byte boundary)
    BYTE ignored:1;         // =? ignored by software
    BYTE sleepType:3;       // [SLP_TYPx] type of sleeping state the system enters when sleepEnable=1 (taken from \_Sx object)
    BYTE sleepEnable:1;     // [SLP_EN] transfer system into the sleeping state associated with sleepType
    BYTE reserved2:2;       // =0
};

union PMTIMER {        // [PM_TMR] Power Management Timer
    // uses 3579545 Hz clock (14.31818 MHz / 4)
    struct {
        DWORD timerValue24:24;  // [TMR_VAL] 24-bit counter value (24-bit mode)
        DWORD unused:8;
    };
    DWORD timerValue32;     // [TMR_VAL,E_TMR_VAL] 32-bit counter value (32-bit mode)
};

struct PM2CONTROL {     // [PM2_CNT] PM2 Control Register
    BYTE systemArbiterDisable:1;    // [ARB_DIS] =1: arbiter grants bus to other bus masters; =0: CPU has ownership of the system
    BYTE reserved:7;        // =0?
};

struct CPUCONTROL {     // [P_CNT] Processor Control Register
    DWORD clockValue0:4;    // [CLK_VAL] possible locations for the clock throttling value (dutyOffset, dutyWidth)
    DWORD throttleEnable:1; // [THT_EN] enables clock throttling (set to 0 to adjust clockValue)
    DWORD clockValue1:27;   // [CLK_VAL] possible locations for the clock throttling value (dutyOffset, dutyWidth)
};

struct CPULEVEL2 {      // [P_LVL2] Processor LVL2 Register
    BYTE enterLevel2;       // [P_LVL2] read: return zeros, transfer processor to C2 power state; write: no effect
};

struct CPULEVEL3 {      // [P_LVL3] Processor LVL3 Register
    BYTE enterLevel3;       // [P_LVL3] read: return zeros, transfer processor to C3 power state; write: no effect
};

// [GPE0_STS] General-Purpose Event 0 Status Register
//  BYTE gpe0status[GPE0_BLK_LEN / 2];
//  status bits corresponds to the bits with the same bit positions in the GPE0_EN register

// [GPE0_EN] General-Purpose Event 0 Enable Register
//  BYTE gpe0enable[GPE0_BLK_LEN / 2];
//  enable bits corresponds to the bits with the same bit positions in the GPE0_STS register

// [GPE1_STS] General-Purpose Event 1 Status Register
//  BYTE gpe1status[GPE1_BLK_LEN / 2];
//  status bits corresponds to the bits with the same bit positions in the GPE1_EN register

// [GPE1_EN] General-Purpose Event 1 Enable Register
//  BYTE gpe1enable[GPE1_BLK_LEN / 2];
//  enable bits corresponds to the bits with the same bit positions in the GPE1_STS register


//////////////////////////////// GAS ////////////////////////////////

enum GAS_ADSP {     // Generic Address Structure - Address Space
    SYSTEM_MEMORY = 0,  // System Memory
    SYSTEM_IO, // 1     // System I/O
    PCI_CONFIG, // 2    // PCI Configuration Space
    EMBED_CTLR, // 3    // Embedded Controller
    SMBUS, // 4         // SMBus
    // 0x05..0x7E: reserved
    FFIXED_HW = 0x7F,   // Functional Fixed Hardware
    // 0x80..0xBF: reserved
    // 0xC0..0xFF: OEM defined
};
enum GAS_ACSI {     // Generic Address Structure - Access Size
    UNDEFINED = 0,      // legacy reasons (?)
    BYTE_ACCESS, // 1
    WORD_ACCESS, // 2
    DWORD_ACCESS, // 3
    QWORD_ACCESS, // 4
};

struct PCIBAR {
    WORD configOffset;  // Offset in configuration space header
    WORD pciFunction;   // PCI Function number
    WORD pciDevice;     // PCI Device number
    WORD reserved;      // =0
};
union GAS_ADDR {    // Generic Address Structure - Address Format
    QWBYTE system;          // System Memory or System I/O
    PCIBAR pciCfg;          // PCI Configuration space address
};

struct GAS {        // Generic Address Structure
    BYTE addrSpaceId;       // address space where the register or data structure exists (enum GAS_ADDR)
    BYTE regBitWidth;       // register size in bits, =0 for data structure
    BYTE regBitOffset;      // register bit offset, =0 for data structure
    BYTE accessSize;        // access size (enum GAS_ACSI)
    GAS_ADDR address;       // address of the register or data structure in the given address space
};


//////////////////////////////// RSDP ////////////////////////////////

#define EFI_ACPI10_GUID     \
    UUID_INIT(0xEB9D2D30, 0x2D88, 0x11D3, 0x9A,0x16, 0x00,0x90,0x27,0x3F,0xC1,0x4D)
#define EFI_ACPI20_GUID     \
    UUID_INIT(0x8868E871, 0xE4F1, 0x11D3, 0xBC,0x22, 0x00,0x80,0xC7,0x3C,0x88,0x81)


#define ACPI_RSDP_SIGN      0x2052545020445352  // 'RSD PTR '

struct RSDPTR {     // Root System Description Pointer
    QWBYTE signature;       // "RSD PTR "
    BYTE checksum;          // checksum of the first 20 bytes (bytes must sum to zero)
    BYTE oemId[6];          // OEM-supplied string that identifies the OEM
    BYTE revision;          // revision number
    DWORD rsdtAddress;      // 32-bit physical address of the RSDT
    // 20 bytes boundary
    DWORD length;           // length of the table, in bytes, including the header
    QWORD xsdtAddress;      // 64-bit physical address of the XSDT
    BYTE extChecksum;       // checksum of the entire table, including both checksum fields
    BYTE reserved[3];       // Reserved field
};

//////////////////////////////// SDTHDR ////////////////////////////////

// ACPI-defined table signatures

#define ACPI_RSDT_SIGN      0x54445352  // 'RSDT' Root System Description Table
#define ACPI_XSDT_SIGN      0x54445358  // 'XSDT' Extended System Description Table

#define ACPI_FADT_SIGN      0x50434146  // 'FACP' Fixed ACPI Description Table
#define ACPI_DSDT_SIGN      0x54445344  // 'DSDT' Differentiated System Description Table
#define ACPI_SSDT_SIGN      0x54445353  // 'SSDT' Secondary System Description Table
#define ACPI_PSDT_SIGN      0x54445350  // 'PSDT' Persistent System Description Table (deleted; evaluate as SSDT)

#define ACPI_MADT_SIGN      0x43495041  // 'APIC' Multiple APIC Description Table
#define ACPI_BERT_SIGN      0x54524542  // 'BERT' Boot Error Record Table
#define ACPI_CPEP_SIGN      0x50455043  // 'CPEP' Corrected Platform Error Polling Table
#define ACPI_ECDT_SIGN      0x54444345  // 'ECDT' Embedded Controller Boot Resources Table
#define ACPI_EINJ_SIGN      0x4A4E4945  // 'EINJ' Error Injection Table
#define ACPI_ERST_SIGN      0x54535245  // 'ERST' Error Record Serialization Table
#define ACPI_FACS_SIGN      0x53434146  // 'FACS' Firmware ACPI Control Structure
#define ACPI_HEST_SIGN      0x54534548  // 'HEST' Hardware Error Source Table
#define ACPI_MSCT_SIGN      0x5443534D  // 'MSCT' Maximum System Characteristics Table
#define ACPI_SBST_SIGN      0x54534253  // 'SBST' Smart Battery Specification Table
#define ACPI_SLIT_SIGN      0x54494C53  // 'SLIT' System Locality Distance Information Table
#define ACPI_SRAT_SIGN      0x54415253  // 'SRAT' System Resource Affinity Table

// ACPI-reserved table signatures

#define ACPIEX_BOOT_SIGN    0x544F4F42  // 'BOOT' [MSFT] Simple Boot Flag Table
#define ACPIEX_DBGP_SIGN    0x50474244  // 'DBGP' [MSFT] Debug Port Table
#define ACPIEX_DMAR_SIGN    0x52414D44  // 'DMAR' [INTEL] DMA Remapping Table
#define ACPIEX_ETDT_SIGN    0x54445445  // 'ETDT' [INTEL] Event Timer Description Table (obsolete; use HPET)
#define ACPIEX_HPET_SIGN    0x54455048  // 'HPET' [INTEL] IA-PC High Precision Event Timer Table
#define ACPIEX_IBFT_SIGN    0x54464249  // 'IBFT' [MSFT] iSCSI Boot Firmware Table
#define ACPIEX_IVRS_SIGN    0x53525649  // 'IVRS' [AMD] I/O Virtualization Reporting Structure
#define ACPIEX_MCFG_SIGN    0x4746434D  // 'MCFG' [PCISIG] PCI Express mem. mapped conf. space base addr. Description Table
#define ACPIEX_MCHI_SIGN    0x4948434D  // 'MCHI' [DMTF] Management Controller Host Interface Table
#define ACPIEX_SPCR_SIGN    0x52435053  // 'SPCR' [MSFT] Serial Port Console Redirection Table
#define ACPIEX_SPMI_SIGN    0x494D5053  // 'SPMI' [INTEL] Server Platform Management Interface Table
#define ACPIEX_TCPA_SIGN    0x41504354  // 'TCPA' [TCG] Trusted Computing Platform Alliance Capabilities Table
#define ACPIEX_UEFI_SIGN    0x49464555  // 'UEFI' [UEFI] UEFI ACPI Data Table
#define ACPIEX_WAET_SIGN    0x54454157  // 'WAET' [MSFT] Windows ACPI Enlightenment Table
#define ACPIEX_WDAT_SIGN    0x54414457  // 'WDAT' [MSFT] Watch Dog Action Table
#define ACPIEX_WDRT_SIGN    0x54524457  // 'WDRT' [MSFT] Watchdog Resource Table

// Common headers

struct SDTHDR {     // System Description Table Header
    DWBYTE signature;   // ASCII string representation of the table identifier
    DWORD length;       // length of the entire table, in bytes, including the header
    BYTE revision;      // revision number
    BYTE checksum;      // checksum of the entire table (bytes must sum to zero)
    ACHAR oemId[6];     // OEM-supplied string that identifies the OEM
    QWBYTE oemTableId;  // OEM-supplied string that identifies particular data table
    DWORD oemRevision;  // OEM-supplied revision number
    DWBYTE creatorId;   // Vendor ID of utility that created the table
    DWORD creatorRev;   // Revision of utility that created the table
};

#define SIZEOF_SDT_DATA(tableLength)    \
    ( tableLength - sizeof(SDTHDR) )

struct SUBHDR {     // MADT, SRAT, CPEP: Common Header
    BYTE type;          // Structure type (enum MAD_TYPE, SRA_TYPE, CPE_TYPE)
    BYTE length;        // Structure length, in bytes
};


//////////////////////////////// RSDT, XSDT ////////////////////////////////

// Platforms provide the RSDT to enable compatibility with ACPI 1.0 operating 
// systems. The XSDT supersedes RSDT functionality.

struct RSDT {       // Root System Description Table
    SDTHDR header;          // signature='RSDT', revision=1
    DWORD addressEntry[1];  // array of 32-bit physical addresses that point to other SDTHDRs
};
#define NUM_RSDT_ENTRIES(tableLength)   \
    ( (tableLength - sizeof(SDTHDR)) / sizeof(DWORD) )

// The XSDT provides identical functionality to the RSDT but accommodates physical 
// addresses that are larger than 32-bits. 
// An ACPI-compatible OS must use the XSDT if present.

struct XSDT {       // Extended System Description Table
    SDTHDR header;          // signature='XSDT', revision=1
    QWORD addressEntry[1];  // array of 64-bit physical addresses that point to other SDTHDRs
};
#define NUM_XSDT_ENTRIES(tableLength)   \
    ( (tableLength - sizeof(SDTHDR)) / sizeof(QWORD) )


//////////////////////////////// FADT ////////////////////////////////

// The Fixed ACPI Description Table (FADT) defines various fixed hardware ACPI 
// information vital to an ACPI-compatible OS, such as the base address for 
// the hardware registers blocks.
// The FADT also has a pointer to the DSDT that contains the Differentiated Definition 
// Block, which in turn provides variable information to an ACPI-compatible OS 
// concerning the base system design.

struct FADT {       // Fixed ACPI Description Table
    SDTHDR header;          // signature='FACP', revision=4
    DWORD facsAddress;      // [FIRMWARE_CTRL] 32-bit physical address of the FACS
    DWORD dsdtAddress;      // [DSDT] 32-bit physical address of the DSDT
    BYTE reserved044;       // eliminated in ACPI 2.0 (=0); defined as INT_MODEL in ACPI 1.0 (=1)
    BYTE preferredPmProfile;    // [Preferred_PM_Profile] Preferred power management profile, set by the OEM (PMP_*)
    WORD sciIntVector;          // [SCI_INT] System vector the SCI interrupt is wired to in 8259 mode
    DWORD smiCmdPortAddress;    // [SMI_CMD] System port address of the SMI Command Port
    BYTE acpiEnableValue;       // [ACPI_ENABLE] The value to write to SMI_CMD to disable SMI ownership of the ACPI hardware registers
    BYTE acpiDisableValue;      // [ACPI_DISABLE] The value to write to SMI_CMD to re-enable SMI ownership of the ACPI hardware registers
    BYTE s4BiosValue;       // [S4BIOS_REQ] The value to write to SMI_CMD to enter the S4BIOS state
    BYTE pstateCnt;         // [PSTATE_CNT] The value OSPM writes to the SMI_CMD register to assume processor performance state control responsibility
    DWORD pm1aEvtAddress;   // [PM1a_EVT_BLK] System port address of the PM1a Event Register Block
    DWORD pm1bEvtAddress;   // [PM1b_EVT_BLK] System port address of the PM1b Event Register Block
    DWORD pm1aCtlAddress;   // [PM1a_CNT_BLK] System port address of the PM1a Control Register Block
    DWORD pm1bCtlAddress;   // [PM1b_CNT_BLK] System port address of the PM1b Control Register Block
    DWORD pm2CtlAddress;    // [PM2_CNT_BLK] System port address of the PM2 Control Register Block
    DWORD pmTmrCtlAddress;  // [PM_TMR_BLK] System port address of the Power Management Timer Control Register Block
    DWORD gpe0rbAddress;    // [GPE0_BLK] System port address of General-Purpose Event 0 Register Block
    DWORD gpe1rbAddress;    // [GPE1_BLK] System port address of General-Purpose Event 1 Register Block
    BYTE pm1EventLength;    // [PM1_EVT_LEN] Number of bytes decoded by PM1a_EVT_BLK and, if supported, PM1b_EVT_BLK. This value is >= 4
    BYTE pm1ControlLength;  // [PM1_CNT_LEN] Number of bytes decoded by PM1a_CNT_BLK and, if supported, PM1b_CNT_BLK. This value is >= 2
    BYTE pm2ControlLength;  // [PM2_CNT_LEN] Number of bytes decoded by PM2_CNT_BLK. If supported, this value is >= 1
    BYTE pmTimerLength;     // [PM_TMR_LEN] Number of bytes decoded by PM_TMR_BLK. This field's value must be 4
    BYTE gpEvent0Length;    // [GPE0_BLK_LEN] Number of bytes decoded by GPE0_BLK. The value is a nonnegative multiple of 2
    BYTE gpEvent1Length;    // [GPE1_BLK_LEN] Number of bytes decoded by GPE1_BLK. The value is a nonnegative multiple of 2
    BYTE gpEvent1Base;      // [GPE1_BASE] Offset within the ACPI general-purpose event model where GPE1 based events start
    BYTE cstCnt;            // [CST_CNT] The value OSPM writes to the SMI_CMD register to indicate OS support for the _CST object and C States Changed notification
    WORD level2Latency;     // [P_LVL2_LAT] The worst-case hardware latency, in microseconds, to enter and exit a C2 state. A value > 100 indicates the system does not support a C2 state
    WORD level3Latency;     // [P_LVL3_LAT] The worst-case hardware latency, in microseconds, to enter and exit a C3 state. A value > 1000 indicates the system does not support a C3 state
    WORD flushSize;         // [FLUSH_SIZE] The number of flush strides that need to be read to completely flush dirty lines from any processor's memory caches
    WORD flushStride;       // [FLUSH_STRIDE] The cache line width, in bytes, of the processor's memory caches
    BYTE dutyOffset;        // [DUTY_OFFSET] The zero-based index of where the processor's duty cycle setting is within the processor's P_CNT register
    BYTE dutyWidth;         // [DUTY_WIDTH] The bit width of the processor's duty cycle setting value in the P_CNT register
    BYTE rtcDayAlarmIndex;      // [DAY_ALRM] The RTC CMOS RAM index to the day-of-month alarm value (0 = not supported)
    BYTE rtcMonthAlarmIndex;    // [MON_ALRM] The RTC CMOS RAM index to the month of year alarm value (0 = not supported)
    BYTE rtcCenturyIndex;       // [CENTURY] The RTC CMOS RAM index to the century of data value (hundred and thousand year decimals) (0 = not supported)
    WORD iapcBootArchFlags;     // [IAPC_BOOT_ARCH] IA-PC Boot Architecture Flags (IBF_*)
    BYTE reserved111;           // = 0
    DWORD fixedFeatFlags;       // [Flags] Fixed feature flags (FFF_*)
    GAS resetRegAddressGA;    // [RESET_REG] The address of the reset register represented in Generic Address Structure format
    BYTE resetValue;        // [RESET_VALUE] The value to write to the RESET_REG port to reset the system
    BYTE reserved129[3];    // = 0
    QWORD facsAddressExt;   // [X_FIRMWARE_CTRL] 64-bit physical address of the FACS
    QWORD dsdtAddressExt;   // [X_DSDT] 64-bit physical address of the DSDT
    GAS pm1aEvtAddressGA;   // [X_PM1a_EVT_BLK] Extended system port address of the PM1a Event Register Block
    GAS pm1bEvtAddressGA;   // [X_PM1b_EVT_BLK] Extended system port address of the PM1b Event Register Block
    GAS pm1aCtlAddressGA;   // [X_PM1a_CNT_BLK] Extended system port address of the PM1a Control Register Block
    GAS pm1bCtlAddressGA;   // [X_PM1b_CNT_BLK] Extended system port address of the PM1b Control Register Block
    GAS pm2CtlAddressGA;    // [X_PM2_CNT_BLK] Extended system port address of the PM2 Control Register Block
    GAS pmTmrCtlAddressGA;  // [X_PM_TMR_BLK] Extended system port address of the Power Management Timer Control Register Block
    GAS gpe0rbAddressGA;    // [X_GPE0_BLK] Extended system port address of General-Purpose Event 0 Register Block
    GAS gpe1rbAddressGA;    // [X_GPE1_BLK] Extended system port address of General-Purpose Event 1 Register Block
};

// Power Management Profile
enum PM_PROFILE {
    // unspecified power management profile
    PMP_UNSPECIFIED = 0,

    // stationary computing device for mainstream corporate or home computing
    PMP_DESKTOP, // 1

    // portable computing device that performs the same task set as a desktop
    PMP_MOBILE, // 2

    // stationary computing device is used to perform large quantities of computations (CAD/CAM)
    PMP_WORKSTATION, // 3

    // stationary computing device that is used to support large-scale networking, database, 
    // communications, or financial operations within a corporation or government
    PMP_ENTERPRISESERVER, // 4

    // stationary computing device that is used to support all of the networking, database, 
    // communications, and financial operations of a small office or home office
    PMP_SOHOSERVER, // 5

    // device specifically designed to operate in a low-noise, high-availability environment
    // such as a consumer’s living rooms (home internet gateways, web pads)
    PMP_APPLIANCEPC, // 6

    // stationary computing device that  is used in an environment where power savings 
    // features are willing to be sacrificed for better performance and quicker responsiveness
    PMP_PERFORMANCESERVER, // 7

    // reserved power management profiles
    PMP_RESERVED // 8...
};

// Fixed Feature Flags
#define FFF_WBINVD          0x00000001  // [WBINVD] WBINVD IA-32 instruction supported
#define FFF_WBINVD_FLUSH    0x00000002  // [WBINVD_FLUSH] only flush cache, no invalidation
#define FFF_C1              0x00000004  // [PROC_C1] C1 power state supported
#define FFF_C2_MP           0x00000008  // [P_LVL2_UP] (0): C2 work on UP/MP system, (1): C2 work on uniprocessor only
#define FFF_POWER_BUTTON    0x00000010  // [PWR_BUTTON] power button is handled as a (0)[fixed feature programming model] (1)[control method device (or absent)]
#define FFF_SLEEP_BUTTON    0x00000020  // [SLP_BUTTON] sleep button is handled as a (0)[fixed feature programming model] (1)[control method device (or absent)]
#define FFF_RTC_WAKE        0x00000040  // [FIX_RTC] RTC wake status is (0)[supported] (1)[not supported] in fixed register space
#define FFF_RTC_S4WAKE      0x00000080  // [RTC_S4] RTC alarm function can wake the system from the S4 state
#define FFF_TIMER_32        0x00000100  // [TMR_VAL_EXT] TMR_VAL is implemented as a (0)[24-bit] (1)[32-bit] value
#define FFF_DOCKING         0x00000200  // [DCK_CAP] system can support docking
#define FFF_RESET_REG       0x00000400  // [RESET_REG_SUP] system supports system reset via the FADT reset register
#define FFF_SEALED_CASE     0x00000800  // [SEALED_CASE] system has no internal expansion capabilities and the case is sealed
#define FFF_HEADLESS        0x00001000  // [HEADLESS] system cannot detect the monitor or input (keyboard / mouse) devices
#define FFF_CPU_SW_SLEEP    0x00002000  // [CPU_SW_SLP] processor native instruction must be executed after writing the SLP_TYPx register
#define FFF_PCIE_WAKE       0x00004000  // [PCI_EXP_WAK] PCI-Express-wake status & disable bits are supported (mandatory on PCIE chipset systems)
#define FFF_PLATFORM_TIMER  0x00008000  // [USE_PLATFORM_CLOCK] OSPM should use HPET or ACPI power management timer
#define FFF_RTC_S4_VALID    0x00010000  // [S4_RTC_STS_VALID] RTC status flag is valid when waking the system from S4
#define FFF_REMOTE_POWERON  0x00020000  // [REMOTE_POWER_ON_CAPABLE] platform is compatible with remote power-on
#define FFF_APIC_CLUSTER_DM 0x00040000  // [FORCE_APIC_CLUSTER_MODEL] all local APICs must be configured for the cluster destination model when delivering interrupts in logical mode
#define FFF_APIC_PHYS_DM    0x00080000  // [FORCE_APIC_PHYSICAL_DESTINATION_MODE] all local xAPICs must be configured for physical destination mode

// IA-PC Boot Architecture Flags
#define IBF_LEGACY_DEVICES  0x00000001  // [LEGACY_DEVICES] motherboard supports user-visible devices on the LPC or ISA bus
#define IBF_KEYBOARD_8042   0x00000002  // [8042] motherboard supports port 60 and 64 based keyboard controller, usually implemented as an 8042 or equivalent micro-controller
#define IBF_VGA_ABSENT      0x00000004  // [VGA Not Present] OSPM must not blindly probe the VGA hardware (that responds to MMIO addresses A0000h-BFFFFh and IO ports 3B0h-3BBh and 3C0h-3DFh)
#define IBF_NO_MSI          0x00000008  // [MSI Not Supported] OSPM must not enable Message Signaled Interrupts
#define IBF_NO_PCIE_ASPM    0x00000010  // [PCIe ASPM Controls] OSPM must not enable OSPM ASPM control


//////////////////////////////// FACS ////////////////////////////////

// The Firmware ACPI Control Structure (FACS) is a structure in read/write memory that 
// the BIOS reserves for ACPI usage. This structure is passed to an ACPI-compatible OS 
// using the FADT. 
// The BIOS aligns the FACS on a 64-byte boundary anywhere within the system’s memory 
// address space.

struct GLOCK {      // FACS: Global Lock
    DWORD pending:1;        // Request for ownership of the Global Lock is pending
    DWORD owned:1;          // Global Lock is Owned
    DWORD reserved:30;      // Reserved for future use
};

struct FACS {       // Firmware ACPI Control Structure
    DWBYTE signature;       // 'FACS'
    DWORD length;           // Length of the entire structure, in bytes
    DWORD hardwareSign;     // base hardware configuration of the system, compared by OSPM, when waking from an S4 state, with saved hardware signature
    DWORD wakeVector;       // 32-bit physical memory address of an OS-specific wake function (in memory below 1 MB; control is transferred while in real mode)
    GLOCK globalLock;       // Global Lock used to synchronize access to shared hardware resources between the OSPM environment and an external controller environment
    DWORD fwCtlFlags;       // Firmware control structure flags
    QWORD wakeVectorExt;    // 64-bit physical memory address of an OS-specific wake function (32-bit / 64-bit environment, based on 64BIT_WAKE_ENV flag)
    BYTE version;           // 2–Version of this table
    BYTE reserved33[3];     // =0
    DWORD ospmFlags;        // OSPM enabled firmware control structure flags (initially zero)
    BYTE reserved40[24];    // =0
};

// Firmware control structure flags
#define FWC_S4BIOS          0x00000001  // [S4BIOS_F] platform supports S4BIOS_REQ. Otherwise, OSPM must be able to save and restore the memory state in order to use the S4 state
#define FWC_64BIT_WAKE      0x00000002  // [64BIT_WAKE_SUPPORTED_F] platform firmware supports a 64 bit execution environment for the waking vector

// OSPM enabled firmware control structure flags
#define FOE_64BIT_WAKE_ENV  0x00000001  // [64BIT_WAKE_F] set by OSPM to indicate to platform firmware that the Wake Vector requires a 64 bit execution environment
// 32-bit execution environment (created by platform firmware):
//  - Interrupts must be disabled (EFLAGS.IF set to 0)
//  - Memory address translation / paging must be disabled
//  - 4 GB flat address space for all segment registers
// 64-bit execution environment (created by platform firmware):
//  - Interrupts must be disabled (EFLAGS.IF set to 0)
//  - Long mode enabled
//  - Paging mode is enabled and physical memory for waking vector is 
//    identity mapped (virtual address equals physical address)
//      + Waking vector must be contained within one physical page
//  - Selectors are set to be flat and are otherwise not used


//////////////////////////////// DSDT, SSDT ////////////////////////////////

// The Differentiated System Description Table (DSDT) is part of the system fixed 
// description. The DSDT is comprised of a system description table header followed 
// by data in Definition Block format. This Definition Block is like all other 
// Definition Blocks, with the exception that it cannot be unloaded. 
// During initialization, OSPM finds the pointer to the DSDT in the Fixed ACPI 
// Description Table and then loads the DSDT to create the ACPI Namespace.

struct DSDT {       // Differentiated System Description Table
    SDTHDR header;          // signature='DSDT', revision: <2: 32-bit AML integers; >=2: 64-bit AML integers
    BYTE fixedDfnBlock[1];  // n bytes of AML code [=SIZEOF_SDT_DATA(header.length)]
};

// Secondary System Description Tables (SSDT) are a continuation of the DSDT. The SSDT 
// is comprised of a system description table header followed by data in Definition 
// Block format. There can be multiple SSDTs present. 
// After OSPM loads the DSDT to create the ACPI Namespace, each secondary system 
// description table listed in the RSDT/XSDT with a unique OEM Table ID is loaded. 
// Note: Additional tables can only add data; they cannot overwrite data from previous tables.

struct SSDT {       // Secondary System Description Table (multiple instances allowed)
    SDTHDR header;          // signature='SSDT', revision=2
    BYTE addDfnBlock[1];    // n bytes of AML code [=SIZEOF_SDT_DATA(header.length)]
};

// The table signature, "PSDT" refers to the Persistent System Description Table (PSDT) 
// defined in the ACPI 1.0 specification. The PSDT has been deleted from follow-on 
// versions of the ACPI specification. 
// OSPM will evaluate a table with the “PSDT” signature in like manner to the 
// evaluation of an SSDT.


//////////////////////////////// MADT ////////////////////////////////

// The ACPI interrupt model describes all interrupts for the entire system in a 
// uniform interrupt model implementation. Supported interrupt models include the 
// PC-AT–compatible dual 8259 interrupt controller and, for Intel processor-based 
// systems, the Intel Advanced Programmable Interrupt Controller (APIC) and Intel 
// Streamlined Advanced Programmable Interrupt Controller (SAPIC). The choice of 
// the interrupt model(s) to support is up to the platform designer. The interrupt 
// model cannot be dynamically changed by the system firmware; OSPM will choose 
// which model to use and install support for that model at the time of installation. 
// If a platform supports both models, an OS will install support for one model or 
// the other; it will not mix models. 
// Multi-boot capability is a feature in many modern operating systems. This means 
// that a system may have multiple operating systems or multiple instances of an OS 
// installed at any one time. Platform designers must allow for this.

struct MADT {       // Multiple APIC Description Table
    SDTHDR header;          // signature='APIC', revision=3
    DWORD localApicAddress; // The 32-bit physical address at which each processor can access its local APIC
    DWORD flags;            // Multiple APIC flags (MAF_*)
    // array of APIC structures
};
#define PTR_MADT_DATA(ptrTable)         \
    ( (BYTE*)ptrTable + sizeof(MADT) )
#define SIZEOF_MADT_DATA(tableLength)   \
    ( tableLength - sizeof(MADT) )

// Multiple APIC flags
#define MAF_AT_DUAL8259     0x00000001  // System has a PC-AT-compatible dual-8259 setup. The 8259 vectors must be disabled (that is, masked) when enabling the ACPI APIC operation

enum MAD_TYPE {     // MADT: Structure Types
    MAD_LAPIC = 0,          // Processor Local APIC
    MAD_IO_APIC, // 1       // I/O APIC (Advanced Programmable Interrupt Controller)
    MAD_INT_SRC_O, // 2     // Interrupt Source Override
    MAD_NMI_SRC, // 3       // Non-maskable Interrupt Source (NMI)
    MAD_LAPIC_NMI, // 4     // Local APIC NMI
    MAD_LAPIC_AO, // 5      // Local APIC Address Override
    MAD_IO_SAPIC, // 6      // I/O SAPIC (Streamlined Advanced Programmable Interrupt Controller)
    MAD_LSAPIC, // 7        // Local SAPIC
    MAD_PLAT_INT_SRCS, // 8 // Platform Interrupt Sources
    MAD_LX2APIC, // 9       // Processor Local x2APIC
    MAD_LX2APIC_NMI, // A   // Local x2APIC NMI
    // 0x0B..0x7F: reserved, skipped by OSPM
    // 0x80..0xFF: reserved for OEM use
};

// When using the APIC interrupt model, each processor in the system is required 
// to have a Processor Local APIC record and an ACPI Processor object.
struct MA_LAPIC {       // Processor Local APIC Structure
    SUBHDR header;          // type=0(MAD_LAPIC), length=8
    BYTE processorId;       // The ProcessorId for which this processor is listed in the ACPI Processor declaration operator
    BYTE lapicId;           // The processor’s local APIC ID
    DWORD lapicFlags;       // Local APIC flags (LAF_*)
};

// Local APIC flags
#define LAF_ENABLED         0x00000001  // This processor is usable (available for operating system)

// The I/O APIC structure declares which global system interrupts are uniquely 
// associated with the I/O APIC interrupt inputs. There is one I/O APIC structure 
// for each I/O APIC in the system.
struct MA_IO_APIC {     // I/O APIC Structure
    SUBHDR header;          // type=1(MAD_IO_APIC), length=12
    BYTE ioapicId;          // The I/O APIC’s ID
    BYTE reserved;          // =0
    DWORD ioapicAddress;    // The 32-bit unique physical address to access this I/O APIC
    DWORD globSysIntBase;   // The global system interrupt number where this I/O APIC’s interrupt inputs start
};

// Interrupt Source Overrides are necessary to describe variances between the 
// IA-PC standard dual 8259 interrupt definition and the platform’s implementation.
struct MA_INT_SRC_O {   // Interrupt Source Override Structure
    SUBHDR header;          // type=2(MAD_INT_SRC_O), length=10
    BYTE bus;               // =0 (ISA)
    BYTE source;            // Bus-relative interrupt source (IRQ)
    DWORD globSysInt;       // The Global System Interrupt that this bus-relative interrupt source will signal
    WORD flags;             // MPS INTI Flags (MIF_*)
};

// MPS INTI Flags
//Polarity of the APIC I/O input signals
#define MIF_POLARITY_BUS    0x00000000  // Conforms to the specifications of the bus (For example, EISA is active-low for level-triggered interrupts)
#define MIF_POLARITY_HIGH   0x00000001  // Active high
#define MIF_POLARITY_LOW    0x00000003  // Active low
//Trigger mode of the APIC I/O Input signals
#define MIF_TRIGGER_BUS     0x00000000  // Conforms to specifications of the bus (For example, ISA is edge-triggered)
#define MIF_TRIGGER_EDGE    0x00000004  // Edge-triggered
#define MIF_TRIGGER_LEVEL   0x0000000C  // Level-triggered

// This structure allows a platform designer to specify which I/O (S)APIC interrupt 
// inputs should be enabled as non-maskable. Any source that is non-maskable will not 
// be available for use by devices.
struct MA_NMI_SRC {     // Non-maskable Interrupt Source Structure
    SUBHDR header;          // type=3(MAD_NMI_SRC), length=8
    WORD flags;             // MPS INTI Flags (MIF_*)
    DWORD globSysInt;       // The Global System Interrupt that this NMI will signal
};

// This structure describes the Local APIC interrupt input (LINTn) that NMI is 
// connected to for each of the processors in the system where such a connection exists. 
// This information is needed by OSPM to enable the appropriate local APIC entry.
struct MA_LAPIC_NMI {   // Local APIC NMI Structure
    SUBHDR header;          // type=4(MAD_LAPIC_NMI), length=6
    BYTE processorId;       // Processor ID corresponding to the ID listed in the processor object (0xFF: applies to all processors)
    WORD flags;             // MPS INTI Flags (MIF_*)
    BYTE lapicInt;          // Local APIC interrupt input LINTn to which NMI is connected
};

// This optional structure supports 64-bit systems by providing an override of 
// the physical address of the local APIC in the MADT’s table header, which is 
// defined as a 32-bit field.
struct MA_LAPIC_AO {    // Local APIC Address Override Structure
    SUBHDR header;          // type=5(MAD_LAPIC_AO), length=12
    WORD reserved;          // =0
    QWORD ioapicAddress;    // The 64-bit physical address of Local APIC
};

// The I/O SAPIC structure is very similar to the I/O APIC structure. If both 
// I/O APIC and I/O SAPIC structures exist for a specific APIC ID, the information 
// in the I/O SAPIC structure must be used.
struct MA_IO_SAPIC {    // I/O SAPIC Structure
    SUBHDR header;          // type=6(MAD_IO_SAPIC), length=16
    BYTE isapicId;          // I/O SAPIC ID
    BYTE reserved;          // =0
    DWORD globSysIntBase;   // The global system interrupt number where this I/O SAPIC’s interrupt inputs start
    QWORD iosapicAddress;   // The 64-bit unique physical address to access this I/O SAPIC
};

// The Processor local SAPIC structure is very similar to the processor local APIC structure. 
// When using the SAPIC interrupt model, each processor in the system is required 
// to have a Processor Local SAPIC record and an ACPI Processor object.
struct MA_LSAPIC {      // Processor Local SAPIC Structure
    SUBHDR header;          // type=7(MAD_LSAPIC), length>=17
    BYTE processorId;       // OSPM associates the Local SAPIC Structure with a processor object declared in the namespace using the Processor statement by matching the processor object’s ProcessorID value with this field
    BYTE lsapicId;          // The processor’s local SAPIC ID
    BYTE lsapicEid;         // The processor’s local SAPIC EID
    BYTE reserved[3];       // =0
    DWORD lsapicFlags;      // Local SAPIC flags (LAF_*)
    DWORD processorUid;     // OSPM associates the Local SAPIC Structure with a processor object declared in the namespace using the Device statement, when the _UID child object of the processor device evaluates to a numeric value, by matching the numeric value with this field
    ACHAR processorUidASZ[1];   // OSPM associates the Local SAPIC Structure with a processor object declared in the namespace using the Device statement, when the _UID child object of the processor device evaluates to a string, by matching the string with this field
};

// The Platform Interrupt Source structure is used to communicate which I/O SAPIC 
// interrupt inputs are connected to the platform interrupt sources.
struct MA_PLAT_INT_SRCS {   // Platform Interrupt Sources Structure
    SUBHDR header;          // type=8(MAD_PLAT_INT_SRCS), length=16
    WORD flags;             // MPS INTI Flags (MIF_*)
    BYTE intType;           // Interrupt Type (ITY_*, enum MA_PI_TYPE)
    BYTE processorId;       // Processor ID of destination
    BYTE processorEid;      // Processor EID of destination
    BYTE iosapicVector;     // Value that OSPM must use to program the vector field of the I/O SAPIC redirection table entry for entries with the PMI interrupt type
    DWORD globSysInt;       // The Global System Interrupt that this platform interrupt will signal
    DWORD platIntSrcFlags;  // Platform Interrupt Source Flags (PIF_*)
};

enum MA_PI_TYPE {       // Platform Interrupt types
    ITY_PMI = 0,            // Platform Management Interrupt
    ITY_INIT, // 1          // INIT messages cause processors to soft reset
    ITY_CPEI, // 2          // Corrected Platform Error Interrupt
};

// Platform Interrupt Source Flags
#define PIF_CPEI_OVERRIDE   0x00000001  // retrieval of error information is allowed from any processor and OSPM is to use the information provided by the processor ID, EID fields as a target processor hint

// The Processor X2APIC structure is very similar to the processor local APIC structure. 
// When using the X2APIC interrupt model, logical processors with APIC ID values of 255 
// and greater are required to have a Processor Device object and must convey the 
// processor’s APIC information to OSPM using the Processor Local X2APIC structure. 
// Logical processors with APIC ID values less than 255 must use the Processor Local 
// APIC structure to convey their APIC information to OSPM.
struct MA_LX2APIC {     // Processor Local x2APIC Structure
    SUBHDR header;          // type=9(MAD_LX2APIC), length=16
    WORD reserved;          // =0
    DWORD x2apicId;         // The processor’s local x2APIC ID
    DWORD lapicFlags;       // Local APIC flags (LAF_*)
    DWORD processorUid;     // OSPM associates the X2APIC Structure with a processor object declared in the namespace using the Device statement, when the _UID child object of the processor device evaluates to a numeric value, by matching the numeric value with this field
};

// This structure describes the Local APIC interrupt input (LINTn) that NMI is 
// connected to for each of the processors in the system where such a connection exists. 
// This information is needed by OSPM to enable the appropriate local APIC entry.
struct MA_LX2APIC_NMI { // Local x2APIC NMI Structure
    SUBHDR header;          // type=0xA(MAD_LX2APIC_NMI), length=12
    WORD flags;             // MPS INTI Flags (MIF_*)
    DWORD processorUid;     // UID corresponding to the ID listed in the processor Device object (0xFFFFFFFF = all processors)
    BYTE lapicInt;          // Local x2APIC interrupt input LINTn to which NMI is connected
    BYTE reserved[3];       // =0
};


//////////////////////////////// SBST ////////////////////////////////

// If the platform supports batteries as defined by the Smart Battery Specification 1.0 
// or 1.1, then an Smart Battery Table (SBST) is present. This table indicates the 
// energy level trip points that the platform requires for placing the system into the 
// specified sleeping state and the suggested energy levels for warning the user to 
// transition the platform into a sleeping state. 
// Notice that while Smart Batteries can report either in current (mA/mAh) or in energy 
// (mW/mWh), OSPM must set them to operate in energy (mW/mWh) mode so that the energy 
// levels specified in the SBST can be used. OSPM uses these tables with the capabilities 
// of the batteries to determine the different trip points. 

struct SBST {       // Smart Battery Description Table
    SDTHDR header;          // signature='SBST', revision=1
    DWORD warningLevel;     // OEM suggested platform energy level in milliWatt-hours (mWh) at which OSPM warns the user
    DWORD lowLevel;         // OEM suggested platform energy level in mWh at which OSPM will transition the system to a sleeping state
    DWORD criticalLevel;    // OEM suggested platform energy level in mWh at which OSPM performs an emergency shutdown
};


//////////////////////////////// ECDT ////////////////////////////////

// This optional table provides the processor-relative, translated resources of an 
// Embedded Controller. The presence of this table allows OSPM to provide Embedded 
// Controller operation region space access before the namespace has been evaluated. 
// If this table is not provided, the Embedded Controller region space will not be 
// available until the Embedded Controller device in the AML namespace has been 
// discovered and enumerated. The availability of the region space can be detected 
// by providing a _REG method object underneath the Embedded Controller device.

struct ECDT {       // Embedded Controller Boot Resources Table
    SDTHDR header;          // signature='ECDT', revision=1
    GAS ecCtlRegGA;         // [EC_CONTROL] Contains the processor relative address of the Embedded Controller Command/Status register (System I/O space and System Memory space only)
    GAS ecDataRegGA;        // [EC_DATA] Contains the processor relative address of the Embedded Controller Data register (System I/O space and System Memory space only)
    DWORD ecId;             // [UID] Unique ID – Same as the value returned by the _UID under the device in the namespace that represents this embedded controller
    BYTE gpeBit;            // [GPE_BIT] The bit assignment of the SCI interrupt within the GPEx_STS register of a GPE block described in the FADT that the embedded controller triggers
    ACHAR ecIdASZ[1];       // [EC_ID] string that contains a fully qualified reference to the namespace object that is this embedded controller device ( i.e. \\_SB.PCI0.ISA.EC )
};


//////////////////////////////// SRAT ////////////////////////////////

// This optional table provides information that allows OSPM to associate processors 
// and memory ranges, including ranges of memory provided by hot-added memory devices, 
// with system localities / proximity domains and clock domains. On NUMA platforms, 
// SRAT information enables OSPM to optimally configure the operating system during a 
// point in OS initialization when evaluation of objects in the ACPI Namespace is not 
// yet possible. OSPM evaluates the SRAT only during OS initialization. 
// The Local APIC ID / Local SAPIC ID / Local x2APIC ID of all processors started at 
// boot time must be present in the SRAT. If the Local APIC ID / Local SAPIC ID / Local 
// x2APIC ID of a dynamically added processor is not present in the SRAT, a _PXM object 
// must exist for the processor’s device or one of its ancestors in the ACPI Namespace.

struct SRAT {       // System Resource Affinity Table
    SDTHDR header;          // signature='SRAT', revision=3
    DWORD reserved36;       // =1
    QWORD reserved40;       // =0?
    // array of SRA structures
};
#define PTR_SRAT_DATA(ptrTable)         \
    ( (BYTE*)ptrTable + sizeof(SRAT) )
#define SIZEOF_SRAT_DATA(tableLength)   \
    ( tableLength - sizeof(SRAT) )

enum SRA_TYPE {     // SRAT: Structure Types
    SRA_LAPIC = 0,          // Processor Local APIC/SAPIC Affinity Structure
    SRA_MEMORY, // 1        // Memory Affinity Structure
    SRA_LX2APIC, // 2       // Processor Local x2APIC Affinity Structure
};

// The Processor Local APIC/SAPIC Affinity structure provides the association between 
// the APIC ID or SAPIC ID/EID of a processor and the proximity domain to which the 
// processor belongs.
struct SR_LAPIC {   // Processor Local APIC/SAPIC Affinity Structure
    SUBHDR header;          // type=0(SRA_LAPIC), length=16
    BYTE proxDomain0;       // Bit[7:0] of the proximity domain to which the processor belongs
    BYTE lapicId;           // The processor’s local APIC ID
    DWORD lapicFlags;       // Processor Local APIC/SAPIC Affinity flags (PLA_*)
    BYTE lsapicEid;         // The processor’s local SAPIC EID
    BYTE proxDomain1;       // Bit[15:8] of the proximity domain to which the processor belongs
    WORD proxDomain2;       // Bit[31:16] of the proximity domain to which the processor belongs
    DWORD clockDomain;      // The clock domain to which the processor belongs
};

// Processor Local APIC/SAPIC Affinity flags
#define PLA_ENABLED         0x00000001  // This processor is usable (available for operating system)

// The Memory Affinity structure provides the following topology information statically 
// to the operating system:
// - The association between a range of memory and the proximity domain to which it belongs
// - Information about whether the range of memory can be hot-plugged
// [baseAddress & length are merged from two pairs of 32-bit fields]
struct SR_MEMORY {  // Memory Affinity Structure
    SUBHDR header;          // type=1(SRA_MEMORY), length=40
    DWORD proxDomain;       // Integer that represents the proximity domain to which the processor belongs (?)
    WORD reserved06;        // =0?
    QWORD baseAddress;      // The Base Address of the memory range
    QWORD length;           // The length of the memory range
    BYTE reserved24[4];     // =0?
    DWORD flags;            // Memory Affinity flags (SMA_*)
    BYTE reserved32[8];     // =0?
};

// Memory Affinity flags
#define SMA_ENABLED         0x00000001  // If clear, the OSPM ignores the contents of the Memory Affinity Structure
#define SMA_HOTPLUGGABLE    0x00000001  // The system hardware supports hot-add and hot-remove of this memory region
#define SMA_NONVOLATILE     0x00000001  // The memory region represents Non-Volatile memory

// The Processor Local x2APIC Affinity structure provides the association between the 
// local x2APIC ID of a processor and the proximity domain to which the processor belongs.
struct SR_LX2APIC { // Processor Local x2APIC Affinity Structure
    SUBHDR header;          // type=2(SRA_LX2APIC), length=24
    WORD reserved02;        // =0
    DWORD proxDomain;       // The proximity domain to which the logical processor belongs
    DWORD x2apicId;         // The processor’s local x2APIC ID
    DWORD lapicFlags;       // Processor Local APIC/SAPIC Affinity flags (PLA_*)
    DWORD clockDomain;      // The clock domain to which the processor belongs
    BYTE reserved20[4];     // =0?
};


//////////////////////////////// SLIT ////////////////////////////////

// This optional table provides a matrix that describes the relative distance 
// (memory latency) between all System Localities, which are also referred to 
// as Proximity Domains. Systems employing a Non Uniform Memory Access (NUMA) 
// architecture contain collections of hardware resources including for example,
// processors, memory, and I/O buses, that comprise what is known as a "NUMA node". 
// Processor accesses to memory or I/O resources within the local NUMA node is 
// generally faster than processor accesses to memory or I/O resources outside 
// of the local NUMA node.

struct SLIT {       // System Locality Distance Information Table
    SDTHDR header;          // signature='SLIT', revision=1
    QWORD numSysLocs;       // The number of System Localities in the system
    // BYTE matrix[numSysLocs][numSysLocs];     // square matrix of 
    //  relative distances from System Locality i to every other System Locality j 
    //  in the system (including itself).
    // The relative distance from System Locality i to System Locality j is 
    //  the i*N + j entry in the matrix, where N is the number of System Localities.
    // Diagonal elements (value = 1.0) are normalized to a value of 10.
    // Non-diagonal elements are scaled to be relative to 10. For example, if 
    //  the relative distance is 2.4, a value of 24 is stored in table entry.
    // Values less than 10 are reserved.
    // Value equal to 255 means unreachable locality.
};
#define PTR_SLIT_DATA(ptrTable)         \
    ( (BYTE*)ptrTable + sizeof(SLIT) )
// caller is responsible for unpredictable results, caused by incorrect arguments
#define SLIT_GET_DISTANCE(ptrSliTable,fromSysLoc,toSysLoc)  \
    ( *(PTR_SLIT_DATA(ptrSliTable) + (fromSysLoc * sliTable->numSysLocs) + toSysLoc) )


//////////////////////////////// CPEP ////////////////////////////////

// Platforms may contain the ability to detect and correct certain operational 
// errors while maintaining platform function. These errors may be logged by the 
// platform for the purpose of retrieval. Depending on the underlying hardware 
// support, the means for retrieving corrected platform error information varies. 
// If the platform hardware supports interrupt-based signaling of corrected 
// platform errors, the MADT Platform Interrupt Source Structure describes the 
// Corrected Platform Error Interrupt (CPEI). 
// Alternatively, OSPM may poll processors for corrected platform error information.

struct CPEP {       // Corrected Platform Error Polling Table
    SDTHDR header;          // signature='CPEP', revision=1
    BYTE reserved[8];       // =0
    // array of CPE structures
};

enum CPE_TYPE {     // CPEP: Structure Types
    CPE_LAPIC = 0,          // C.P.E.P. Processor structure for APIC/SAPIC based processors
};

// The Corrected Platform Error Polling Processor structure provides information 
// on the specific processors OSPM polls for error information.
struct CP_LAPIC {   // Corrected Platform Error Polling Processor Structure
    SUBHDR header;          // type=0(CPE_LAPIC), length=8
    BYTE processorId;       // Processor ID of destination
    BYTE processorEid;      // Processor EID of destination
    DWORD pollingInterval;  // Platform-suggested polling interval (in milliseconds)
};


//////////////////////////////// MSCT ////////////////////////////////

// This table provides OSPM with information characteristics of a system’s maximum 
// topology capabilities. If the system maximum topology is not known up front at 
// boot time, then this table is not present. OSPM will use information provided by 
// the MSCT only when the System Resource Affinity Table (SRAT) exists. 
// The MSCT must contain all proximity and clock domains defined in the SRAT.

struct MSCT {       // Maximum System Characteristics Table
    SDTHDR header;          // signature='MSCT', revision=1
    DWORD pdiOffset;        // Offset in bytes to the Proximity Domain Information Structure table entry
    DWORD maxProxDomainIdx; // The maximum number of Proximity Domains ever possible in the system. The number reported in this field is (maximum domains – 1). For example if there are 0x10000 possible domains in the system, this field would report 0xFFFF
    DWORD maxClkDomainIdx;  // The maximum number of Clock Domains ever possible in the system. The number reported in this field is (maximum domains – 1).
    QWORD maxPhysAddress;   // The maximum Physical Address ever possible in the system. Note: this is the top of the reachable physical address.
    //...
    // MPDI mpdi[maxProxDomainIdx+1];   // array of PDI structures  //?? maxIdx+1 or maxIdx ??//
};

// The Maximum Proximity Domain Information Structure is used to report system 
// maximum characteristics. These structures must be organized in ascending order 
// of the proximity domain enumerations. 
// All proximity domains within the Maximum Number of Proximity Domains reported 
// in the MSCT must be covered by one of these structures.
struct MPDI {        // Maximum Proximity Domain Information Structure
    BYTE revision;          // =1
    BYTE length;            // =22
    DWORD pdRangeStart;     // The starting proximity domain for the proximity domain range that this structure is providing information
    DWORD pdRangeEnd;       // The ending proximity domain for the proximity domain range that this structure is providing information
    DWORD maxProcCapacity;  // The Maximum Processor Capacity of each of the Proximity Domains specified in the range (0 = no processors in domains)
    QWORD maxMemCapacity;   // The Maximum Memory Capacity (size in bytes) of the Proximity Domains specified in the range (0 = no memory in domains)
};







void initAcpi(void);


EXTERN_C_END


#endif //_ACPI_H_INC_
