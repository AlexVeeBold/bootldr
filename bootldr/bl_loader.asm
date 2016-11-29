; boot starter (net-load)
; created: 17.08.2010 12:49

; Compile with NASM


%include "const.inc"


;%define LDRSIZE     2*1024      ; bytes
%define BLOCKS_IN_BOOTAREA  2
%define LDRSIZE     BLOCKS_IN_BOOTAREA * SECTORS_IN_BLOCK * BYTES_IN_SECTOR



; boot data
%include "btdata.inc"


struc GSD       ; generic segment descriptor
    gsd_limit_0: resb 1     ; segment limit: bits 0..7
    gsd_limit_8: resb 1     ; segment limit: bits 8..15
    gsd_base_0: resb 1      ; base address: bits 0..7
    gsd_base_8: resb 1      ; base address: bits 8..15
    gsd_base_16: resb 1     ; base address: bits 16..23
    gsd_type: resb 1        ; bit7: p; bits6-5: dpl; bit4: s; bits3-0: type
    gsd_limit_16_a: resb 1  ; 7: g; 6: b; 5: 0; 4: avl; bits3-0: limit bits 16..19
    gsd_base_24: resb 1     ; base address: bits 24..31
endstruc
; p=present, dpl=descriptor privilege-level, s=system/user
; type bits: (s:user=1)
;  3:code=1, 2:conforming(control transfer check), 1:readable, 0:accessed
;  3:data=0, 2:expand down(base/limit type), 1:writable, 0:accessed
; g=limit granularity (byte/page), b=default operand size (bits), 
; avl=available to software


%define NUM_GDTENTRIES  8



;;;;;;;;;;;;;;;; MAIN (16-BIT) ;;;;;;;;;;;;;;;;

[BITS 16]   ; Code for real mode (16-bit code)
[ORG 0]     ; Tell the compiler that this is offset 0.

    ; we should have btdata in ss:bp
    ; and be loaded at bt_kern_seg:bt_kern_entry
start:
    ; Update the segment registers
    cli
    mov ax, [bp+bt_kern_seg] ;BT_SEG ;cs
    mov cx, [bp+bt_stk_seg] ; ax, STK_SEG
    mov dx, [bp+bt_stk_ptr] ; bx, STK_PTR
    xor bx, bx
    xor ebp, ebp
    mov ds, ax
    mov es, ax
    mov fs, bx
    mov gs, bx
    mov ss, cx
    mov sp, dx
    mov bp, dx
    sti
    cld         ; set forward direction

    ; save 16-bit segments' bases
    mov [bp+bt_seg16_code], ax
    mov [bp+bt_seg16_data], ax
    mov [bp+bt_seg16_extra], bx
    mov [bp+bt_seg16_stack], cx

    ; save 32-bit segments' selectors
    mov ax, 0008h       ; code descriptor
    mov cx, 0010h       ; data descriptor
    mov dx, 0018h       ; extra descriptor
    mov bx, 0020h       ; stack descriptor
    mov [bp+bt_seg32_code], ax
    mov [bp+bt_seg32_data], cx
    mov [bp+bt_seg32_extra], dx
    mov [bp+bt_seg32_stack], bx

    ; save 32-to-16 segments' selectors
    mov ax, 0030h       ; code descriptor
    mov cx, 0038h       ; data descriptor
    mov [bp+bt_seg3216_code], ax
    mov [bp+bt_seg3216_data], cx

    ; save gdtr & idtr
    sgdt [bp+bt_gdt16_limit]
    sidt [bp+bt_idt16_limit]

    call kSerialPortInit

    call kDbgLf

    mov si, msgLoading     ; Print msg
    call kDbgStr
    mov dl, [bp+bt_drive]
    call kDbgByte
    call kDbgLf

    mov dx, [bp+bt_kern_size]
    call kDbgWord

    mov dl, SPC
    call kDbgChar

    mov dx, [bp+bt_kern_seg]
    call kDbgWord
    mov dl, COLON
    call kDbgChar
    mov dx, [bp+bt_kern_entry]
    call kDbgWord

    ;call kDbgLf
    mov dl, SPC
    call kDbgChar

    mov dx, [bp+bt_stk_seg]
    call kDbgWord
    mov dl, COLON
    call kDbgChar
    mov dx, [bp+bt_stk_ptr]
    call kDbgWord

    call kDbgLf

    ;;;; get kernel base address (32-bit)
    xor eax, eax
    mov ax, [bp+bt_kern_seg]
    shl eax, 4
    mov [bp+bt_image_addr], eax
    ; rebase gdt descriptor
    mov ebx, eax
    add ebx, gdtable
    ;mov dx, 07FFh
    mov dx, 003Fh
    mov [bp+bt_gdt32_limit], dx
    mov [bp+bt_gdt32_base], ebx
    mov dl, NUM_GDTENTRIES
    mov [ebp+bt_gdt_count], dl
    ; rebase code offset
    mov edx, eax
    add edx, pentry
    mov [bp+bt_ptr32_code], edx

    ; print pe message
    mov si, msgPEEntry
    call kDbgStr
    mov dx, [bp+bt_seg32_code] ; print p.m.entry seg:offset
    call kDbgWord
    mov dl, COLON
    call kDbgChar
    mov edx, [bp+bt_ptr32_code]
    call kDbgDword
    call kDbgLf

    ; set jmp-to-pe-mode target
    lea edi, [bp+bt_ptr32_code]

    ; disable interrupts
    cli
    ; enable A20 line
    call kPs2PortEnableA20_fast
    ; load global desriptors table
    lgdt [bp+bt_gdt32_limit]
    ; enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    ; reload data/extra/stack segment registers
    mov ax, [bp+bt_seg32_data]
    mov cx, [bp+bt_seg32_extra]
    mov dx, [bp+bt_seg32_stack]
    mov ds, ax
    mov es, ax
    mov fs, cx
    mov gs, cx
    mov ss, dx

    ; reload code segment register
    jmp dword far [di]

