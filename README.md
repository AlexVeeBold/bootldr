# bootldr
Operating System proof-of-concept

#### Main source files:
- bootldr/**bl_boot.asm** — OS loader, stage 1 (boot record)
- bootldr/**bl_loader.asm** — OS loader, stage 2 (loader)
- vkern/**vkern.cpp** — OS kernel

#### Hardware-related source files:
- bootldr/*serial.inc* — 16-bit Serial port routines
- bootldr/*serial32.inc* — 32-bit Serial port routines
- vkern/*hw.c* — Hardware abstraction library
- vkern/*pic.cpp* — Programmable Interrupt Controller
- vkern/*rtc.cpp* — Real-Time Clock
- vkern/*serial.cpp* — Serial port
