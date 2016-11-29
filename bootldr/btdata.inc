; BOOT DATA structure
; created: 13.10.2010 16:41

; !! *must* be SYNCHRONIZED with struct BTDATA (vkern.cpp) !!

;;data
struc BTDATA
    bt_drive: resb 1        ; drive number
    bt_gdt_count: resb 1    ; number of gdt entries
    bt_kern_size: resw 1    ; kernel size
    bt_kern_seg: resw 1     ; kernel address (high 16 bit of 20-bit address)
    bt_kern_entry: resw 1   ; kernel entry offset
    bt_stk_seg: resw 1      ; stack segment
    bt_stk_ptr: resw 1      ; stack pointer
    bt_image_addr: resd 1   ; loaded image address (before relocation)
    ; 
    bl_intcall_addr: resd 1 ; real mode interrupt call (ic) address
    bt_hll_size: resd 1     ; h.l.l. module size
    bt_hll_entry: resd 1    ; h.l.l. module entry
    bt_hll_addr: resd 1     ; h.l.l. module address
    ; 
    bt_hll_codesel: resw 1  ; h.l.l. code segment selector (usually 8)
    bt_ptr16_code: resw 1   ; 16-bit code offset
    bt_seg16_code: resw 1   ; 16-bit code segment base
    bt_gdt16_limit: resw 1  ; 16-bit gdt (limit)
    bt_gdt16_base: resq 1   ; 16-bit gdt (base)
    ; 
    bt_seg16_data: resw 1   ; 16-bit data segment base
    bt_seg16_extra: resw 1  ; 16-bit extra segment base
    bt_seg16_stack: resw 1  ; 16-bit stack segment base
    bt_idt16_limit: resw 1  ; 16-bit idt (limit)
    bt_idt16_base: resq 1   ; 16-bit idt (base)
    ; 
    bt_ptr32_code: resd 1   ; 32-bit code offset
    bt_seg32_code: resw 1   ; 32-bit code segment selector
    bt_gdt32_limit: resw 1  ; 32-bit gdt (limit)
    bt_gdt32_base: resq 1   ; 32-bit gdt (base)
    ; 
    bt_seg32_data: resw 1   ; 32-bit data segment selector
    bt_seg32_extra: resw 1  ; 32-bit extra segment selector
    bt_seg32_stack: resw 1  ; 32-bit stack segment selector
    bt_idt32_limit: resw 1  ; 32-bit idt (limit)
    bt_idt32_base: resq 1   ; 32-bit idt (base)
    ; 
    bt_ptr3216_code: resd 1 ; 32-to-16 code offset
    bt_seg3216_code: resw 1 ; 32-to-16 data segment selector
    bt_seg3216_data: resw 1 ; 32-to-16 data segment selector
    bl_ic_isr: resd 1       ; interrupt service routine number
    bl_ic_es: resw 1        ; segment registers
    bl_ic_ds: resw 1        ; 
    ; 
    bl_ic_eax: resd 1       ; generic registers
    bl_ic_ecx: resd 1       ; 
    bl_ic_edx: resd 1       ; 
    bl_ic_ebx: resd 1       ; 
    ;
    bl_ic_esp: resd 1       ; stack registers
    bl_ic_ebp: resd 1       ; 
    bl_ic_esi: resd 1       ; data registers
    bl_ic_edi: resd 1       ; 
    ; 
    ; up to 256 bytes total
endstruc