hang:
    hlt
    jmp hang

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; PS/2 PORT FUNCTIONS ;;;;;;;;;;;;;;;;

%include "ps2.inc"

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; SERIAL PORT FUNCTIONS ;;;;;;;;;;;;;;;;

%include "serial.inc"

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; DEBUG FUNCTIONS ;;;;;;;;;;;;;;;;

%include "debug.inc"

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; MAIN "FUNCTION" (32-BIT) ;;;;;;;;;;;;;;;;

[BITS 32]   ; code for protected mode (32 bit)
pentry:

    mov edi, [ebp+bt_image_addr]
    call kDbgLf32
    mov esi, edi
    add esi, msgProtected
    call kDbgStr32
    ;call kDbgLf32

    xor edx, edx
    mov dx, [ebp+bt_kern_size] ;print kernel file size
    call kDbgDword32
    call kDbgLf32

    mov esi, edi    ;[ebp+bt_image_addr]
    add esi, kIntCall
    mov [ebp+bl_intcall_addr], esi

    ; check image, relocate it & jump into h.l.l. part
    mov esi, edi
    add esi, LDRSIZE
    call kCheckImage

    push eax
    mov dl, al
    add dl, 30h
    call kDbgChar32
    call kDbgLf32
    pop eax

    test al, al
    jz hangloop
    call kRelocateImage
    call kEnterHLLM

hangloop:           ; Hang!
    hlt
    jmp hangloop

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; HIGH-LEVEL LANGUAGE MODULE CALLER (32-BIT) ;;;;;;;;;;;;;;;;

; DS:ESI = pe-hdr address
; uses EAX, ECX, EDX, ESI, stack
kCheckImage:
    ;;;; verify signatures ;;;;

    ;; check 'Mark Zbikowski' signature ;;

    mov dx, [esi]  ; print sign
    call kDbgWord32
    mov dl, SPC
    call kDbgChar32

    mov dx, [esi]
    cmp dx, 0x5A4D      ; 'MZ'
    jnz .failure

    ; get pe offset
    lea edx, [esi+003Ch]
    mov edx, [edx]  ; take pe header offset
    add esi, edx    ; move to pe header

    ;; check 'Portable Executable' signature ;;

    mov dx, [esi]  ; print sign
    call kDbgWord32
    mov dl, SPC
    call kDbgChar32

    mov dx, [esi]
    cmp dx, 0x4550      ; 'PE'
    jnz .failure

    ;; check "optional" header signature ;;

    mov dx, [esi+0018h]  ; print sign
    call kDbgWord32
    mov dl, SPC
    call kDbgChar32

    mov dx, [esi+0018h]
    cmp dx, 0x010B      ; OPTIONAL_HDR32_MAGIC
    jnz .failure

.success:
    mov al, 1
    jmp .quit
.failure:
    xor al, al
.quit:      ; failure (or not?)
    ret
;endof kCheckImage



