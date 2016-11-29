; binary debug interface (11.12.2009)
;

; types
%define DBG_BYTE    31h     ; print byte value:  [type] [byte] <end>
%define DBG_WORD    32h     ; print word value:  [type] [bytelo][bytehi] <end>
%define DBG_DWORD   34h     ; print dword value: [type] [bytelo][bytemedlo][bytemedhi][bytehi] <end>
%define DBG_ASTR    41h     ; print ansi string: [type] [byte][byte]...[byte] <end>
%define DBG_LF      0Ah     ; print end-of-line: [type] <end>
%define DBG_END     00h     ; end of packet

%define DBG_READIMAGE   4Bh     ; get image: [type] <end>
%define DBG_READIMAGE_N 4Eh     ; get image next byte: [type]
%define DBG_READIMAGE_R 6Bh     ; get image reply:
        ; [type] [imageSize: word] [loadAddr: wordL] [entry: word] [image bytes]

%define DBG_READIMAGE_BLOCK     64 ;16
%define DBG_READIMAGE_SHIFT     6


;;;;;;;;;;;;;;;; DEBUG FUNCTIONS ;;;;;;;;;;;;;;;;

;%define kDbgSendByte    kSerialPortTransferByte

dbgbuf16    db 3,3,3,3,3,3,3,3

[BITS 16]   ; Code for real mode (16-bit code)

kDbgByte:  ; DL = byte, uses AL, DX, DI, SI
    mov di, dbgbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_BYTE
    stosb       ; type
    mov al, dl
    stosb       ; byte
    xor al, al
    stosb       ; eop
    mov cx, 3
    call kSerialPortTransferBuffer
    ret
;endof kDbgByte

kDbgWord:   ; DX = word, uses AX, DX, DI, SI
    mov di, dbgbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_WORD
    stosb       ; type
    mov ax, dx
    stosw       ; word
    xor al, al
    stosb       ; eop
    mov cx, 4
    call kSerialPortTransferBuffer
    ret
;endof kDbgWord

kDbgDword:   ; EDX = dword, uses EAX, EDX, DI, SI
    mov di, dbgbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_DWORD
    stosb       ; type
    mov eax, edx
    stosd       ; dword
    xor al, al
    stosb       ; eop
    mov cx, 6
    call kSerialPortTransferBuffer
    ret
;endof kDbgDword

kDbgChar:   ; DL = char, uses AL, DX, DI, SI
    mov di, dbgbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_ASTR
    stosb       ; type
    mov al, dl
    stosb       ; char
    xor al, al
    stosb       ; eop
    mov cx, 3
    call kSerialPortTransferBuffer
    ret
;endof kDbgChar

kDbgStr:    ; DS:SI = zero-terminated ansi string, uses AL, CL, DX, DI, SI
    mov dl, DBG_ASTR
    call kSerialPortTransferByte    ; type
    mov cx, 2
    call kSerialPortTransferBuffer  ; char(s) & eop
    ret
;endof kDbgStr

kDbgLf:     ; send end-of-line, uses AL, DX, DI, SI
    mov di, dbgbuf16  ; DBG_PTR
    mov si, di
    mov al, DBG_LF
    stosb       ; type
    xor al, al
    stosb       ; eop
    mov cx, 2
    call kSerialPortTransferBuffer
    ret
;endof kDbgLf


