; boot record (net-load)
; created: 15.08.2010 04:24

; Compile with NASM


%include "const.inc"


; minimal size of harddrive image for VirtualBox: 1 MB
%define BLOCKS_IN_BOOTAREA  1024
%define HD_SIZE     BLOCKS_IN_BOOTAREA * SECTORS_IN_BLOCK * BYTES_IN_SECTOR



; boot data
%include "btdata.inc"



;;;;;;;;;;;;;;;; MAIN ;;;;;;;;;;;;;;;;

BITS 16     ; Code for real mode (16-bit code)
[ORG 0]     ; Tell the compiler that this is offset 0.

entry:
    jmp BT_SEG:start    ; reload cs

start:
    ; Update the segment registers
    cli
    mov ax, BT_SEG
    xor bx, bx      ; set SS:SP to 0000:7FFF  (0x00007FFF)
    mov ds, ax
    mov es, ax
    mov fs, bx
    mov gs, bx
    mov ax, STK_SEG
    mov bx, STK_PTR
    mov ss, ax
    mov sp, bx
    mov bp, bx
    sti
    cld         ; set forward direction

    ; Save drive number
    mov [bp+bt_drive], dl

    call kSerialPortInit

    mov si, msgStart    ; Print msg
    call kDbgStr
    mov dl, [bp+bt_drive]
    call kDbgByte
    call kDbgLf

    call kLoadImage

hangloop:           ; Hang!
    hlt
    jmp hangloop



;;;;;;;;;;;;;;;; OTHER ;;;;;;;;;;;;;;;;

%include "serial.inc"

%include "debug.inc"



;;;;;;;;;;;;;;;; LOAD ;;;;;;;;;;;;;;;;

reqbuf16    db 3,3,3,3

kLoadImage:
    mov di, reqbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_READIMAGE   ; send get-image request
    stosb       ; type
    xor al, al
    stosb       ; eop
    mov cx, 2
    call kSerialPortTransferBuffer
    call kSerialPortReceiveByte
    cmp dl, DBG_READIMAGE_R     ; check reply
    jnz .quit

;;;;    receive header (bt_kern_size, bt_kern_seg, bt_kern_entry)
    mov bx, 6
    lea bp, [bp+bt_kern_size]  ;mov bp, DBG_PTR
.loop_hdr:
    call kSerialPortReceiveByte
    mov [bp], dl

    call kDbgByte  ; print header byte
    mov dl, SPC    ;
    call kDbgChar  ;

    dec bx
    inc bp
    test bx, bx
    jg .loop_hdr

;;;;    get image data
    mov bp, STK_PTR
    mov bx, [bp+bt_kern_size]

    mov dx, bp     ; print addr & size
    call kDbgWord  ;
    mov dl, SPC    ;
    call kDbgChar  ;
    mov dx, bx     ;
    call kDbgWord  ;
    call kDbgLf    ;

    mov ax, [bp+bt_kern_seg]    ; load kernel segment:offset
    xor di, di
    mov es, ax
    mov [bp+bt_stk_seg], ss
    mov [bp+bt_stk_ptr], bp
    shr bx, DBG_READIMAGE_SHIFT ; num. of bytes -> num. of blocks
.loop_img_next:     ; next block
    mov dl, DBG_READIMAGE_N     ; retrieve image data
    call kSerialPortTransferByte
    mov cl, DBG_READIMAGE_BLOCK
.loop_img_curr:     ; next byte in block
    call kSerialPortReceiveByte
    mov al, dl
    stosb
    dec cl
    test cl, cl
    jnz .loop_img_curr
    dec bx
    test bx, bx
    jnz .loop_img_next

;;;;    jump into image
    mov dx, [bp+bt_kern_seg]    ; load kernel address
    mov ax, [bp+bt_kern_entry]
    mov [jmp_seg], dx
    mov [jmp_off], ax
    jmp word far [jmp_addr]    ; teleport into kernel
.quit:
    ret
;endof kLoadImage


; make vc-style margin with nop's
align 16, db NOP



;;;;;;;;;;;;;;;; DATA SECTION ;;;;;;;;;;;;;;;;

; startup message string
msgStart    db 'Boot drive: ',0

align 4, db 0
jmp_addr:
jmp_off    dw 0202h
jmp_seg    dw 0101h


; make vc-style margin with zero's
align 16, db 0


;;;;;;;;;;;;;;;; BOOT RECORD TAIL ;;;;;;;;;;;;;;;;

; $$  = segment origin
; $   = current position
times (BYTES_IN_SECTOR-2)-($-$$) db 0
dw 0AA55h


;;;;;;;;;;;;;;;; BINARY IMAGE TAIL ;;;;;;;;;;;;;;;;

; $$  = segment origin
; $   = current position
times (HD_SIZE)-($-$$) db 00h