; DS:ESI = pe-hdr address, SS:EBP = boot data
; uses EAX, EBX, ECX, EDX, ESI, EDI, stack
kRelocateImage:
    ;;;; prepare for h.l.l. image relocation
    ; pe = mz + [ DWORD mz+0x3C ]
    ; [pe+0006h] word: number of sections
    ; [pe+0014h] word: offset to first section header
    ; [pe+0028h] dword: entry offset
    ; [pe+0034h] dword: base address
    ; [pe+0038h] dword: memory alignment
    ; [pe+003Ch] dword: file alignment
    ; [pe+0050h] dword: size of image (in memory)
    ; hll code segment selector is saved later
    mov eax, [esi+0034h]
    mov [ebp+bt_hll_addr], eax      ; save new base address
    mov eax, [esi+0050h]
    mov [ebp+bt_hll_size], eax      ; save new size
    mov eax, [ebp+bt_hll_addr]
    add eax, [esi+0028h]
    mov [ebp+bt_hll_entry], eax     ; save new entry
    mov ax, [ebp+bt_seg32_code]
    mov [ebp+bt_hll_codesel], ax    ; save segment descriptor selector

    mov edx, [esi+0034h] ;+001Ch] ;print base address
    call kDbgDword32
    mov dl, SPC
    call kDbgChar32
    mov edx, [esi+0028h] ;+0010h] ;print entry offset
    call kDbgDword32
    ;mov dl, SPC
    ;call kDbgChar32
    ;mov edx, [esi+0038h] ;+0020h] ;print memory alignment
    ;call kDbgDword32
    ;mov dl, SPC
    ;call kDbgChar32
    ;mov edx, [esi+003Ch] ;+0024h] ;print file alignment
    ;call kDbgDword32
    call kDbgLf32

    ;;;; create 'relocate section task' array (on stack) ;;;;
    ; scn[0] = pe + sizeof(pe) + sizeof(pe_file_hdr) + opt_hdr_size
    ; [scn+0008h] dword: real data size
    ; [scn+000Ch] dword: memory offset
    ; [scn+0010h] dword: file data size
    ; [scn+0014h] dword: file data offset
    ; build struct array:
    ; ecx=Count
    ; (ds:)dwSrcOffset, (gs:)dwDstOffset, dwCopyLength
    xor ecx, ecx
    mov cx, [esi+0014h] ; opt_hdr size
    mov edi, esi        ; ptr to pe
    add edi, 0018h      ; sizeof pe + pe_file_hdr
    add edi, ecx        ; sizeof opt_hdr
    mov ax, [esi+0006h] ; section count
    mov ecx, 0028h      ; sizeof scn_hdr
    mul ecx             ; *= section_count
    add edi, eax            ; edi = ptr to last+1 scn_hdr
    xor ecx, ecx
    mov cx, [esi+0006h]     ; ecx = count of sections
    mov ebx, [esi+0034h]    ; ebx = new base address
.loop_set_item:
    sub edi, 0028h      ; move to previous scn_hdr
    dec ecx
    push dword [edi+0010h]  ; store size
    mov eax, ebx
    add eax, [edi+000Ch]
    push eax                ; store dst
    mov eax, [ebp+bt_image_addr]
    add eax, LDRSIZE
    add eax, [edi+0014h]
    push eax                ; store src
    test ecx, ecx
    jnz .loop_set_item
    ; store hdr item
    push dword [edi+0014h]  ; store size
    mov eax, ebx
    push eax                ; store dst
    mov eax, [ebp+bt_image_addr]
    add eax, LDRSIZE
    push eax                ; store src

    ;;;; execute saved array ;;;;
    ; (([gs:edi] <- [ds:esi]) * ecx) * <eax>
    xor eax, eax
    mov ax, [esi+0006h] ; section count + header
    inc eax
.loop_copy_scn:
    pop esi     ; load src
    pop edi     ; load dst
    pop ecx     ; load size
    shr ecx, 2  ; size in bytes -> size in dwords
    mov dx, gs
    mov es, dx  ; set es base:limit to 0:4GB
    rep movsd
    mov dx, ds
    mov es, dx  ; restore es
    dec eax
    test eax, eax
    jnz .loop_copy_scn

    ret
;endof kRelocateImage


; DS:ESI = pe-hdr address, SS:EBP = boot data
; uses EAX, EBX, ECX, EDX, ESI, EDI, stack
kEnterHLLM:
    ; set jmp-to-hll target
    mov edx, [ebp+bt_hll_entry]
    mov [ebp+bt_ptr32_code], edx

    ; print hllm message
    mov esi, [ebp+bt_image_addr]
    add esi, msgHLLMEntry
    call kDbgStr32
    ; print kernel segment:offset
    mov dx, [ebp+bt_seg32_code]
    call kDbgWord32
    mov dl, COLON
    call kDbgChar32
    mov edx, [ebp+bt_ptr32_code]
    call kDbgDword32
    mov dl, SPC
    call kDbgChar32
    mov dl, EQSIGN
    call kDbgChar32
    mov dl, SPC
    call kDbgChar32
    ; print dword at located entry
    mov eax, [ebp+bt_ptr32_code]
    mov edx, [gs:eax]
    call kDbgDword32
    call kDbgLf32

    ;;;; jump into high-level-language code ;;;;
    lea edi, [bp+bt_ptr32_code]
    mov eax, [ebp+bt_hll_addr]  ; load image base
    mov esp, eax        ; place stack right under loaded image
    ; reload cs register by jumping into h.l.l.
    jmp dword far [edi]

