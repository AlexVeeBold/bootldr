; binary debug interface
;

;;;;;;;;;;;;;;;; DEBUG FUNCTIONS ;;;;;;;;;;;;;;;;

align 16, db 0
dbgbuf32    db 3,3,3,3,3,3,3,3,3,3,3,3
align 16, db 0

[BITS 32]   ; Code for protected mode (32 bit)

kDbgByte32:  ; DL = byte, uses AL, DX, stack
    push edi
    push esi
    mov di, dbgbuf32
    mov si, di
    mov al, DBG_BYTE
    stosb       ; type
    mov al, dl
    stosb       ; byte
    xor al, al
    stosb       ; eop
    mov cx, 3
    call kSerialPortTransferBuffer32
    pop esi
    pop edi
    ret
;endof kDbgByte32

kDbgWord32:     ; DX = word, uses AX, DX, stack
    push edi
    push esi
    mov edi, dbgbuf32
    mov esi, edi
    mov al, DBG_WORD
    stosb       ; type
    mov ax, dx
    stosw       ; word
    xor al, al
    stosb       ; eop
    mov cx, 4
    call kSerialPortTransferBuffer32
    pop esi
    pop edi
    ret
;endof kDbgWord32

kDbgDword32:    ; EDX = dword, uses EAX, EDX, stack
    push edi
    push esi
    mov edi, dbgbuf32
    mov esi, edi
    mov al, DBG_DWORD
    stosb       ; type
    mov eax, edx
    stosd       ; dword
    xor al, al
    stosb       ; eop
    mov cx, 6
    call kSerialPortTransferBuffer32
    pop esi
    pop edi
    ret
;endof kDbgDword32

kDbgChar32:   ; DL = char, uses AL, DX, stack
    push edi
    push esi
    mov edi, dbgbuf32
    mov esi, edi
    mov al, DBG_ASTR
    stosb       ; type
    mov al, dl
    stosb       ; char
    xor al, al
    stosb       ; eop
    mov cx, 3
    call kSerialPortTransferBuffer32
    pop esi
    pop edi
    ret
;endof kDbgChar

kDbgStr32:      ; DS:ESI = zero-terminated ansi string, uses AL, CL, DX, stack
    push edi
    push esi
    mov dl, DBG_ASTR
    call kSerialPortTransferByte32    ; type
    mov cx, 2
    call kSerialPortTransferBuffer32  ; char(s) & eop
    pop esi
    pop edi
    ret
;endof kDbgStr32

kDbgLf32:       ; no parameters, uses AL, DX, stack
    push edi
    push esi
    mov edi, dbgbuf32
    mov esi, edi
    mov al, DBG_LF
    stosb       ; type
    xor al, al
    stosb       ; eop
    mov cx, 2
    call kSerialPortTransferBuffer32
    pop esi
    pop edi
    ret
;endof kDbgLf32
