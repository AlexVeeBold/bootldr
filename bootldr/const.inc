; char constants
%define CR      0x0D
%define LF      0x0A
%define SPC     0x20
%define EXCLMK  0x21
%define COLON   0x3A
%define EQSIGN  0x3D
%define QUESTMK 0x3F

; opcode constants
%define NOP     0x90
%define INT     0xCD
%define INT3    0xCC
%define PRE_AO  0x66    ; address-override prefix
%define JMPFAR  0xEA


; kernel less than 32k:
;002000...007FFF  (24k) kernel stack
;008000...00FFFF  (32k) kernel image
;
; kernel equal or greater than 32k:
;002000...00FFFF  (60k) kernel stack
;010000...08FFFF  (512k) kernel image

;memory map:
; stack:        0x00002000...0x00007DFF  (overwriting boot sector, which done its work)
; boot data:    0x00007E00...0x00007EFF
; kernel:       <variable>...<variable>

;available:
; kernel(1):    0x00008000...0x0008FFFF  (544 KB)
; kernel(2):    0x00008000...0x0009FFFF  (608 KB, up to video memory)

%define BT_SEG      007C0h  ; boot record loaded at 07C0:0000 (0x00007C00)
%define STK_SEG     00000h ;00200h
%define STK_PTR     07E00h ;05E00h  ; stack [growns down] & boot data [grows up]


; Volume defs
%define BYTES_IN_SECTOR     512
%define SECTORS_IN_BLOCK    2