.quit:      ; failure
    ret
;endof kEnterHLLM


; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; SERIAL PORT / DEBUG FUNCTIONS (32-BIT) ;;;;;;;;;;;;;;;;

%include "serial32.inc"

; make vc-style margin with nop's
align 16, db NOP

%include "debug32.inc"

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; PE FUNCTIONS (16/32-BIT) ;;;;;;;;;;;;;;;;

; intcall:
;ic_func (called by hllm) located in 0000..FFFF
;create stack located in 0000..FFFF
;save esp & ebp on created stack
;switch to created stack
;jmpfar: set seg. limit to FFFF
;disable pe mode (and paging, if any)
;jmpfar: reload seg.regs
;load regs from ic_*
;call int
;save regs to ic_*
;load ebp & esp from created stack
;switch to old stack
;enable pe mode
;jmpfar: reload seg.regs
;return to hllm caller

kIntCall:
    ;ic_func (called by hllm), located in 0000..FFFF
[BITS 32]
    cli
    push ebp                            ;      esp
    mov ebp, [esp+8]    ; load arg0     ; xxxx  \/ ebp  retaddr arg0 xxxx
    push esi
    push edi
    push eax
    push ecx
    push edx
    push ebx
    xor eax, eax
    xor ecx, ecx
    xor esi, esi
    ; create stack located in 0000..FFFF
    mov si, [ebp+bt_stk_seg]
    shl esi, 4
    mov ax, [ebp+bt_stk_ptr]
    add esi, eax
    sub esi, 0100h ;!-TMP-!;
    ; save esp & cr0 on created stack
    mov eax, cr0
    lea esi, [esi-4]
    mov [esi], esp
    lea esi, [esi-4]
    mov [esi], eax
  ;;  mov [ebp+bl_ic_esp], esp
    ; switch to created stack
    ; reload segment registers (set limit to FFFF)
    mov eax, [ebp+bt_image_addr]
    add eax, limit64k
    mov dx, [ebp+bt_seg3216_data]
    mov [ebp+bt_ptr3216_code], eax
    lea edi, [ebp+bt_ptr3216_code]
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx
    mov esp, esi
    jmp dword far [edi]
limit64k:
[BITS 16]
    xor esi, esi
    mov ax, realmode
    mov [bp+bt_ptr16_code], ax
    lea di, [bp+bt_ptr16_code]
    mov cx, [bp+bt_seg16_data]
    mov dx, [bp+bt_seg16_extra]
    mov bx, [bp+bt_seg16_stack]
    ; recalculate stack pointer (now as segmented address)
    mov si, [bp+bt_stk_ptr]
    sub si, 0100h ;!-TMP-!;
    lea si, [si-8]
    ; switch to 16-bit gdt/idt
    sgdt [bp+bt_gdt32_limit]
    lgdt [bp+bt_gdt16_limit]
    sidt [bp+bt_idt32_limit]
    lidt [bp+bt_idt16_limit]
    ; disable pe mode (and paging, if any)
    mov eax, cr0
  ;  and al, 0FEh
    and eax, 7FFFFFFEh  ; reset PG (bit 31) and PE (bit 0) bits
    mov cr0, eax
    ; reload segment registers (switch to real mode)
    mov ds, cx
    mov es, cx
    mov fs, dx
    mov gs, dx
    mov ss, bx
    mov sp, si
    jmp word far [gs:di]
realmode:
    ; set interrupt number
    mov di, intvector
    mov al, [bp+bl_ic_isr]
    mov [di], al
    ; load register values from ic_*
    mov eax, [bp+bl_ic_eax]
    mov ecx, [bp+bl_ic_ecx]
    mov edx, [bp+bl_ic_edx]
    mov ebx, [bp+bl_ic_ebx]
    mov esi, [bp+bl_ic_esi]
    mov edi, [bp+bl_ic_edi]
    mov es, [bp+bl_ic_es]
    mov ds, [bp+bl_ic_ds]
    sti
    ; call interrupt service routine
    db INT
