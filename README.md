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

#### Build/Run files:
- bootldr/nasm.exe — Netwide Assembler *(not included)*
- bootldr/asm_ldr1.bat — assemble stage 1 loader
- bootldr/asm_ldr2.bat — assemble stage 2 loader
- _vm_config.bat — set VirtualBox VM paths & configuration *(not included)*
- make_ldr1.bat — [re]build «stage 1 loader» image and update VM HDD image
- make_ldr2.bat — [re]build «stage 2 loader + kernel» image
- piper.exe — Debug Host app *(using host OS pipe that is used by VM serial port)*
- RUNvm.bat — run VM & Debug Host app