intvector:
    db 0
    cli
    ; save register values to ic_*
    mov [bp+bl_ic_eax], eax
    mov [bp+bl_ic_ecx], ecx
    mov [bp+bl_ic_edx], edx
    mov [bp+bl_ic_ebx], ebx
    mov [bp+bl_ic_esi], esi
    mov [bp+bl_ic_edi], edi
    mov [bp+bl_ic_es], es
    mov [bp+bl_ic_ds], ds
    ; switch to 32-bit gdt/idt
    sgdt [bp+bt_gdt16_limit]
    lgdt [bp+bt_gdt32_limit]
    sidt [bp+bt_idt16_limit]
    lidt [bp+bt_idt32_limit]
    ; load cr0 & esp from created stack
    pop eax
    pop esi
    ; enable pe mode
    mov cr0, eax
    ; switch to old stack
    ; reload segment registers (switch to protected mode)
    mov edi, [ebp+bt_image_addr]
    add edi, pemode
    mov ax, [bp+bt_seg32_code]
    mov cx, [bp+bt_seg32_data]
    mov dx, [bp+bt_seg32_extra]
    mov bx, [bp+bt_seg32_stack]
    mov [bp+bt_ptr32_code], edi
    mov [bp+bt_seg32_code], ax
    lea edi, [bp+bt_ptr32_code]
    mov ds, cx
    mov es, cx
    mov fs, dx
    mov gs, dx
    mov ss, bx
    mov esp, esi
    jmp dword far [edi]
[BITS 32]
pemode:
    ;return to hllm caller
    pop ebx
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    sti
    retn 4  ; remove arg0 from stack
;endof kIntCall

; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; DATA SECTION ;;;;;;;;;;;;;;;;

; message strings
msgLoading      db  'Booted from drive: ',0
msgPEEntry      db  'PM Entry: ',0
msgProtected    db  'Protected mode enabled.',CR,LF,0
msgHLLMEntry    db  'HLLM Entry: ',0



align 8, db 0

gdtable:                    ; NUM_GDTENTRIES
; 0000h - null selector
gd_null: istruc GSD
    dd 00000000h
    dd 00000000h
iend
; 0008h - code: base=0, limit=256MB, 32-bit, present, readable
gd_code: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10011010b    ; present=1, dpl=0, s=1(user), type=code,readable
    db 11000000b    ; granularity=4k page, b=32bit, limit 16..19 = 0x0
    db 00h      ; base 24..31 = 0x00
iend
; 0010h - data: base=0, limit=256MB, 32-bit, present, writable
gd_data: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10010010b    ; present=1, dpl=0, s=1(user), type=data,readable
    db 11000000b    ; granularity=4k page, b=32bit, limit 16..19 = 0x0
    db 00h      ; base 24..31 = 0x00
iend
; 0018h - extra: base=0, limit=4GB, 32-bit, present, writable
gd_extra: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10010010b    ; present=1, dpl=0, s=1(user), type=data,readable
    db 11001111b    ; granularity=4k page, b=32bit, limit 16..19 = 0xF
    db 00h      ; base 24..31 = 0x00
iend
; 0020h - stack: base=0, limit=256MB, 32-bit, present, writable
gd_stack: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10010010b    ; present=1, dpl=0, s=1(user), type=data,readable
    db 11000000b    ; granularity=4k page, b=32bit, limit 16..19 = 0x0
    db 00h      ; base 24..31 = 0x00
iend
; 0028h - empty selector
gd_empty: istruc GSD
    dd 00000000h
    dd 00000000h
iend
; 0030h - code: base=0, limit=64KB, 16-bit, present, readable
gd_code16: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10011010b    ; present=1, dpl=0, s=1(user), type=code,readable
    db 00000000b    ; granularity=1 byte, b=16bit, limit 16..19 = 0x0
    db 00h      ; base 24..31 = 0x00
iend
; 0038h - data: base=0, limit=64KB, 32-bit, present, writable
gd_data16: istruc GSD
    dw 0FFFFh   ; limit 0..15 = 0xFFFF
    dw 00000h   ; base 0..15 = 0x0000
    db 00h      ; base 16..23 = 0x00
    db 10010010b    ; present=1, dpl=0, s=1(user), type=data,readable
    db 00000000b    ; granularity=1 byte, b=16bit, limit 16..19 = 0x0
    db 00h      ; base 24..31 = 0x00
iend

; make vc-style margin with zero's
align 16, db 0



;;;;;;;;;;;;;;;; TAIL ;;;;;;;;;;;;;;;;

; $$  = segment origin
; $   = current position
times (LDRSIZE)-($-$$) db 00h

